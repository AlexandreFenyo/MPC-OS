%start depart
%union {
	long i;
	char *s;
}
	
%token <i> NUM 
%token <s> ID IDT STRING
%token DO SET NETWORK NODE OF AT TO router processor generator WITH CONNECT END

%type <i> type
%type <s> free_ID

%%

depart:   declarations depart
	| NETWORK ID DO line_list END { Go=0; }
	| NETWORK DO line_list END { Go=0; }
	;

fab_liste_identificateurs : ID { id[nb_id++]=$1; }
                          | ID { id[nb_id++]=$1; }
                            ',' fab_liste_identificateurs
                          ;

declarations : optional_tableau NODE  fab_liste_identificateurs ':' 
		{
		 nb_interv=0;
		 nb_id=0;
			
		}
	| optional_tableau NODE fab_liste_identificateurs OF type ':' 
		{ int i,tempo;
		  char nom[100];
		  Noeud *n;	
                  for (i=0;i<nb_id;i++)
		  { sprintf(nom,"@%s",id[i]);
			/*printf("\n%s",id[i]);*/
			/* ajout d'une classe routeur/processeur */
		    if ($5==Routeur) tempo=ChercherDansConfig("\"router\"");
		    else tempo=ChercherDansConfig("\"generator\"");
		    if (tempo==Inconnu)
			Erreur("Type par defaut non trouvable.\n",-1);	
		    Desc=Ajouter(Desc,(n=Nouveau(nom,tempo,Configuration[tempo].nb_ports)));
		  }
		  nb_interv=0;
		  nb_id=0;
		}
	;


line_list: realstart line_list
	|
	;

realstart:  CONNECT { 	part=0;
			found=NULL;
			for_second=NULL;
		    }
	    free_ID {
			for_first=found;
			first_num=found_num;
			found=NULL;
		    }
	    TO free_ID 
		    {
			for_second=found;
			second_num=found_num;
		    }
	    optional_with
	{
	 	Noeud *first,* second;
		struct Champ *o;
		int i,first_pos,second_pos;
		if (for_first==NULL || for_second==NULL)
			Erreur("A port name was expected.\n",-1);
	  	first=Select($3);
	  	i=first->N.Type;
		o=FindIn(Configuration[i].Interface,for_first,&first_pos);
		if (first_num<o->First || first_num>o->Last)
			Erreur("Indice hors interval.\n",-1);
		first_pos+=first_num-o->First;

	  	second=Select($6);
	  	i=second->N.Type;
		o=FindIn(Configuration[i].Interface,for_second,&second_pos);
		if (second_num<o->First || second_num>o->Last)
			Erreur("Indice hors interval.\n",-1);
		second_pos+=second_num-o->First;
		/*printf(" %d %d\n",first_pos,second_pos);*/
	 	Ajouter_Connexion(first_pos,first,second_pos,second);
	}
	| SET { part=1; } free_ID '(' free_ID ':' '=' some_types ')'
	{ if (strcasecmp($5,"type")==0)
	  { if ( tempo == Inconnu )
	    { char s[40];
	      sprintf(s,"\nComponent %s has an unknown type.\n",$3);
	      Erreur(s,-1);
	    } else { Noeud *n;
		     /*printf(".%s.",$2);*/	
	  	     if ((n=Chercher(Desc,$3))==NULL)
			Desc=Ajouter(Desc,(n=Nouveau($3,tempo,Configuration[tempo].nb_ports)));
		     else n->N.Type=tempo;
		     if (Prime==NULL) Prime=n;
		   }
	  }
	  else {	int i;
			Noeud *n;
			sprintf(s,"\"%s\"",$3);
			i=ChercherDansConfig(s);
			if (keep_probes && i!=Inconnu)
			{
			  /*if (i==Inconnu) Erreur("????\n",-1);*/
			  NameOnly($5,s);	
			  if (FindIn(Configuration[i].Probes,s,&i)!=NULL)
				 Go=0;
			}
  	     	        n=Chercher(Desc,$3);
			NameOnly($5,s);
			if (keep_config)
			{ 	
	  	     	  if (n!=NULL)
			  {
			     if (FindIn(Configuration[n->N.Type].CompoCfg,s,&i)!=NULL)
				 Go=0;	
			  }
			}

	  	     	if (n!=NULL)
			{	
			    if (FindIn(Configuration[n->N.Type].ToUpdate,s,&i)!=NULL)
				 Go=0;
			}

		}

	}			
	;

free_ID: IDT 	{	
			strcpy(s,$1);
		}
	 tableau { 
			/*sprintf(s,"%s%s",$1,s);*/
			$$ = strdup(s);
		 }
	| ID { $$ = $1; }
	| generator { $$ = "generator"; }
	| processor { $$ = "generator"; }
	| router { $$ = "router"; }
	;

tableau: '[' NUM ']' { sprintf(s,"%s[%d]",s,$2); } tableau
	| '[' NUM ']' { sprintf(s,"%s[%d]",s,$2); }
	| '[' ID ']' '[' NUM ']'
	 {	
		/*sprintf(s,"%s[%s][%d]",s,$2,$5);*/
		found=$2;
		found_num=$5;
	 }
	| '[' ID ']'
	 {
		/*sprintf(s,"%s[%s]",s,$2);*/
		found=$2;
		found_num=0;
	 }
	
	;

optional_tableau: tableau
	| 
	;

optional_with: WITH ID
	|
	;
	
type: router { $$=Routeur; }
    | processor { $$ = Processeur; }
    | generator	{ $$ = Processeur; }
    ;

some_types: NUM
	| ID
	| STRING 
	  { tempo=ChercherDansConfig($1);
	  }
	;

%%
#include <string.h>

#include "autorouting.h"
#include "reseau.h"
#include "globals.h"
#include "misc.h"

int nb_interv=0,nb_id=0,nb_element=0,found_num,first_num,second_num;
char indic=0,tempo,part=0;
int interv[10];
char *id[10];
char s[50],*found,*for_first,*for_second;

Noeud *Select(char *name)
{ Noeud *first,*n;
  char name_o[100];	
  int i=Inconnu,j=0;
  NameOnly(name,name_o);
  if ((first=Chercher(Desc,name))==NULL
      || (first->N.Type==Inconnu)
     )
  {	
	n=Premier(Desc);
	while (n!=NULL)
	{
	  if (n->N.Nom[0]=='@'
	      && strcasecmp(&n->N.Nom[1],name_o)==0
	     ) { i=n->N.Type;
		 /*printf(" %s %d*",n->N.Nom,n->N.Type);*/
                 n=NULL;	
               } else n=Suivant(Desc);
	}
	if (i==Inconnu)
	    { char s[80];
	      sprintf(s,"\nComponent %s hasn't been set yet.\n",name);
	      Erreur(s,-1);	
	    }			
	if (first==NULL) Desc=Ajouter(Desc,(first=Nouveau(name,i,Configuration[i].nb_ports)));
	else first->N.Type=i;

	if (Prime==NULL && Configuration[i].types==Routeur) Prime=first;
	/*printf("%s %d\n",first->N.Nom,first->N.Type);*/
  } 
  return first;
}				

int ChercherDansConfig(char *name) 
{char s[42];
 int i,tempo;
 /*printf("/%s/ %d \n",name,nb_config);*/
 i=0;
 while (i<nb_config)
  { 
	/*printf(";%s\n",Configuration[i].nom);*/
	if (strcasecmp(name,Configuration[i].nom)==0) break;
	i++;
  }
/*printf("*%d*",i);*/
 if (i<nb_config) tempo=i;
 else tempo=Inconnu;

 return tempo;
}













