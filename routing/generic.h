#include <stdio.h>

extern int NbProcessor;
extern int NbRouter;

struct LLs
{ char Nom[50];
  int Num;
  int Valeur;
  char ToProc;
};


typedef struct LLs Liste;



