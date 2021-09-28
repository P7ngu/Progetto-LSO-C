//Gestione utente -> liste
//Registrazione e accesso
//Lista utenti collegati
//Gettoni di partenza per ogni utente
//Utenti.txt: nickname | password | gettoni_residui | isOnline | numeroPuntato | gettoniPuntati

//Gestione partita 
//Ultimo numero uscito in un file dedicato,
// puntata, classifica
//Fase di gioco e tempo per puntare
//File di log

//Notifica risultati: server -> app android
//int_operazione ### dati ###


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> 
#define MAX_NAME 10
#define MAX_PASSWORD 15
#define MAX_PARAM 100

struct nodoUtenti{
    char nickname[MAX_NAME];
    char password[MAX_PASSWORD];
    int gettoni;
    int isOnline;
    int numeroPuntato;
    int gettoniPuntati;
    struct nodoUtenti* next;
};

struct param_thread{
    int*sock;
    FILE*file;
    struct nodoUtenti* lista;
    char param0[MAX_PARAM];
    char param1[MAX_PARAM];
    char param2[MAX_PARAM];
    char param3[MAX_PARAM];
    char param4[MAX_PARAM];
};


struct param_thread* CreaParametriThread(int* sock, FILE* file, char* p0, char*p1, char*p2, char*p3, char*p4, struct nodoUtenti* lista){
    struct param_thread* nuovoParametro=(struct param_thread*)malloc(sizeof(struct param_thread));
    if(!nuovoParametro) return NULL;
    nuovoParametro->sock=sock;  //?
    nuovoParametro->file=file;
    strcpy(nuovoParametro->param0, p0);
    strcpy(nuovoParametro->param1, p1);
    strcpy(nuovoParametro->param2, p2);
    strcpy(nuovoParametro->param3, p3);
    strcpy(nuovoParametro->param4, p4);
    nuovoParametro->lista=lista;

    return nuovoParametro;
}   


struct nodoUtenti* CreaNodoUtenti (char* nickname, char*password, int gettoni, int isOnline, int numeroPuntato, int gettoniPuntati)
{
    struct nodoUtenti* nuovoNodo=(struct nodoUtenti*)malloc(sizeof(struct nodoUtenti));
    if (!nuovoNodo) return NULL;
    strcpy(nuovoNodo->nickname, nickname);
    strcpy(nuovoNodo->password, password);
    nuovoNodo->gettoni=gettoni;
    nuovoNodo->isOnline=isOnline;
    nuovoNodo->numeroPuntato=numeroPuntato;
    nuovoNodo->gettoniPuntati=gettoniPuntati;
    nuovoNodo->next=NULL;
     printf("%s %s %d %d %d - ", nickname, password, gettoni, isOnline, numeroPuntato);

    return nuovoNodo;
}

struct nodoUtenti* InserisciCoda (struct nodoUtenti* lista, char* nickname, char*password, int gettoni, int isOnline, int numeroPuntato, int gettoniPuntati)
{
    if (!lista) return CreaNodoUtenti (nickname, password, gettoni, isOnline, numeroPuntato, gettoniPuntati);
    lista->next=InserisciCoda (lista->next, nickname, password, gettoni, isOnline, numeroPuntato, gettoniPuntati);
    return lista;
}

struct nodoUtenti* LeggiFile (struct nodoUtenti* lista, FILE* fp)
{
    int gettoni; int numeroPuntato; int gettoniPuntati;
    char nickname[20], password[20];
    int isOnline;
    while(!feof(fp)){
        if(fscanf (fp, "%s %s %d %d %d %d", nickname, password, &gettoni, &isOnline, &numeroPuntato, &gettoniPuntati)==6){
              lista=InserisciCoda(lista, nickname, password, gettoni, isOnline, numeroPuntato, gettoniPuntati);
        }
      
    }
    return lista;
    
}

int LunghezzaLista(struct nodoUtenti* lista)
{
    if(!lista)return 0;
    return (1+LunghezzaLista(lista->next));
}

