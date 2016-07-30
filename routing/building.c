#include <stdio.h>
#include <string.h>
#include "globals.h"
#include "building.h"


int FilesBuilding(char *command,char *mypath)
{ int i,nb=0,k=0;
  FILE *f;
  char tab0[500];
  char tab1[500];
  char temp[100];


  sprintf(command,"gcc -I../routing/include %smoteur.o BUILDED.c ",mypath);
  strcpy(tab0,"TT functions[] = {");
  
  strcpy(tab1,"char *names [] = {");
  
 if ((f=fopen("BUILDED.c","wt"))==NULL) return 1;
  
  fprintf(f,"#include <stdio.h>\n");

  fprintf(f,"struct LLs\n{ char Nom[50];\n  int Num;\n  int Valeur;\n  char ToProc;\n};\n");
  fprintf(f,"typedef struct LLs Liste;\n");
  fprintf(f,"typedef int (* TT)(FILE *, char *, int , int, Liste *);\n");
  
  for (i=0;i<nb_config;i++)
    {
      if (Configuration[i].used)
	{ 
	  strcpy(temp,&Configuration[i].nom[1]);
	  temp[strlen(Configuration[i].nom)-2]='\0'; 
	  strcat(tab1,"\"\\\"");
	  strcat(tab1,temp);	 
	  strcat(tab1,"\\\"\",");
	  strcat(tab0,temp);
	  strcat(tab0,"_main,");
	  nb++;
	  strcat(command,Configuration[i].name);
	  strcat(command," ");
	  fprintf(f,"extern int %s_main(FILE *, char *, int, int,Liste *);\n",temp);
	}
    }
 
  strcat(command,"-o RUN.TMP ");
  tab0[strlen(tab0)-1]='}';
  tab1[strlen(tab1)-1]='}';
  strcat(tab0,";\n");	
  strcat(tab1,";\n");	
  fprintf(f,"int number=%d;\n",nb);
  fprintf(f,"%s%s",tab0,tab1);
  fclose(f);
  return 0;
}








