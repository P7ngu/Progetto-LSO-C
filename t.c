#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<pthread.h> 
#include <time.h>
#include "test1.c"
#define MAX_CHAR 100
#define PORT 18000
#define MAX_SIZE 128
pthread_mutex_t SEMAFORO = PTHREAD_MUTEX_INITIALIZER; 
#define GETTONI_INIZIALI 1000000
int bet_time=20; //5 minuti tra due bet
long int start_time;
long int current_time;
char utentiOnline[MAX_SIZE];

void *connection_handler(void*socket_desc);
void* startTheTimer();
int registraUtente(char* data, struct nodoUtenti* lista, FILE*fp);
char* checkUtentiOnline(char* data, struct nodoUtenti* lista, FILE*fp);

//FIX: ALLO START RESETTARE NUMEROPUNTATO E GETTONI PUNTATI

int main(){

     pthread_t timer_thread;
   if( pthread_create(&timer_thread, NULL, startTheTimer, NULL) <0 ){
    perror("Error while creating timer thread");
        return 1;
   }

      FILE *fp;
    struct nodoUtenti* lista=NULL;
    fp=fopen("Utenti.txt", "r");
    if(!fp) {perror("ERRORE\n"); exit(0);}

    lista=LeggiFile(lista, fp);
    fclose(fp);

    //if(lista) resettaPuntatePrecedenti(lista);

    printf("Lista utenti: \n");
    StampaLista(lista);

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

    struct nodoUtenti* lista = myParametri->lista;
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
            if(registraUtente(buff, lista, fp)==1){
                char *str;
                str = malloc (sizeof (char) * MAX_SIZE);
                strcpy (str, "register_success\n\n");
                send(newSocket, str, strlen(str), 0);
                printf("%s", str);
            } else{
                char *str;
                str = malloc (sizeof (char) * MAX_SIZE);
                strcpy (str, "register_fail");
                strcat (str, "\n");
                send(newSocket, str, strlen(str), 0);
                printf("%s", str);
            }
            pthread_mutex_unlock( & SEMAFORO); // FINE MEMORIA CRITICA
            }

            if (strncmp("login", buff, 5) == 0){
                //log in corso
            pthread_mutex_lock( & SEMAFORO); // INIZIO MEMORIA CRITICA
            if(accessoUtente(buff, lista, fp)==1){
                char *str;
                str = malloc (sizeof (char) * MAX_SIZE);
                strcpy (str, "login_success");
                strcat (str, "\n");
                send(newSocket, str, strlen(str), 0);
                printf("%s", str);
            } else{
                printf("\n \n Ecco la lista: \n\n");
                StampaLista(lista);
                char *str;
                str = malloc (sizeof (char) * MAX_SIZE);
                strcpy (str, "login_fail");
                strcat (str, "\n");
                send(newSocket, str, strlen(str), 0);
                printf("%s", str);
            }
            pthread_mutex_unlock( & SEMAFORO); // FINE MEMORIA CRITICA
            }

            if (strncmp("online", buff, 6) == 0){
            pthread_mutex_lock( & SEMAFORO); // INIZIO MEMORIA CRITICA
            char onlineUsers[MAX_SIZE];

            char* str;
            str=malloc(sizeof(char)*MAX_SIZE);
            *str = checkUtentiOnline(buff, lista, fp);
            strcat(utentiOnline, "\n");
           
            send(newSocket, utentiOnline, strlen(utentiOnline), 0);
            printf("\ndopo send online\n");
            printf("\n lista utenti online: %s", utentiOnline);
            //send
            pthread_mutex_unlock( & SEMAFORO); // FINE MEMORIA CRITICA
            }

            if (strncmp("puntata", buff, 7) == 0){
            pthread_mutex_lock( & SEMAFORO); // INIZIO MEMORIA CRITICA
            betNumber(buff, lista, fp);
            printf("E' arrivata una bet\n");
            pthread_mutex_unlock( & SEMAFORO); // FINE MEMORIA CRITICA
            }
            if (strncmp("estrazione", buff, 10) == 0){
            pthread_mutex_lock( & SEMAFORO); // INIZIO MEMORIA CRITICA
            int numeroEstratto = extractNumber();
             char *str;
                str = malloc (sizeof (char) * MAX_SIZE);
                //strcpy (str, time_left);
                sprintf(str, "%d", numeroEstratto);
                strcat (str, "\n");
                send(newSocket, str, strlen(str), 0);
                printf("%s", str);
                if(lista) aggiornaDatiUtentiDopoBet(numeroEstratto, lista);
            pthread_mutex_unlock( & SEMAFORO); // FINE MEMORIA CRITICA
            }
              if (strncmp("latestnumber", buff, 12) == 0){
            pthread_mutex_lock( & SEMAFORO); // INIZIO MEMORIA CRITICA
            printf("Richiesta ultimo numero \n");
            int numeroLetto = readLatestNumber();
            char *str;
                str = malloc (sizeof (char) * MAX_SIZE);
                //strcpy (str, time_left);
                sprintf(str, "%d", numeroLetto);
                strcat (str, "\n");
                send(newSocket, str, strlen(str), 0);
                printf("%s", str);
            pthread_mutex_unlock( & SEMAFORO); // FINE MEMORIA CRITICA
            }
            if (strncmp("timeleft", buff, 8) == 0){
            pthread_mutex_lock( & SEMAFORO); // INIZIO MEMORIA CRITICA
            int time_left = getTimerTimeLeft();
            printf("Chiamato metodo %d \n", time_left);
            char *str;
                str = malloc (sizeof (char) * MAX_SIZE);
                //strcpy (str, time_left);
                sprintf(str, "%d", time_left);
                strcat (str, "\n");
                send(newSocket, str, strlen(str), 0);
                printf("%s", str);
            pthread_mutex_unlock( & SEMAFORO); // FINE MEMORIA CRITICA
            }

            bzero(buff, size);
           // n = 0;
            // copy server message in the buffer
            //while ((sbuff[n++] = getchar()) != '\n');
        // and send that buffer to client
            //write(newSocket, sbuff, sizeof(sbuff));
            //bzero(sbuff,size);

        }


    //close(newSocket);
    //free(socket_desc);
    }

    return 0;
}

