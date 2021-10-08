#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<pthread.h> 
#include <stdbool.h> 
#include <time.h>
#include "test2.c"

#define MAX_CONNECTION 20
#define MAX_CHAR 100
#define PORTDEFAULT 18000
#define MAX_SIZE 128
pthread_mutex_t SEMAFORO = PTHREAD_MUTEX_INITIALIZER; 
#define GETTONI_INIZIALI 1000000

int bet_time=45; //5 minuti tra due bet
long int start_time;
long int current_time;
char utentiOnline[MAX_SIZE];
char listaUtentiExt[MAX_SIZE];
FILE *currentFileServerLog;

struct nodoUtenti* aggiornaDatiUtentiDopoBet(int numero, struct nodoUtenti* lista);
void scriviLogSuFile(char* message);
void *connection_handler(void*socket_desc);
void* startTheTimer();
struct nodoUtenti* registraUtente(char* data, struct nodoUtenti* lista, FILE*fp);
char* checkUtentiOnline(char* data, struct nodoUtenti* lista, FILE*fp);
void checkListaUtenti(char* data, struct nodoUtenti* lista, FILE*fp);
bool isNumeroRosso(int numero);
bool isNumeroNero(int numero);
logoutUtente(char* data, struct nodoUtenti* lista, FILE*fp);

long int PORT;

//FIX: ALLO START RESETTARE NUMEROPUNTATO E GETTONI PUNTATI fatto

int main(int argc, char* argv[]){
    if( argc == 2 ) {
      printf("The argument supplied is %s\n", argv[1]);
        PORT=atoi(argv[1]);
   } else perror("Fornire una porta da riga di comando");

     pthread_t timer_thread;
   if( pthread_create(&timer_thread, NULL, startTheTimer, NULL) <0 ){
    perror("Error while creating timer thread");
        return 1;
   }

      FILE *fp;
    struct nodoUtenti* lista=NULL;
    struct nodoUtenti* copiaLista=NULL;
    fp=fopen("Utenti.txt", "r");
    if(!fp) {perror("ERRORE FILE UTENTI\n"); exit(0);}

    //currentFileServerLog = fopen("ServerLog.txt", "w");
    //if(!currentFileServerLog){perror("Errore apertura file di log\n");}
    scriviLogSuFile("FILE LOG APERTO \n");


    lista=LeggiFile(lista, fp);
    fclose(fp);

    if(lista) copiaLista = resettaPuntatePrecedenti(lista);

    printf("\nLista utenti: \n");
    StampaLista(lista);
    printf("\ndiwjdiwjdiwejdw");

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

    if(listen(socketfd, MAX_CONNECTION) == 0){ //10 max parallel requests queued
        printf("Listening...\n");
        scriviLogSuFile("Bind effettuato, in ascolto...\n");
    }else{
        printf("Error in binding\n");
    }
    pthread_t tid[60];
    int i=0;

    while(1){
        //Accept creates a new socket for the incoming connection

        while( (newSocket = accept(socketfd, (struct sockaddr*)&newAddr, &addr_size) )){
            if( newSocket < 0){
             fprintf(stderr, "socket() failed: %s\n", strerror(errno));
             printf("No socket: %d\n", newSocket);
             exit(1);
            }
            else puts("Connection accepted");
            scriviLogSuFile("Connessione accettata\n");

            pthread_t new_thread;
            new_sock=malloc(1);
            *new_sock=newSocket;

            struct param_thread* parametri=CreaParametriThread(new_sock, fp, "", "", "", "", "", lista);
            //for each client request creates a thread and assign the client request to it to process
            //so the main thread can entertain next request
            if( pthread_create(&tid[i++], NULL, connection_handler, (void*) parametri) != 0 )
            printf("Failed to create thread\n");

            if( i >= MAX_CONNECTION)
            {
                i = 0;
                while(i < MAX_CONNECTION){
                    pthread_join(tid[i++],NULL);
                    puts("Handler assigned");
                    scriviLogSuFile("Handler assegnato\n");
                } 
                i = 0;
            }
        }

        if(newSocket < 0){
            perror("Accept failed");
            return 1;
        }
    }

        fclose(currentFileServerLog);
    return 0;
}


            
            