void ScriviFile (struct nodoUtenti* lista, FILE*fp)
{
    int n=LunghezzaLista(lista);
    for (int i=0; i<n; i++){
        fprintf (fp, "%s %s %d %d %d %d", lista->nickname, lista->password, lista->gettoni, lista->isOnline, lista->numeroPuntato, lista->gettoniPuntati);
        lista=lista->next;
    }
}

void StampaLista(struct nodoUtenti* lista)
{
    if(!lista) return NULL;
    printf ("n:%s p:%s g:%d o:%d n:%d gp:%d\n", lista->nickname, lista->password, lista->gettoni, lista->isOnline, lista->numeroPuntato, lista->gettoniPuntati);
    return StampaLista(lista->next);
}

bool contains (struct nodoUtenti* lista, char*data)
{
    if(!lista) return false;
    if(strcmp(lista->nickname, data)==0) return true;
    else return (contains(lista->next, data));
}

void StampaListaToFileInOrdine(struct nodoUtenti *lista, FILE *fp)
{
    struct nodoUtenti** temp, *copia;
  int n = LunghezzaLista(lista);
  temp=(struct nodoUtenti**)malloc(sizeof(struct nodoUtenti*));
  for(int i=0; i<n; i++, lista=lista->next){
      temp[i]=lista;
  }
  for(int i=0; i<n; i++){
      for(int j=0; j<n; j++){
          if(temp[i]->gettoni < temp[j]->gettoni){
              copia = temp[i];
              temp[i]=temp[j];
              temp[j]=copia;
          }
      }
  }
 for(int i = 0; i < n; i++){
    fprintf(fp, "%s %s %d %d %d %d \n", temp[i]->nickname, temp[i]->password, temp[i]->gettoni, temp[i]->isOnline, temp[i]->numeroPuntato, temp[i]->gettoniPuntati);
  }
}

struct nodoUtenti* resettaPuntatePrecedenti(struct nodoUtenti* lista){
    int lunghezzaLista=LunghezzaLista(lista);
    int n=LunghezzaLista;

    struct nodoUtenti** temp, *copia;
    temp=(struct nodoUtenti**)malloc(sizeof(struct nodoUtenti*));

     for(int i=0; i<LunghezzaLista; i++, lista=lista->next){
      if(lista) temp[i]=lista;
      else break;
      printf("\n 1 \n %s %s\n", temp[i]->nickname, lista->nickname);
    }


    for(int i=0; i<lunghezzaLista; i++){
        if(temp[i]!=NULL){
        temp[i]->numeroPuntato=-1;
        printf("\n2 %s %s\n", temp[i], lista);
        temp[i]->gettoniPuntati=0;
        printf(" 3 %s %s\n", temp[i], lista);
        temp[i]->isOnline=0;
        printf("4 %s %s \n", temp[i], lista);
    }
        
    }
    if(temp[0]) return temp[0];
}

struct nodoUtenti* logOutUser(struct nodoUtenti* lista, char*nomeDaSloggare){
    int lunghezzaLista=LunghezzaLista(lista);
    int n=LunghezzaLista;

    struct nodoUtenti** temp, *copia;
    temp=(struct nodoUtenti**)malloc(sizeof(struct nodoUtenti*));

     for(int i=0; i<LunghezzaLista; i++, lista=lista->next){
      if(lista) temp[i]=lista;
      else break;
      printf("\n LOGOUT, lista : \n %s %s\n", temp[i]->nickname, lista->nickname);
    }


    for(int i=0; i<lunghezzaLista; i++){
        if(temp[i]!=NULL){
            if(strcmp(temp[i]->nickname, nomeDaSloggare)==0){
               temp[i]->isOnline=0; 
            }
    }
        
    }
    if(temp[0]) return temp[0];
}

/*/
int main()
{
   FILE *fp;
    struct nodoUtenti* lista=NULL;
    fp=fopen("Utenti.txt", "r");
    if(!fp) {perror("ERRORE\n"); exit(0);}
    lista=LeggiFile(lista, fp);
    fclose(fp);
    
    printf("\n lista:\n");
    StampaLista(lista);
   if( contains(lista, 3))
   printf("okk\n");

   fp=fopen("Destinazione.txt", "w");
   if(!fp){perror("Errore!"); exit(0);}
   StampaListaToFileInOrdine(lista, fp);
      

}
/*/
