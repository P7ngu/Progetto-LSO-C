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
#include <pthread.h>

#define SERVER_PORT 8989
#define MAXLINE 4096
#define SOCKETERROR (-1)
#define SERVER_BACKLOG 100

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

void* handle_connection(void* p_client_socket);
int check(int exp, const char*msg);

int main(int argc, char**argv){
    int server_socket, client_socket, addr_size;
    SA_IN server_addr, client_addr;

    check((server_socket = socket(AF_INET, SOCK_STREAM, 0)), 
    "Failed to create socket \n");

    //initialize address struct
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=INADDR_ANY;
    server_addr.sin_port=htons(SERVER_PORT);

    check(bind(server_socket, (SA*)&server_addr, sizeof(server_addr)),
    "Bind failed! \n");
    check(listen(server_socket, SERVER_BACKLOG), 
    "Listen failed! \n");

    while(1){
        printf("Waiting for connections...\n");

        addr_size=sizeof(SA_IN);

        check(client_socket = accept(server_socket, (SA*)&client_addr, (socklen_t*)&addr_size),
        "Accept failed\n");

        printf("Connected! \n");

        handle_connection(client_socket);
        pthread_t t;
        int*pclient=malloc(sizeof(int));
        *pclient=client_socket;
        pthread_create(&t, NULL, handle_connection, pclient);
    }

    return 0;

}

int check(int exp, const char*msg){
    if(exp == SOCKETERROR){
        perror(msg);
        exit(1);
    }
    return exp;
}

void* handle_connection(void* p_client_socket){
    int client_socket = *((int*)p_client_socket);
    free(p_client_socket);
    char buffer[BUFSIZ];
    size_t bytes_read;
    int msgsize = 0;
    char actualpath[_PC_PATH_MAX+1];

    //read the client's message: which file to read

    while((bytes_read = read(client_socket, buffer+msgsize, sizeof((buffer)-msgsize-1)) >0 )){
        msgsize+= bytes_read;
        if(msgsize>BUFSIZ-1 || buffer[msgsize-1]=='\n') break;
    }

    check(bytes_read, "recv error \n");

    buffer[msgsize-1]=0;

    printf("REQUEST: %s \n", buffer);
    fflush(stdout);

    //validity check
    if(realpath(buffer, actualpath)==NULL){
        printf("ERROR BAD PATH : %s \n", buffer);
        close(client_socket);
        return NULL;
    }

    //read file
    FILE*fp=fopen(actualpath, "r");
    if(!fp){
        printf("ERROR OPEN : %s \n", buffer);
        close(client_socket);
        return NULL;
    }

    while((bytes_read=fread(buffer, 1, BUFSIZ, fp))>0){
        printf("Sending %zu bytes \n", bytes_read);
        write(client_socket, buffer, bytes_read);
    }

    close(client_socket);
    fclose(fp);
    printf("Closing connection \n");

    return NULL;

    //CTRL C TO TERMINATE IN THE TERMINAL

}