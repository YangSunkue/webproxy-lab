#include "csapp.h"

int main(int argc, char **argv) {

    int clientfd;
    char *host, *port, buf[MAXLINE];
    rio_t rio;

    if(argc != 3) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }

    // host, port 가져오기
    host = argv[1];
    port = argv[2];

    // 클라이언트 측 소켓 열기
    clientfd = Open_clientfd(host, port);
    Rio_readinitb(&rio, clientfd);

    while(Fgets(buf, MAXLINE, stdin) != NULL) {
        Rio_writen(clientfd, buf, strlen(buf));
        Rio_readlineb(&rio, buf, MAXLINE);
        Fputs(buf, stdout);
    }
    // exit 하면서 커널이 자동으로, 열렸던 식별자들을 닫아주지만 명시적으로 닫아 주는 게 좋은 습관
    Close(clientfd);

    exit(0);
}