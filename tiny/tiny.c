/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content.
 *
 * Updated 11/2019 droh
 *   - Fixed sprintf() aliasing issue in serve_static(), and clienterror().
 */
#include "csapp.h"

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg);

int main(int argc, char **argv) {
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;
  
  // 포트번호 인자가 없으면 사용법 출력
  /* Check command line args */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  
  // listen 소켓 열기
  listenfd = Open_listenfd(argv[1]);

  // 클라의 요청을 받는 무한 루프 시작
  while (1) {
    // 클라이언트 주소 구조체 크기
    clientlen = sizeof(clientaddr);

    // 연결 요청 accept, 연결된 소켓 connfd에 저장
    connfd = Accept(listenfd, (SA *)&clientaddr,
                    &clientlen);  // line:netp:tiny:accept
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE,
                0);

    // 연결된 클라이언트 host, port 출력
    printf("Accepted connection from (%s, %s)\n", hostname, port);

    // doit 함수 실행
    doit(connfd);   // line:netp:tiny:doit

    // doit 함수가 종료되면 연결 종료 후 다른 요청을 기다린다.
    Close(connfd);  // line:netp:tiny:close
  }
}


// 요청을 받아 처리하는 함수
void doit(int fd) {
	
	// 정적 요청인지 저장할 변수
	int is_static;

  // 파일의 메타데이터를 저장할 변수
	struct stat sbuf;

	char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
	char filename[MAXLINE], cgiargs[MAXLINE];

	// rio 구조체 선언
	rio_t rio;

	// 클라이언트의 Request Header를 읽는다
	// rio 구조체에 연결 소켓 할당
	Rio_readinitb(&rio, fd);

	// 클라로부터 받은 요청 헤더의 첫 번째 줄을 buf에 저장
  // 메소드, uri, HTTP 버전이 저장됨
	Rio_readlineb(&rio, buf, MAXLINE);

	// 헤더 출력
	printf("Request headers:\n");
	printf("%s", buf);
	
  // buf에 저장된 문자열을 공백으로 구분하여 각각 method, uri, version 변수에 할당한다.
	sscanf(buf, "%s %s %s", method, uri, version);

	// 요청이 GET이 아니라면 오류와 함께 return 한다
	if(strcasecmp(method, "GET")) {
		clienterror(fd, method, "501", "Not implemented",
				"Tiny does not implement this method");
		return;
	}
	// GET 이라면 진행한다. 나머지 요청 헤더를 읽어들인다.
	read_requesthdrs(&rio);

  // URI를 파싱하여 요청 파일 경로와 CGI 인자를 추출, 정적 콘텐츠라면 1 리턴한다.
	is_static = parse_uri(uri, filename, cgiargs);

  // stat 함수로 파일 상태를 확인하고, 파일 정보를 sbuf에 저장한다.
  // 성공하면 0 반환, 파일이 없거나, 접근권한이 없다면 -1 반환
	if(stat(filename, &sbuf) < 0) {

    // -1이 반환되었다면 오류 메시지 출력
		clienterror(fd, filename, "404", "Not found",
				"Tiny couldn't find this file");
		return;
	}

  // 정적 콘텐츠일 경우
	if(is_static) {

    // 일반 파일이 아니거나 OR 사용자 읽기 권한이 없다면 오류
    // 일반 파일이 아닌 것 : 디렉토리, 특수 파일, 장치 파일, 소켓 등등...
		if(!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {

      // 파일이 존재하지 않거나 읽을 수 없는 경우 오류 메시지 출력
			clienterror(fd, filename, "403", "Forbidden",
					"Tiny couldn't read the file");
			return;
		}
    // 파일을 클라이언트에게 전송
		serve_static(fd, filename, sbuf.st_size);
	}

  // 동적 콘텐츠일 경우
	else {
    
    // 일반 파일이 아니거나 OR 사용자 실행 권한이 없다면 오류
		if(!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {

      // 오류 메시지 출력
			clienterror(fd, filename, "403", "Forbidden",
					"Tiny couldn't run the CGI program");
			return;
		}
    // 프로그램을 실행하여 생성된 결과를 클라이언트에게 전송
		serve_dynamic(fd, filename, cgiargs);
	}
}

// 에러 메시지를 클라이언트에게 보내는 함수
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) {

	char buf[MAXLINE], body[MAXBUF];
	
	// HTTP Response body 제작
	sprintf(body, "<html><title>Tiny Error</title>");
	sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
	sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
	sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
	sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

	// HTTP Response 출력, Header Body 순서로
	// HTTP Response Header 출력
	sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
	Rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "Content-type: text/html\r\n");
	Rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
	Rio_writen(fd, buf, strlen(buf));

	// HTTP Response Body 출력
	// 출력 시 Response body의 크기를 쉽게 구하기 위해, body 라는 하나의 변수에 담은 것이다.
	Rio_writen(fd, body, strlen(body));
}


