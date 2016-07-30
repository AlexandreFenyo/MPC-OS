#include <stdio.h>
#include <string.h>
#include "config.h"
#include "globals.h"
#include "misc.h"
#include <malloc.h>

int FindChar(FILE *f,char y)
{ char t;
  do { if (fscanf(f,"%c",&t)<=0) return 1;
     } while (t==' ' || UpCase(y)!=UpCase(t));
  return 0;
}
 
char GoToChar(char *f,char t,int *ind)
{ int i=*ind-1;
  do { i++;
       if (f[i]=='\0') return 1;
     } while (UpCase(f[i])!=UpCase(t));
  *ind=i;
  return f[i];
}
int FindString(FILE *f,char *y)
{
  int indic=0;
  do { if (FindChar(f,y[indic]))
         { if (indic) return 1;
           else return 2;
         }
       indic++;
     } while (y[indic]!='\0');
  return 0;
}
 
ML *MLhead=NULL;
int nblib=0;
int ReadML()
{
  FILE *f;
  ML *t;
  char n[100];
  char temp[100];
  if ((f=fopen("config.ml","rt"))==NULL) return 1;
  do
    { 
      do
	{
	  if ((fscanf(f,"%s",n))<=0) 
	    {
	      fclose(f);
	      return 0;
	    }
	  if (n[0]=='%')
	    while (fgetc(f)!='\n');
	} while (n[0]=='%');
       if ((t=(ML *)malloc(sizeof(ML)))==NULL) return 1;
      sprintf(temp,"%s",n);
      t->name=strdup(temp);
       if ((fscanf(f,"%s",n))<=0) return 1;
       t->path=strdup(n);
      t->next=MLhead;
      MLhead=t;
      nblib++;
    } while(1);
}
int ReadChamp(char *path,char *file,CFG *ST)
{
  char s[100];
  char suite[100];
  int i,c=0,k,look_up=0,u,lg;
  struct Champ **C;
  struct Champ *j;
  FILE *f;
  sprintf(s,"%s/%s",path,file);
  /*printf("\r\t%s",s);*/
  if ((f=fopen(s,"rt"))==NULL) return 1;
  fscanf(f,"%s",s); /* component */
  fscanf(f,"%s",s); /* name */
  /*sprintf(suite,"\"%s\"",s);
  ST->nom=strdup(suite);*/
  fscanf(f,"%s",s); /* begin */  
  do
    {
      fscanf(f,"%s",s);
      if (strcasecmp(s,"interface")==0)
	{
	  C=&ST->Interface;
	  look_up=0;
	}
      else if (strcasecmp(s,"node")==0)
	{
	  C=&ST->NodeCfg;
	  look_up=1;
	  fscanf(f,"%s",s);
	}
      else if (strcasecmp(s,"component")==0)
	{
	  C=&ST->CompoCfg;
	  look_up=2;
	  fscanf(f,"%s",s);
	}
      else if (strcasecmp(s,"probes")==0)
	{
	  C=&ST->Probes;
	  look_up=3;
	}
      else if (strcasecmp(s,"end")==0)
	{
	  fclose(f);
	  return 0;
	}
      else return 1;
      do
	{
	  fscanf(f,"%s",s); /* sens /end */
	  if (strcasecmp(s,"end")==0) break;
	  if (look_up==0 || look_up==3) fscanf(f,"%s",s); /* position or type*/
	  i=0;
	  while (!feof(f) && (s[i]=fgetc(f))!=';') i++;
	  if (feof(f)) return 1;
	  s[i]='\0';
	  i=0;
	  lg=0;
	  do 
	    {
	      while (s[i]!='\0' && (s[i]==' ' || s[i]=='\n' || s[i]=='\t')) i++;
	      if (s[i]=='\0') break;
	      k=i;
	      while (s[i]!='\0' && s[i]!='[' && s[i]!='=' && s[i]!=';' && s[i]!=',') i++;
	      if (s[i]=='\0') break;
	      if (s[i]==',') lg=i+1;
	      else
		{
		  lg=i;
		  while (s[lg]!='\0' && s[lg]!=';' && s[lg]!=',') lg++;
		  if (s[lg]==',') lg++;
		  else lg=0;
		}

	      s[i]='\0';
	      /*printf("%s\n",&s[k]);*/
	      u=i+1;
	      j=*C;
	      while (j!=NULL && strcasecmp(&s[k],j->Nom)!=0) j=j->next;
	      if (j==NULL)
		{
		  if ((j=malloc(sizeof(struct Champ)))==NULL) return 1;
		  if (look_up==0) ST->nb_ports++;
		  j->First=j->Last=0;
		  j->Nom=strdup(&s[k]);
		  /*printf("%s\n",j->Nom);*/
		  j->next=*C;
		  *C=j;
		}
	      if (look_up==0)
		{ int temp;
		  i=atoi(&s[u]);
		  if (GoToChar(s,',',&u)==1)
		    {
		      i=0;
		      k=0;
		    }
		  else
		    { u++;
		      if (lg!=0)
			{
			  lg=u;
			  while (s[lg]!='\0' && s[lg]!=';' && s[lg]!=',') lg++;
			  if (s[lg]==',') lg++;
			  else lg=0;
			}
		      k=atoi(&s[u]);
		    }
		  ST->nb_ports-=(j->Last-j->First)+1;
		  if (j->First>i) j->First=i;
		  if (j->Last<k) j->Last=k;
		  ST->nb_ports+=(j->Last-j->First)+1;
		}
	      if (lg==0) break;
	      i=lg;
	    } while (1);

	} while (1);
      } while (1);
}
struct Champ *FindIn(struct Champ *name,char *what,int *pos)
{
  struct Champ *j;
  *pos=0;
  j=name;
  while (j!=NULL && strcasecmp(what,j->Nom)!=0)
    {
      *pos+=j->Last-j->First+1;
      /*printf(" %s ",j->Nom);*/
      j=j->next;
    }
  return j;
}
int ReadCFiG(char *path,char *file,Library *l,int from)
{
  char n[100];
  char suite[100];
  int i,c=0,k,look_up=0,u,lg,a;
  FILE *f;
  struct Champ *j;
  sprintf(n,"%s/%s",path,file);

  if ((f=fopen(n,"rt"))==NULL) return 1;
  do
    { 
      if ((a=FindString(f,"Component"))==2) break;
      else if (a) return 1;
      if (fscanf(f,"%s",n)<=0) return 1;
      i=from;

      while (i<l->nbconf && strcasecmp(l->Conf[i].nom,n)!=0)
	{

	  i++;
	}
      if (i>=l->nbconf)
	{
	  printf("Model %s in <types.cfg> does'nt exist in <Catalog>.\n",n);
	  return 1;
	}

      if (FindString(f,"is")) return 1;
      if (fscanf(f,"%s",n)<=0) return 1;
      if (strcasecmp(n,"ROUTER")==0) l->Conf[i].types=Routeur;
      else if (strcasecmp(n,"PROCESSOR")==0) l->Conf[i].types=Processeur;
      else return 1;
      if (FindString(f,"Updated")) return 1;
      if (FindString(f,"List")) return 1;
      u=0;
      while (!feof(f) && (n[u]=fgetc(f))!=';')
	{
	  if (n[u]!='\t' && n[u]!='\n' && n[u]!=' ')
	    u++;
	}
      if (feof(f)) return 1;
      n[u]=';';
      n[u+1]='\0';
      u=0;
      do 
	{
	  a=u;
	  while (n[a]!=',' && n[a]!='\0' && n[a]!=';') a++;
	  if (n[a]=='\0') break;
	  n[a]='\0';
	  if ((j=malloc(sizeof(struct Champ)))==NULL) return 1;
	  j->Nom=strdup(&n[u]);
	  j->next=l->Conf[i].ToUpdate;
	  l->Conf[i].ToUpdate=j;
	  u=a+1;
	} while (1);
      if (FindString(f,"End")) return 1;

    } while (1);

  fclose(f);
  return 0;
}
 
