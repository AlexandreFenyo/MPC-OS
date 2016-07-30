#include "generic.h"

int generator_main(FILE *f,char *a,int nb,int internal_number,Liste *liste)
{
  fprintf(f,"SET %s ( label := %d )\n",a,internal_number);
  fprintf(f,"SET %s ( range := %d )\n\n",a,NbProcessor);
  return 0;
}
