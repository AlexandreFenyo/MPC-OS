
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

typedef char status_space_t[STATUS_SIZE_SPACE]; /* One status = one byte */

int put_getnode();
volatile pvm_contigmem_t * pvm_get_status_mem();
int pvm_get_one_status();
int pvm_unmap_cmem_slot(volatile pvm_contigmem_t *cm);

static int dev = -1;

int
main(ac, av)
     int ac;
     char **av;
{
  char *chaine;
  int i, j, res, compteur, status_nbr, status_nbs, mynode;
  int taille = 1;
  volatile pvm_contigmem_t * status_slot;
  volatile char * scrut;
  double tps[NB];
  sendrecv rs;

  /****** INITIALISATIONS *********/
  dev = open("/dev/pvmhsl", O_RDWR);
  if (dev < 0)
    {
      perror("open");
      exit(1);
    }
  mynode = put_getnode();

  printf("Mynode = %d NBCOM = %d\n",mynode,NBCOM);

  status_slot = pvm_get_status_mem();
  status_nbs = pvm_get_one_status();
  status_nbr = pvm_get_one_status();
  scrut = (volatile char *) status_slot->vptr + status_nbr;
  *scrut = (char) 0;

  chaine = (char *) malloc(1<<(NB-1));
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
      rs.channel = 3;
      rs.inter = 1;
#ifdef TIMEC
      init_timec();
      tc = 0;
#endif
    }
  else
    {
      rs.dest = 0;
      rs.data = (caddr_t) chaine;
      rs.channel = 3;
      rs.inter = 1;
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
	  
	      for (i = 0; i < taille; i++) chaine[i] = 'A' + compteur;
#ifdef TIMEC
      tc = 0;
      topc1();
#endif	      
	      rs.sr = 1;
	      rs.num = status_nbs;
	      
	      /* EMISSION NODE 0 */
	      if (ioctl(dev, HSLSLRVSENDRECV, &rs)) 
		{
		  perror("ioctl HSLSLRVSENDRECV");
		  exit(1);
		}
	      /* ne retourne que lorsque les donnees sont effectivement parties */
	      /*      for (i=0; i<10000000; i++);*/
	      
	      /* RECEPTION NODE 0 */
	      /*	      printf("Reception taille %d\n",taille);*/
	      
	      for (i = 0; i < taille; i++) chaine[i] = 255;
	      
	      rs.sr = 0;
	      rs.num = status_nbr;
	      
	      if (ioctl(dev, HSLSLRVSENDRECV, &rs)) 
		{
		  perror("ioctl HSLSLRVSENDRECV");
		  exit(1);
		}
	      
	      /* on attend l'interruption */
	      while ((*scrut) == 0);
	      
	      /* on remet le status a zero */
	      *scrut = (char) 0;
#ifdef TIMEC
      topc2();
  /*    printf("%ld ",tc/2);*/
      tps[compteur] += (double) tc; 
#endif	      
	      /*for (i=0; i<1000000000; i++);*/
	      
	      for (i = 0; i < taille; i++) 
		{
		  if (chaine[i] != ('A'+compteur))
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
	      
	      rs.sr = 0;
	      rs.num = status_nbr;
	      
	      if (ioctl(dev, HSLSLRVSENDRECV, &rs)) 
		{
		  perror("ioctl HSLSLRVSENDRECV");
		  exit(1);
		}
	      
	      /* on attend l'interruption */
	      while ((*scrut) == 0);
	      
	      /* on remet le status a zero */
	      *scrut = (char) 0;
	      
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
	      
	      rs.sr = 1;
	      rs.num = status_nbs;
	      
	      if (ioctl(dev, HSLSLRVSENDRECV, &rs)) 
		{
		  perror("ioctl HSLSLRVSENDRECV");
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
	  printf("%8.0lf %5d\n",tps[i],1<<i);
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
  
  pvm_unmap_cmem_slot(status_slot);
  close(dev); 
  return 0;
}

/* Get the number of local node */
int
put_getnode() 
{
  int res=0;
  int n;

  res = ioctl(dev,PVMHSLGETNODE,&n);
  if (res != 0)
    printf("IOCTL  put get node  %d  %d \n",dev,res);

  return n;
}

/* Get the adress of CMEM slot for PVM status and map it in processus virtual memory 
 * Return NULL if error
 * [ENXIO]  : can't open driver
 * [ENOMEM] : ioctl PVM_GETSTATUSMEM error or not enough cmem memory
 * [EBUSY]  : can't open /dev/kmem
 * [EPERM]  : mmap error 
 */
volatile pvm_contigmem_t * pvm_get_status_mem()
{
  static volatile pvm_contigmem_t statusmem;
  int res,fd;
  static volatile caddr_t mmap_addr;


  statusmem.size = (size_t) sizeof(status_space_t);
  res = ioctl(dev, PVM_GETSTATUSMEM, &statusmem);

  if (res < 0 || !statusmem.kptr)
    {
      errno = ENOMEM;
      printf("pvm_get_status_mem : errno ENOMEM\n");
      return NULL;
    }

  /* mapping in process virtual memory */
  fd = open("/dev/kmem", O_RDWR);
  if (fd < 0)
    {
      errno = EBUSY;
      printf("pvm_get_status_mem : errno EBUSY\n");
      return NULL;
    }

  mmap_addr = mmap(NULL, statusmem.size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, (int) statusmem.kptr);
  close(fd);

  if (mmap_addr == (caddr_t) -1)
    {
      errno = EPERM;
      printf("pvm_get_status_mem : errno EPERM\n");
      return NULL;
    }

  statusmem.vptr = mmap_addr;

  return &statusmem;
}

/* Return the number of a free status buffer from 0 to STATUS_SPACE_SIZE - 1
 * return -1 if no free status or error 
 */
int pvm_get_one_status()
{
  int res,s;

  res = ioctl(dev, PVM_GET_STATUS , &s);
  if (res < 0) 
    {
      return -1;
    }

  return s;
}

/* Unmap the slot 
 * Input : the structure pvm_contigmem_t corresponding to a cmem slot
 * Return -1 if error
 * [ENOTTY]: bad cmem slot
 */
int pvm_unmap_cmem_slot(volatile pvm_contigmem_t *cm)
{

  if (cm->slot > 0)
    {
      munmap(cm->kptr,cm->size);
      return 0;
    }

  errno = ENOTTY;
  return -1;
}
