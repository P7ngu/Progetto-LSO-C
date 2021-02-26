
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> 

//libreria liste

struct nodo{
    int valore;
    struct nodo* next;
};

struct nodo* CreaNodo (int valore)
{
    struct nodo* nuovoNodo=(struct nodo*)malloc(sizeof(struct nodo));
    if (!nuovoNodo) return NULL;
    nuovoNodo->valore=valore;
    nuovoNodo->next=NULL;
}

struct nodo* InserisciCoda (struct nodo* lista, int valore)
{
    if (!lista) return CreaNodo (valore);
    lista->next=InserisciCoda (lista->next, valore);
    return lista;
}

struct nodo* LeggiFile (struct nodo* lista, FILE* fp)
{
    int valore;
    while(!feof(fp)){
        if(fscanf (fp, "%d", &valore)==1)
        lista=InserisciCoda(lista, valore);
    }
    return lista;
    
}

struct nodo* EliminaNodi (struct nodo* lista)
{
    if(!lista) return NULL;
    if(lista->valore<10) return EliminaNodi(lista->next);
    lista->next=EliminaNodi(lista->next);
    return lista;
}

struct nodo* DuplicaNodo (struct nodo* lista)
{
    struct nodo* Nuovo;
    Nuovo=CreaNodo(lista->valore);
    return Nuovo;
}

struct nodo* DuplicaElementi (struct nodo* lista)
{
    struct nodo* nodo;
    if(lista==NULL) return NULL;
    lista->next=DuplicaElementi(lista->next);
    if(lista->valore<1500){
        nodo=DuplicaNodo(lista);
        nodo->next=lista->next;
        lista->next=nodo;
    }
    return lista;
}

int LunghezzaLista(struct nodo* lista)
{
    if(!lista)return 0;
    return (1+LunghezzaLista(lista->next));
}

void ScriviFile (struct nodo* lista, FILE*fp)
{
    int n=LunghezzaLista(lista);
    for (int i=0; i<n; i++){
        fprintf (fp, "%d", lista->valore);
        lista=lista->next;
    }
}

void StampaLista(struct nodo* lista)
{
    if(!lista) return NULL;
    printf ("%d \n", lista->valore);
    return StampaLista(lista->next);
}

bool contains (struct nodo* lista, int numero)
{
    if(!lista) return false;
    if(lista->valore == numero) return true;
    else return (contains(lista->next, numero));
}

void StampaListaToFile(struct nodo *lista, FILE *fp)
{
    struct nodo** temp, *copia;
  int n = LunghezzaLista(lista);
  temp=(struct nodo**)malloc(sizeof(struct nodo*));
  for(int i=0; i<n; i++, lista=lista->next){
      temp[i]=lista;
  }
  for(int i=0; i<n; i++){
      for(int j=0; j<n; j++){
          if(temp[i]->valore < temp[j]->valore){
              copia = temp[i];
              temp[i]=temp[j];
              temp[j]=copia;
          }
      }
  }
 for(int i = 0; i < n; i++){
    fprintf(fp, "%s %d %d %d %d %d\n", temp[i]->valore);
  }
}

int main()
{
   FILE *fp;
    struct nodo* lista=NULL;
    fp=fopen("dati.txt", "r");
    if(!fp) {perror("ERRORE\n"); exit(0);}
    lista=LeggiFile(lista, fp);
    fclose(fp);
    
    printf("\n lista:\n");
    StampaLista(lista);
   if( contains(lista, 3))
   printf("okk\n");
      

}