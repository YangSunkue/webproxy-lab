/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
#include "csapp.h"

int main(void) {
	char *buf, *p;
	char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
	int n1 = 0, n2 = 0;
	
	// QUERY_STRING 환경변수에 저장된, HTTP Request 인자로 받은 값을 getenv 함수로 가져온다
	if((buf = getenv("QUERY_STRING")) != NULL) {
		p = strchr(buf, '&');
		*p = '\0';
		strcpy(arg1, buf);
		strcpy(arg2, p+1);
		n1 = atoi(arg1);
		n2 = atoi(arg2);
	}

	// Response Body 만들기
	sprintf(content, "QUERY_STRING=%s", buf);
	sprintf(content, "Welcome to add.com: ");
	sprintf(content, "%sTHE Internet addition portal. \r\n<p>", content);
	sprintf(content, "%sThe answer is: <h1>%d + %d = %d</h1>\r\n<p>", content, n1, n2, n1 + n2);
	sprintf(content, "%sThanks for visiting!\r\n", content);

	// Response Header/Body 출력
	printf("Connection: close\r\n");
	printf("Content-length: %d\r\n", (int)strlen(content));
	printf("Content-type: text/html\r\n\r\n");
	printf("%s", content);
	fflush(stdout);



  exit(0);
}
/* $end adder */
