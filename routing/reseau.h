#ifndef __RESEAU
#define __RESEAU

#define Inconnu      -1
#define Processeur   0
#define Routeur      1

#define Nb_Lien_Max  80

struct Noeu
        { char Type;
          char *Nom;
        } Noeu;

struct SNoeud
        { struct SNoeud **Succ;
          int *lien;
	  int *toproc;
	  int *randorder;
          int Nb_Succ;
          struct Noeu N;
          int numero;
          int mark;
	  int GotProc;
	  int logical_number;
	  int last_group;
	  int flag;
        } ;

typedef struct SNoeud Noeud;

struct SListe
        { Noeud *M;
          struct SListe *suivant;
        } ;

typedef struct SListe Liste;


extern Noeud *Nouveau(char *nom,char type,int);
extern void Ajouter_Connexion(int,Noeud *a_qui,int,Noeud *qui);
extern Liste *Ajouter(Liste *L,Noeud *p);
extern Noeud *Chercher(Liste *L,char *nom);
extern Noeud *Premier(Liste *L);
extern Noeud *Suivant();
#endif
