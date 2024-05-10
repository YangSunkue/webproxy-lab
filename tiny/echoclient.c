#include "csapp.h"

// 인자 3개를 받는다.
int main(int argc, char **argv) {

    // 클라 소켓의 fd를 저장할 변수
    int clientfd;
    // 서버호스트명, 서버포트번호, 버퍼 저장할 변수
    char *host, *port, buf[MAXLINE];
    // rio 구조체 선언
    rio_t rio;

    // 이 프로그램 실행 시, 인자가 3개가 아니면 사용법을 출력한다
    // 인자 1 : ./echoclient, 인자 2 : 호스트, 인자 3 : 포트번호
    if(argc != 3) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }

    // 인자로 받은 host, port 변수에 저장
    host = argv[1];
    port = argv[2];

    // 지정한 서버 호스트, 포트에 연결된 클라이언트 소켓 열기
    clientfd = Open_clientfd(host, port);

    // rio 구조체에 클라이언트 소켓 할당하기
    Rio_readinitb(&rio, clientfd);

    // 사용자가 입력한 데이터를 서버로 전송하고, 응답을 출력하는 무한 루프 가동
    // 아무것도 입력하지 않고 엔터를 누르면 \n이 전송되고(1byte) 연결이 끊기지 않는다.
    // 즉 클라이언트가 Ctrl + C를 누르거나 Ctrl + D 를 누를 경우에만 종료된다.
    // 사용자 입력값은 buf에 저장된다
    while(Fgets(buf, MAXLINE, stdin) != NULL) {

        // 사용자로부터 입력받은 데이터를 소켓을 통해 서버로 전송한다
        Rio_writen(clientfd, buf, strlen(buf));
        // 소켓으로 서버로부터 받은 응답을 buf에 저장한다
        Rio_readlineb(&rio, buf, MAXLINE);
        // buf에 저장된 값을 표준 출력한다
        Fputs(buf, stdout);
    }
    // exit 하면서 커널이 자동으로, 열렸던 식별자들을 닫아주지만 명시적으로 닫아 주는 게 좋은 습관
    Close(clientfd);

    exit(0);
}