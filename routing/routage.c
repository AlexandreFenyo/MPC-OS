#include "reseau.h"
#include "routage.h"
#include "erreurs.h"
#include "globals.h"

int i,n,logical,lastgroup,lastlastgroup=0;
Noeud *x;
extern int unidir;
void Routage_Par_Intervalle(Noeud *reseau,int nb)
{ i=0;
  n=nb;
  logical=0;
  x=reseau;
  lastgroup=0;
  lastlastgroup=0;
  Label(reseau,reseau);
}

/*char s[100];*/
void Label(Noeud *u,Noeud *v)
{ int z,j,temp,nblinks,a,mynumber,smallerlink=-1;
  Noeud *w;
  v->numero=i;
  temp=i;
  v->logical_number=logical;
  v->last_group=lastgroup;
  logical++;
  z=v->N.Type;
  nblinks=0;
  if (z==Inconnu)
    {
      char s[100];
      sprintf(s,"\n\r\t\t->Instance %s's type is not set",u->N.Nom);
      Erreur(s,-1);
    }
  v->GotProc=0;
  for ( z=0 ; z<v->Nb_Succ ; z++ )
  {

    if (!(v->Succ[z]==NULL || v->lien[z]>=1000))
      {
	w=v->Succ[z];
	if (Configuration[w->N.Type].types==Processeur)
	  {
	    v->toproc[z]=1;
	    w->numero=i;
	    v->lien[z]=i;
	    /*printf("(%s %d)",w->N.Nom,i);*/
	    i=(i + 1) % n;
	    w->lien[0]=i;
	    v->GotProc=1;
	  }
      }
  }
  if (v->GotProc)
    {
      lastgroup=temp;
    }
/*  if (i==0 && !v->GotProc)
    printf("\tAie!   : router '%s' should not exist in the network (surely a dead lock)\n",v->N.Nom);
*/
  mynumber=i;

  for ( a=0 ; a<v->Nb_Succ ; a++ )
    {
      z=a;
      if ((!(v->Succ[z]==NULL || v->lien[z]>=1000)) && (Configuration[v->Succ[z]->N.Type].types==Routeur))
	{
	  w=v->Succ[z];
	  nblinks++;
/*	  printf("-> %s::%s %d\n",v->N.Nom,w->N.Nom,i);*/
	  if (w->numero==-1)
	    {
	      /*printf("%s %d %d %d\n",v->N.Nom,z,v->lien[z],i);*/
	      v->lien[z]=i;
	      mynumber=i;
	      Label(v,w);
	      if (i==mynumber && !v->GotProc)
		{
		  v->lien[z]=-1;unidir++;
		}
	      if (v->lien[z]!=-1 && v->lien[z]<smallerlink)
		smallerlink=v->lien[z];
	    }
	  else
	    {
	      if (w->GotProc==1)
		{
		  if (w!=u) 
		    { 
		      /*printf("%s %d %d %d)\n",v->N.Nom,z,v->lien[z],i);*/
		      v->lien[z]=w->numero;
		      if (w->numero==0) { v->mark=1;
					}
		    }
		}
	      else 
		{
		  if (w->flag & 1)
		    {v->lien[z]=-1;unidir++;}
		  else
		    {
		      v->lien[z]=w->numero;
		      if (v->numero==w->numero)
			{
			  if (v->logical_number>w->logical_number)
			    {
			      v->lien[z]=w->last_group;
			      /*printf("\n(%s %d-> %s %d)",v->N.Nom,v->lien[z],w->N.Nom,w->numero);*/
			    }
			  /*else
			    if (i==v->numero) v->lien[z]=-1;*/
			}
		    }
		}
	    }
	}
    }


  if (i==v->numero && (smallerlink==-1 || smallerlink==v->last_group))
    v->flag|=1;
  /* recherche de la position de u */
  if (u!=v)
    {
      z=0;temp=0;
      while (z<v->Nb_Succ && temp==0)
	{
	  if (v->lien[z]<1000 && v->Succ[z]==u) temp++;
	  else z++;
	}
      
      if (z>=v->Nb_Succ) Erreur("\n\r\t\tUne anomalie s'est produite dans la construction du reseau\n",-1);
    }

  if (v!=x)
    {
      if (i==0 && v->mark==1)
	{
	  /*printf("%s %d %d %d\n",v->N.Nom,z,v->lien[z],i);*/
	  v->lien[z]=u->numero;
	}
      else
	{
	  /*printf("%s %d %d %d\n",v->N.Nom,z,v->lien[z],i);*/
	  /*if (i==v->numero)
	    printf("hi hi hi!!!\n");*/
	  v->lien[z]=i;
	}
    }
  if (nblinks==1)
    {
      printf("\tWarning: possible dead lock for router '%s' receiving a message to processors labelled [%d..?]\n",v->N.Nom,i);
    }
  
}












