
/* $Id: testsendv.c,v 1.1.1.1 1998/10/28 21:07:33 alex Exp $ */

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/mman.h>

#include "../MPC_OS/mpcshare.h"

#include "../modules/driver.h"
#include "../modules/put.h"
#include "../modules/ddslrpp.h"
#include "../modules/pvmdriver.h"

#define TIMEC 1

#ifdef TIMEC
   
   struct timeval tc1,tc2;
   unsigned long tcr;
   unsigned long tc;
 
#define topc1() gettimeofday(&tc1,NULL)
#define topc2() { gettimeofday(&tc2,NULL); tc = 1000000L*tc2.tv_sec + tc2.tv_usec - (1000000L*tc1.tv_sec + tc1.tv_usec) - tcr + tc; }
 
   void init_timec(void)
   {
      tcr = 0L;
      tc = 0L;
      topc1();
      topc2();
      tcr = tc;
   }
 
#endif

#define NBCOM 10000
#define NB 17
#define KO 1024

int put_getnode();

#if 0
static int devpvm = -1;
#endif
static int devhsl = -1;

int
main(ac, av)
     int ac;
     char **av;
{
  volatile char *chaine;
  int i, j, res, compteur, mynode;
  int taille = 1;
  double tps[NB];
  latenceslr_t rs;

  /****** INITIALISATIONS *********/
#if 0
  devpvm = open("/dev/pvmhsl", O_RDWR);
#endif
  devhsl = open("/dev/hsl", O_RDWR);
#if 0
  if (devpvm < 0)
    {
      perror("open PVM");
      exit(1);
    }
#endif
  if (devhsl < 0)
    {
      perror("open HSL");
      exit(1);
    }

  mynode = put_getnode();

  printf("Mynode = %d NBCOM = %d\n",mynode,NBCOM);


  chaine = (volatile char *) malloc(1<<(NB-1));
  if (!chaine) 
    {
      perror("malloc");
      exit(1);
    }
  res = mlock(chaine, 1<<(NB-1));
  if (res < 0) 
    {
      perror("mlock");
      exit(1);
    }

  if (mynode == 0)
    {
      rs.dest = 1;
      rs.data = (caddr_t) chaine;
#ifdef TIMEC
      init_timec();
      tc = 0;
#endif
    }
  else
    {
      rs.dest = 0;
      rs.data = (caddr_t) chaine;
    }
  /*********************************/
  
  for (compteur=0; compteur<NB; compteur++)
    {         
      taille = 1 << compteur;
      rs.size = (size_t) taille;

#ifdef TIMEC
      if (mynode==0)
	{
	  tc = 0;
	  tps[compteur] = 0;
	  printf("Emission taille %d\n",taille);
	}
#endif

      for (j=0; j<NBCOM; j++)
	{

	  if (mynode == 0)
	    {
	      /*    printf("Emission taille %d\n",taille);*/
	  
	      for (i = 0; i < taille; i++) chaine[i] = 'A';
#ifdef TIMEC
      tc = 0;
      topc1();
#endif	      

	      
	      /* EMISSION NODE 0 */
	      if (ioctl(devhsl, HSLTESTSLRVLAT, &rs)) 
		{
		  perror("ioctl HSLTESTSLRVLAT");
		  exit(1);
		}

	      /*      for (i=0; i<10000000; i++);*/
	      
	      /* RECEPTION NODE 0 */
	      /*	      printf("Reception taille %d\n",taille);*/
	      
	      for (i = 0; i < taille; i++) chaine[i] = 255;
	      
	      
	      if (ioctl(devhsl, HSLTESTSLRV2LAT, &rs)) 
		{
		  perror("ioctl HSLTESTSLRV2LAT");
		  exit(1);
		}
	      /* ne retourne que lorsque les donnees sont effectivement recues */
#ifdef TIMEC
      topc2();
  /*    printf("%ld ",tc/2);*/
      tps[compteur] += (double) tc; 
#endif	      
	      /*for (i=0; i<1000000000; i++);*/
	      
	      for (i = 0; i < taille; i++) 
		{
		  if (chaine[i] != 'A')
		    {
		      printf("*[%d,%d]*i=%d*%d* ", taille, j, i, (unsigned char) chaine[i]);
		    }
		}
	    }
	  else
	    {
	      /* RECEPTION NODE 3 */
	      if (j==0) 
		printf("Reception taille %d\n",taille);
	      
	      for (i = 0; i < taille; i++) chaine[i] = 255;
	      
	      
	      if (ioctl(devhsl, HSLTESTSLRV2LAT, &rs)) 
		{
		  perror("ioctl HSLTESTSLRV2LAT");
		  exit(1);
		}
	      /* ne retourne que lorsque les donnees sont effectivement recues */
	      
	      /*	  for (i=0; i<1000000000; i++);*/
	      
	      /* On regarde si les donnees sont correctes */
	      /*	  for (i = 0; i < taille; i++) 
			  {
			  if (chaine[i] != ('A'+compteur))
			  printf("*taille=%d i=%d *%d*", taille, i, (unsigned char) chaine[i]);
			  }*/

	      /* EMISSION DES DONNES FRAICHEMENT RECUES */
	      if (j==NBCOM-1)
		printf("Emission taille %d\n",taille);
	      
	      
	      if (ioctl(devhsl, HSLTESTSLRVLAT, &rs)) 
		{
		  perror("ioctl HSLTESTSLRVLAT");
		  exit(1);
		}
	      /* ne retourne que lorsque les donnees sont effectivement parties */
	    }                  
	}
#ifdef TIMEC
      if (mynode == 0)
	tps[compteur] /= (double) (2.0*NBCOM);
#endif
    }
#ifdef TIMEC
  if (mynode == 0)
    {
      printf("\n");
      for (i=0; i<NB; i++)
	{
	  /*printf("%8.0lf %5d\n",tps[i],1<<i);*/
	  printf("%8.0f %5d\n",tps[i],1<<i);
	}
   }
#endif
  

  
  res = munlock(chaine, 1<<(NB-1));
  if (res < 0) 
    {
      perror("munlock");
      exit(1);
    }  
  free(chaine);  
  
  close(devhsl);
#if 0
  close(devpvm); 
#endif
  return 0;
}

/* Get the number of local node */
int
put_getnode() 
{
  int res=0;
  int n;

#if 0
  res = ioctl(devpvm,PVMHSLGETNODE,&n);
  if (res != 0)
    printf("IOCTL  put get node  %d  %d \n",devpvm,res);

  return n;
#endif

  res = ioctl(devhsl,
	      HSLGETNODE,
	      &n);

  if (res < 0)
    return -1;

  return n;
}

