#include "csapp.h"

// 연결 식별자 fd를 받아서 클라에게 메시지 에코해준다.
void echo(int connfd);

// 인자 2개를 받는다.
int main(int argc, char **argv) {

    // 리스닝 소켓, 연결된 소켓 fd 저장할 변수
    int listenfd, connfd;
    // 클라이언트 주소 구조체의 크기를 저장할 변수
    socklen_t clientlen;
    // 클라 주소 정보 저장하는 구조체를 저장할 변수
    // sockaddr_storage는 모든 소켓 주소 유형을 저장할 수 있는 범용적인 구조체다
    struct sockaddr_storage clientaddr;
    // 클라 호스트이름, 클라 포트번호
    char client_hostname[MAXLINE], client_port[MAXLINE];

    // 이 프로그램 실행 시, 인자가 2개가 아니면 사용법을 출력한다
    // 인자 1 : ./echoserveri , 인자 2 : 포트번호
    if(argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    // 인자로 받은 포트에서 클라 연결을 수신하기 위한 listen 소켓 열기
    listenfd = Open_listenfd(argv[1]);

    // 무한루프 시작, 클라이언트 측 연결 요청 계속 받기
    while(1) {
        clientlen = sizeof(struct sockaddr_storage);

        // Accept 함수로 클라이언트 측 연결 수락하고, 연결식별자 fd 반환
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

        // Getnameinfo 함수로 클라이언트 호스트이름, 포트번호 가져오기
        Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);

        // 최초 연결 시 클라이언트 호스트이름, 포트번호 출력하기
        printf("Connected to (%s, %s)\n", client_hostname, client_port);

        // 클라가 연결 종료하거나 EOF 보낼 때까지 echo 함수 실행
        // 클라가 보낸 메시지를 똑같이 echo 한다
        echo(connfd);

        // echo가 끝났다면 클라와 연결되었던 소켓을 닫아준다
        Close(connfd);
    }

    exit(0);
}