void scriviLogSuFile(char* message){
     currentFileServerLog = fopen("ServerLog.txt", "a");
    if(!currentFileServerLog){perror("Errore apertura file di log\n");}
   // fseek (currentFileServerLog , 0 , SEEK_END );
    fprintf(currentFileServerLog, "%s \n", message);
     fclose(currentFileServerLog);
    
}


void *connection_handler(void* parametri)
{
    puts("Handler started");
    scriviLogSuFile("Inizio connection_handler\n");
    struct param_thread* myParametri = ((struct param_thread*)parametri);
    int newSocket = *(int*)myParametri->sock;

    struct nodoUtenti* lista = myParametri->lista;
    FILE*fp=myParametri->file;
    struct nodoUtenti*tmp=(struct nodoUtenti*)malloc(sizeof(struct nodoUtenti));
    tmp = lista; //salvo la testa


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
            scriviLogSuFile("Messaggio dal client:");
            scriviLogSuFile(buff);
            scriviLogSuFile("\n \n");
            printf("From client: %s\t To client : ", buff);
            // if msg contains "Exit" then server exit and chat ended.
            if (strncmp("exit", buff, 4) == 0) {
                printf("Server Exit...\n");
                break;
            }
           
            if (strncmp("register", buff, 8 ) == 0){
                 printf("Sto stampando prima del register \n riga 165: \n");
                StampaLista(lista);
		    pthread_mutex_lock( & SEMAFORO); // INIZIO MEMORIA CRITICA
            lista = registraUtente(buff, lista, fp);
                printf("Sto stampando nel register \n riga 169: \n");
                if(lista)StampaLista(lista);
                char *str;
                str = malloc (sizeof (char) * MAX_SIZE);
                strcpy (str, "register_success\n\n");
                send(newSocket, str, strlen(str), 0);
                printf("%s", str);
          
            pthread_mutex_unlock( & SEMAFORO); // FINE MEMORIA CRITICA
            }

            if (strncmp("login", buff, 5) == 0){
                 printf("Sto stampando nel login \n riga 188:");
                StampaLista(lista);
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

            if (strncmp("listautenti", buff, 11) == 0){
            pthread_mutex_lock( & SEMAFORO); // INIZIO MEMORIA CRITICA
            char listaUtenti[MAX_SIZE*3];

            char* str;
            str=malloc(sizeof(char)*MAX_SIZE);
             checkListaUtenti(buff, lista, fp);
            //strcat(listaUtenti, str);
            strcat(listaUtentiExt, "\n");
           
            send(newSocket, listaUtentiExt, strlen(listaUtentiExt), 0);
            printf("\ndopo send lista utenti\n");
            printf("\n lista utenti : %s", listaUtentiExt);
            //send
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
             if (strncmp("logout", buff, 6) == 0){
            pthread_mutex_lock( & SEMAFORO); // INIZIO MEMORIA CRITICA
            logoutUtente(buff, lista, fp);
            send(newSocket, buff, strlen(buff), 0);
            printf("E' arrivata un logout per %s\n", buff);
            pthread_mutex_unlock( & SEMAFORO); // FINE MEMORIA CRITICA
            }
            if (strncmp("estrazione", buff, 10) == 0){
            pthread_mutex_lock( & SEMAFORO); // INIZIO MEMORIA CRITICA
            int numeroEstratto = extractNumber();
             char *str;
                str = malloc (sizeof (char) * MAX_SIZE);
                //strcpy (str, time_left);
                sprintf(str, "%d", numeroEstratto);
                strcat (str, "**\n");
                send(newSocket, str, strlen(str), 0);
                printf("%s", str);
                if(lista) lista=aggiornaDatiUtentiDopoBet(numeroEstratto, lista);
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
                strcat (str, "--\n");
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

 logoutUtente(char* data, struct nodoUtenti* lista, FILE*fp)
 {
     StampaLista(lista);
    char part1[7]; //11
    char nomeDaSloggare[10]; //2

memmove(part1, &data[0], 6); //0, 10
part1[6] = '\0'; //10
memmove(nomeDaSloggare, &data[6], 10); //10, 2
nomeDaSloggare[10] = '\0'; //2
//TODO QUI 

printf("LOGOUT  nome: %s \n", nomeDaSloggare);
strtok(part1, "\n");

strtok(nomeDaSloggare, "\n");
remove_spaces(nomeDaSloggare);

 logOutUser(lista, nomeDaSloggare);
 aggiornaFileUtenteDopoBet(lista);
 StampaLista(lista);
 scriviLogSuFile("Logout completato\n");
 }


aggiornaFileUtenteDopoBet(struct nodoUtenti* lista){
FILE*fp = fopen("Utenti.txt", "w");
if(!fp) {perror("Errore apertura aggiorna file utente dopo bet \n"); exit(-1);}
ScriviFile(lista, fp);
fclose(fp);
scriviLogSuFile("Aggiornamento file completato\n");


}

struct nodoUtenti* aggiornaDatiUtentiDopoBet(int numero, struct nodoUtenti* lista){
    printf("\n Lista prima aggiornamento ");
    StampaLista(lista);
    struct nodoUtenti* tmp = (struct nodoUtenti*)malloc(sizeof(struct nodoUtenti));
    tmp =lista;
    tmp->next=lista->next;

    int lung=LunghezzaLista(lista);
    printf("\n end\n");

    for(int i=0; i<lung; i++){  
    if(lista->numeroPuntato == numero){
        printf("\n Match, vittoria \n");
    lista->numeroPuntato=-1;
    lista->gettoni = lista->gettoni + (lista->gettoniPuntati)*30;
    lista->gettoniPuntati=0;
    if(lista->next) lista=lista->next;

    }else if(lista->numeroPuntato < 37) { //Sconfitta
    lista->numeroPuntato=-1;
    lista->gettoniPuntati=0;
    if(lista->next) lista=lista->next;
        }
        else{// Bet speciale
        printf("La bet non era su numeri da 1 a 36... /t");
        
        if(lista->numeroPuntato==37){//Rossi
        printf("La bet era su Numeri Rossi /t");
        if(isNumeroRosso(numero)){ //Vinto
        //Aggiornare i crediti
        lista->numeroPuntato=-1;
        //Vincita
        lista->gettoni = lista->gettoni + (lista->gettoniPuntati)*30;
        lista->gettoniPuntati=0;
        if(lista->next) lista=lista->next;
        } else { //Perso, non è uscito un rosso
            lista->numeroPuntato=-1;
            lista->gettoniPuntati=0;
            if(lista->next) lista=lista->next;
        }
        }//Fine check Rossi

        if(lista->numeroPuntato==38){ //Neri
        if(isNumeroNero(numero)){ //Vittoria
        lista->numeroPuntato=-1;
        lista->gettoni = lista->gettoni + (lista->gettoniPuntati)*30;
        lista->gettoniPuntati=0;
        if(lista->next) lista=lista->next;
        } else { //Perso, non è uscito un nero
            lista->numeroPuntato=-1;
            lista->gettoniPuntati=0;
            if(lista->next) lista=lista->next;
        }
        } //Fine check neri

        if(lista->numeroPuntato==39){
            if(numero%2 != 0){
            lista->numeroPuntato=-1;
            lista->gettoni = lista->gettoni + (lista->gettoniPuntati)*30;
            lista->gettoniPuntati=0;
            if(lista->next) lista=lista->next;

            } else {
                 lista->numeroPuntato=-1;
                lista->gettoniPuntati=0;
                if(lista->next) lista=lista->next;

            }
        } //Fine check dispari

        if(lista->numeroPuntato==40){
            if(numero%2 == 0){
            lista->numeroPuntato=-1;
            lista->gettoni = lista->gettoni + (lista->gettoniPuntati)*30;
            lista->gettoniPuntati=0;
            if(lista->next) lista=lista->next;
            } else {
                 lista->numeroPuntato=-1;
                lista->gettoniPuntati=0;
                if(lista->next) lista=lista->next;

            }
        } //Fine check pari

        if(lista->numeroPuntato==41){
            if(numero < 19){
            lista->numeroPuntato=-1;
            lista->gettoni = lista->gettoni + (lista->gettoniPuntati)*30;
            lista->gettoniPuntati=0;
            if(lista->next) lista=lista->next;
            } else {
                 lista->numeroPuntato=-1;
                lista->gettoniPuntati=0;
                if(lista->next) lista=lista->next;

            }
        } //Fine check bassi

           if(lista->numeroPuntato==42){
            if(numero > 18){
            lista->numeroPuntato=-1;
            lista->gettoni = lista->gettoni + (lista->gettoniPuntati)*30;
            lista->gettoniPuntati=0;
           if(lista->next) lista=lista->next;
            } else {
                 lista->numeroPuntato=-1;
                lista->gettoniPuntati=0;
                if(lista->next) lista=lista->next;

            }
        } //Fine check alti

         if(lista->numeroPuntato==43){
            if(numero < 13){
            lista->numeroPuntato=-1;
            lista->gettoni = lista->gettoni + (lista->gettoniPuntati)*30;
            lista->gettoniPuntati=0;
            if(lista->next) lista=lista->next;
            } else {
                 lista->numeroPuntato=-1;
                lista->gettoniPuntati=0;
                if(lista->next) lista=lista->next;

            }
        } //Fine check 1°colonna

         if(lista->numeroPuntato==44){
            if(12 < numero < 25){
            lista->numeroPuntato=-1;
            lista->gettoni = lista->gettoni + (lista->gettoniPuntati)*30;
            lista->gettoniPuntati=0;
            if(lista->next) lista=lista->next;
            } else {
                 lista->numeroPuntato=-1;
                lista->gettoniPuntati=0;
                if(lista->next) lista=lista->next;

            }
        } //Fine check 2°colonna

        if(lista->numeroPuntato==45){
            if(24 < numero < 37){
            lista->numeroPuntato=-1;
            lista->gettoni = lista->gettoni + (lista->gettoniPuntati)*30;
            lista->gettoniPuntati=0;
           if(lista->next) lista=lista->next;
            } else {
                 lista->numeroPuntato=-1;
                lista->gettoniPuntati=0;
                if(lista->next) lista=lista->next;

            }
        } //Fine check 3°colonna
        
        
        }//Fine bet speciali


        }//Fine scorrimento lista

    printf("\nLista dopo aggiornamento ");
    
    if(tmp)StampaLista(tmp);
    aggiornaFileUtenteDopoBet(tmp);  
    scriviLogSuFile("Aggiornamento lista completato\n");
    return tmp;
}

//Check numeri
bool isNumeroRosso(int input){
int rossi[18] = {1, 3, 5, 7, 9, 12, 14, 16, 18, 19, 21, 23, 25, 27, 30, 32, 34, 36};
for(int i = 0; i<18; i++){
if(rossi[i]==input){
printf("\n NUMERO ROSSO\n");
scriviLogSuFile("Il numero è rosso\n");
return true;
}
}
return false;
}

bool isNumeroNero(int numeroDaControllare){
    if( isNumeroRosso(numeroDaControllare) || numeroDaControllare==0 || numeroDaControllare>36 ) return false;
    printf("\n NUMERO NERO\n");
    scriviLogSuFile("Il numero è nero\n");
    return true;
}

int readLatestNumber(int numeroLetto){
    FILE*fp=fopen("UltimoNumeroEstratto.txt", "r");
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
     scriviLogSuFile("Il numero estratto è: ");
     char str[3];
    sprintf(str, "%d", randomNumber);
     scriviLogSuFile(str);
     scriviLogSuFile("\n");
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
scriviLogSuFile("Inserimento bet completato\n");

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
    struct nodoUtenti*tmp=(struct nodoUtenti*)malloc(sizeof(struct nodoUtenti));
    tmp = lista; //salvo la testa
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
   // lista=tmp;
        StampaLista(lista);
}

char* checkUtentiOnline(char* data, struct nodoUtenti* lista, FILE*fp){
    char listaOnline[MAX_SIZE];
    int len=LunghezzaLista(lista);
    memset(utentiOnline, 0, strlen(utentiOnline));
   
    for(int i=0; i<len; i++){
        if(lista->isOnline == 1){
        strcat (utentiOnline, lista->nickname);
        strcat(utentiOnline, ",,");
         int gettoni = lista->gettoni;
            char strg[20];
        sprintf(strg, "%d", gettoni);
        strcat(utentiOnline, strg);
        strcat(utentiOnline, ",,");
        }
        lista=lista->next;
    }
    printf("\n Dentro check, utenti online: %s \n", utentiOnline);
    scriviLogSuFile("Lista utenti online aggiornata\n");
    return utentiOnline; //o inviare il messaggio direttamente
}

void checkListaUtenti (char* data, struct nodoUtenti* lista, FILE*fp){

    fp=fopen("Utenti.txt", "r");
    if(!fp) {perror("ERRORE\n"); exit(0);}
    struct nodoUtenti* tmp=(struct nodoUtenti*)malloc(sizeof(struct nodoUtenti));
    tmp=LeggiFile(tmp, fp);
    fclose(fp);
    StampaLista(tmp);
    strcpy(listaUtentiExt, "");

    int len=LunghezzaLista(tmp);
    for(int i=0; i<len; i++){
        int lunNome = strlen(tmp->nickname);
        if(lunNome > 3){
        strcat (listaUtentiExt, tmp->nickname);
        strcat(listaUtentiExt, ";;");
         int gettoni = tmp->gettoni;
            char str[20];
        sprintf(str, "%d", gettoni);
        strcat(listaUtentiExt, str);
        strcat(listaUtentiExt, ";;");
        }
        tmp=tmp->next;
    }
    printf("\n Dentro check, utenti lista: %s \n", listaUtentiExt);
    scriviLogSuFile("Lista utenti aggiornata\n");
    //return listaUtenti; //o inviare il messaggio direttamente
}

struct nodoUtenti* registraUtente(char* data, struct nodoUtenti* lista, FILE*fp){
    //struct nodoUtenti*tmp=(struct nodoUtenti*)malloc(sizeof(struct nodoUtenti));
    //tmp = lista; //salvo la testa
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
lista = InserisciCoda(lista, part2, part3, GETTONI_INIZIALI, 1, -1, -1);
printf("Dopo inserimento in coda \n");
//lista = InserisciCoda(part2, part3, GETTONI_INIZIALI, true, -1, lista);
fp=fopen("Utenti.txt", "w");
 if(!fp) {perror("ERRORE\n"); exit(0);}
StampaListaToFileInOrdine(lista, fp);
StampaLista(lista);
fclose(fp);

scriviLogSuFile("Registrazione effettuata\n");
return lista;

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

if(accessoUtente_server(part2, part3, lista) ==1){
    scriviLogSuFile("Login effettuato\n");
return 1;
} 
else return 0;
scriviLogSuFile("Login fallito\n");


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
    scriviLogSuFile("Timer inizializzato\n");

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

