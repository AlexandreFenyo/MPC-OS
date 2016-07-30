#include <string.h>

void NameOnly(char *s,char *d)
{
 int i=0;
 while (i<strlen(s) && s[i]!='[')
   { d[i]=s[i];
     i++;
   }
 d[i]='\0';	
}

char UpCase(char e)
{
  if (e>='a' && e<='z') return 'A'+e-'a';
  return e;
}


