#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "erreurs.h"

void Erreur(char *st,int num)
{ char s[200];
  if (num>0) sprintf(s,st,num);
  else strcpy(s,st);
  fprintf(stderr,s);
  exit(1);
}