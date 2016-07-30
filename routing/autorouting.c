#include <stdio.h>
#include <malloc.h>
#include "erreurs.h"
#include "globals.h"
#include "config.h"
#include "reseau.h"
#include "building.h"

int ligne;
int unidir=0;
yyerror(char *s)
{
/*  char s0[80];
  sprintf(s0,"\n%s at or near line %d.\n",s,ligne);
  Erreur(s0,-1);*/
}

void display(char *s)
{
        if (verbose) printf("%s",s);
}

extern FILE *yyin;
extern char **environ;

main(int argc,char *argv[])
{
  FILE *f,*fichier_in,*temp_fichier;
  Noeud *temp,*go,*golast;
  int i,k,l,m,lockcount,nbp=0,nbr=0,start=1,j,keep_temp=0,test_in=0;
  char command[200],tempo[100];
  struct Champ *tre;
  verbose=1;
  ligne=1;
  fprintf(stderr,"Interval Network Autolabelling??\n");
  if (argc<2 || argc>7)
    { 
      Erreur("Usage ir [-c] [-p] [-k] [-t] <input file name> <output file name>\n"
	     "\t -c : do not keep configuration\n"
	     "\t -p : do not keep probes\n"
	     "\t -t : check for loops in the network\n"
	     "\t -k : do not delete temporary file\n"
	     ,-1
	     );
    }
  strcpy(command,"rcube");
  do
    { if (argv[start][0]=='-')
	{
	  if (argv[start][1]=='c')
	    keep_config=1;
	  else if (argv[start][1]=='p')
	    keep_probes=1;
	  else if (argv[start][1]=='k')
	    keep_temp=1;
	  else if (argv[start][1]=='t') 
	    {
	      test_in=1;
	    }
	}
      else
	break;
      start++;
    } while (start<argc);

  if (start+2>argc)
    { 
      Erreur("\tCommand line is not complete.\n",-1);
    }
  i=0;

  /*
  while(environ[i]!=NULL)
    {
      if(strncmp(environ[i],"ARCHI",5)==0)
	archi=&environ[i][6];
      i++;
    }
  if (archi==NULL)
    Erreur("\tTarget architecture variable ARCHI not set! :)\n",-1);
  printf("Working on %s\n",archi);
  */

  if ((fichier_in=fopen(argv[start],"rt"))==NULL)
    {
      Erreur("\t->Input file not found.\n",-1);
    }
  
  yyin = fichier_in;

  if ((sor=fopen(argv[start+1],"wt"))==NULL)
    {
      Erreur("\t->Output file can not be created.\n",-1);
    }

  display("\rLoading 'config.ml'\n");
  if (ReadML())
    { 
      Erreur(" Error\n",-1);
    }
  display("\rLoading libraries configuration file 'Catalog'\n");
  if (LoadConfigurationFile("types.cfg",command))
    { 
      Erreur(" Error\n",-1);
    }

  if (Configuration==NULL)
    {
      printf("Library '%s' isn't present in 'config.ml'\n",command);
      Erreur("",-1);
    }
  if ((dummy=malloc(10000))==NULL) return 1;

  strcpy(dummy,"");
  
  display("\n\r\t-> Parsing input file and creating net.");


  Prime=NULL; /* initializing the net */
  Desc=NULL; /* initializing the node and processor list */


  yyparse();
  display("\n");

  if (Prime==NULL) { display("\n\r\tHey! There's no net in this file.");
		     exit (1);
		   }
  
  temp=Premier(Desc);
  while (temp!=NULL)
    {
      if (temp->N.Type==Inconnu)
	{ char s[100];
	  sprintf(s,"\n\r\t\t->Instance %s's type is not set");
	  Erreur(s,-1);
	  exit(1);
	}
      if (temp->N.Nom[0]!='@')
	{
	  if (Configuration[temp->N.Type].types==Processeur)
	    nbp++;
	  else
	    nbr++;
	  
	  if (Configuration[temp->N.Type].used==0)
	    Configuration[temp->N.Type].used++;
	}
      
      temp=Suivant(Desc);
    }
/*  nbp+=nbr;*/
  display("\r\t-> Creating temporary net file");
  if ((f=fopen("NET.TMP","wt"))==NULL) Erreur(" : Failed\n",-1);
  
  display("\n");
  
  fprintf(f,"%3d %3d\n",nbp,nbr);

  Routage_Par_Intervalle(Prime,nbp);
  temp=Premier(Desc);
  while (temp!=NULL)
    {
      if (temp->N.Nom[0]!='@')
	{
	  if (temp->numero==-1)
	    {
	      Erreur("Heu! All instances in this graph aren't connected\n",-1);
	    }
	  fprintf(f,"%s %s\t%d\t%d \n",Configuration[temp->N.Type].nom,temp->N.Nom,temp->Nb_Succ,temp->numero);
	  tre=Configuration[temp->N.Type].Interface;
	  j=tre->First;
	  for (i=0;i<temp->Nb_Succ;i++)
	    { if (tre->First!=tre->Last) fprintf(f,"\t%s %d ",tre->Nom,j);
	      else fprintf(f,"\t%s %d",tre->Nom,-1);
	      fprintf(f,"%5d\t %c\n",temp->lien[i],temp->toproc[i]==0?'-':'*');
	      j++;
	      if (j>tre->Last)
		{
		  if ((tre=tre->next)==NULL && i<temp->Nb_Succ-1)
		    Erreur("Heuuuu !!! this error should never happened\n",-1);
		  if (i<temp->Nb_Succ-1) 
		    j=tre->First;
		}
	    }
	  fprintf(f,"\n");
	}
      temp=Suivant(Desc);
    }
  fclose(f);
  fclose(fichier_in);
  fprintf(sor,dummy);
  fclose(sor);
  if (unidir>0)
    printf("\t\t(%d links are unidirectional to avoid loops)\n",unidir);

if (test_in)
  {
    m=10;lockcount=0;
    temp=Premier(Desc);
    while (temp!=NULL)
      {
	if (temp->N.Nom[0]!='@' && Configuration[temp->N.Type].types==Processeur)
	  { 
	    int label[16],list[16][8],nb;
	    for (i=0;i<nbp;i++)
	      {
		m++;
		go=temp;
		printf("\r\t-> Checking %03d -> %03d\t",temp->numero,i);
		do 
		  { int nbl[8],p;
		    go->mark=m;
		    /*printf(" %s ",go->N.Nom);*/
		    nb=0;
		    for (j=0;j<8;j++)
		      nbl[j]=0;
		    for (j=0;j<go->Nb_Succ;j++)
		      {
			if (go->lien[j]<1000 && go->lien[j]!=-1)
			  {
			    for (k=0;k<nb;k++)
			      if (label[k]>=go->lien[j]) break;
			    if (k<nb && label[k]==go->lien[j])
			      {
				list[k][nbl[k]]=j;nbl[k]++;
			      }
			    else
			      if (k==nb || label[k]!=go->lien[j])
				{
				  for (l=nb;l>k;l--)
				    {
				      label[l]=label[l-1];
				      for (p=0;p<nbl[l-1];p++)
					list[l][p]=list[l-1][p];
				      nbl[l]=nbl[l-1];
				    }
				  nbl[k]=1;
				  list[k][0]=j;
				  label[k]=go->lien[j];
				  nb++;
				}
			  }
		      }
		    /*		  for (j=0;j<nb;j++)
				  printf("[%d;%d]",label[j],list[j]);
				  printf(" | ");
				  */
		    for (j=0;j<nb-1;j++)
		      if (i>=label[j] && i<label[j+1]) break;
		    
		    if (j==nb-1 && (!(i>=label[j] || i<label[0])))
		      j=-1;
		    /*
		       printf("H %d H\n",j);
		       */
		    if (j==-1)
		      {
			fprintf(stderr,"!!!!Erreur!!!!\n");
			exit(1);
		      }
		    /*
		       printf("\r\t%d -> %d (%s %s)",temp->numero,i,go->N.Nom,go->Succ[list[j]]->N.Nom);
		       */
		    golast=go;
		    /*printf(",%d,\n",nbl[j]);*/
		    go=go->Succ[list[j][lrand48() % nbl[j]]];
		    if (go->mark==m && Configuration[go->N.Type].types==Routeur)
		      {
			printf("\r\t-> Dead lock on way %03d -> %03d between '%s' and '%s'\t\n",temp->numero,i,golast->N.Nom,go->N.Nom);
			lockcount++;
			break;
		      }
		    if (Configuration[go->N.Type].types==Processeur && go->numero==i)
		      break;
		  } while (1);
	      }
	  }
	temp=Suivant(Desc);
      }
    if (lockcount==0) printf("\r\t-> No dead lock found.\n");
    else printf("\r\t-> %d ( %d %%) dead locks found.\n",lockcount,(int)((lockcount*100)/(m-10)));
  }  

  
  /* where am i? */

  i=strlen(argv[0]);
  while (i>=0 && argv[0][i]!='/') i--;
  argv[0][i+1]='\0';
  display("\r\t-> Preparing compilation\n");
  if (FilesBuilding(command,argv[0])) Erreur("\t Failed\n",-1);
 
printf("COMMAND = %s\n", command);  

  if (system(command))
    Erreur("\t Failed\n",-1);

  display("\r\t-> Compiling structure filling process\n");
  if (system(command))
    Erreur("Failed\n",-1);

  display("\r\t-> Running\n");
  sprintf(command,"./RUN.TMP NET.TMP %s",argv[start+1]);
  if (system(command))
    Erreur("\t Failed\n",-1);
  else 
    {
      if (!keep_temp)
	system("rm -f NET.TMP");
    }
  system("rm -f BUILDED.c");
  system("rm -f RUN.TMP");
  display("\t Done. \n");

printf("%s",argv[0]);
  return 0;
}