int LoadConfigurationFile(char *nom,char *libra)
{ 
  int i=0,a=0,b;
  FILE *f;
  ML *t;
  char n[100];
  char temp[100];
  Library *k;

  if ((k=(Library *)malloc(sizeof(Library)*nblib))==NULL)
    return 1;

  Configuration=NULL;
  i=0;
  t=MLhead;
      k->nbconf=0;
  while (t!=NULL)
    {
      k->nom=t->name;
      b=k->nbconf;
      /*printf("%s\n",t->name);*/
      sprintf(n,"%s/Catalog",t->path);
      printf("Library <%s> in %s\n",t->name,t->path);
      if ((f=fopen(n,"rt"))==NULL) return 1;
      printf("Models :\n");
      do
	{
	  if (fscanf(f,"%s",n)<=0) break;
	  if (n[0]!='%')
	    {
	      sprintf(temp,"../routing/%s_driver.o",n);
	      k->Conf[k->nbconf].name=strdup(temp);
	      k->Conf[k->nbconf].nom=strdup(n);
	      strcat(n,".mmd");
	      printf("\t- %s\n",n);
	      k->Conf[k->nbconf].used=0;
	      k->Conf[k->nbconf].nb_ports=0;
	      k->Conf[k->nbconf].Interface=NULL;
	      k->Conf[k->nbconf].NodeCfg=NULL;
	      k->Conf[k->nbconf].CompoCfg=NULL;
	      k->Conf[k->nbconf].Probes=NULL;
	      k->Conf[k->nbconf].ToUpdate=NULL;
	      if (ReadChamp(t->path,n,&k->Conf[k->nbconf])) return 1;
	      k->nbconf++;
	      
	    }
	} while (1);
      fclose(f);

      if (ReadCFiG(t->path,"types.cfg",k,b)) return 1;
      for (a=0;a<k->nbconf;a++)
	{
	  sprintf(temp,"\"%s\"",k->Conf[a].nom);
	  k->Conf[a].nom=strdup(temp);
	}
      i++;
      t=t->next;
    }
	  Configuration=k->Conf;
	  nb_config=k->nbconf;
  return 0;
}



