aggiornaFileUtenteDopoBet(struct nodoUtenti* lista){
FILE*fp = fopen("Utenti.txt", "w");
if(!fp) {perror("Errore apertura aggiorna file utente dopo bet \n"); exit(-1);}
ScriviFile(lista, fp);
fclose(fp);

}

aggiornaDatiUtentiDopoBet(int numero, struct nodoUtenti* lista){
    printf("\n Lista prima aggiornamento ");
    StampaLista(lista);
    struct nodoUtenti*tmp=(struct nodoUtenti*)malloc(sizeof(struct nodoUtenti));
    tmp = lista; //salvo la testa
    tmp->next=lista->next;
    int lung=LunghezzaLista(lista);
    for(int i=0; i<lung; i++){  
    if(lista->numeroPuntato == numero){
    //Aggiornare i crediti
    lista->numeroPuntato=-1;
    lista->gettoni=lista->gettoni + (lista->gettoniPuntati)*30;
    lista->gettoniPuntati=0;
    lista=lista->next;
    }else {
    lista->numeroPuntato=-1;
    lista->gettoniPuntati=0;
    lista=lista->next;
        }
    }
    printf("\nLista dopo aggiornamento ");
    lista=tmp;
    lista->next=tmp->next;
    StampaLista(tmp);
    aggiornaFileUtenteDopoBet(lista);  
}

int readLatestNumber(int numeroLetto){
    FILE*fp=fopen("UltimoNumeroEstratto.txt", "w");
    if(!fp) {perror("Errore apertura ultimo numero \n"); exit(-1);}
    fscanf(fp, "%d", &numeroLetto);
    fclose(fp);
    return numeroLetto;

}


int extractNumber(){
    int randomNumber=rand() &36;
    readLatestNumber(randomNumber);
    FILE*fp=fopen("UltimoNumeroEstratto.txt", "w");
     if(!fp) {perror("Errore apertura estrazione numero \n"); exit(-1);}
     fprintf(fp, "%d\n", randomNumber);
     printf("\nNumero estratto: %d \n", randomNumber);
     fclose(fp);
     return randomNumber;
}

int  betNumber(char* data, struct nodoUtenti* lista,FILE* fp){
char part1[11];
char part2[2];
char part3[11];
char part4[11];

memmove(part1, &data[0], 10);
part1[10] = '\0';
memmove(part2, &data[10], 2);
part2[2] = '\0';
memmove(part3, &data[12], 10);
part3[10]='\0';
memmove(part4, &data[22], 10);
part4[10]='\0';

printf("BET: %s \t numero: %s \t nome: %s \t amount: %s \n", part1, part2, part3, part4);
strtok(part1, "\n");
strtok(part2, "\n");
strtok(part3, "\n");
strtok(part4, "\n");

//aggiorno file log
 printf("\nPrima ins scomm\n");
StampaLista(lista);

remove_spaces(part3);
inserisciScommessa(lista, part2, part3, part4);

printf("Lista dopo bet\n");
StampaLista(lista);

}

