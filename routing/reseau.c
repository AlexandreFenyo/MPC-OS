#include <malloc.h>
#include <string.h>
#include <math.h>

#include "reseau.h"
#include "erreurs.h"


Noeud *Nouveau(char *nom,char type,int nb_succ)
{ Noeud *n;
  int i,j;
  if ((n=(Noeud *)malloc(sizeof(Noeud)))!=NULL)
  { n->N.Nom=strdup(nom);
    n->N.Type=type;
    n->Nb_Succ=nb_succ;
    n->mark=0;
    n->numero=-1;
    n->lien=(int *)malloc(sizeof(int)*nb_succ);
    n->toproc=(int *)malloc(sizeof(int)*nb_succ);
    n->randorder=(int *)malloc(sizeof(int)*nb_succ);
    n->Succ=(struct SNoeud **)malloc(sizeof(struct SNoeud *)*nb_succ);
    n->flag=0;
    for (i=0;i<nb_succ;i++)
      { n->lien[i]=-1;
	n->toproc[i]=0;
	n->Succ[i]=NULL;
      }
  }
  return n;
}

void Ajouter_Connexion(int pos_a_qui,Noeud *a_qui,int pos_qui,Noeud *qui)
{ int i;


/*  printf("%d %s <-> %d %s (%p %p) \n",pos_qui,qui->N.Nom,pos_a_qui,a_qui->N.Nom,qui,a_qui);
*/
  for (i=0;i<qui->Nb_Succ;i++)
    if ((qui->Succ[i]==a_qui) && (qui->lien[i]<1000))
      {
	
	qui->lien[i]=1000+pos_qui;
/*	printf(" %d %d ",i,qui->lien[i]);*/
      }
    else
      {
/*	printf("*");*/
      }
/*  printf("\n");*/
  for (i=0;i<a_qui->Nb_Succ;i++)
      if ((a_qui->Succ[i]==qui) && (a_qui->lien[i]<1000)) 
	{ 
	  a_qui->lien[i]=1000+pos_a_qui;
/*	  printf(" %d %d ",i,a_qui->lien[i]);*/
	}
      else
	{
/*	  printf("*");*/
	}
/*  printf("\n");*/
  a_qui->Succ[pos_a_qui]=qui;
  qui->Succ[pos_qui]=a_qui;
}


static Liste *derniere_recherche=NULL;
static Liste *directory=NULL;

Liste *Ajouter(Liste *L,Noeud *p)
{ Liste *n;
  if ((n=(Liste *)malloc(sizeof(Liste)))!=NULL)
  { 
    n->M=p;
    n->suivant=L;
  }
  return n;
}

Noeud *Chercher(Liste *L,char *nom)
{ Liste *temp=L;
  if (derniere_recherche!=NULL && !strcmp(derniere_recherche->M->N.Nom,nom)) return derniere_recherche->M;
  while (temp!=NULL)
  { if (!strcmp(temp->M->N.Nom,nom)) break;
    temp=temp->suivant;
  }
  if (temp!=NULL) derniere_recherche=temp;
  else return NULL;
 
  return temp->M;
}

Noeud *Premier(Liste *L)
{ directory=L;
  if (L==NULL) return NULL;
  return L->M;
}

Noeud *Suivant()
{ if (directory!=NULL) directory=directory->suivant;
  if (directory==NULL) return NULL;
  return directory->M;
}

