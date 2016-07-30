#include "generic.h"

int RCUBE_main(FILE *f,char *a,int nb,Liste *liste)
{ 
  int i,j=0,last=0,temp;
  int interv[nb],who[nb];
  for (j=0;j<nb;j++)
    {
      if ((liste[j].Valeur==-1) || (liste[j].Valeur>=1000)) continue;
      else
	{
	  i=0;
	  while ((i<last) && (interv[i]<liste[j].Valeur)) i++;
	  if ((i==last) || (interv[i]!=liste[j].Valeur))
	    {
	      for (temp=last;temp>i;temp--)
		{
		  interv[temp]=interv[temp-1];
		  who[temp]=who[temp-1];
		}
	      interv[i]=liste[j].Valeur;
	      who[i]=j;
	      last++;
	    }
	}
    }

 while (last<nb)
    {
      interv[last]=interv[last-1];
      who[last]=who[last-1];
      last++;
    }
/*
  j=0;
  while (interv[j]==-1) j++;
  
  for (i=0;i<nb;i++)
    {
      last=j;
      j=(j + 1) % nb;
      if (interv[j]==-1)
	interv[j]=interv[last];
    }
*/
  for (i=0;i<nb;i++)
    {
      if (liste[i].Valeur<1000)
	{
	 /* if (liste[i].Valeur==-1)
	    {
	      fprintf(f,"SET %s ( used_links[%d] := FALSE )\n",a,i);
	    }	  
	  else */
	    {
	      /*fprintf(f,"SET %s ( used_links[%d] := TRUE )\n",a,i);*/
	      for (temp=0;temp<nb;temp++)
		{
		  fprintf(f,"SET %s ( intervals[%d][%d] := %d )\n",a,i,temp,interv[temp]);
		}

	      for (temp=0;temp<nb;temp++)
		{
		  last=0;
		  for (j=0;j<nb;j++)
		    if (liste[j].Valeur==liste[who[temp]].Valeur || (liste[j].Valeur>=1000 && liste[liste[j].Valeur-1000].Valeur==liste[who[temp]].Valeur))
		      fprintf(f,"SET %s ( lists[%d][%d][%d] := %d )\n",a,i,temp,last++,j);
		  fprintf(f,"SET %s ( lists[%d][%d][%d] := 255 )\n",a,i,temp,last);
		}
	    }
	}
    }
  fprintf(f,"\n");
  return 0;
}
