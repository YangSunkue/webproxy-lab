#include "csapp.h"

// 클라이언트와의 연결 소켓 fd를 입력받는다
void echo(int connfd) {

    // 수신한 데이터의 크기를 저장할 변수 n
    size_t n;
    // 수신한 데이터를 저장할 버퍼 선언
    // MAXLINE은 버퍼 최대 크기를 나타내는 상수
    char buf[MAXLINE];
    // 리오 구조체. 버퍼링된 입출력을 지원하는 라이브러리
    // 버퍼링된 입출력 : 데이터를 버퍼에 저장한 뒤 한 번에 입출력하여 효율적인 입/출력을 가능케 함, bufferedreader 같은친구
    rio_t rio;

    // rio를 connfd와 연결한다. rio 구조체는 소켓의 입/출력을 다룬다.
    Rio_readinitb(&rio, connfd);

    // 무한루프 시작. 데이터가 buf에 저장, n에 읽은 바이트 수 저장
    // n == 0 ( 데이터를 읽지 못하면, 즉 클라가 연결을 끊었을 때 ) 루프 종료
    // Rio_readlineb 함수는 클라가 연결을 종료하거나, EOF를 보냈을 경우에 0을 리턴한다!
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {

        // 수신된 데이터 크기 출력
        printf("server received %d bytes\n", (int)n);

        // connfd 소켓에 있는 buf 데이터를 n만큼 연결된 곳으로(클라) 전송한다
        Rio_writen(connfd, buf, n);
    }
}