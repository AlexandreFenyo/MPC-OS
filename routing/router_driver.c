#include "generic.h"

int router_main(FILE *f,char *a,int nb,int internal_number,Liste *liste)
{ 
  int i,j=0,last=0,temp,qui,pass;
  int interv[8],who[8];
  for (j=0;j<8;j++)
    {
      if (liste[j].Valeur>=0 && liste[j].Valeur<1000)
	{
	  i=0;
	  while (i<last && interv[i]<liste[j].Valeur) i++;
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

 while (last<8)
    {
      interv[last]=interv[last-1];
      who[last]=who[last-1];
      last++;
    }

  for (i=0;i<8;i++)
    {
      for (temp=0;temp<8;temp++)
	fprintf(f,"SET %s ( intervals[%d][%d] := %d )\n",a,i,temp,interv[temp]);
      for (temp=0;temp<8;temp++)
	{
	  last=0;
	  for (j=0;j<8;j++)
	    {
	      if (liste[j].Valeur>=1000) qui=liste[j].Valeur-1000;
	      else qui=j;
	      if (interv[temp]==liste[qui].Valeur)
		fprintf(f,"SET %s ( lists[%d][%d][%d] := %d )\n",a,i,temp,last++,j);
	    }
	      
	  fprintf(f,"SET %s ( lists[%d][%d][%d] := 255 )\n",a,i,temp,last);
	}
    }
  fprintf(f,"\n");
  return 0;
}

