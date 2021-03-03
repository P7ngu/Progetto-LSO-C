#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <string.h>

#define SERVER_PORT 18000
#define MAXLINE 4096
#define SA struct sockaddr

char*bin2hex(const unsigned char*input, size_t len){
    char *result;
    char*hexits="0123456789ABCDEF";

    if(input == NULL || len <=0)
    return NULL;

    int resultlength=(len*3)+1;

    result = malloc(resultlength);
    bzero(result, resultlength);

    for(int i=0; i<len; i++){
        result[i*3]=hexits[input[i]>>4];
        result[(i*3)+1]=hexits[input[i]&0X0F];
        result[(i*3)+2]=' ';
    }

    return result;
}

int main(int argc, char**argv){
    int listenfd, connfd, n;
    struct sockaddr_in servaddr;
    uint8_t buff[MAXLINE+1], recvline[MAXLINE+1];

    if( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) <0)
    printf("Errore socket \n");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    //servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    //servaddr.sin_addr.s_addr = inet_addr("2.0.205.146");
    servaddr.sin_port = htons(SERVER_PORT);

    if( (bind(listenfd, (SA*) &servaddr, sizeof(servaddr))<0))
    printf("Bind error\n");

    if((listen(listenfd, 10))<0)
    printf("Listen error\n");


    for( ; ; ){
        struct sockaddr_in addr;
        socklen_t addr_len;
        char client_address[MAXLINE+1];

        printf("Waiting for a connection on port %d", SERVER_PORT);
        fflush(stdout);
        connfd=accept(listenfd, (SA*) &addr, &addr_len);
        inet_ntop(AF_INET, &addr, client_address, MAXLINE);
        printf("Client connected: %s\n", client_address);

        memset(recvline, 0, MAXLINE);

        while( (n=read(connfd, recvline, MAXLINE-1))>0){
            fprintf(stdout, "\n%s\n\n%s", bin2hex(recvline, n), recvline);
            //detect end of message
            if(recvline[n-1]=='\n')
            break;

            memset(recvline, 0, MAXLINE);
        }

        if(n<0) printf("Read error \n");

        //now send a response
        snprintf((char*)buff, sizeof(buff), "HTTP/1.0 200 OK \r\n\r\nHello");

        write(connfd, (char*)buff, strlen( (char*)buff ) );
        close(connfd);
    }

}