// 요청 헤더를 읽고 출력하는 함수
// 간단한 서버이거나, 응답이 헤더에 의존되지 않는다거나 하는 여러 가지 이유로 헤더를 읽고 무시할 수 있다.
void read_requesthdrs(rio_t *rp) {

	// 버퍼 선언	
	char buf[MAXLINE];

	// rio로 받은 데이터 buf에 저장, readlineb는 1줄씩 읽는다.
	Rio_readlineb(rp, buf, MAXLINE);

  // 읽어온 데이터가 \r\n일 때 까지 반복한다. ( 헤더가 끝날 때까지 반복한다 )
	while(strcmp(buf, "\r\n")) {

    // 읽어온 헤더를 출력
		Rio_readlineb(rp, buf, MAXLINE);
		printf("%s", buf);
	}
	return;
}


// 동적/정적 컨텐츠에 따라서, URI를 파싱해 파일 경로와 인자를 추출한다
// 정적 : 파일 경로만 추출, 동적 : 파일 경로 + cgi인자 추출
int parse_uri(char *uri, char *filename, char *cgiargs) {
	
	// 포인터 변수 선언
	char *ptr;
	
	// 정적 컨텐츠일 경우
  // uri에 cgi-bin이 포함되어 있지 않으면 정적 컨텐츠로 간주
	if(!strstr(uri, "cgi-bin")) {

    // cgiargs를 비운다
		strcpy(cgiargs, "");

    // filename을 "." 으로 한다
		strcpy(filename, ".");

    // filename에 uri를 추가한다  . -> ./파일명
    // 즉 파일명은 "/" 로 시작해야 한다.
		strcat(filename, uri);
		
		// 파일경로 마지막 글자가 '/' 일 때
		// -> '/' 경로만 입력되었을 때, 즉 접속하면 기본 페이지로 home.html을 출력한다
		if(uri[strlen(uri)-1] == '/') {
			strcat(filename, "home.html");
		}
		return 1;
	}

	// 동적 컨텐츠일 경우 ( cgi-bin이 uri에 포함되었을 경우 )
	else {

		// '?' 위치를 ptr에 담는다. '?'를 기준으로 인자가 시작되기 때문
		ptr = index(uri, '?');

		// '?' 문자가 존재하면 실행
		if(ptr) {

      // 인자를 cgiargs에 담기. ptr+1은 "?" 다음이므로 인자를 의미한다.
      // "?" 이후의 모든 인자들을 cgiargs에 담아주는 것이다.
			strcpy(cgiargs, ptr+1);

      // "?"를 널 문자로 바꿔준다.
			*ptr = '\0';
		}
		// '?' 문자가 없을 경우 ( 인자가 없을 경우 )
		else {

      // cgiargs를 빈 값으로 바꿔준다
			strcpy(cgiargs, "");
		}

    // filename을 "." 으로 설정한다
		strcpy(filename, ".");

    // filename에 파일 경로를 추가한다.
		strcat(filename, uri);
		return 0;
	}
}
		


