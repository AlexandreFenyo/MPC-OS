#include <stdio.h>
#include <string.h>
#include "erreurs.h"
#include "generic.h"

typedef int (* TT)(FILE *, char *, int, int,Liste *);

extern int number;
extern TT functions[];
extern char *names[];

int NbRouter,NbProcessor;

int CALL(FILE *f,char *famille,char *nom,int nb,int interne,Liste *l)
{
  int i;
  for (i=0;i<number;i++)
    {
      if (strcasecmp(famille,names[i])==0) return functions[i](f,nom,nb,interne,l);
    }
  return 1;
}

main(int argc,char *argv[])
{ 
  int i=0,nb,internal_number;
  FILE *f,*g;
  char n[1000],nom[100],famille[100];
  Liste *l;
  if (argc<2 || argc>3) 
    {
      printf("Dynamic router/processor data format encoding\n"
		      "Usage moteur <interval file> [<output filew name>]\n"
		      "\tdefault : output is standard output\n"
		      );
      return 1;
    }
  if ((f=fopen(argv[1],"rt"))==NULL)
    {
      printf("Failed to open interval file\n");
      return 1;
    }
  if (argc==2)
    {
      g=stdout;
    }
  else 
    {
      if ((g=fopen(argv[2],"at"))==NULL) 
	{
	  printf("Failed to open output file\n");
	  return 1;
	}
    }
  
  if (fscanf(f,"%s",n)<=0) return 1;
  NbProcessor=atoi(n);
  if (fscanf(f,"%s",n)<=0) return 1;
  NbRouter=atoi(n);
  
  while (!feof(f))
    { 
      if (fscanf(f,"%s",famille)<=0) break;
      if (fscanf(f,"%s",nom)<=0) return 1;
      if (fscanf(f,"%s",n)<=0) return 1;
      nb=atoi(n);
      if (fscanf(f,"%s",n)<=0) return 1;
      internal_number=atoi(n);
      l=(Liste *)malloc(sizeof(Liste)*nb);
      for (i=0;i<nb;i++)
	{
	  if (fscanf(f,"%s",&l[i].Nom)<=0) return 1;
	  if (fscanf(f,"%d",&l[i].Num)<=0) return 1;
          if (fscanf(f,"%d",&l[i].Valeur)<=0) return 1;

	  if (fscanf(f,"%s",n)<=0) return 1;
	  if (n[0]=='-') l[i].ToProc=0;
	  else if (n[0]=='*') l[i].ToProc=1;
	  else return 1;
	}
      if (CALL(g,famille,nom,nb,internal_number,l)) return 1;
      free (l);
    }
  fclose(f);
  fprintf(g,"\n:\n");
  if (g!=stdout) fclose(g);
  return 0;
}