void remove_spaces(char* s) {
    const char* d = s;
    do {
        while (*d == ' ') {
            ++d;
        }
    } while (*s++ = *d++);
}


int inserisciScommessa(struct nodoUtenti* lista, char* numero, char*nome, char*amount){
    if(lista){
        printf("Dentro ins Scomm fuori if2\n");
        //TODO FIX CHAR E INT
        if(strcmp(lista->nickname, nome)==0){
        int numeroI = atoi(numero);
        int amountI = atoi(amount);

        lista->numeroPuntato=numeroI;
        lista->gettoniPuntati=amountI;
        lista->gettoni=(lista->gettoni)-(lista->gettoniPuntati); //AGGIUNGERE CHECK GETTONI PUNTATI
        }
       else inserisciScommessa(lista->next, numero, nome, amount);
    }
    aggiornaFileUtenteDopoBet(lista);
    printf("\nFine ins scomm\n");
        StampaLista(lista);
}

char* checkUtentiOnline(char* data, struct nodoUtenti* lista, FILE*fp){
    char listaOnline[MAX_SIZE];
    int len=LunghezzaLista(lista);

    for(int i=0; i<len; i++){
        if(lista->isOnline == 1){
        strcat (utentiOnline, lista->nickname);
        strcat(utentiOnline, "$$$");
        }
        lista=lista->next;
    }
    printf("\n Dentro check, utenti online: %s \n", utentiOnline);
    return utentiOnline; //o inviare il messaggio direttamente
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

printf("%s - %s - %s \t \n", part1, part2, part3);
strtok(part1, "\n");
strtok(part2, "\n");
strtok(part3, "\n");
remove_spaces(part3);
remove_spaces(part2);
remove_spaces(part1);

if(contains(lista, part2)){
    printf("Username già esistente \n");
return 0;
} 
printf("Prima inserimento in coda \n");
lista=InserisciCoda(lista, part2, part3, GETTONI_INIZIALI, 1, -1, -1);
printf("Dopo inserimento in coda \n");
//lista = InserisciCoda(part2, part3, GETTONI_INIZIALI, true, -1, lista);
fp=fopen("Utenti.txt", "w");
 if(!fp) {perror("ERRORE\n"); exit(0);}
StampaListaToFileInOrdine(lista, fp);
StampaLista(lista);
fclose(fp);
return 1;

}

int accessoUtente(char* data, struct nodoUtenti* lista, FILE*fp){
    //fp=fopen("Utenti.txt", "r");
    //if(!fp) {perror("ERRORE\n"); exit(0);}
//lista=LeggiFile(lista, fp);
//fclose(fp);
char part1[11];
char part2[11];
char part3[11];
memmove(part1, &data[0], 10);
part1[10] = '\0';
memmove(part2, &data[10], 10);
part2[10] = '\0';
memmove(part3, &data[20], 10);
part3[10] = '\0';

printf("%s - %s - %s \n", part1, part2, part3);
strtok(part1, "\n");
strtok(part2, "\n");
strtok(part3, "\n");
remove_spaces(part3);
remove_spaces(part2);
remove_spaces(part1);

if(accessoUtente_server(part2, part3, lista) ==1) return 1;
else return 0;


}

int accessoUtente_server(char* nome, char*password, struct nodoUtenti* lista){
    if(lista){ 
        if(  (strcmp(nome, lista->nickname) ==0) && (strcmp(password, lista->password) ==0)  ){
            lista->isOnline=1;
        return 1;
        }
        else return accessoUtente_server(nome, password, lista->next);
    }
    return 0;
}

// Timer stuff
//int gettimeofday(struct timeval *restrict tp, void *restrict tzp); The gettimeofday() function shall obtain the current time,
// expressed as seconds and microseconds since the Epoch, and store it in the timeval structure pointed to by tp. 

void* startTheTimer(){
    printf("\n Timer inizializzato! \n");

    struct timeval *restrict time=(struct timeval*)malloc(sizeof(struct timeval));
    gettimeofday(time, NULL); //inizia il countdown
    printf("\n %d <-time \n", time->tv_sec);
    start_time=time->tv_sec;

    //in un thread
    while(1){
        gettimeofday(time, NULL);
        current_time=time->tv_sec;
        if (current_time-start_time == bet_time){
        //extractNumber();
        startTheTimer();

        
        }
    }
   
}

int getTimerTimeLeft(){
    struct timeval *restrict time=(struct timeval*)malloc(sizeof(struct timeval));
    gettimeofday(time, NULL);
    current_time = time->tv_sec;
    return (bet_time - (current_time-start_time));
}

