//Gestione utente -> liste
//Registrazione e accesso
//Lista utenti collegati
//Gettoni di partenza per ogni utente
//Utenti.txt: nickname | password | gettoni_residui | isOnline | numeroPuntato -> in ordine di gettoni?

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

struct nodoUtenti{
    char nickname[MAX_NAME];
    char password[MAX_PASSWORD];
    int gettoni;
    bool isOnline;
    int numeroPuntato;
    struct nodoUtenti* next;
};

struct nodoUtenti* CreaNodoUtenti (char* nickname, char*password, int gettoni, bool isOnline, int numeroPuntato)
{
    struct nodoUtenti* nuovoNodo=(struct nodoUtenti*)malloc(sizeof(struct nodoUtenti));
    if (!nuovoNodo) return NULL;
    strcpy(nuovoNodo->nickname, nickname);
    strcpy(nuovoNodo->password, password);
    nuovoNodo->gettoni=gettoni;
    nuovoNodo->isOnline=isOnline;
    nuovoNodo->numeroPuntato=numeroPuntato;
    nuovoNodo->next=NULL;

    return nuovoNodo;
}

struct nodoUtenti* InserisciCoda (struct nodoUtenti* lista, char* nickname, char*password, int gettoni, bool isOnline, int numeroPuntato)
{
    if (!lista) return CreaNodoUtenti (nickname, password, gettoni, isOnline, numeroPuntato);
    lista->next=InserisciCoda (lista->next, nickname, password, gettoni, isOnline, numeroPuntato);
    return lista;
}

struct nodoUtenti* LeggiFile (struct nodoUtenti* lista, FILE* fp)
{
    int gettoni; int numeroPuntato;
    char nickname[20], password[20];
    int isOnline;
    while(!feof(fp)){
        if(fscanf (fp, "%s %s %d %d %d", nickname, password, &gettoni, &isOnline, &numeroPuntato)==5){
              lista=InserisciCoda(lista, nickname, password, gettoni, isOnline, numeroPuntato);
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
        fprintf (fp, "%s %s %d %d %d", lista->nickname, lista->password, lista->gettoni, lista->isOnline, lista->numeroPuntato);
        lista=lista->next;
    }
}

void StampaLista(struct nodoUtenti* lista)
{
    if(!lista) return NULL;
    printf ("%s %s %d %d %d \n", lista->nickname, lista->password, lista->gettoni, lista->isOnline, lista->numeroPuntato);
    return StampaLista(lista->next);
}

bool contains (struct nodoUtenti* lista, int numero)
{
    if(!lista) return false;
    if(lista->gettoni == numero) return true;
    else return (contains(lista->next, numero));
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
    fprintf(fp, "%s %s %d %d %d\n", temp[i]->nickname, temp[i]->password, temp[i]->gettoni, temp[i]->isOnline, temp[i]->numeroPuntato);
  }
}



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