// 정적 컨텐츠를 클라이언트로 전송하는 함수
void serve_static(int fd, char *filename, int filesize) {

	int srcfd;
	char *srcp, filetype[MAXLINE], buf[MAXBUF];
	
	// 파일 타입(확장자) 구하기 ( html, txt, gif, png, jpeg )
	get_filetype(filename, filetype);

	// Response Header 만들기
	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
	sprintf(buf, "%sConnection: close\r\n", buf);
	sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
	sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);

	// Response Header 전송하기
	Rio_writen(fd, buf, strlen(buf));

	// 전송한 Header를 서버측 로컬에 한번 띄우기
	printf("Response headers:\n");
	printf("%s", buf);

	// Response Body 만들어 전송하기
	// 요청받은 파일 열기
	srcfd = Open(filename, O_RDONLY, 0);

	// 파일을 가상메모리 영역으로 매핑하기
	srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
	
	// 매핑해놨으니 파일은 닫기
	Close(srcfd);

	// 가상메모리에 있던 파일 클라이언트로 보내기 ( Response Body 전송 )
	Rio_writen(fd, srcp, filesize);

	// 파일이 차지했던 메모리 반환
	Munmap(srcp, filesize);
}



// 파일 이름으로부터 확장자 추출
void get_filetype(char *filename, char *filetype) {

  // 파일명의 확장자를 통해 filetype을 지정한다
  if(strstr(filename, ".html")) {
    strcpy(filetype, "text/html");
  }
  else if(strstr(filename, ".gif")) {
    strcpy(filetype, "image/gif");
  }
  else if(strstr(filename, ".png")) {
    strcpy(filetype, "image/png");
  }
  else if(strstr(filename, ".jpg")) {
    strcpy(filetype, "image/jpeg");
  }
  else {
    strcpy(filetype, "text/plain");
  }
}



// 동적 컨텐츠를 클라이언트로 전송하는 함수
void serve_dynamic(int fd, char *filename, char *cgiargs) {

	char buf[MAXLINE], *emptylist[] = {NULL};

	// 요청 성공 메시지만 우선적으로 전송한다
	// 나머지 응답(헤더, 바디)은 CGI 프로그램이 전송해야 한다.
	sprintf(buf, "HTTP/1.0 200 OK\r\n");
	Rio_writen(fd, buf, strlen(buf));
	sprintf(buf, "Server: Tiny Web Server\r\n");
	Rio_writen(fd, buf, strlen(buf));

	// Fork로 자식 프로세스 생성, 생성되면 0을 반환한다
  // 자식 안 만들고 부모가 하게 되면, 클라와의 연결을 유지하면서 CGI 프로그램도 실행해야 돼서
  // 매우 비효율적이게 된다. 따라서 자식을 만들어 일 시킨다.
  // 다중 연결이 가능한 서버였다면 자식 프로세스를 만들어 CGI를 시키는 건 더욱 중요해진다.
	if(Fork() == 0) {
    // 이렇게 조건을 걸면, if문 블록은 전부 자식 프로세스가 실행하게 된다!!!
		
		// CGI 프로그램에 인자를 전달하기 위해, QUERY_STRING 환경변수의 값을 설정한다
		setenv("QUERY_STRING", cgiargs, 1);

		// 자식은 자신의 표준 출력을 연결 식별자로 재지정
    // STDOUT_FILENO는 표준 출력을 가리키는 상수이다.
    // fd를 복제하여, STDOUT_FILENO를 통해 하는 작업이 fd를 통해 하는 것과 동일한 효과를 낸다.
		// 즉 자식의 출력이 클라에게 전송됨
		Dup2(fd, STDOUT_FILENO);

		// CGI 프로그램 실행
		// CGI 프로그램에서 나머지 헤더와 바디를 전송한다.
		Execve(filename, emptylist, environ);
	}
  // 이 부분에 다른 코드들이 있었다면, 자식 프로세스(if문 코드)와 부모 프로세스가 동시에 활동할 수도 있다.
	// Wait을 통해 부모는 자식의 일이 끝나기를 기다리며, Wait이후엔 다시 부모가 활동한다.
  // 즉 Wait가 있다면 자식이 끝나기 전에 부모가 프로그램을 종료해버리는 일을 방지할 수 있다.
	Wait(NULL);
}



