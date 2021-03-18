#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<pthread.h> 
#include "test1.c"
#define MAX_CHAR 100
#define PORT 18000
pthread_mutex_t SEMAFORO = PTHREAD_MUTEX_INITIALIZER; 
#define GETTONI_INIZIALI 1000000
void *connection_handler(void*socket_desc);
int registraUtente(char* data, struct nodoUtenti* lista, FILE*fp);

int main(){
      FILE *fp;
    struct nodoUtenti* lista=NULL;
    fp=fopen("Utenti.txt", "r");
    if(!fp) {perror("ERRORE\n"); exit(0);}

    lista=LeggiFile(lista, fp);
    fclose(fp);

    int socketfd, ret;
    struct sockaddr_in serverAddr;

    int newSocket;
    int *new_sock;

    struct sockaddr_in newAddr;

    socklen_t addr_size;

    //char buffer[1024];
    //pid_t childpid;

    socketfd = socket(AF_INET,SOCK_STREAM,0);

    if(socketfd < 0){
        printf("\n error in socket creation");
        return -1;
    }
    printf("\n Server socket is created\n");

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    ret = bind(socketfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

    if(ret < 0){
        printf("Error in binding\n");
        return -1;
    }
    printf("[*]Bind to port %d\n", PORT);

    if(listen(socketfd, 10) == 0){
        printf("Listening...\n");
    }else{
        printf("Error in binding\n");
    }

   while( (newSocket = accept(socketfd, (struct sockaddr*)&newAddr, &addr_size) )){
    if( newSocket < 0){
        printf("No socket\n");
        exit(1);
    }
    else puts("Connection accepted");

    pthread_t new_thread;
    new_sock=malloc(1);
    *new_sock=newSocket;

    struct param_thread* parametri=CreaParametriThread(new_sock, fp, "", "", "", "", "", lista);

    if(pthread_create (&new_thread, NULL, connection_handler, (void*) parametri) <0){
        perror("Error while creating thread");
        return 1;
    }
    pthread_join(new_thread, NULL);
    puts("Handler assigned");
   }

   if(newSocket < 0){
       perror("Accept failed");
       return 1;
   }

   return 0;
}


void *connection_handler(void* parametri)
{
    puts("Handler started");
    struct param_thread* myParametri = ((struct param_thread*)parametri);
    int newSocket = *(int*)myParametri->sock;

    struct nodoUtenti* lista=myParametri->lista;
    FILE*fp=myParametri->file;

    int size = 1024;
    char buff[size];
    char sbuff[size];
    int n;
    int reader;
    memset(buff, 0, size);
    memset(sbuff, 0, size);
    // infinite loop for receiving and sending
    for (;;) {
        // read the message from client and copy it in buffer
        reader = recv(newSocket, buff, 1024 * sizeof(char), 0);
        if (reader == -1) {
            perror("recv()");
            break;
        } else if (reader == 0) {
            break;
        } else {
            // print buffer which contains the client contents
            printf("From client: %s\t To client : ", buff);
            // if msg contains "Exit" then server exit and chat ended.
            if (strncmp("exit", buff, 4) == 0) {
                printf("Server Exit...\n");
                break;
            }
        
            if (strncmp("register", buff, 8 ) == 0){
		    pthread_mutex_lock( & SEMAFORO); // INIZIO MEMORIA CRITICA
            registraUtente(buff, lista, fp);
            pthread_mutex_unlock( & SEMAFORO); // FINE MEMORIA CRITICA
            }

            if (strncmp("login", buff, 6) == 0){
            pthread_mutex_lock( & SEMAFORO); // INIZIO MEMORIA CRITICA
            accessoUtente(buff, lista, fp);
            pthread_mutex_unlock( & SEMAFORO); // FINE MEMORIA CRITICA
            }
            if (strncmp("online", buff, 8) == 0){
            pthread_mutex_lock( & SEMAFORO); // INIZIO MEMORIA CRITICA
            checkUtentiOnline(buff, lista, fp);
            pthread_mutex_unlock( & SEMAFORO); // FINE MEMORIA CRITICA
            }


            bzero(buff, size);

            n = 0;
            // copy server message in the buffer

            while ((sbuff[n++] = getchar()) != '\n');

        // and send that buffer to client
            write(newSocket, sbuff, sizeof(sbuff));
            bzero(sbuff,size);

        }

        }

    //close(newSocket);
    //free(socket_desc);

    return 0;
}

int checkUtentiOnline(char* data, struct nodoUtenti* lista, FILE*fp){
      fp=fopen("Utenti.txt", "r");
    if(!fp) {perror("ERRORE\n"); exit(0);}
    lista=LeggiFile(lista, fp);
    int i=0;

    while(lista){
        if(lista->isOnline == 1)
        i++;
        else(checkUtentiOnline(data,lista->next, fp));
    }
    fclose(fp);
    return i; //o inviare il messaggio direttamente
}

int registraUtente(char* data, struct nodoUtenti* lista, FILE*fp){
    //register = 8 - 10
    //nomeUtente max 8, se <8 i successivi sono spazi -10
    //password max 10, se <10 i successivi sono spazi -10
char part1[11];
char part2[11];
char part3[11];

memmove(part1, &data[0], 10);
part1[10] = '\0';
memmove(part2, &data[10], 10);
part2[10] = '\0';
memmove(part3, &data[20], 10);
part3[10] = '\0';

printf("%s - %s - %s \t", part1, part2, part3);
strtok(part1, "\n");
strtok(part2, "\n");
strtok(part3, "\n");

lista=InserisciCoda(lista, part2, part3, GETTONI_INIZIALI, true, -1);
//lista = InserisciCoda(part2, part3, GETTONI_INIZIALI, true, -1, lista);
fp=fopen("Utenti.txt", "w");
 if(!fp) {perror("ERRORE\n"); exit(0);}
StampaListaToFileInOrdine(lista, fp);
StampaLista(lista);
fclose(fp);

}

int accessoUtente(char* data, struct nodoUtenti* lista, FILE*fp){
    fp=fopen("Utenti.txt", "r");
    if(!fp) {perror("ERRORE\n"); exit(0);}
lista=LeggiFile(lista, fp);
fclose(fp);
char part1[11];
char part2[11];
char part3[11];
memmove(part1, &data[0], 10);
part1[10] = '\0';
memmove(part2, &data[10], 10);
part2[10] = '\0';
memmove(part3, &data[20], 10);
part3[10] = '\0';

printf("%s - %s - %s \t", part1, part2, part3);
strtok(part1, "\n");
strtok(part2, "\n");
strtok(part3, "\n");

accessoUtente_server(part2, part3, lista);
}

int accessoUtente_server(char* nome, char*password, struct nodoUtenti* lista){
    while(lista){
        if(  (strcmp(nome, lista->nickname) ==0) && (strcmp(password, lista->password) ==0)  )
        return 1;
        else accessoUtente_server(nome, password, lista->next);
    }
    return 0;
}
