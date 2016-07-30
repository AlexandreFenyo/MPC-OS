# 1 "driver.c"

 

 







# 1 "/usr/include/sys/cdefs.h" 1 3
 

















































 





























# 106 "/usr/include/sys/cdefs.h" 3


 






















 



















# 169 "/usr/include/sys/cdefs.h" 3


































# 12 "driver.c" 2

# 1 "/usr/include/sys/types.h" 1 3
 













































 
# 1 "/usr/include/machine/ansi.h" 1 3
 






































 





















 










 











 












 







typedef	int __attribute__((__mode__(__DI__)))		 __int64_t;
typedef	unsigned int __attribute__((__mode__(__DI__)))	__uint64_t;






# 48 "/usr/include/sys/types.h" 2 3

# 1 "/usr/include/machine/types.h" 1 3
 







































typedef struct _physadr {
	int r[1];
} *physadr;

typedef struct label_t {
	int val[6];
} label_t;


typedef	unsigned int	vm_offset_t;
typedef	__int64_t	vm_ooffset_t;
typedef	unsigned int	vm_pindex_t;
typedef	unsigned int	vm_size_t;

 



typedef	signed  char		   int8_t;
typedef	unsigned char		 u_int8_t;
typedef	short			  int16_t;
typedef	unsigned short		u_int16_t;
typedef	int			  int32_t;
typedef	unsigned int		u_int32_t;
typedef	__int64_t		  int64_t;
typedef	__uint64_t		u_int64_t;

typedef	int32_t			register_t;

typedef int32_t			ufs_daddr_t;


typedef	int		intfptr_t;
typedef	unsigned int	uintfptr_t;
typedef	int		intptr_t;
typedef	unsigned int	uintptr_t;
typedef	__uint64_t	uoff_t;


 
typedef u_int32_t		intrmask_t;

 
typedef	void			inthand2_t  (void *_cookie)  ;


# 49 "/usr/include/sys/types.h" 2 3



typedef	unsigned char	u_char;
typedef	unsigned short	u_short;
typedef	unsigned int	u_int;
typedef	unsigned long	u_long;
typedef	unsigned short	ushort;		 
typedef	unsigned int	uint;		 


typedef	u_int64_t	u_quad_t;	 
typedef	int64_t		quad_t;
typedef	quad_t *	qaddr_t;

typedef	char *		caddr_t;	 
typedef	int32_t		daddr_t;	 
typedef	u_int32_t	dev_t;		 
typedef	u_int32_t	fixpt_t;	 
typedef	u_int32_t	gid_t;		 
typedef	u_int32_t	ino_t;		 
typedef	long		key_t;		 
typedef	u_int16_t	mode_t;		 
typedef	u_int16_t	nlink_t;	 
typedef	__int64_t 	off_t;		 
typedef	int 	pid_t;		 
typedef	quad_t		rlim_t;		 



typedef	int32_t		segsz_t;	 

typedef	int32_t		swblk_t;	 
typedef	u_int32_t	uid_t;		 


typedef	int		boolean_t;
typedef	struct vm_page	*vm_page_t;



 









# 1 "/usr/include/machine/endian.h" 1 3
 






































 







 













 
unsigned long	htonl  (unsigned long)  ;
unsigned short	htons  (unsigned short)  ;
unsigned long	ntohl  (unsigned long)  ;
unsigned short	ntohs  (unsigned short)  ;
 








# 84 "/usr/include/machine/endian.h" 3

















 

















# 100 "/usr/include/sys/types.h" 2 3



typedef	unsigned long 	clock_t;




typedef	int 	clockid_t;




typedef	unsigned int 	size_t;




typedef	int 	ssize_t;




typedef	long 	time_t;




typedef	int 	timer_t;




typedef	unsigned char 	uint8_t;




typedef	unsigned short 	uint16_t;




typedef	unsigned int 	uint32_t;






 









typedef	long	fd_mask;






typedef	struct fd_set {
	fd_mask	fds_bits[((( 1024  ) + ((  (sizeof(fd_mask) * 8 )  ) - 1)) / (  (sizeof(fd_mask) * 8 )  )) ];
} fd_set;







 




# 201 "/usr/include/sys/types.h" 3





# 13 "driver.c" 2


# 1 "/usr/include/sys/sysent.h" 1 3
 





































struct proc;

typedef	int	sy_call_t  (struct proc *, void *)  ;

struct sysent {		 
	int	sy_narg;	 
	sy_call_t *sy_call;	 
};

   

struct image_params;
struct trapframe;

struct sysentvec {
	int		sv_size;	 
	struct sysent	*sv_table;	 
	u_int		sv_mask;	 
	int		sv_sigsize;	 
	int		*sv_sigtbl;	 
	int		sv_errsize;	 
	int 		*sv_errtbl;	 
	int		(*sv_transtrap)  (int, int)  ;
					 
	int		(*sv_fixup)  (long **, struct image_params *)  ;
					 
	void		(*sv_sendsig)  (void (*)(int), int, int, u_long)  ;
					 
	char 		*sv_sigcode;	 
	int 		*sv_szsigcode;	 
	void		(*sv_prepsyscall)  (struct trapframe *, int *,
					       u_int *, caddr_t *)  ;
	char		*sv_name;	 
	int		(*sv_coredump)  (struct proc *p)  ;
					 
};


extern struct sysentvec aout_sysvec;
extern struct sysent sysent[];



# 15 "driver.c" 2

# 1 "/usr/include/sys/param.h" 1 3
 

























































 






# 1 "/usr/include/sys/syslimits.h" 1 3
 































































# 66 "/usr/include/sys/param.h" 2 3












 


# 1 "/usr/include/sys/errno.h" 1 3
 






























































					 


























 



 






 













 

















 






 





 























 






# 81 "/usr/include/sys/param.h" 2 3

# 1 "/usr/include/sys/time.h" 1 3
 








































 



struct timeval {
	long	tv_sec;		 
	long	tv_usec;	 
};



struct timespec {
	time_t	tv_sec;		 
	long	tv_nsec;	 
};











struct timezone {
	int	tz_minuteswest;	 
	int	tz_dsttime;	 
};








 
















































struct timecounter;
typedef unsigned timecounter_get_t  (struct timecounter *)  ;
typedef void timecounter_pps_t  (struct timecounter *)  ;

struct timecounter {
	 
	timecounter_get_t	*tc_get_timecount;
	timecounter_pps_t	*tc_poll_pps;
	unsigned 		tc_counter_mask;
	u_int32_t		tc_frequency;
	char			*tc_name;
	void			*tc_priv;
	 
	int			tc_cost;
	int32_t			tc_adjustment;
	u_int32_t		tc_scale_micro;
	u_int32_t		tc_scale_nano_i;
	u_int32_t		tc_scale_nano_f;
	unsigned 		tc_offset_count;
	u_int32_t		tc_offset_sec;
	u_int32_t		tc_offset_micro;
	u_int64_t		tc_offset_nano;
	struct timeval		tc_microtime;
	struct timespec		tc_nanotime;
	struct timecounter	*tc_other;
	struct timecounter	*tc_tweak;
};



 







# 175 "/usr/include/sys/time.h" 3

# 184 "/usr/include/sys/time.h" 3

 








 



# 224 "/usr/include/sys/time.h" 3


 







struct	itimerval {
	struct	timeval it_interval;	 
	struct	timeval it_value;	 
};

 


struct clockinfo {
	int	hz;		 
	int	tick;		 
	int	tickadj;	 
	int	stathz;		 
	int	profhz;		 
};

 













extern struct timecounter *timecounter;
extern time_t	time_second;

void	getmicrouptime  (struct timeval *tv)  ;
void	getmicrotime  (struct timeval *tv)  ;
void	getnanouptime  (struct timespec *tv)  ;
void	getnanotime  (struct timespec *tv)  ;
void	init_timecounter  (struct timecounter *tc)  ;
int	itimerdecr  (struct itimerval *itp, int usec)  ;
int	itimerfix  (struct timeval *tv)  ;
void	microuptime  (struct timeval *tv)  ;
void	microtime  (struct timeval *tv)  ;
void	nanouptime  (struct timespec *ts)  ;
void	nanotime  (struct timespec *ts)  ;
void	set_timecounter  (struct timespec *ts)  ;
void	timecounter_timespec  (unsigned count, struct timespec *ts)  ;
void	timevaladd  (struct timeval *, struct timeval *)  ;
void	timevalsub  (struct timeval *, struct timeval *)  ;
int	tvtohz  (struct timeval *)  ;
# 297 "/usr/include/sys/time.h" 3



# 82 "/usr/include/sys/param.h" 2 3











 
# 1 "/usr/include/machine/param.h" 1 3
 









































 








 
















 



























				 

 
















 



 


 


 













 














# 94 "/usr/include/sys/param.h" 2 3





 



























 










				 



 


















 











 





 








 





 





















 














# 16 "driver.c" 2




# 1 "/usr/include/sys/systm.h" 1 3
 











































# 1 "/usr/include/machine/cpufunc.h" 1 3
 


































 









# 1 "/usr/include/machine/lock.h" 1 3
 































# 129 "/usr/include/machine/lock.h" 3


# 194 "/usr/include/machine/lock.h" 3

















 



struct simplelock {
	volatile int	lock_data;
};

 
void	s_lock_init		 (struct simplelock *)  ;
void	s_lock			 (struct simplelock *)  ;
int	s_lock_try		 (struct simplelock *)  ;
void	s_unlock		 (struct simplelock *)  ;
void	ss_lock			 (struct simplelock *)  ;
void	ss_unlock		 (struct simplelock *)  ;
void	s_lock_np		 (struct simplelock *)  ;
void	s_unlock_np		 (struct simplelock *)  ;

 
extern struct simplelock	imen_lock;
extern struct simplelock	cpl_lock;
extern struct simplelock	fast_intr_lock;
extern struct simplelock	intr_lock;
extern struct simplelock	clock_lock;
extern struct simplelock	com_lock;
extern struct simplelock	mpintr_lock;
extern struct simplelock	mcount_lock;

# 248 "/usr/include/machine/lock.h" 3





# 46 "/usr/include/machine/cpufunc.h" 2 3
















static __inline void
breakpoint(void)
{
	__asm volatile ("int $3");
}

static __inline void
disable_intr(void)
{
	__asm volatile ("cli" : : : "memory");
	 ;
}

static __inline void
enable_intr(void)
{
	 ;
	__asm volatile ("sti");
}



static __inline int
ffs(int mask)
{
	int	result;
	 









	__asm volatile ("testl %0,%0; je 1f; bsfl %0,%0; incl %0; 1:"
			 : "=r" (result) : "0" (mask));
	return (result);
}



static __inline int
fls(int mask)
{
	int	result;
	__asm volatile ("testl %0,%0; je 1f; bsrl %0,%0; incl %0; 1:"
			 : "=r" (result) : "0" (mask));
	return (result);
}








 





























static __inline u_char
inbc(u_int port)
{
	u_char	data;

	__asm volatile ("inb %1,%0" : "=a" (data) : "id" ((u_short)(port)));
	return (data);
}

static __inline void
outbc(u_int port, u_char data)
{
	__asm volatile ("outb %0,%1" : : "a" (data), "id" ((u_short)(port)));
}



static __inline u_char
inbv(u_int port)
{
	u_char	data;
	 




	__asm volatile ("inb %%dx,%0" : "=a" (data) : "d" (port));
	return (data);
}

static __inline u_int
inl(u_int port)
{
	u_int	data;

	__asm volatile ("inl %%dx,%0" : "=a" (data) : "d" (port));
	return (data);
}

static __inline void
insb(u_int port, void *addr, size_t cnt)
{
	__asm volatile ("cld; rep; insb"
			 : : "d" (port), "D" (addr), "c" (cnt)
			 : "di", "cx", "memory");
}

static __inline void
insw(u_int port, void *addr, size_t cnt)
{
	__asm volatile ("cld; rep; insw"
			 : : "d" (port), "D" (addr), "c" (cnt)
			 : "di", "cx", "memory");
}

static __inline void
insl(u_int port, void *addr, size_t cnt)
{
	__asm volatile ("cld; rep; insl"
			 : : "d" (port), "D" (addr), "c" (cnt)
			 : "di", "cx", "memory");
}

static __inline void
invd(void)
{
	__asm volatile ("invd");
}


# 250 "/usr/include/machine/cpufunc.h" 3


static __inline void
invlpg(u_int addr)
{
	__asm   volatile ("invlpg %0"::"m"(*(char *)addr):"memory");
}


static __inline void
invltlb(void)
{
	u_int	temp;
	 



	__asm volatile ("movl %%cr3, %0; movl %0, %%cr3" : "=r" (temp)
			 : : "memory");



}




static __inline u_short
inw(u_int port)
{
	u_short	data;

	__asm volatile ("inw %%dx,%0" : "=a" (data) : "d" (port));
	return (data);
}

static __inline u_int
loadandclear(u_int *addr)
{
	u_int	result;

	__asm volatile ("xorl %0,%0; xchgl %1,%0"
			 : "=&r" (result) : "m" (*addr));
	return (result);
}

static __inline void
outbv(u_int port, u_char data)
{
	u_char	al;
	 





	al = data;
	__asm volatile ("outb %0,%%dx" : : "a" (al), "d" (port));
}

static __inline void
outl(u_int port, u_int data)
{
	 




	__asm volatile ("outl %0,%%dx" : : "a" (data), "d" (port));
}

static __inline void
outsb(u_int port, const void *addr, size_t cnt)
{
	__asm volatile ("cld; rep; outsb"
			 : : "d" (port), "S" (addr), "c" (cnt)
			 : "si", "cx");
}

static __inline void
outsw(u_int port, const void *addr, size_t cnt)
{
	__asm volatile ("cld; rep; outsw"
			 : : "d" (port), "S" (addr), "c" (cnt)
			 : "si", "cx");
}

static __inline void
outsl(u_int port, const void *addr, size_t cnt)
{
	__asm volatile ("cld; rep; outsl"
			 : : "d" (port), "S" (addr), "c" (cnt)
			 : "si", "cx");
}

static __inline void
outw(u_int port, u_short data)
{
	__asm volatile ("outw %0,%%dx" : : "a" (data), "d" (port));
}

static __inline u_int
rcr2(void)
{
	u_int	data;

	__asm volatile ("movl %%cr2,%0" : "=r" (data));
	return (data);
}

static __inline u_int
read_eflags(void)
{
	u_int	ef;

	__asm volatile ("pushfl; popl %0" : "=r" (ef));
	return (ef);
}

static __inline u_int64_t
rdmsr(u_int msr)
{
	u_int64_t rv;

	__asm volatile (".byte 0x0f, 0x32" : "=A" (rv) : "c" (msr));
	return (rv);
}

static __inline u_int64_t
rdpmc(u_int pmc)
{
	u_int64_t rv;

	__asm volatile (".byte 0x0f, 0x33" : "=A" (rv) : "c" (pmc));
	return (rv);
}

static __inline u_int64_t
rdtsc(void)
{
	u_int64_t rv;

	__asm volatile (".byte 0x0f, 0x31" : "=A" (rv));
	return (rv);
}

static __inline void
setbits(volatile unsigned *addr, u_int bits)
{
	__asm volatile (



			 "orl %1,%0" : "=m" (*addr) : "ir" (bits));
}

static __inline void
wbinvd(void)
{
	__asm volatile ("wbinvd");
}

static __inline void
write_eflags(u_int ef)
{
	__asm volatile ("pushl %0; popfl" : : "r" (ef));
}

static __inline void
wrmsr(u_int msr, u_int64_t newval)
{
	__asm volatile (".byte 0x0f, 0x30" : : "A" (newval), "c" (msr));
}

# 455 "/usr/include/machine/cpufunc.h" 3


void	load_cr0	 (u_int cr0)  ;
void	load_cr3	 (u_int cr3)  ;
void	load_cr4	 (u_int cr4)  ;
void	ltr		 (u_short sel)  ;
u_int	rcr0		 (void)  ;
u_int	rcr3		 (void)  ;
u_int	rcr4		 (void)  ;
void	i686_pagezero	 (void *addr)  ;


# 45 "/usr/include/sys/systm.h" 2 3

# 1 "/usr/include/sys/callout.h" 1 3
 











































# 1 "/usr/include/sys/queue.h" 1 3
 






































 




































































 






 




 
 






























# 164 "/usr/include/sys/queue.h" 3

 













 







































# 231 "/usr/include/sys/queue.h" 3

 













 













































 

















 























# 342 "/usr/include/sys/queue.h" 3









# 359 "/usr/include/sys/queue.h" 3

















 














 

















# 417 "/usr/include/sys/queue.h" 3


# 427 "/usr/include/sys/queue.h" 3


# 437 "/usr/include/sys/queue.h" 3


# 447 "/usr/include/sys/queue.h" 3








# 466 "/usr/include/sys/queue.h" 3



 




struct quehead {
	struct quehead *qh_link;
	struct quehead *qh_rlink;
};



static __inline void
insque(void *a, void *b)
{
	struct quehead *element = a, *head = b;

	element->qh_link = head->qh_link;
	element->qh_rlink = head;
	head->qh_link = element;
	element->qh_link->qh_rlink = element;
}

static __inline void
remque(void *a)
{
	struct quehead *element = a;

	element->qh_link->qh_rlink = element->qh_rlink;
	element->qh_rlink->qh_link = element->qh_link;
	element->qh_rlink = 0;
}











# 45 "/usr/include/sys/callout.h" 2 3


struct  callout_list  {	struct   callout  *slh_first;	} ;
struct  callout_tailq  {	struct   callout  *tqh_first;	struct   callout  **tqh_last;	} ;

struct callout {
	union {
		struct {	struct  callout  *sle_next;	}  sle;
		struct {	struct  callout  *tqe_next;	struct  callout  **tqe_prev;	}  tqe;
	} c_links;
	int	c_time;				 
	void	*c_arg;				 
	void	(*c_func)  (void *)  ;	 
};

struct callout_handle {
	struct callout *callout;
};


extern struct callout_list callfree;
extern struct callout *callout;
extern int	ncallout;
extern struct callout_tailq *callwheel;
extern int	callwheelsize, callwheelbits, callwheelmask, softticks;



# 46 "/usr/include/sys/systm.h" 2 3


extern int securelevel;		 

extern int cold;		 
extern const char *panicstr;	 
extern int safepri;		 
extern char version[];		 
extern char copyright[];	 

extern int nblkdev;		 
extern int nchrdev;		 
extern int nswap;		 

extern int selwait;		 

extern u_char curpriority;	 

extern int physmem;		 

extern dev_t dumpdev;		 
extern long dumplo;		 

extern dev_t rootdev;		 
extern dev_t rootdevs[2];	 
extern char *rootdevnames[2];	 
extern struct vnode *rootvp;	 

extern struct vnode *swapdev_vp; 

extern int boothowto;		 
extern int bootverbose;		 

 



struct clockframe;
struct malloc_type;
struct proc;
struct timeval;
struct tty;
struct uio;

void	Debugger  (const char *msg)  ;
int	nullop  (void)  ;
int	eopnotsupp  (void)  ;
int	einval  (void)  ;
int	seltrue  (dev_t dev, int which, struct proc *p)  ;
int	ureadc  (int, struct uio *)  ;
void	*hashinit  (int count, struct malloc_type *type, u_long *hashmask)  ;
void	*phashinit  (int count, struct malloc_type *type, u_long *nentries)  ;

void	panic  (const char *, ...)   __attribute__((__noreturn__))  __attribute__((__format__ (__printf__,  1 ,   2 ))) ;
void	cpu_boot  (int)  ;
void	cpu_rootconf  (void)  ;
void	cpu_dumpconf  (void)  ;
void	tablefull  (const char *)  ;
int	addlog  (const char *, ...)   __attribute__((__format__ (__printf__,  1 ,   2 ))) ;
int	kvprintf  (char const *, void (*)(int, void*), void *, int,
		      char * )   __attribute__((__format__ (__printf__,  1 ,   0 ))) ;
void	log  (int, const char *, ...)   __attribute__((__format__ (__printf__,  2 ,   3 ))) ;
void	logwakeup  (void)  ;
int	printf  (const char *, ...)   __attribute__((__format__ (__printf__,  1 ,   2 ))) ;
int	sprintf  (char *buf, const char *, ...)   __attribute__((__format__ (__printf__,  2 ,   3 ))) ;
void	uprintf  (const char *, ...)   __attribute__((__format__ (__printf__,  1 ,   2 ))) ;
void	vprintf  (const char *, char * )   __attribute__((__format__ (__printf__,  1 ,   0 ))) ;
int     vsprintf  (char *buf, const char *, char * )   __attribute__((__format__ (__printf__,  2 ,   0 ))) ;
void	ttyprintf  (struct tty *, const char *, ...)   __attribute__((__format__ (__printf__,  2 ,   3 ))) ;

void	bcopy  (const void *from, void *to, size_t len)  ;
void	ovbcopy  (const void *from, void *to, size_t len)  ;

extern void	(*bzero)  (void *buf, size_t len)  ;




void	*memcpy  (void *to, const void *from, size_t len)  ;

int	copystr  (const void *kfaddr, void *kdaddr, size_t len,
		size_t *lencopied)  ;
int	copyinstr  (const void *udaddr, void *kaddr, size_t len,
		size_t *lencopied)  ;
int	copyin  (const void *udaddr, void *kaddr, size_t len)  ;
int	copyout  (const void *kaddr, void *udaddr, size_t len)  ;

int	fubyte  (const void *base)  ;
int	subyte  (void *base, int byte)  ;
int	suibyte  (void *base, int byte)  ;
long	fuword  (const void *base)  ;
int	suword  (void *base, long word)  ;
long	fusword  (void *base)  ;
int	susword  (void *base, long word)  ;

void	realitexpire  (void *)  ;

void	hardclock  (struct clockframe *frame)  ;
void	softclock  (void)  ;
void	statclock  (struct clockframe *frame)  ;

void	startprofclock  (struct proc *)  ;
void	stopprofclock  (struct proc *)  ;
void	setstatclockrate  (int hzrate)  ;

void	hardpps  (struct timeval *tvp, long usec)  ;

char	*getenv  (char *name)  ;
extern char *kern_envp;





# 1 "/usr/include/sys/libkern.h" 1 3
 









































 
extern u_char const	bcd2bin_data[];
extern u_char const	bin2bcd_data[];
extern char const	hex2ascii_data[];





static __inline int imax(int a, int b) { return (a > b ? a : b); }
static __inline int imin(int a, int b) { return (a < b ? a : b); }
static __inline long lmax(long a, long b) { return (a > b ? a : b); }
static __inline long lmin(long a, long b) { return (a < b ? a : b); }
static __inline u_int max(u_int a, u_int b) { return (a > b ? a : b); }
static __inline u_int min(u_int a, u_int b) { return (a < b ? a : b); }
static __inline quad_t qmax(quad_t a, quad_t b) { return (a > b ? a : b); }
static __inline quad_t qmin(quad_t a, quad_t b) { return (a < b ? a : b); }
static __inline u_long ulmax(u_long a, u_long b) { return (a > b ? a : b); }
static __inline u_long ulmin(u_long a, u_long b) { return (a < b ? a : b); }

 
int	 bcmp  (const void *, const void *, size_t)  ;






int	 locc  (int, char *, u_int)  ;
void	 qsort  (void *base, size_t nmemb, size_t size,
		    int (*compar)(const void *, const void *))  ;
u_long	 random  (void)  ;
char	*index  (const char *, int)  ;
char	*rindex  (const char *, int)  ;
int	 scanc  (u_int, const u_char *, const u_char *, int)  ;
int	 skpc  (int, int, char *)  ;
void	 srandom  (u_long)  ;
char	*strcat  (char *, const char *)  ;
int	 strcmp  (const char *, const char *)  ;
char	*strcpy  (char *, const char *)  ;
size_t	 strlen  (const char *)  ;
int	 strncmp  (const char *, const char *, size_t)  ;
char	*strncpy  (char *, const char *, size_t)  ;


# 160 "/usr/include/sys/systm.h" 2 3


 
void	consinit  (void)  ;
void	cpu_initclocks  (void)  ;
void	nchinit  (void)  ;
void	usrinfoinit  (void)  ;
void	vntblinit  (void)  ;

 
void	shutdown_nice  (void)  ;

 


void	inittodr  (time_t base)  ;
void	resettodr  (void)  ;
void	startrtclock  (void)  ;

 
typedef void timeout_t  (void *)  ;	 



void	callout_handle_init  (struct callout_handle *)  ;
struct	callout_handle timeout  (timeout_t *, void *, int)  ;
void	untimeout  (timeout_t *, void *, struct callout_handle)  ;

 


void		setdelayed  (void)  ;
void		setsoftast  (void)  ;
void		setsoftcambio  (void)  ;
void		setsoftcamnet  (void)  ;
void		setsoftclock  (void)  ;
void		setsoftnet  (void)  ;
void		setsofttty  (void)  ;
void		setsoftvm  (void)  ;
void		schedsoftcamnet  (void)  ;
void		schedsoftcambio  (void)  ;
void		schedsoftnet  (void)  ;
void		schedsofttty  (void)  ;
void		schedsoftvm  (void)  ;
intrmask_t	softclockpending  (void)  ;
void		spl0  (void)  ;
intrmask_t	splbio  (void)  ;
intrmask_t	splcam  (void)  ;
intrmask_t	splclock  (void)  ;
intrmask_t	splhigh  (void)  ;
intrmask_t	splimp  (void)  ;
intrmask_t	splnet  (void)  ;



intrmask_t	splsoftcam  (void)  ;
intrmask_t	splsoftcambio  (void)  ;
intrmask_t	splsoftcamnet  (void)  ;
intrmask_t	splsoftclock  (void)  ;
intrmask_t	splsofttty  (void)  ;
intrmask_t	splsoftvm  (void)  ;
intrmask_t	splstatclock  (void)  ;
intrmask_t	spltty  (void)  ;
intrmask_t	splvm  (void)  ;
void		splx  (intrmask_t ipl)  ;
void		splz  (void)  ;






 





extern intrmask_t bio_imask;	 
extern intrmask_t cam_imask;	 
extern intrmask_t net_imask;	 
extern intrmask_t stat_imask;	 
extern intrmask_t tty_imask;	 

 
extern const intrmask_t soft_imask;     
extern const intrmask_t softnet_imask;  
extern const intrmask_t softtty_imask;  

 



 
typedef void (*exitlist_fn)  (struct proc *procp)  ;

int	at_exit  (exitlist_fn function)  ;
int	rm_at_exit  (exitlist_fn function)  ;

 
typedef void (*forklist_fn)  (struct proc *parent, struct proc *child,
				 int flags)  ;

int	at_fork  (forklist_fn function)  ;
int	rm_at_fork  (forklist_fn function)  ;

 




typedef void (*bootlist_fn)  (int, void *)  ;

int	at_shutdown  (bootlist_fn function, void *arg, int position)  ;
int	rm_at_shutdown  (bootlist_fn function, void *arg)  ;

 






typedef void (*watchdog_tickle_fn)  (void)  ;

extern watchdog_tickle_fn	wdog_tickler;

 



int	tsleep  (void *chan, int pri, const char *wmesg, int timo)  ;
void	wakeup  (void *chan)  ;


# 20 "driver.c" 2





# 1 "/usr/include/sys/signalvar.h" 1 3
 






































# 1 "/usr/include/sys/signal.h" 1 3
 












































# 1 "/usr/include/sys/_posix.h" 1 3



 




























 














 





 















# 96 "/usr/include/sys/_posix.h" 3


# 46 "/usr/include/sys/signal.h" 2 3

# 1 "/usr/include/machine/signal.h" 1 3
 






































 



typedef int sig_atomic_t;



# 1 "/usr/include/machine/trap.h" 1 3
 









































 


























 

 






 








 





 



# 48 "/usr/include/machine/signal.h" 2 3


 






struct	sigcontext {
	int	sc_onstack;		 
	int	sc_mask;		 
	int	sc_esp;			 
	int	sc_ebp;
	int	sc_isp;
	int	sc_eip;
	int	sc_efl;
	int	sc_es;
	int	sc_ds;
	int	sc_cs;
	int	sc_ss;
	int	sc_edi;
	int	sc_esi;
	int	sc_ebx;
	int	sc_edx;
	int	sc_ecx;
	int	sc_eax;
	int	sc_gs;
	int	sc_fs;





	int	sc_trapno;
	int	sc_err;
};




# 47 "/usr/include/sys/signal.h" 2 3



















































 















typedef void __sighandler_t  (int)  ;






typedef unsigned int sigset_t;

 


struct	sigaction {
	__sighandler_t *sa_handler;	 
	sigset_t sa_mask;		 
	int	sa_flags;		 
};












 







typedef	__sighandler_t	*sig_t;	 






 


struct	sigaltstack {
	char	*ss_sp;			 
	size_t	ss_size;		 
	int	ss_flags;		 
};





 



struct	sigvec {
	__sighandler_t *sv_handler;	 
	int	sv_mask;		 
	int	sv_flags;		 
};








 


struct	sigstack {
	char	*ss_sp;			 
	int	ss_onstack;		 
};

 












 

union sigval {
	int	sival_int;		 
	void	*sival_ptr;		 
};

typedef struct siginfo {
	int	si_signo;		 
	int	si_code;		 
	union sigval si_value;		 
} siginfo_t;

struct sigevent {
	int	sigev_notify;		 
	int	sigev_signo;		 
	union sigval sigev_value;	 
};






 



 
__sighandler_t *signal  (int, __sighandler_t *)  ;
 


# 40 "/usr/include/sys/signalvar.h" 2 3


 




 



struct	sigacts {
	sig_t	ps_sigact[32 ];	 
	sigset_t ps_catchmask[32 ];	 
	sigset_t ps_sigonstack;		 
	sigset_t ps_sigintr;		 
	sigset_t ps_sigreset;		 
	sigset_t ps_signodefer;		 
	sigset_t ps_oldmask;		 
	int	ps_flags;		 
	struct	sigaltstack ps_sigstk;	 
	int	ps_sig;			 
	u_long	ps_code;		 
	sigset_t ps_usertramp;		 
};

 



 



 




 










 




 












# 148 "/usr/include/sys/signalvar.h" 3





struct pgrp;
struct proc;

extern int sugid_coredump;	 

 


void	execsigs  (struct proc *p)  ;
char	*expand_name  (const char*, int, int)  ;
void	gsignal  (int pgid, int sig)  ;
int	issignal  (struct proc *p)  ;
void	killproc  (struct proc *p, char *why)  ;
void	pgsignal  (struct pgrp *pgrp, int sig, int checkctty)  ;
void	postsig  (int sig)  ;
void	psignal  (struct proc *p, int sig)  ;
void	sigexit  (struct proc *p, int signum)  ;
void	siginit  (struct proc *p)  ;
void	trapsignal  (struct proc *p, int sig, u_long code)  ;

 


void	sendsig  (sig_t action, int sig, int returnmask, u_long code)  ;


# 25 "driver.c" 2

# 1 "/usr/include/sys/conf.h" 1 3
 











































 



struct buf;
struct proc;
struct tty;
struct uio;
struct vnode;

typedef int d_open_t  (dev_t dev, int oflags, int devtype, struct proc *p)  ;
typedef int d_close_t  (dev_t dev, int fflag, int devtype, struct proc *p)  ;
typedef void d_strategy_t  (struct buf *bp)  ;
typedef int d_ioctl_t  (dev_t dev, u_long cmd, caddr_t data,
			   int fflag, struct proc *p)  ;
typedef int d_dump_t  (dev_t dev)  ;
typedef int d_psize_t  (dev_t dev)  ;

typedef int d_read_t  (dev_t dev, struct uio *uio, int ioflag)  ;
typedef int d_write_t  (dev_t dev, struct uio *uio, int ioflag)  ;
typedef void d_stop_t  (struct tty *tp, int rw)  ;
typedef int d_reset_t  (dev_t dev)  ;
typedef struct tty *d_devtotty_t  (dev_t dev)  ;
typedef int d_poll_t  (dev_t dev, int events, struct proc *p)  ;
typedef int d_mmap_t  (dev_t dev, int offset, int nprot)  ;

typedef int l_open_t  (dev_t dev, struct tty *tp)  ;
typedef int l_close_t  (struct tty *tp, int flag)  ;
typedef int l_read_t  (struct tty *tp, struct uio *uio, int flag)  ;
typedef int l_write_t  (struct tty *tp, struct uio *uio, int flag)  ;
typedef int l_ioctl_t  (struct tty *tp, u_long cmd, caddr_t data,
			   int flag, struct proc *p)  ;
typedef int l_rint_t  (int c, struct tty *tp)  ;
typedef int l_start_t  (struct tty *tp)  ;
typedef int l_modem_t  (struct tty *tp, int flag)  ;

 








 








 


struct cdevsw {
	d_open_t	*d_open;
	d_close_t	*d_close;
	d_read_t	*d_read;
	d_write_t	*d_write;
	d_ioctl_t	*d_ioctl;
	d_stop_t	*d_stop;
	d_reset_t	*d_reset;	 
	d_devtotty_t	*d_devtotty;
	d_poll_t	*d_poll;
	d_mmap_t	*d_mmap;
	d_strategy_t	*d_strategy;
	char		*d_name;	 
	void		*d_spare;
	int		d_maj;
	d_dump_t	*d_dump;
	d_psize_t	*d_psize;
	u_int		d_flags;
	int		d_maxio;
	int		d_bmaj;
};


extern struct cdevsw *bdevsw[];
extern struct cdevsw *cdevsw[];


 


struct linesw {
	l_open_t	*l_open;
	l_close_t	*l_close;
	l_read_t	*l_read;
	l_write_t	*l_write;
	l_ioctl_t	*l_ioctl;
	l_rint_t	*l_rint;
	l_start_t	*l_start;
	l_modem_t	*l_modem;
	u_char		l_hotchar;
};


extern struct linesw linesw[];
extern int nlinesw;

int ldisc_register  (int , struct linesw *)  ;
void ldisc_deregister  (int)  ;



 


struct swdevt {
	dev_t	sw_dev;
	int	sw_flags;
	int	sw_nblks;
	struct	vnode *sw_vp;
};





d_open_t	noopen;
d_close_t	noclose;
d_read_t	noread;
d_write_t	nowrite;
d_ioctl_t	noioctl;
d_stop_t	nostop;
d_reset_t	noreset;
d_devtotty_t	nodevtotty;
d_mmap_t	nommap;

 



 





d_dump_t	nodump;

 




d_open_t	nullopen;
d_close_t	nullclose;



l_read_t	l_noread;
l_write_t	l_nowrite;

 


# 249 "/usr/include/sys/conf.h" 3


int	cdevsw_add  (dev_t *descrip,struct cdevsw *new,struct cdevsw **old)  ;
void	cdevsw_add_generic  (int bdev, int cdev, struct cdevsw *cdevsw)  ;
dev_t	chrtoblk  (dev_t dev)  ;
int	iskmemdev  (dev_t dev)  ;
int	iszerodev  (dev_t dev)  ;
void	setconf  (void)  ;



# 26 "driver.c" 2


# 1 "/usr/include/sys/mount.h" 1 3
 






































# 1 "/usr/include/sys/ucred.h" 1 3
 






































 


struct ucred {
	u_short	cr_ref;			 
	uid_t	cr_uid;			 
	short	cr_ngroups;		 
	gid_t	cr_groups[16  ];	 
};







struct ucred	*crcopy  (struct ucred *cr)  ;
struct ucred	*crdup  (struct ucred *cr)  ;
void		crfree  (struct ucred *cr)  ;
struct ucred	*crget  (void)  ;
int		suser  (struct ucred *cred, u_short *acflag)  ;
int		groupmember  (gid_t gid, struct ucred *cred)  ;



# 40 "/usr/include/sys/mount.h" 2 3


# 1 "/usr/include/sys/lock.h" 1 3
 













































 




struct lock {
	struct	simplelock lk_interlock;  
	u_int	lk_flags;		 
	int	lk_sharecount;		 
	int	lk_waitcount;		 
	short	lk_exclusivecount;	 
	short	lk_prio;		 
	char	*lk_wmesg;		 
	int	lk_timo;		 
	pid_t	lk_lockholder;		 
};
 








































 












 










 









 





 
















 





void dumplockinfo(struct lock *lkp);
struct proc;

void	lockinit  (struct lock *, int prio, char *wmesg, int timo,
			int flags)  ;
int	lockmgr  (struct lock *, u_int flags,
			struct simplelock *, struct proc *p)  ;
void	lockmgr_printinfo  (struct lock *)  ;
int	lockstatus  (struct lock *)  ;

# 185 "/usr/include/sys/lock.h" 3










# 42 "/usr/include/sys/mount.h" 2 3


typedef struct fsid { int32_t val[2]; } fsid_t;	 

 





struct fid {
	u_short		fid_len;		 
	u_short		fid_reserved;		 
	char		fid_data[16 ];	 
};

 






struct statfs {
	long	f_spare2;		 
	long	f_bsize;		 
	long	f_iosize;		 
	long	f_blocks;		 
	long	f_bfree;		 
	long	f_bavail;		 
	long	f_files;		 
	long	f_ffree;		 
	fsid_t	f_fsid;			 
	uid_t	f_owner;		 
	int	f_type;			 
	int	f_flags;		 
	long    f_syncwrites;		 
	long    f_asyncwrites;		 
	char	f_fstypename[16 ];  
	char	f_mntonname[90 ];	 
	char	f_mntfromname[90 ]; 
};

 




struct  vnodelst  {	struct   vnode  *lh_first;	} ;

struct mount {
	struct {	struct  mount  *cqe_next;	struct  mount  *cqe_prev;	}  mnt_list;		 
	struct vfsops	*mnt_op;		 
	struct vfsconf	*mnt_vfc;		 
	struct vnode	*mnt_vnodecovered;	 
	struct vnode	*mnt_syncer;		 
	struct vnodelst	mnt_vnodelist;		 
	struct lock	mnt_lock;		 
	int		mnt_flag;		 
	int		mnt_kern_flag;		 
	int		mnt_maxsymlinklen;	 
	struct statfs	mnt_stat;		 
	qaddr_t		mnt_data;		 
	time_t		mnt_time;		 
};

 
















 









 









 













 









 










 








 








 








 


struct fhandle {
	fsid_t	fh_fsid;	 
	struct	fid fh_fid;	 
};
typedef struct fhandle	fhandle_t;

 


struct export_args {
	int	ex_flags;		 
	uid_t	ex_root;		 
	struct	ucred ex_anon;		 
	struct	sockaddr *ex_addr;	 
	int	ex_addrlen;		 
	struct	sockaddr *ex_mask;	 
	int	ex_masklen;		 
	char	*ex_indexfile;		 
};

 



struct nfs_public {
	int		np_valid;	 
	fhandle_t	np_handle;	 
	struct mount	*np_mount;	 
	char		*np_index;	 
};

 




struct vfsconf {
	struct	vfsops *vfc_vfsops;	 
	char	vfc_name[16 ];	 
	int	vfc_typenum;		 
	int	vfc_refcount;		 
	int	vfc_flags;		 
	struct	vfsconf *vfc_next;	 
};

struct ovfsconf {
	void	*vfc_vfsops;
	char	vfc_name[32];
	int	vfc_index;
	int	vfc_refcount;
	int	vfc_flags;
};

 















extern int maxvfsconf;		 
extern int nfs_mount_type;	 
extern struct vfsconf *vfsconf;	 

 



struct nameidata;
struct mbuf;


struct vfsops {
	int	(*vfs_mount)	 (struct mount *mp, char *path, caddr_t data,
				    struct nameidata *ndp, struct proc *p)  ;
	int	(*vfs_start)	 (struct mount *mp, int flags,
				    struct proc *p)  ;
	int	(*vfs_unmount)	 (struct mount *mp, int mntflags,
				    struct proc *p)  ;
	int	(*vfs_root)	 (struct mount *mp, struct vnode **vpp)  ;
	int	(*vfs_quotactl)	 (struct mount *mp, int cmds, uid_t uid,
				    caddr_t arg, struct proc *p)  ;
	int	(*vfs_statfs)	 (struct mount *mp, struct statfs *sbp,
				    struct proc *p)  ;
	int	(*vfs_sync)	 (struct mount *mp, int waitfor,
				    struct ucred *cred, struct proc *p)  ;
	int	(*vfs_vget)	 (struct mount *mp, ino_t ino,
				    struct vnode **vpp)  ;
	int	(*vfs_fhtovp)	 (struct mount *mp, struct fid *fhp,
				    struct sockaddr *nam, struct vnode **vpp,
				    int *exflagsp, struct ucred **credanonp)  ;
	int	(*vfs_vptofh)	 (struct vnode *vp, struct fid *fhp)  ;
	int	(*vfs_init)	 (struct vfsconf *)  ;
	int	(*vfs_uninit)	 (struct vfsconf *)  ;
	struct sysctl_oid *vfs_oid;
};














# 351 "/usr/include/sys/mount.h" 3


# 1 "/usr/include/sys/module.h" 1 3
 






























typedef enum {
    MOD_LOAD,
    MOD_UNLOAD,
    MOD_SHUTDOWN
} modeventtype_t;

struct module;
typedef struct module *module_t;

typedef int (*modeventhand_t)(module_t mod, modeventtype_t what, void *arg);

 


typedef struct moduledata {
    char*		name;	 
    modeventhand_t	evhand;	 
    void*		priv;	 
    void*		_file;	 
} moduledata_t;







void module_register_init(void *data);
int module_register(const char *name, modeventhand_t callback, void *arg,
		    void *file);
module_t module_lookupbyname(const char *name);
module_t module_lookupbyid(int modid);
void module_reference(module_t mod);
void module_release(module_t mod);
int module_unload(module_t mod);
int module_getid(module_t mod);
module_t module_getfnext(module_t mod);

# 80 "/usr/include/sys/module.h" 3










struct module_stat {
    int		version;	 
    char	name[32 ];
    int		refs;
    int		id;
};

# 108 "/usr/include/sys/module.h" 3



# 353 "/usr/include/sys/mount.h" 2 3


# 389 "/usr/include/sys/mount.h" 3






# 1 "/usr/include/net/radix.h" 1 3
 










































 



struct radix_node {
	struct	radix_mask *rn_mklist;	 
	struct	radix_node *rn_p;	 
	short	rn_b;			 
	char	rn_bmask;		 
	u_char	rn_flags;		 



	union {
		struct {			 
			caddr_t	rn_Key;		 
			caddr_t	rn_Mask;	 
			struct	radix_node *rn_Dupedkey;
		} rn_leaf;
		struct {			 
			int	rn_Off;		 
			struct	radix_node *rn_L; 
			struct	radix_node *rn_R; 
		} rn_node;
	}		rn_u;





};








 



struct radix_mask {
	short	rm_b;			 
	char	rm_unused;		 
	u_char	rm_flags;		 
	struct	radix_mask *rm_mklist;	 
	union	{
		caddr_t	rmu_mask;		 
		struct	radix_node *rmu_leaf;	 
	}	rm_rmu;
	int	rm_refs;		 
};













typedef int walktree_f_t  (struct radix_node *, void *)  ;

struct radix_node_head {
	struct	radix_node *rnh_treetop;
	int	rnh_addrsize;		 
	int	rnh_pktsize;		 
	struct	radix_node *(*rnh_addaddr)	 
		 (void *v, void *mask,
		     struct radix_node_head *head, struct radix_node nodes[])  ;
	struct	radix_node *(*rnh_addpkt)	 
		 (void *v, void *mask,
		     struct radix_node_head *head, struct radix_node nodes[])  ;
	struct	radix_node *(*rnh_deladdr)	 
		 (void *v, void *mask, struct radix_node_head *head)  ;
	struct	radix_node *(*rnh_delpkt)	 
		 (void *v, void *mask, struct radix_node_head *head)  ;
	struct	radix_node *(*rnh_matchaddr)	 
		 (void *v, struct radix_node_head *head)  ;
	struct	radix_node *(*rnh_lookup)	 
		 (void *v, void *mask, struct radix_node_head *head)  ;
	struct	radix_node *(*rnh_matchpkt)	 
		 (void *v, struct radix_node_head *head)  ;
	int	(*rnh_walktree)			 
		 (struct radix_node_head *head, walktree_f_t *f, void *w)  ;
	int	(*rnh_walktree_from)		 
		 (struct radix_node_head *head, void *a, void *m,
		     walktree_f_t *f, void *w)  ;
	void	(*rnh_close)	 
		 (struct radix_node *rn, struct radix_node_head *head)  ;
	struct	radix_node rnh_nodes[3];	 
};















void	 rn_init  (void)  ;
int	 rn_inithead  (void **, int)  ;
int	 rn_refines  (void *, void *)  ;
struct radix_node
	 *rn_addmask  (void *, int, int)  ,
	 *rn_addroute  (void *, void *, struct radix_node_head *,
			struct radix_node [2])  ,
	 *rn_delete  (void *, void *, struct radix_node_head *)  ,
	 *rn_lookup  (void *v_arg, void *m_arg,
		        struct radix_node_head *head)  ,
	 *rn_match  (void *, struct radix_node_head *)  ;



# 395 "/usr/include/sys/mount.h" 2 3




 


struct netcred {
	struct	radix_node netc_rnodes[2];
	int	netc_exflags;
	struct	ucred netc_anon;
};

 


struct netexport {
	struct	netcred ne_defexported;		       
	struct	radix_node_head *ne_rtable[31 +1];  
};

extern	char *mountrootfsname;

 


int	dounmount  (struct mount *, int, struct proc *)  ;
int	vfs_setpublicfs			     
	   (struct mount *, struct netexport *, struct export_args *)  ;
int	vfs_lock  (struct mount *)  ;          
void	vfs_msync  (struct mount *, int)  ;
void	vfs_unlock  (struct mount *)  ;        
int	vfs_busy  (struct mount *, int, struct simplelock *, struct proc *)  ;
int	vfs_export			     
	   (struct mount *, struct netexport *, struct export_args *)  ;
struct	netcred *vfs_export_lookup	     
	   (struct mount *, struct netexport *, struct sockaddr *)  ;
int	vfs_allocate_syncvnode  (struct mount *)  ;
void	vfs_getnewfsid  (struct mount *)  ;
struct	mount *vfs_getvfs  (fsid_t *)  ;       
int	vfs_mountedon  (struct vnode *)  ;     
int	vfs_rootmountalloc  (char *, char *, struct mount **)  ;
void	vfs_unbusy  (struct mount *, struct proc *)  ;
void	vfs_unmountall  (void)  ;
int	vfs_register  (struct vfsconf *)  ;
int	vfs_unregister  (struct vfsconf *)  ;
extern	struct  mntlist  {	struct   mount  *cqh_first;	struct   mount  *cqh_last;	}  mountlist;	 
extern	struct simplelock mountlist_slock;
extern	struct nfs_public nfs_pub;

# 470 "/usr/include/sys/mount.h" 3



# 28 "driver.c" 2

# 1 "/usr/include/sys/exec.h" 1 3
 











































 







struct ps_strings {
	char	**ps_argvstr;	 
	int	ps_nargvstr;	 
	char	**ps_envstr;	 
	int	ps_nenvstr;	 
};

 






struct image_params;

struct execsw {
	int (*ex_imgact)  (struct image_params *)  ;
	const char *ex_name;
};

# 1 "/usr/include/machine/exec.h" 1 3
 









































# 74 "/usr/include/sys/exec.h" 2 3





int exec_map_first_page  (struct image_params *)  ;        
void exec_unmap_first_page  (struct image_params *)  ;       

int exec_register  (const struct execsw *)  ;
int exec_unregister  (const struct execsw *)  ;




# 117 "/usr/include/sys/exec.h" 3




# 29 "driver.c" 2


# 1 "/usr/include/sys/lkm.h" 1 3
 









































 


typedef enum loadmod {
	LM_SYSCALL,
	LM_VFS,
	LM_DEV,
	LM_STRMOD,
	LM_EXEC,
	LM_MISC
} MODTYPE;





 




 


struct lkm_syscall {
	MODTYPE	lkm_type;
	int	lkm_ver;
	const char	*lkm_name;
	u_long	lkm_offset;		 
	struct sysent	*lkm_sysent;
	struct sysent	lkm_oldent;	 
};

 


struct lkm_vfs {
	MODTYPE	lkm_type;
	int	lkm_ver;
	const char	*lkm_name;
	u_long	lkm_offset;
	struct  linker_set *lkm_vnodeops;
	struct	vfsconf *lkm_vfsconf;
};

 


typedef enum devtype {
	LM_DT_BLOCK,
	LM_DT_CHAR
} DEVTYPE;

 


struct lkm_dev {
	MODTYPE	lkm_type;
	int	lkm_ver;
	const char	*lkm_name;
	u_long	lkm_offset;
	DEVTYPE	lkm_devtype;
	union {
		void	*anon;
		struct cdevsw	*bdev;
		struct cdevsw	*cdev;
	} lkm_dev;
	union {
		struct cdevsw	*bdev;
		struct cdevsw	*cdev;
	} lkm_olddev;
};

 


struct lkm_strmod {
	MODTYPE	lkm_type;
	int	lkm_ver;
	const char	*lkm_name;
	u_long	lkm_offset;
	 


};

 


struct lkm_exec {
	MODTYPE	lkm_type;
	int	lkm_ver;
	const char	*lkm_name;
	u_long	lkm_offset;
	const struct execsw	*lkm_exec;
	struct execsw	lkm_oldexec;
};

 


struct lkm_misc {
	MODTYPE	lkm_type;
	int	lkm_ver;
	const char	*lkm_name;
	u_long	lkm_offset;
};

 


struct lkm_any {
	MODTYPE	lkm_type;
	int	lkm_ver;
	const char	*lkm_name;
	u_long	lkm_offset;
};


 



union lkm_generic {
	struct lkm_any		*lkm_any;
	struct lkm_syscall	*lkm_syscall;
	struct lkm_vfs		*lkm_vfs;
	struct lkm_dev		*lkm_dev;
	struct lkm_strmod	*lkm_strmod;
	struct lkm_exec		*lkm_exec;
	struct lkm_misc		*lkm_misc;
};

union lkm_all {
	struct lkm_any		lkm_any;
	struct lkm_syscall	lkm_syscall;
	struct lkm_vfs		lkm_vfs;
	struct lkm_dev		lkm_dev;
	struct lkm_strmod	lkm_strmod;
	struct lkm_exec		lkm_exec;
	struct lkm_misc		lkm_misc;
};

 


struct lkm_table {
	int	type;
	u_long	size;
	u_long	offset;
	u_long	area;
	char	used;

	int	ver;		 
	int	refcnt;		 
	int	depcnt;		 
	int	id;		 

	int	(*entry)  (struct lkm_table *, int, int)  ;
				 
	union lkm_generic	private;	 
};






 




 



















# 244 "/usr/include/sys/lkm.h" 3


# 255 "/usr/include/sys/lkm.h" 3


# 265 "/usr/include/sys/lkm.h" 3









 









# 306 "/usr/include/sys/lkm.h" 3

 





int lkmdispatch  (struct lkm_table *lkmtp, int cmd)  ;
int lkmexists	 (struct lkm_table *lkmtp)  ;
int lkm_nullcmd  (struct lkm_table *lkmtp, int cmd)  ;



 

 













 




 


struct lmc_resrv {
	u_long	size;		 
	const char	*name;		 
	int	slot;		 
	u_long	addr;		 
};


 



struct lmc_loadbuf {
	int	cnt;		 
	char	*data;		 
};


 


struct lmc_load {
	caddr_t	address;	 
	int	status;		 
	int	id;		 
};

 


struct lmc_unload {
	int	id;		 
	const char	*name;	 
	int	status;		 
};


 


struct lmc_stat {
	int	id;			 
	char	name[32 ];	 
	u_long	offset;			 
	MODTYPE	type;			 
	u_long	area;			 
	u_long	size;			 
	u_long	private;		 
	int	ver;			 
};


# 31 "driver.c" 2

# 1 "/usr/include/a.out.h" 1 3
 







































# 1 "/usr/include/sys/imgact_aout.h" 1 3
 

































































 







 
 







 



 




 



 



 



 


 





struct exec {
     unsigned long	a_midmag;	 
     unsigned long	a_text;		 
     unsigned long	a_data;		 
     unsigned long	a_bss;		 
     unsigned long	a_syms;		 
     unsigned long	a_entry;	 
     unsigned long	a_trsize;	 
     unsigned long	a_drsize;	 
};


 





 










 







struct proc;

 
int aout_coredump  (struct proc *)  ;
 



# 41 "/usr/include/a.out.h" 2 3

# 1 "/usr/include/machine/reloc.h" 1 3
 






































 
struct relocation_info {
	int r_address;			   
	unsigned int   r_symbolnum : 24,   
			   r_pcrel :  1,   
			  r_length :  2,   
			  r_extern :  1,   
			 r_baserel :  1,   
			r_jmptable :  1,   
			r_relative :  1,   
			    r_copy :  1;   
};


# 42 "/usr/include/a.out.h" 2 3



# 1 "/usr/include/nlist.h" 1 3
 












































 



 




struct nlist {

	union {
		char *n_name;	 
		long n_strx;	 
	} n_un;



	unsigned char n_type;	 
	char n_other;		 
	short n_desc;		 
	unsigned long n_value;	 
};



 










 





 







 











 
 






 
int nlist  (const char *, struct nlist *)  ;
 


# 45 "/usr/include/a.out.h" 2 3



# 32 "driver.c" 2


# 1 "/usr/include/sys/file.h" 1 3
 














































struct proc;
struct uio;

 



struct file {
	struct {	struct  file  *le_next;	struct  file  **le_prev;	}  f_list; 
	short	f_flag;		 




	short	f_type;		 
	short	f_count;	 
	short	f_msgcount;	 
	struct	ucred *f_cred;	 
	struct	fileops {
		int	(*fo_read)	 (struct file *fp, struct uio *uio,
					    struct ucred *cred)  ;
		int	(*fo_write)	 (struct file *fp, struct uio *uio,
					    struct ucred *cred)  ;
		int	(*fo_ioctl)	 (struct file *fp, u_long com,
					    caddr_t data, struct proc *p)  ;
		int	(*fo_poll)	 (struct file *fp, int events,
					    struct ucred *cred, struct proc *p)  ;
		int	(*fo_close)	 (struct file *fp, struct proc *p)  ;
	} *f_ops;
	int	f_seqcount;	 



	off_t	f_nextread;	 


	off_t	f_offset;
	caddr_t	f_data;		 
};





struct  filelist  {	struct   file  *lh_first;	} ;
extern struct filelist filehead;  
extern struct fileops vnops;
extern int maxfiles;		 
extern int maxfilesperproc;	 
extern int nfiles;		 




# 34 "driver.c" 2

# 1 "/usr/include/sys/syslog.h" 1 3
 









































 


















				 



# 90 "/usr/include/sys/syslog.h" 3


 











				 
				 
				 



	 











				 


# 152 "/usr/include/sys/syslog.h" 3






 





 














# 199 "/usr/include/sys/syslog.h" 3



# 35 "driver.c" 2

# 1 "/usr/include/vm/vm.h" 1 3
 






































typedef char vm_inherit_t;	 
typedef u_char vm_prot_t;	 

union vm_map_object;
typedef union vm_map_object vm_map_object_t;

struct vm_map_entry;
typedef struct vm_map_entry *vm_map_entry_t;

struct vm_map;
typedef struct vm_map *vm_map_t;

struct vm_object;
typedef struct vm_object *vm_object_t;

# 71 "/usr/include/vm/vm.h" 3



# 36 "driver.c" 2







# 1 "/usr/include/vm/pmap.h" 1 3
 

































































 







 





struct pmap_statistics {
	long resident_count;	 
	long wired_count;	 
};
typedef struct pmap_statistics *pmap_statistics_t;

# 1 "/usr/include/machine/pmap.h" 1 3
 

















































 



				 














 






 







 















 


















 









typedef unsigned int *pd_entry_t;
typedef unsigned int *pt_entry_t;




 




extern pt_entry_t PTmap[], APTmap[], Upte;
extern pd_entry_t PTD[], APTD[], PTDpde, APTDpde, Upde;

extern pd_entry_t IdlePTD;	 



 









 





static __inline vm_offset_t
pmap_kextract(vm_offset_t va)
{
	vm_offset_t pa;
	if ((pa = (vm_offset_t) PTD[va >> 22 ]) & 0x080 ) {
		pa = (pa & ~((1<< 22 )  - 1)) | (va & ((1<< 22 )  - 1));
	} else {
		pa = *(vm_offset_t *)(PTmap + ((unsigned)(  va  ) >> 12 ) ) ;
		pa = (pa & (~((1<< 12 ) -1) ) ) | (va & ((1<< 12 ) -1) );
	}
	return pa;
}











 


struct	pv_entry;
typedef struct {
	int pv_list_count;
	struct vm_page		*pv_vm_page;
	struct    {	struct  pv_entry  *tqh_first;	struct  pv_entry  **tqh_last;	} 	pv_list;
} pv_table_t;

struct pmap {
	pd_entry_t		*pm_pdir;	 
	vm_object_t		pm_pteobj;	 
	struct    {	struct  pv_entry  *tqh_first;	struct  pv_entry  **tqh_last;	} 	pm_pvlist;	 
	int			pm_count;	 
	int			pm_flags;	 
	struct pmap_statistics	pm_stats;	 
	struct	vm_page		*pm_ptphint;	 
};






typedef struct pmap	*pmap_t;


extern pmap_t		kernel_pmap;


 



typedef struct pv_entry {
	pmap_t		pv_pmap;	 
	vm_offset_t	pv_va;		 
	struct {	struct  pv_entry  *tqe_next;	struct  pv_entry  **tqe_prev;	} 	pv_list;
	struct {	struct  pv_entry  *tqe_next;	struct  pv_entry  **tqe_prev;	} 	pv_plist;
	vm_page_t	pv_ptem;	 
} *pv_entry_t;











struct {
	u_int64_t base, mask;
} PPro_vmtrr[8 ];

extern caddr_t	CADDR1;
extern pt_entry_t *CMAP1;
extern vm_offset_t avail_end;
extern vm_offset_t avail_start;
extern vm_offset_t clean_eva;
extern vm_offset_t clean_sva;
extern vm_offset_t phys_avail[];
extern char *ptvmmap;		 
extern vm_offset_t virtual_avail;
extern vm_offset_t virtual_end;

void	pmap_bootstrap  ( vm_offset_t, vm_offset_t)  ;
pmap_t	pmap_kernel  (void)  ;
void	*pmap_mapdev  (vm_offset_t, vm_size_t)  ;
unsigned *pmap_pte  (pmap_t, vm_offset_t)   __attribute__((__const__)) ;
vm_page_t pmap_use_pt  (pmap_t, vm_offset_t)  ;
void	pmap_set_opt	 (unsigned *)  ;
void	pmap_set_opt_bsp	 (void)  ;
void 	getmtrr  (void)  ;
void	putmtrr  (void)  ;
void	putfmtrr  (void)  ;
void	pmap_setdevram  (unsigned long long, unsigned)  ;
void	pmap_setvidram  (void)  ;






# 87 "/usr/include/vm/pmap.h" 2 3




struct proc;




void		 pmap_change_wiring  (pmap_t, vm_offset_t, boolean_t)  ;
void		 pmap_clear_modify  (vm_offset_t pa)  ;
void		 pmap_clear_reference  (vm_offset_t pa)  ;
void		 pmap_copy  (pmap_t, pmap_t, vm_offset_t, vm_size_t,
		    vm_offset_t)  ;
void		 pmap_copy_page  (vm_offset_t, vm_offset_t)  ;
void		 pmap_destroy  (pmap_t)  ;
void		 pmap_enter  (pmap_t, vm_offset_t, vm_offset_t, vm_prot_t,
		    boolean_t)  ;
vm_offset_t	 pmap_extract  (pmap_t, vm_offset_t)  ;
void		 pmap_growkernel  (vm_offset_t)  ;
void		 pmap_init  (vm_offset_t, vm_offset_t)  ;
boolean_t	 pmap_is_modified  (vm_offset_t pa)  ;
boolean_t	 pmap_ts_referenced  (vm_offset_t pa)  ;
void		 pmap_kenter  (vm_offset_t, vm_offset_t)  ;
void		 pmap_kremove  (vm_offset_t)  ;
vm_offset_t	 pmap_map  (vm_offset_t, vm_offset_t, vm_offset_t, int)  ;
void		 pmap_object_init_pt  (pmap_t pmap, vm_offset_t addr,
		    vm_object_t object, vm_pindex_t pindex, vm_offset_t size,
		    int pagelimit)  ;
boolean_t	 pmap_page_exists  (pmap_t, vm_offset_t)  ;
void		 pmap_page_protect  (vm_offset_t, vm_prot_t)  ;
void		 pmap_pageable  (pmap_t, vm_offset_t, vm_offset_t,
		    boolean_t)  ;
vm_offset_t	 pmap_phys_address  (int)  ;
void		 pmap_pinit  (pmap_t)  ;
void		 pmap_pinit0  (pmap_t)  ;
void		 pmap_protect  (pmap_t, vm_offset_t, vm_offset_t,
		    vm_prot_t)  ;
void		 pmap_qenter  (vm_offset_t, vm_page_t *, int)  ;
void		 pmap_qremove  (vm_offset_t, int)  ;
void		 pmap_reference  (pmap_t)  ;
void		 pmap_release  (pmap_t)  ;
void		 pmap_remove  (pmap_t, vm_offset_t, vm_offset_t)  ;
void		 pmap_remove_pages  (pmap_t, vm_offset_t, vm_offset_t)  ;
void		 pmap_zero_page  (vm_offset_t)  ;
void		 pmap_prefault  (pmap_t, vm_offset_t, vm_map_entry_t)  ;
int		 pmap_mincore  (pmap_t pmap, vm_offset_t addr)  ;
void		 pmap_new_proc  (struct proc *p)  ;
void		 pmap_dispose_proc  (struct proc *p)  ;
void		 pmap_swapout_proc  (struct proc *p)  ;
void		 pmap_swapin_proc  (struct proc *p)  ;
void		 pmap_activate  (struct proc *p)  ;
vm_offset_t	 pmap_addr_hint  (vm_object_t obj, vm_offset_t addr, vm_size_t size)  ;
void		pmap_init2  (void)  ;




# 43 "driver.c" 2

# 1 "/usr/include/vm/vm_map.h" 1 3
 

































































 






 







 





union vm_map_object {
	struct vm_object *vm_object;	 
	struct vm_map *share_map;	 
	struct vm_map *sub_map;		 
};

 





struct vm_map_entry {
	struct vm_map_entry *prev;	 
	struct vm_map_entry *next;	 
	vm_offset_t start;		 
	vm_offset_t end;		 
	union vm_map_object object;	 
	vm_ooffset_t offset;		 
	u_char eflags;			 
	 
	vm_prot_t protection;		 
	vm_prot_t max_protection;	 
	vm_inherit_t inheritance;	 
	int wired_count;		 
};








 





struct vm_map {
	struct lock lock;		 
	struct vm_map_entry header;	 
	int nentries;			 
	vm_size_t size;			 
	unsigned char	is_main_map;		 
	unsigned char	system_map;			 
	vm_map_entry_t hint;		 
	unsigned int timestamp;		 
	vm_map_entry_t first_free;	 
	struct pmap *pmap;		 


};

 




struct vmspace {
	struct vm_map vm_map;	 
	struct pmap vm_pmap;	 
	int vm_refcnt;		 
	caddr_t vm_shm;		 
 

	segsz_t vm_rssize;	 
	segsz_t vm_swrss;	 
	segsz_t vm_tsize;	 
	segsz_t vm_dsize;	 
	segsz_t vm_ssize;	 
	caddr_t vm_taddr;	 
	caddr_t vm_daddr;	 
	caddr_t vm_maxsaddr;	 
	caddr_t vm_minsaddr;	 
};


 








typedef struct {
	int main_timestamp;
	vm_map_t share_map;
	int share_timestamp;
} vm_map_version_t;

 











# 212 "/usr/include/vm/vm_map.h" 3







# 235 "/usr/include/vm/vm_map.h" 3









static __inline__ int
_vm_map_lock_upgrade(vm_map_t map, struct proc *p) {



	return lockmgr(&(map)->lock, 0x00000004 , (void *)0, p);
}

























 






 




 






 










extern vm_offset_t kentry_data;
extern vm_size_t kentry_data_size;

boolean_t vm_map_check_protection  (vm_map_t, vm_offset_t, vm_offset_t, vm_prot_t)  ;
int vm_map_copy  (vm_map_t, vm_map_t, vm_offset_t, vm_size_t, vm_offset_t, boolean_t, boolean_t)  ;
struct pmap;
vm_map_t vm_map_create  (struct pmap *, vm_offset_t, vm_offset_t)  ;
void vm_map_deallocate  (vm_map_t)  ;
int vm_map_delete  (vm_map_t, vm_offset_t, vm_offset_t)  ;
int vm_map_find  (vm_map_t, vm_object_t, vm_ooffset_t, vm_offset_t *, vm_size_t, boolean_t, vm_prot_t, vm_prot_t, int)  ;
int vm_map_findspace  (vm_map_t, vm_offset_t, vm_size_t, vm_offset_t *)  ;
int vm_map_inherit  (vm_map_t, vm_offset_t, vm_offset_t, vm_inherit_t)  ;
void vm_map_init  (struct vm_map *, vm_offset_t, vm_offset_t)  ;
int vm_map_insert  (vm_map_t, vm_object_t, vm_ooffset_t, vm_offset_t, vm_offset_t, vm_prot_t, vm_prot_t, int)  ;
int vm_map_lookup  (vm_map_t *, vm_offset_t, vm_prot_t, vm_map_entry_t *, vm_object_t *,
    vm_pindex_t *, vm_prot_t *, boolean_t *)  ;
void vm_map_lookup_done  (vm_map_t, vm_map_entry_t)  ;
boolean_t vm_map_lookup_entry  (vm_map_t, vm_offset_t, vm_map_entry_t *)  ;
int vm_map_pageable  (vm_map_t, vm_offset_t, vm_offset_t, boolean_t)  ;
int vm_map_user_pageable  (vm_map_t, vm_offset_t, vm_offset_t, boolean_t)  ;
int vm_map_clean  (vm_map_t, vm_offset_t, vm_offset_t, boolean_t, boolean_t)  ;
int vm_map_protect  (vm_map_t, vm_offset_t, vm_offset_t, vm_prot_t, boolean_t)  ;
void vm_map_reference  (vm_map_t)  ;
int vm_map_remove  (vm_map_t, vm_offset_t, vm_offset_t)  ;
void vm_map_simplify  (vm_map_t, vm_offset_t)  ;
void vm_map_startup  (void)  ;
int vm_map_submap  (vm_map_t, vm_offset_t, vm_offset_t, vm_map_t)  ;
void vm_map_madvise  (vm_map_t, pmap_t, vm_offset_t, vm_offset_t, int)  ;
void vm_map_simplify_entry  (vm_map_t, vm_map_entry_t)  ;
void vm_init2  (void)  ;
int vm_uiomove  (vm_map_t, vm_object_t, off_t, int, vm_offset_t, int *)  ;
void vm_freeze_copyopts  (vm_object_t, vm_pindex_t, vm_pindex_t)  ;



# 44 "driver.c" 2




# 1 "/usr/include/vm/vm_prot.h" 1 3
 

































































 






 










 





 






# 48 "driver.c" 2

# 1 "/usr/include/vm/vm_object.h" 1 3
 

































































 







# 1 "/usr/include/machine/atomic.h" 1 3
 





























 



























# 75 "/usr/include/vm/vm_object.h" 2 3


enum obj_type { OBJT_DEFAULT, OBJT_SWAP, OBJT_VNODE, OBJT_DEVICE, OBJT_DEAD };
typedef enum obj_type objtype_t;

 





struct vm_object {
	struct {	struct  vm_object  *tqe_next;	struct  vm_object  **tqe_prev;	}  object_list;  
	struct    {	struct   vm_object  *tqh_first;	struct   vm_object  **tqh_last;	}  shadow_head;  
	struct {	struct  vm_object  *tqe_next;	struct  vm_object  **tqe_prev;	}  shadow_list;  
	struct    {	struct   vm_page  *tqh_first;	struct   vm_page  **tqh_last;	}  memq;	 
	int generation;			 
	objtype_t type;			 
	vm_size_t size;			 
	int ref_count;			 
	int shadow_count;		 
	int pg_color;			 
	int	id;					 
	u_short flags;			 
	u_short paging_in_progress;	 
	u_short	behavior;		 
	int resident_page_count;	 
	int cache_count;			 
	int	wire_count;			 
	vm_ooffset_t paging_offset;	 
	struct vm_object *backing_object;  
	vm_ooffset_t backing_object_offset; 
	vm_offset_t last_read;		 
	vm_page_t page_hint;		 
	struct {	struct  vm_object  *tqe_next;	struct  vm_object  **tqe_prev;	}  pager_object_list;  
	void *handle;
	union {
		struct {
			off_t vnp_size;  
		} vnp;
		struct {
			struct    {	struct   vm_page  *tqh_first;	struct   vm_page  **tqh_last;	}  devp_pglist;  
		} devp;
		struct {
			int swp_nblocks;
			int swp_allocsize;
			struct swblock *swp_blocks;
			short swp_poip;
		} swp;
	} un_pager;
};

 
























struct  object_q  {	struct   vm_object  *tqh_first;	struct   vm_object  **tqh_last;	} ;

extern struct object_q vm_object_list;	 

  

extern vm_object_t kernel_object;	 
extern vm_object_t kmem_object;





static __inline void
vm_object_set_flag(vm_object_t object, u_int bits)
{
	(*(u_short*)( &object->flags ) |= (  bits )) ;
}

static __inline void
vm_object_clear_flag(vm_object_t object, u_int bits)
{
	(*(u_short*)( &object->flags ) &= ~(  bits )) ;
}

static __inline void
vm_object_pip_add(vm_object_t object, int i)
{
	(*(u_short*)( &object->paging_in_progress ) += (  i )) ;
}

static __inline void
vm_object_pip_subtract(vm_object_t object, int i)
{
	(*(u_short*)( &object->paging_in_progress ) -= (  i )) ;
}

static __inline void
vm_object_pip_wakeup(vm_object_t object)
{
	(*(u_short*)( &object->paging_in_progress ) -= (  1 )) ;
	if ((object->flags & 0x0040 ) && object->paging_in_progress == 0) {
		vm_object_clear_flag(object, 0x0040 );
		wakeup(object);
	}
}

static __inline void
vm_object_pip_sleep(vm_object_t object, char *waitid)
{
	int s;

	if (object->paging_in_progress) {
		s = splvm();
		if (object->paging_in_progress) {
			vm_object_set_flag(object, 0x0040 );
			tsleep(object, 4 , waitid, 0);
		}
		splx(s);
	}
}

static __inline void
vm_object_pip_wait(vm_object_t object, char *waitid)
{
	while (object->paging_in_progress)
		vm_object_pip_sleep(object, waitid);
}

vm_object_t vm_object_allocate  (objtype_t, vm_size_t)  ;
void _vm_object_allocate  (objtype_t, vm_size_t, vm_object_t)  ;
boolean_t vm_object_coalesce  (vm_object_t, vm_pindex_t, vm_size_t, vm_size_t)  ;
void vm_object_collapse  (vm_object_t)  ;
void vm_object_copy  (vm_object_t, vm_pindex_t, vm_object_t *, vm_pindex_t *, boolean_t *)  ;
void vm_object_deallocate  (vm_object_t)  ;
void vm_object_terminate  (vm_object_t)  ;
void vm_object_vndeallocate  (vm_object_t)  ;
void vm_object_init  (void)  ;
void vm_object_page_clean  (vm_object_t, vm_pindex_t, vm_pindex_t, boolean_t)  ;
void vm_object_page_remove  (vm_object_t, vm_pindex_t, vm_pindex_t, boolean_t)  ;
void vm_object_pmap_copy  (vm_object_t, vm_pindex_t, vm_pindex_t)  ;
void vm_object_pmap_copy_1  (vm_object_t, vm_pindex_t, vm_pindex_t)  ;
void vm_object_pmap_remove  (vm_object_t, vm_pindex_t, vm_pindex_t)  ;
void vm_object_reference  (vm_object_t)  ;
void vm_object_shadow  (vm_object_t *, vm_ooffset_t *, vm_size_t)  ;
void vm_object_madvise  (vm_object_t, vm_pindex_t, int, int)  ;
void vm_object_init2  (void)  ;



# 49 "driver.c" 2


# 1 "/usr/include/sys/kernel.h" 1 3
 
















































 

 
extern long hostid;
extern char hostname[256 ];
extern int hostnamelen;
extern char domainname[256 ];
extern int domainnamelen;
extern char kernelname[1024  ];

 
extern struct timeval boottime;

extern struct timezone tz;			 

extern int tick;			 
extern int tickadj;			 
extern int hz;				 
extern int psratio;			 
extern int stathz;			 
extern int profhz;			 
extern int ticks;
extern int lbolt;			 
extern int tickdelta;
extern long timedelta;



 





# 110 "/usr/include/sys/kernel.h" 3


 















 























enum sysinit_sub_id {
	SI_SUB_DUMMY		= 0x0000000,	 
	SI_SUB_DONE		= 0x0000001,	 
	SI_SUB_CONSOLE		= 0x0800000,	 
	SI_SUB_COPYRIGHT	= 0x0800001,	 
	SI_SUB_VM		= 0x1000000,	 
	SI_SUB_KMEM		= 0x1800000,	 
	SI_SUB_CPU		= 0x2000000,	 
	SI_SUB_KLD		= 0x2100000,	 
	SI_SUB_DEVFS		= 0x2200000,	 
	SI_SUB_DRIVERS		= 0x2300000,	 
	SI_SUB_CONFIGURE	= 0x2400000,	 
	SI_SUB_INTRINSIC	= 0x2800000,	 
	SI_SUB_RUN_QUEUE	= 0x3000000,	 
	SI_SUB_VM_CONF		= 0x3800000,	 
	SI_SUB_VFS		= 0x4000000,	 
	SI_SUB_CLOCKS		= 0x4800000,	 
	SI_SUB_MBUF		= 0x5000000,	 
	SI_SUB_CLIST		= 0x5800000,	 
	SI_SUB_SYSV_SHM		= 0x6400000,	 
	SI_SUB_SYSV_SEM		= 0x6800000,	 
	SI_SUB_SYSV_MSG		= 0x6C00000,	 
	SI_SUB_P1003_1B		= 0x6E00000,	 
	SI_SUB_PSEUDO		= 0x7000000,	 
	SI_SUB_EXEC		= 0x7400000,	 
	SI_SUB_PROTO_BEGIN	= 0x8000000,	 
	SI_SUB_PROTO_IF		= 0x8400000,	 
	SI_SUB_PROTO_DOMAIN	= 0x8800000,	 
	SI_SUB_PROTO_END	= 0x8ffffff,	 
	SI_SUB_KPROF		= 0x9000000,	 
	SI_SUB_KICK_SCHEDULER	= 0xa000000,	 
	SI_SUB_INT_CONFIG_HOOKS	= 0xa800000,	 
	SI_SUB_ROOT_CONF	= 0xb000000,	 
	SI_SUB_DUMP_CONF	= 0xb200000,	 
	SI_SUB_MOUNT_ROOT	= 0xb400000,	 
	SI_SUB_ROOT_FDTAB	= 0xb800000,	 
	SI_SUB_SWAP		= 0xc000000,	 
	SI_SUB_INTRINSIC_POST	= 0xd000000,	 
	SI_SUB_KTHREAD_INIT	= 0xe000000,	 
	SI_SUB_KTHREAD_PAGE	= 0xe400000,	 
	SI_SUB_KTHREAD_VM	= 0xe800000,	 
	SI_SUB_KTHREAD_UPDATE	= 0xec00000,	 
	SI_SUB_KTHREAD_IDLE	= 0xee00000,	 
	SI_SUB_SMP		= 0xf000000,	 
	SI_SUB_RUN_SCHEDULER	= 0xfffffff	 
};


 


enum sysinit_elem_order {
	SI_ORDER_FIRST		= 0x0000000,	 
	SI_ORDER_SECOND		= 0x0000001,	 
	SI_ORDER_THIRD		= 0x0000002,	 
	SI_ORDER_MIDDLE		= 0x1000000,	 
	SI_ORDER_ANY		= 0xfffffff	 
};


 




typedef enum sysinit_elem_type {
	SI_TYPE_DEFAULT		= 0x00000000,	 
	SI_TYPE_KTHREAD		= 0x00000001,	 
	SI_TYPE_KPROCESS	= 0x00000002	 
} si_elem_t;


 




struct sysinit {
	unsigned int	subsystem;		 
	unsigned int	order;			 
	void		(*func)  (void *)  ;	 
	void		*udata;			 
	si_elem_t	type;			 
};


 



# 250 "/usr/include/sys/kernel.h" 3

 




# 264 "/usr/include/sys/kernel.h" 3


# 274 "/usr/include/sys/kernel.h" 3


 




struct kproc_desc {
	char		*arg0;			 
	void		(*func)  (void)  ;	 
	struct proc	**global_procpp;	 
};

void	kproc_start  (void *udata)  ;
void	sysinit_add  (struct sysinit **set)  ;

# 310 "/usr/include/sys/kernel.h" 3


 




# 338 "/usr/include/sys/kernel.h" 3



struct linker_set {
	int		ls_length;
	const void	*ls_items[1];		 

};

extern struct linker_set execsw_set;


# 51 "driver.c" 2






# 1 "/usr/include/sys/uio.h" 1 3
 






































 



struct iovec {
	char	*iov_base;	 
	size_t	 iov_len;	 
};

enum	uio_rw { UIO_READ, UIO_WRITE };

 
enum uio_seg {
	UIO_USERSPACE,		 
	UIO_SYSSPACE,		 
	UIO_USERISPACE,		 
	UIO_NOCOPY		 
};



struct uio {
	struct	iovec *uio_iov;
	int	uio_iovcnt;
	off_t	uio_offset;
	int	uio_resid;
	enum	uio_seg uio_segflg;
	enum	uio_rw uio_rw;
	struct	proc *uio_procp;
};

 





struct vm_object;

int	uiomove  (caddr_t, int, struct uio *)  ;
int	uiomoveco  (caddr_t, int, struct uio *, struct vm_object *)  ;
int	uioread  (int, struct uio *, struct vm_object *, int *)  ;

# 92 "/usr/include/sys/uio.h" 3



# 57 "driver.c" 2



# 1 "/sys/i386/isa/isa_device.h" 1
 






































 



 







struct isa_device {
	int	id_id;		 
	struct	isa_driver *id_driver;
	int	id_iobase;	 
	u_int	id_irq;		 
	int	id_drq;		 
	caddr_t id_maddr;	 
	int	id_msize;	 
	inthand2_t *id_intr;	 
	int	id_unit;	 
	int	id_flags;	 
	int	id_scsiid;	 
	int	id_alive;	 

	u_int	id_ri_flags;	 
	int	id_reconfig;	 
	int	id_enabled;	 
	int	id_conflicts;	 
	struct isa_device *id_next;  
};

 









 






struct isa_driver {
	int	(*probe)  (struct isa_device *idp)  ;
					 
	int	(*attach)  (struct isa_device *idp)  ;
					 
	char	*name;			 
	int	sensitive_hw;		 
};



extern struct isa_device isa_biotab_fdc[];
extern struct isa_device isa_biotab_wdc[];
extern struct isa_device isa_devtab_bio[];
extern struct isa_device isa_devtab_net[];
extern struct isa_device isa_devtab_cam[];
extern struct isa_device isa_devtab_null[];
extern struct isa_device isa_devtab_tty[];

struct isa_device *
	find_display  (void)  ;
struct isa_device *
	find_isadev  (struct isa_device *table, struct isa_driver *driverp,
			 int unit)  ;
int	haveseen_isadev  (struct isa_device *dvp, u_int checkbits)  ;
void	isa_configure  (void)  ;
void	isa_dmacascade  (int chan)  ;
void	isa_dmadone  (int flags, caddr_t addr, int nbytes, int chan)  ;
void	isa_dmainit  (int chan, u_int bouncebufsize)  ;
void	isa_dmastart  (int flags, caddr_t addr, u_int nbytes, int chan)  ;
int	isa_dma_acquire  (int chan)  ;
void	isa_dma_release  (int chan)  ;
int	isa_dmastatus  (int chan)  ;
int	isa_dmastop  (int chan)  ;
void	reconfig_isadev  (struct isa_device *isdp, u_int *mp)  ;

typedef	void	ointhand2_t  (int unit)  ;

 









ointhand2_t 	adintr;
ointhand2_t 	ahaintr;
ointhand2_t 	aicintr;
ointhand2_t 	alogintr;
ointhand2_t 	arintr;
ointhand2_t 	ascintr;



ointhand2_t 	csintr;
ointhand2_t 	cxintr;
ointhand2_t 	cyintr;
ointhand2_t 	edintr;
ointhand2_t 	egintr;
ointhand2_t 	elintr;
ointhand2_t 	epintr;
ointhand2_t 	exintr;
ointhand2_t 	fdintr;
ointhand2_t 	feintr;
ointhand2_t 	gusintr;
ointhand2_t 	ieintr;
ointhand2_t 	labpcintr;
ointhand2_t 	le_intr;
ointhand2_t 	lncintr;
ointhand2_t 	loranintr;
ointhand2_t 	lptintr;
ointhand2_t 	m6850intr;
ointhand2_t 	mcdintr;
ointhand2_t 	mseintr;
ointhand2_t 	ncaintr;
ointhand2_t 	npxintr;
ointhand2_t 	pasintr;
ointhand2_t 	pcmintr;
ointhand2_t 	pcrint;
ointhand2_t 	ppcintr;
ointhand2_t 	pcfintr;
ointhand2_t 	psmintr;
ointhand2_t 	rcintr;
ointhand2_t 	sbintr;
ointhand2_t 	scintr;
ointhand2_t 	seaintr;
ointhand2_t 	siointr;
ointhand2_t 	sndintr;
ointhand2_t 	spigintr;
ointhand2_t 	srintr;
ointhand2_t 	sscapeintr;
ointhand2_t 	stlintr;
ointhand2_t 	twintr;
ointhand2_t 	uhaintr;
ointhand2_t 	wdintr;
ointhand2_t 	wdsintr;
ointhand2_t 	wlintr;
ointhand2_t 	wtintr;
ointhand2_t 	zeintr;
ointhand2_t 	zpintr;






# 60 "driver.c" 2

# 1 "/sys/pci/pcivar.h" 1
 



































# 1 "/sys/pci/pci_ioctl.h" 1



# 1 "/usr/include/sys/ioccom.h" 1 3
 






































 





















 


# 73 "/usr/include/sys/ioccom.h" 3



# 4 "/sys/pci/pci_ioctl.h" 2





typedef enum {
    PCI_GETCONF_LAST_DEVICE,
    PCI_GETCONF_LIST_CHANGED,
    PCI_GETCONF_MORE_DEVS,
    PCI_GETCONF_ERROR
} pci_getconf_status;

typedef enum {
    PCI_GETCONF_NO_MATCH	= 0x00,
    PCI_GETCONF_MATCH_BUS	= 0x01,
    PCI_GETCONF_MATCH_DEV	= 0x02,
    PCI_GETCONF_MATCH_FUNC	= 0x04,
    PCI_GETCONF_MATCH_NAME	= 0x08,
    PCI_GETCONF_MATCH_UNIT	= 0x10,
    PCI_GETCONF_MATCH_VENDOR	= 0x20,
    PCI_GETCONF_MATCH_DEVICE	= 0x40,
    PCI_GETCONF_MATCH_CLASS	= 0x80
} pci_getconf_flags;

struct pcisel {
    u_int8_t		pc_bus;		 
    u_int8_t		pc_dev;		 
    u_int8_t		pc_func;	 
};

struct	pci_conf {
    struct pcisel	pc_sel;		 
    u_int8_t		pc_hdr;		 
    u_int16_t		pc_subvendor;	 
    u_int16_t		pc_subdevice;	 

    u_int16_t		pc_vendor;	 
    u_int16_t		pc_device;	 

    u_int8_t		pc_class;	 
    u_int8_t		pc_subclass;	 
    u_int8_t		pc_progif;	 
    u_int8_t		pc_revid;	 
    char		pd_name[16  + 1];   

    u_long		pd_unit;	 
};

struct pci_match_conf {
    struct pcisel	pc_sel;		 
    char		pd_name[16  + 1];   

    u_long		pd_unit;	 
    u_int16_t		pc_vendor;	 
    u_int16_t		pc_device;	 
    u_int8_t		pc_class;	 
    pci_getconf_flags	flags;		 
};

struct	pci_conf_io {
    u_int32_t		  pat_buf_len;	 



    u_int32_t		  num_patterns;  



    struct pci_match_conf *patterns;	 


    u_int32_t		  match_buf_len; 



    u_int32_t		  num_matches;	 



    struct pci_conf	  *matches;	 



    u_int32_t		  offset;	 










    u_int32_t		  generation;	 










    pci_getconf_status	  status;	 



};

struct pci_io {
    struct pcisel	pi_sel;		 
    int			pi_reg;		 
    int			pi_width;	 
    u_int32_t		pi_data;	 
};
	







# 37 "/sys/pci/pcivar.h" 2



 










 




typedef u_int32_t pci_addr_t;	 


 

typedef struct {
    u_int32_t	base;
    u_int8_t	type;



    u_int8_t	ln2size;
    u_int8_t	ln2range;
 
} pcimap;

 

typedef struct pcicfg {
    pcimap	*map;		 
    void	*hdrspec;	 

    u_int16_t	subvendor;	 
    u_int16_t	subdevice;	 
    u_int16_t	vendor;		 
    u_int16_t	device;		 

    u_int16_t	cmdreg;		 
    u_int16_t	statreg;	 

    u_int8_t	baseclass;	 
    u_int8_t	subclass;	 
    u_int8_t	progif;		 
    u_int8_t	revid;		 

    u_int8_t	hdrtype;	 
    u_int8_t	cachelnsz;	 
    u_int8_t	intpin;		 
    u_int8_t	intline;	 

    u_int8_t	mingnt;		 
    u_int8_t	maxlat;		 
    u_int8_t	lattimer;	 

    u_int8_t	mfdev;		 
    u_int8_t	nummaps;	 

    u_int8_t	bus;		 
    u_int8_t	slot;		 
    u_int8_t	func;		 

    u_int8_t	secondarybus;	 
    u_int8_t	subordinatebus;	 
} pcicfgregs;

 












typedef struct {
    pci_addr_t	pmembase;	 
    pci_addr_t	pmemlimit;	 
    u_int32_t	membase;	 
    u_int32_t	memlimit;	 
    u_int32_t	iobase;		 
    u_int32_t	iolimit;	 
    u_int16_t	secstat;	 
    u_int16_t	bridgectl;	 
    u_int8_t	seclat;		 
} pcih1cfgregs;

 

typedef struct {
    u_int32_t	membase0;	 
    u_int32_t	memlimit0;	 
    u_int32_t	membase1;	 
    u_int32_t	memlimit1;	 
    u_int32_t	iobase0;	 
    u_int32_t	iolimit0;	 
    u_int32_t	iobase1;	 
    u_int32_t	iolimit1;	 
    u_int32_t	pccardif;	 
    u_int16_t	secstat;	 
    u_int16_t	bridgectl;	 
    u_int8_t	seclat;		 
} pcih2cfgregs;

 

typedef struct pciattach {
    int		unit;
    int		pcibushigh;
    struct pciattach *next;
} pciattach;

struct pci_devinfo {
    	struct {	struct  pci_devinfo  *stqe_next;	}  pci_links;
	struct pci_device	*device;   
	pcicfgregs		cfg;
	struct pci_conf		conf;
};

extern u_int32_t pci_numdevs;


 

int pci_probe (pciattach *attach);
void pci_drvattach(struct pci_devinfo *dinfo);

 

int pci_cfgopen (void);
int pci_cfgread (pcicfgregs *cfg, int reg, int bytes);
void pci_cfgwrite (pcicfgregs *cfg, int reg, int data, int bytes);




 



typedef pcicfgregs *pcici_t;
typedef unsigned pcidi_t;
typedef void pci_inthand_t(void *arg);



 

extern struct linker_set pcidevice_set;
extern int pci_mechanism;

struct pci_device {
    char*    pd_name;
    char*  (*pd_probe ) (pcici_t tag, pcidi_t type);
    void   (*pd_attach) (pcici_t tag, int     unit);
    u_long  *pd_count;
    int    (*pd_shutdown) (int, int);
};

struct pci_lkm {
	struct pci_device *dvp;
	struct pci_lkm	*next;
};


typedef u_short pci_port_t;




u_long pci_conf_read (pcici_t tag, u_long reg);
void pci_conf_write (pcici_t tag, u_long reg, u_long data);
void pci_configure (void);
int pci_map_port (pcici_t tag, u_long reg, pci_port_t* pa);
int pci_map_mem (pcici_t tag, u_long reg, vm_offset_t* va, vm_offset_t* pa);
int pci_map_dense (pcici_t tag, u_long reg, vm_offset_t* va, vm_offset_t* pa);
int pci_map_bwx (pcici_t tag, u_long reg, vm_offset_t* va, vm_offset_t* pa);
int pci_map_int (pcici_t tag, pci_inthand_t *func, void *arg, unsigned *maskptr);
int pci_unmap_int (pcici_t tag);
int pci_register_lkm (struct pci_device *dvp, int if_revision);



# 61 "driver.c" 2

# 1 "/sys/pci/pcireg.h" 1



 




























 









 






 




















 










 


























 
























 




























































































 






 























# 62 "driver.c" 2





# 1 "../MPC_OS/mpcshare.h" 1




 




















typedef u_short seq_t;

 

enum { HSL_PROTO_PUT,
       HSL_PROTO_SLRP_P, HSL_PROTO_SCP_P,
       HSL_PROTO_SLRP_V, HSL_PROTO_SCP_V,
       HSL_PROTO_MDCP };

extern char **environ;

typedef u_short pnode_t;
typedef u_short channel_t;



 
 
 
















 
extern char *__progname;

typedef u_long appclassname_t;

typedef unsigned long long int primary_t;

typedef union _appsubclassname {
  u_short is_raw;  
  struct {
    u_short is_raw;  
    u_short secondary;
    primary_t primary;
  } raw;
  struct {
    u_short prefnode_cluster;  
    pnode_t prefnode_pnode;
    primary_t value;
  } controlled;
} appsubclassname_t;







 






typedef struct _mpcmsg_get_channel {
  appclassname_t mainclass;       
  appsubclassname_t subclass;     
  int protocol;                   
  channel_t channel_pair_0;       
  channel_t channel_pair_1;       
  int status;                     
} mpcmsg_get_channel_t;


typedef struct _mpcmsg_close_channel {
  appclassname_t mainclass;       
  appsubclassname_t subclass;     
  int status;                     
} mpcmsg_close_channel_t;


typedef struct _mpcmsg_get_local_infos {
  u_short cluster;                
  pnode_t pnode;                  
  int nclusters;                  
} mpcmsg_get_local_infos_t;


typedef struct _mpcmsg_get_node_count {
  u_short cluster;                
  int node_count;                 
} mpcmsg_get_node_count_t;



typedef struct _mpcmsg_spawn_task {
  char cmdline[256 ];     
  u_short cluster;                
  pnode_t pnode;                  
  appclassname_t mainclass;       
  int status;                     
} mpcmsg_spawn_task_t;


typedef struct _mpcmsg_test {
  int val1;
  int val2;
} mpcmsg_test_t;


# 152 "../MPC_OS/mpcshare.h"


typedef struct _mpcmsg_myname {
  char name[32];                  
} mpcmsg_myname_t;


 

 


typedef struct _mpc_chan_set {
  pnode_t   dest;
  channel_t channel;
  int   is_set;  
   








  int data_ready;
} mpc_chan_set_t;

typedef struct {
  int max_index;  
  mpc_chan_set_t chan_set[20 ];
} mpc_chan_set;



# 67 "driver.c" 2


# 1 "data.h" 1

 




# 1 "/usr/include/vm/vm_param.h" 1 3
 

































































 






# 1 "/usr/include/machine/vmparam.h" 1 3
 












































 





 





















 






 











 
























 




 








 






 





# 74 "/usr/include/vm/vm_param.h" 2 3


 





# 99 "/usr/include/vm/vm_param.h" 3



extern vm_size_t page_mask;
extern int page_shift;



 
















# 137 "/usr/include/vm/vm_param.h" 3

 

















extern vm_size_t mem_size;	 
extern vm_offset_t first_addr;	 
extern vm_offset_t last_addr;	 



# 7 "data.h" 2


# 1 "/usr/include/stddef.h" 1 3
 







































typedef	int 	ptrdiff_t;



typedef	int  	rune_t;










typedef	int  	wchar_t;










# 9 "data.h" 2






# 1 "/usr/include/sys/proc.h" 1 3
 











































# 1 "/usr/include/machine/proc.h" 1 3
 






































 


struct mdproc {
	struct trapframe *md_regs;	 
};


# 45 "/usr/include/sys/proc.h" 2 3


# 1 "/usr/include/sys/rtprio.h" 1 3
 



































 



 





 








 



 






struct rtprio {
	u_short type;
	u_short prio;
};










# 47 "/usr/include/sys/proc.h" 2 3

# 1 "/usr/include/sys/select.h" 1 3
 






































 



struct selinfo {
	pid_t	si_pid;		 
	short	si_flags;	 
};



struct proc;

void	selrecord  (struct proc *selector, struct selinfo *)  ;
void	selwakeup  (struct selinfo *)  ;



# 48 "/usr/include/sys/proc.h" 2 3








 


struct	session {
	int	s_count;		 
	struct	proc *s_leader;		 
	struct	vnode *s_ttyvp;		 
	struct	tty *s_ttyp;		 
	char	s_login[(((( 17  )+((  sizeof(long) )-1))/(  sizeof(long) ))*(  sizeof(long) )) ];	 
};

 


struct	pgrp {
	struct {	struct  pgrp  *le_next;	struct  pgrp  **le_prev;	}  pg_hash;	 
	struct    {	struct   proc  *lh_first;	}  pg_members;	 
	struct	session *pg_session;	 
	pid_t	pg_id;			 
	int	pg_jobc;	 
};

 










struct	proc {
	struct {	struct  proc  *tqe_next;	struct  proc  **tqe_prev;	}  p_procq;	 
	struct {	struct  proc  *le_next;	struct  proc  **le_prev;	}  p_list;	 

	 
	struct	pcred *p_cred;		 
	struct	filedesc *p_fd;		 
	struct	pstats *p_stats;	 
	struct	plimit *p_limit;	 
	struct	vm_object *p_upages_obj; 
	struct	sigacts *p_sigacts;	 




	int	p_flag;			 
	char	p_stat;			 
	char	p_pad1[3];

	pid_t	p_pid;			 
	struct {	struct  proc  *le_next;	struct  proc  **le_prev;	}  p_hash;	 
	struct {	struct  proc  *le_next;	struct  proc  **le_prev;	}  p_pglist;	 
	struct	proc *p_pptr;	 	 
	struct {	struct  proc  *le_next;	struct  proc  **le_prev;	}  p_sibling;	 
	struct    {	struct   proc  *lh_first;	}  p_children;	 

	struct callout_handle p_ithandle;  



 


	pid_t	p_oppid;	  
	int	p_dupfd;	  

	struct	vmspace *p_vmspace;	 

	 
	u_int	p_estcpu;	  
	int	p_cpticks;	  
	fixpt_t	p_pctcpu;	  
	void	*p_wchan;	  
	const char *p_wmesg;	  
	u_int	p_swtime;	  
	u_int	p_slptime;	  

	struct	itimerval p_realtimer;	 
	u_int64_t	p_runtime;	 
	struct	timeval p_switchtime;	 
	u_quad_t p_uticks;		 
	u_quad_t p_sticks;		 
	u_quad_t p_iticks;		 

	int	p_traceflag;		 
	struct	vnode *p_tracep;	 

	int	p_siglist;		 

	struct	vnode *p_textvp;	 

	char	p_lock;			 
	char	p_oncpu;		 
	char	p_lastcpu;		 
	char	p_pad2;			 

	short	p_locks;		 
	short	p_simple_locks;		 
	unsigned int	p_stops;	 
	unsigned int	p_stype;	 
	char	p_step;			 
	unsigned char	p_pfsflags;	 
	char	p_pad3[2];		 
	register_t p_retval[2];		 

 


 


	sigset_t p_sigmask;	 
	sigset_t p_sigignore;	 
	sigset_t p_sigcatch;	 

	u_char	p_priority;	 
	u_char	p_usrpri;	 
	char	p_nice;		 
	char	p_comm[16 +1];

	struct 	pgrp *p_pgrp;	 

	struct 	sysentvec *p_sysent;  

	struct	rtprio p_rtprio;	 
 

	struct	user *p_addr;	 
	struct	mdproc p_md;	 

	u_short	p_xstat;	 
	u_short	p_acflag;	 
	struct	rusage *p_ru;	 

	int	p_nthreads;	 
	void	*p_aioinfo;	 
	int	p_wakeup;	 
	struct proc *p_peers;	
	struct proc *p_leader;
};




 






 
















 



 





 





 






struct	pcred {
	struct	ucred *pc_ucred;	 
	uid_t	p_ruid;			 
	uid_t	p_svuid;		 
	gid_t	p_rgid;			 
	gid_t	p_svgid;		 
	int	p_refcnt;		 
};








 













extern void stopevent(struct proc*, unsigned int, unsigned int);



 







extern struct  pidhashhead  {	struct   proc  *lh_first;	}  *pidhashtbl;
extern u_long pidhash;


extern struct  pgrphashhead  {	struct   pgrp  *lh_first;	}  *pgrphashtbl;
extern u_long pgrphash;

extern struct proc *curproc;		 
extern struct proc proc0;		 
extern int nprocs, maxproc;		 
extern int maxprocperuid;		 
extern struct timeval switchtime;	 

struct  proclist  {	struct   proc  *lh_first;	} ;
extern struct proclist allproc;		 
extern struct proclist zombproc;	 
extern struct proc *initproc, *pageproc;  


extern struct prochd qs[];
extern struct prochd rtqs[];
extern struct prochd idqs[];
extern int	whichqs;	 
extern int	whichrtqs;	 
extern int	whichidqs;	 
struct	prochd {
	struct	proc *ph_link;		 
	struct	proc *ph_rlink;
};

struct proc *pfind  (pid_t)  ;	 
struct pgrp *pgfind  (pid_t)  ;	 

struct vm_zone;
extern struct vm_zone *proc_zone;

int	chgproccnt  (uid_t uid, int diff)  ;
int	enterpgrp  (struct proc *p, pid_t pgid, int mksess)  ;
void	fixjobc  (struct proc *p, struct pgrp *pgrp, int entering)  ;
int	inferior  (struct proc *p)  ;
int	leavepgrp  (struct proc *p)  ;
void	mi_switch  (void)  ;
void	procinit  (void)  ;
void	resetpriority  (struct proc *)  ;
int	roundrobin_interval  (void)  ;
void	setrunnable  (struct proc *)  ;
void	setrunqueue  (struct proc *)  ;
void	sleepinit  (void)  ;
void	remrq  (struct proc *)  ;
void	cpu_switch  (struct proc *)  ;
void	unsleep  (struct proc *)  ;
void	wakeup_one  (void *chan)  ;

void	cpu_exit  (struct proc *)   __attribute__((__noreturn__)) ;
void	exit1  (struct proc *, int)   __attribute__((__noreturn__)) ;
void	cpu_fork  (struct proc *, struct proc *)  ;
int		fork1  (struct proc *, int)  ;
int	trace_req  (struct proc *)  ;
void	cpu_wait  (struct proc *)  ;
int	cpu_coredump  (struct proc *, struct vnode *, struct ucred *)  ;
void		setsugid  (struct proc *p)  ;



# 15 "data.h" 2
















































 


















extern char debug_stage [128 ];
extern char debug_stage2[128 ];

 








                       










 













 
 
typedef u_short node_t;  
typedef u_long  mi_t;





 





 



























typedef enum { INVALID, VALID, LPE } valid_t;

 



 












 








 











 




typedef struct _prot_range {
  vm_offset_t start;
  size_t      size;
} prot_range_t;


typedef struct _pagedescr {
  caddr_t addr;
  size_t  size;
} pagedescr_t;

typedef struct _send_phys_addr_entry {
  caddr_t address;
  size_t  size;
  struct _send_phys_addr_entry *next;
  valid_t valid;  
} send_phys_addr_entry_t;

extern int send_phys_addr_alloc_entries;

typedef struct _send_seq_entry {
  seq_t seq;







} send_seq_entry_t;

typedef enum _terminate_type { SET_DEAD, CLEAR_DEAD } terminate_type_t;

 








typedef struct _received_receive_entry_general {
  node_t dest;

   
  mi_t   mi;

  channel_t channel;
  seq_t     seq;

  ptrdiff_t pages;
  size_t    size;
} received_receive_entry_general_t;

typedef union _received_receive_entry {
 

 
  received_receive_entry_general_t general;
} received_receive_entry_t;

typedef struct _recv_valid_entry_terminate {
  valid_t valid;  
} recv_valid_entry_terminate_t;

typedef enum { DEAD, ALIVE } dead_t;

typedef struct _recv_valid_entry_general {
  valid_t valid;  
  dead_t  dead;
  size_t  size;  






} recv_valid_entry_general_t;

typedef union _recv_valid_entry {
  recv_valid_entry_terminate_t terminate;
  recv_valid_entry_general_t   general;
} recv_valid_entry_t;

typedef struct _recv_phys_addr_entry {
  caddr_t address;
  size_t  size;
} recv_phys_addr_entry_t;

 



typedef struct _copy_phys_addr_entry {
  pagedescr_t pagedescr;
} copy_phys_addr_entry_t;

typedef struct _pending_receive_entry {
  node_t sender;

   
  mi_t mi;

  channel_t channel;
  seq_t     seq;


  copy_phys_addr_entry_t *pages;



  size_t    size;

  ptrdiff_t dist_pages;
  size_t    dist_size;

  struct proc *proc;

  void (*fct)  (caddr_t)  ;
  caddr_t param;

# 337 "data.h"



  prot_range_t prot_map_ref_tab[3 ];

  int post_treatment_lock;

  valid_t valid;  

  received_receive_entry_t received_receive;

   

  int reorder_interrupts;
} pending_receive_entry_t;







typedef struct _recv_seq_entry {
  seq_t seq;







} recv_seq_entry_t;


typedef struct _pending_send_entry {
  node_t    dest;

   
  mi_t      mi;

  channel_t channel;
  seq_t     seq;

  send_phys_addr_entry_t *pages;
  size_t    size;

  struct proc *proc;

  void (*fct)  (caddr_t,
		   int)  ;
  caddr_t param;
  size_t size_sent;


  prot_range_t prot_map_ref_tab[3 ];

  int post_treatment_lock;

  received_receive_entry_t *timeout;  
  valid_t valid;  

} pending_send_entry_t;

 






# 69 "driver.c" 2

# 1 "driver.h" 1

 







# 1 "put.h" 1

 










# 1 "/sys/pci/pcireg.h" 1



 




























 









 






 




















 










 


























 
























 




























































































 






 























# 13 "put.h" 2













typedef struct _set_mode {
  int max_nodes;



  node_t node;

  int val;
} set_mode_t;

extern int set_mode_read   (int, set_mode_t *)  ;
extern int set_mode_write  (int, set_mode_t *)  ;

extern u_long hsl_conf_read  (int, u_long)  ;
extern void hsl_conf_write   (int, u_long, u_long)  ;




extern void put_set_device  (int, pcicfgregs)  ;


typedef struct _tables_info {
  u_long  lpe_phys;
  caddr_t lpe_virt;
  int     lpe_size;

  u_long  lmi_phys;
  caddr_t lmi_virt;
  int     lmi_size;

  u_long  lrm_phys;
  caddr_t lrm_virt;
  int     lrm_size;
} tables_info_t;

extern void get_tables_info  (int, tables_info_t *)  ;

 
 


extern vm_offset_t opt_contig_space;
extern vm_size_t   opt_contig_size;
extern caddr_t     opt_contig_space_phys;
extern int         opt_slot;

 
extern vm_offset_t misalign_contig_space;
extern vm_size_t   misalign_contig_size;
extern caddr_t     misalign_contig_space_phys;
extern int         misalign_slot;

 


boolean_t          misalign_in_use;




extern int hslgetlpe_ident;
extern int hslfreelpe_ident;

typedef struct _lpe_entry {
  u_short page_length;
  u_short routing_part;

  u_long control;




 










 







 























  caddr_t PRSA, PLSA;
} lpe_entry_t;

typedef struct _sm_emu_buf {
  u_long control;
  u_long data1;
  u_long data2;
} sm_emu_buf_t;

typedef struct _lmi_entry {
  u_short packet_number;
  u_char r3_status;
  u_char reserved;
  u_long control;
  u_long data1;
  u_long data2;
} lmi_entry_t;











typedef struct _lrm_entry {
  u_short rpn;  
  u_short epn;  
} lrm_entry_t;

typedef struct _put_info {
  lpe_entry_t *lpe;
  long nentries;

  boolean_t remote_hsl[(1<< 4 ) ];  



  node_t node;
  long lpe_high;  
  long lpe_low;   

  boolean_t in_use[6 ];

  void (*IT_sent[6 ])  (int,
				mi_t,
				lpe_entry_t)  ;
  void (*IT_received[6 ])  (int,
				    mi_t,
				    u_long  ,
				    u_long  )  ;
  struct _mi_space {
    boolean_t in_use;
    mi_t   mi_start;
    u_long mi_size;
    struct _mi_space *prev;
    struct _mi_space *next;
  } mi_space[6 ];
  struct _mi_space *first_space;

  boolean_t hsl_found;



  pcicfgregs hsl_probe;


  int                hsl_contig_slot;
  vm_offset_t        hsl_contig_space;
  vm_offset_t        hsl_contig_space_backup;
  vm_size_t          hsl_contig_size;
  caddr_t            hsl_contig_space_phys;

  lpe_entry_t (*hsl_lpe)[1024 ];
  lmi_entry_t (*hsl_lmi)[1024 ];
  lrm_entry_t (*hsl_lrm)[(1<< 15 ) ];

  lpe_entry_t *lpe_softnew_virt;

   




  lpe_entry_t *lpe_new_virt;
  lpe_entry_t *lpe_current_virt;

  lmi_entry_t *lmi_current_virt;

  boolean_t    lpe_loaded;  

  boolean_t    stop_put;  
} put_info_t;

extern int put_init_SAP  (void)  ;

extern int put_end_SAP  (void)  ;

extern int put_init  (int,
			 u_long,
			 node_t,
			 void *)  ;

extern int put_end  (int)  ;

extern int put_get_node  (int)  ;

extern mi_t put_get_mi_start  (int,
				  int)  ;

extern long put_get_lpe_high  (int)  ;

extern long put_get_lpe_low  (int)  ;

extern long put_get_lpe_free  (int)  ;

extern int put_register_SAP  (int,
				 void (*)  (int, mi_t, lpe_entry_t)  ,
				 void (*)  (int, mi_t, u_long, u_long)  )  ;

extern int put_unregister_SAP  (int,
				   int)  ;

extern int put_attach_mi_range  (int,
				    int,
				    u_long)  ;

extern int put_add_entry  (int,
			      lpe_entry_t *)  ;

extern int put_get_entry  (int,
			      lpe_entry_t *)  ;

extern int put_flush_entry  (int)  ;

extern int put_simulate_interrupt  (int,
				       lpe_entry_t)  ;










extern void put_flush_lpe  (int)  ;

extern void put_flush_lmi  (int)  ;



# 10 "driver.h" 2


 
 















 






























































 



 

typedef struct _driver_stats {
   

   
  int calls_slrpp_send;
   
  int calls_slrpp_recv;

   
  int calls_slrpv_send;
   
  int calls_slrpv_recv;

   
  int calls_timeout;
   
  int timeout_timeout;

   

   
  int calls_put_add_entry;
   
  int calls_put_interrupt;
   
  int unattended_interrupts;

} driver_stats_t;

extern driver_stats_t driver_stats;

extern int perfmon_invalid;
extern struct timeval perfmon_tv_start;
extern struct timeval perfmon_tv_end;








 







struct _virtualRegion {
  caddr_t start;
  size_t  size;
};






struct _physicalWrite {
  caddr_t addr;
  int value;
};

























typedef struct _hsl2tty {
  caddr_t addr;
  node_t  dest;
  int     size;
} hsl2tty_t;








typedef struct _hsl_bench_latence {
  boolean_t first;
  int size;
  node_t dist_node;
  u_long dist_addr;  
  u_long local_addr;  
  caddr_t local_addr_virt;  
  int count;
  int wait_count;
  int result;
} hsl_bench_latence_t;


typedef struct _hsl_bench_throughput {
  boolean_t first;
  int size;
  node_t dist_node;
  u_long dist_addr;  
  u_long local_addr;  
  caddr_t local_addr_virt;  
  int count;
  int wait_count;
  int result;
} hsl_bench_throughput_t;


 



typedef struct _hsl_put_init {
  int length;
  node_t node;
  void *routing_table;
} hsl_put_init_t;














typedef struct _slr_config {
  node_t node;
  caddr_t contig_space;
} slr_config_t;


typedef struct _physical_read {
  caddr_t src;  
  caddr_t data;
  u_long  len;
 } physical_read_t;


typedef struct _physical_write {
  caddr_t dst;  
  caddr_t data;
  u_long  len;
} physical_write_t;




typedef struct _opt_contig_mem {
  u_char *ptr;
  size_t  size;
  u_char *phys;
} opt_contig_mem_t;








 




typedef struct _trace_var {
   
  int pending_send_entry;

   
  int pending_receive_entry;

   
  int send_phys_addr;

   
  int copy_phys_addr;

   
  int channel_0_in, channel_0_out;

   
  int sequence_0_in, sequence_0_out;

   
  int channel_1_in, channel_1_out;

   
  int sequence_1_in, sequence_1_out;
} trace_var_t;

typedef enum {
  TRACE_VAR,
  TRACE_STR
} trace_type_t;

typedef enum {
  TRACE_NO_TYPE,
  TRACE_INIT_DRIVER,
  TRACE_CLOSE_DRIVER,  
  TRACE_INIT_PUT,
  TRACE_END_PUT,
  TRACE_INIT_SLR_P,
  TRACE_END_SLR_P,
  TRACE_INIT_SLR_V,
  TRACE_END_SLR_V,
  TRACE_DECR_PS_ENTRY,
  TRACE_INCR_PS_ENTRY,
  TRACE_DECR_PR_ENTRY,
  TRACE_INCR_PR_ENTRY,
  TRACE_DECR_SPA_ENTRY,
  TRACE_INCR_SPA_ENTRY,
  TRACE_DECR_CPA_ENTRY,
  TRACE_INCR_CPA_ENTRY,
  TRACE_CHAN_0_IN,
  TRACE_CHAN_0_OUT,
  TRACE_CHAN_1_IN,
  TRACE_CHAN_1_OUT
} trace_type_entry_t;

typedef union {
  trace_var_t var;
  char        str[80 ];
} trace_t;

typedef struct _trace_event {
  trace_type_t       type;
  trace_type_entry_t entry_type;
  
  int          index;
  trace_t      trace;

  struct timeval tv;
} trace_event_t;
































extern trace_var_t   trace_var;
extern trace_event_t trace_events[10000 ];

extern void init_events          (void)  ;

extern void add_event            (trace_type_t,
				     trace_type_entry_t,
			             trace_t)  ;

extern void add_event_string     (char *)  ;
extern void add_event_tv         (char *)  ;

extern trace_event_t *get_event  (void)  ;



 

typedef struct _mpc_pci_conf {
  u_char bus;
  u_char device;
  u_char func;
  u_long reg;
  u_long data;
} mpc_pci_conf_t;
















 





typedef struct _manager_open_channel {
  node_t node;
  channel_t chan0;
  channel_t chan1;
  appclassname_t classname;
  int protocol;
} manager_open_channel_t;



typedef struct _manager_shutdown_1stStep {
  node_t node;
  channel_t chan0;
  channel_t chan1;
  seq_t ret_seq_send_0;
  seq_t ret_seq_recv_0;
  seq_t ret_seq_send_1;
  seq_t ret_seq_recv_1;
  appclassname_t classname;
  int protocol;
} manager_shutdown_1stStep_t;



typedef struct _manager_shutdown_2ndStep {
  node_t node;
  channel_t chan0;
  channel_t chan1;
  seq_t seq_send_0;
  seq_t seq_recv_0;
  seq_t seq_send_1;
  seq_t seq_recv_1;
  int protocol;
} manager_shutdown_2ndStep_t;



typedef struct _manager_shutdown_3rdStep {
  node_t node;
  channel_t chan0;
  channel_t chan1;
  int protocol;
} manager_shutdown_3rdStep_t;



typedef struct _manager_set_appclassname {
  appclassname_t cn;
  uid_t uid;
} manager_set_appclassname_t;



 





typedef struct _libmpc_read {
  node_t    node;
  channel_t channel;
  void     *buf;
  size_t    nbytes;  
} libmpc_read_t;



typedef struct _libmpc_write {
  node_t      node;
  channel_t   channel;
  const void *buf;
  size_t      nbytes;  
} libmpc_write_t;



typedef struct _libmpc_select {
  mpc_chan_set *mpcchanset_in, *mpcchanset_ou, *mpcchanset_ex;
  int mpcchanset_in_max_index, mpcchanset_ou_max_index, mpcchanset_ex_max_index;
  int nd;
  fd_set *in, *ou, *ex;
  struct timeval *tv;
  int retval;
} libmpc_select_t;





# 70 "driver.c" 2


# 1 "ddslrpp.h" 1

 
















 



 



typedef enum { CHAN_OPEN, CHAN_SHUTDOWN_0, CHAN_SHUTDOWN_1,
	       CHAN_SHUTDOWN_2, CHAN_CLOSE } channel_state_t;
extern channel_state_t channel_state[(1<< 4 )  - 0 ][8 ];

extern appclassname_t channel_classname[(1<< 4 )  - 0 ][8 ];
extern int channel_protocol[(1<< 4 )  - 0 ][8 ];

extern struct selinfo channel_select_info_in[(1<< 4 )  - 0 ]
                                            [8 ];
extern struct selinfo channel_select_info_ou[(1<< 4 )  - 0 ]
                                            [8 ];
extern struct selinfo channel_select_info_ex[(1<< 4 )  - 0 ]
                                            [8 ];
extern int channel_data_ready[(1<< 4 )  - 0 ]
                             [8 ];

extern uid_t classname2uid[0x2000L ];

extern node_t my_node;

typedef u_char nodes_tab_t[32];

extern send_seq_entry_t send_seq[(1<< 4 )  - 0 ][8 ];
extern pending_receive_entry_t (*pending_receive)[10 ];
extern received_receive_entry_t (*received_receive)[(1<< 4 )  - 0 ]
                                                   [(1<<8)  ];
extern pending_send_entry_t pending_send[10 ];
extern recv_valid_entry_t recv_valid[(1<< 4 )  - 0 ][(1<<8)  ];
extern recv_seq_entry_t recv_seq[(1<< 4 )  - 0 ][8 ];

extern boolean_t timeout_in_progress;

extern struct callout_handle timeout_handle;


extern int pending_send_free_ident;
extern int send_phys_addr_free_ident;

extern vm_offset_t contig_space;
extern caddr_t     contig_space_phys;

extern boolean_t SEQ_INF_STRICT  (seq_t, seq_t)  ;
extern boolean_t SEQ_SUP_STRICT  (seq_t, seq_t)  ;
extern boolean_t SEQ_INF_EQUAL   (seq_t, seq_t)  ;
extern boolean_t SEQ_SUP_EQUAL   (seq_t, seq_t)  ;

extern int init_slr  (int)  ;

extern void end_slr  (void)  ;

extern void close_slr  (void)  ;

extern void slr_clear_config  (void)  ;

extern int set_config_slr  (slr_config_t)  ;

extern nodes_tab_t *slrpp_get_nodes  (void)  ;

extern boolean_t slrpp_cansend  (node_t,
				    channel_t)  ;

extern __inline int slrpp_send  (node_t,
				    channel_t,
				    pagedescr_t *,
				    size_t,
				    void (*)  (caddr_t, int)  ,
				    caddr_t,
				    struct proc *)  ;

extern int _slrpp_send  (node_t,
 		 	    channel_t,
	 		    pagedescr_t *,
			    size_t,
			    void (*)  (caddr_t, int)  ,
			    caddr_t,
			    pending_send_entry_t **,
			    struct proc *,
			    boolean_t)  ;

extern __inline int slrpp_recv  (node_t,
				    channel_t,
				    pagedescr_t *,
				    size_t,
				    void (*)  (caddr_t)  ,
				    caddr_t,
				    struct proc *)  ;

extern int _slrpp_recv  (node_t,
			    channel_t,
			    pagedescr_t *,
			    size_t,
			    void (*)  (caddr_t)  ,
			    caddr_t,
			    pending_receive_entry_t **,
			    struct proc *,
			    boolean_t)  ;

extern void slrpp_reset_channel  (node_t,
                                     channel_t)  ;

extern boolean_t slrpp_channel_check_rights  (node_t,
						 channel_t,
						 struct proc *)  ;

extern void slrpp_set_appclassname  (appclassname_t,
					uid_t)  ;

extern void ddslrp_timeout  (void *)  ;








extern int slrpp_set_last_callback  (node_t,
					channel_t,
					boolean_t (*)  (caddr_t)  ,
					caddr_t,
					struct proc *)  ;

extern int slrpp_open_channel      (node_t,
				       channel_t,
				       appclassname_t,
				       struct proc *)  ;
extern int slrpp_shutdown0_channel  (node_t,
					channel_t,
					seq_t *,
					seq_t *,
					struct proc *)  ;
extern int slrpp_shutdown1_channel  (node_t,
					channel_t,
					seq_t,
					seq_t,
					struct proc *)  ;
extern int slrpp_close_channel     (node_t,
				       channel_t,
				       struct proc *)  ;



# 72 "driver.c" 2

# 1 "ddslrpv.h" 1

 








extern boolean_t slrpv_cansend  (node_t,
				    channel_t)  ;

extern int slrpv_send  (node_t,
			   channel_t,
			   caddr_t,
			   size_t,
			   void (*)  (caddr_t, int)  ,
			   caddr_t,
			   struct proc *)  ;


extern int slrpv_send_prot  (node_t,
				channel_t,
				caddr_t,
				size_t,
				void (*)  (caddr_t, int)  ,
				caddr_t,
				struct proc *)  ;


extern int slrpv_send_piggy_back  (node_t,
				     channel_t,
       				     caddr_t,
				     size_t,
				     caddr_t,
				     size_t,
				     void (*)  (caddr_t, int)  ,
				     caddr_t,
				     struct proc *)  ;

extern int slrpv_send_piggy_back_phys  (node_t,
					   channel_t,
					   pagedescr_t *,
					   size_t,
					   caddr_t,
					   size_t,
					   void (*)  (caddr_t, int)  ,
					   caddr_t,
					   struct proc *)  ;


extern int slrpv_send_piggy_back_prot  (node_t,
					   channel_t,
					   caddr_t,
					   size_t,
					   caddr_t,
					   size_t,
					   void (*)  (caddr_t, int)  ,
					   caddr_t,
					   struct proc *)  ;

extern int slrpv_send_piggy_back_prot_phys  (node_t,
						channel_t,
						pagedescr_t *,
						size_t,
						caddr_t,
						size_t,
						void (*)  (caddr_t, int)  ,
						caddr_t,
						struct proc *)  ;


extern int slrpv_recv  (node_t,
			   channel_t,
			   caddr_t,
			   size_t,
			   void (*)  (caddr_t)  ,
			   caddr_t,
			   struct proc *)  ;


extern int slrpv_recv_prot  (node_t,
				channel_t,
				caddr_t,
				size_t,
				void (*)  (caddr_t)  ,
				caddr_t,
				struct proc *)  ;


extern int slrpv_recv_piggy_back  (node_t,
			              channel_t,
				      caddr_t,
				      size_t,
				      caddr_t,
				      size_t,
				      void (*)  (caddr_t)  ,
				      caddr_t,
				      struct proc *)  ;

extern int slrpv_recv_piggy_back_phys  (node_t,
					   channel_t,
					   pagedescr_t *,
					   size_t,
					   caddr_t,
					   size_t,
					   void (*)  (caddr_t)  ,
					   caddr_t,
					   struct proc *)  ;


extern int slrpv_recv_piggy_back_prot  (node_t,
					   channel_t,
					   caddr_t,
					   size_t,
					   caddr_t,
					   size_t,
					   void (*)  (caddr_t)  ,
					   caddr_t,
					   struct proc *)  ;

extern int slrpv_recv_piggy_back_prot_phys  (node_t,
						channel_t,
						pagedescr_t *,
						size_t,
						caddr_t,
						size_t,
						void (*)  (caddr_t)  ,
						caddr_t,
						struct proc *)  ;


extern void slrpv_reset_channel  (node_t,
				     channel_t)  ;

extern int slrpv_mlock  (vm_offset_t,
			    vm_size_t,
			    struct proc *)  ;

extern int slrpv_munlock  (vm_offset_t,
			      vm_size_t,
			      struct proc *)  ;



extern int slrpv_prot_init  (void)  ;
extern int slrpv_prot_end   (void)  ;

extern int slrpv_prot_wire  (vm_offset_t,
				vm_size_t,
				prot_range_t *,
				int *,
				struct proc *)  ;

extern void slrpv_prot_dump  (void)  ;

extern void slrpv_prot_garbage_collection  (void)  ;



extern int slrpv_set_last_callback  (node_t,
					channel_t,
					boolean_t (*)  (caddr_t)  ,
					caddr_t,
					struct proc *)  ;

extern int slrpv_open_channel       (node_t,
					channel_t,
					appclassname_t,
					struct proc *)  ;
extern int slrpv_shutdown0_channel  (node_t,
				        channel_t,
					seq_t *,
					seq_t *,
					struct proc *)  ;
extern int slrpv_shutdown1_channel  (node_t,
				        channel_t,
					seq_t,
					seq_t,
					struct proc *)  ;
extern int slrpv_close_channel      (node_t,
					channel_t,
					struct proc *)  ;


# 73 "driver.c" 2

# 1 "mdcp.h" 1

 








typedef struct _mdcp_param_info {
  node_t    dest;
  channel_t main_channel;
  channel_t bufferized_channel;
  boolean_t blocking;
  boolean_t input_buffering;
  size_t    input_buffering_maxsize;  
  boolean_t output_buffering;
} mdcp_param_info_t;

 












extern int mdcp_init  (void)  ;
extern int mdcp_end   (void)  ;

extern int mdcp_init_com  (node_t, channel_t, channel_t,
			      appclassname_t, struct proc *)  ;
extern int mdcp_shutdown0_com  (node_t, channel_t, seq_t *, seq_t *,
				   seq_t *, seq_t *, struct proc *)  ;
extern int mdcp_shutdown1_com  (node_t, channel_t, seq_t, seq_t,
				   seq_t, seq_t, struct proc *)  ;
extern int mdcp_end_com  (node_t, channel_t, struct proc *)  ;

extern int mdcp_getparams  (node_t, channel_t, mdcp_param_info_t *)  ;
extern int mdcp_setparam_blocking  (node_t, channel_t, boolean_t)  ;
extern int mdcp_setparam_input_buffering  (node_t, channel_t, boolean_t)  ;
extern int mdcp_setparam_input_buffering_maxsize  (node_t, channel_t, size_t)  ;
extern int mdcp_setparam_output_buffering  (node_t, channel_t, boolean_t)  ;

extern int mdcp_write  (node_t, channel_t, caddr_t, size_t,
			   size_t *, struct proc *)  ;
extern int mdcp_read  (node_t, channel_t, caddr_t, size_t,
			  size_t *, struct proc *)  ;

extern int hsl_select  (struct proc *, libmpc_select_t *, int *)  ;



# 74 "driver.c" 2


char debug_stage [128 ];
char debug_stage2[128 ];

static caddr_t protocol_names[] = { "PUT",
				    "SLRP/P", "SCP/P",
				    "SLRP/V", "SCP/V",
				    "MDCP" };

static int open_count = 0;

int perfmon_invalid = 0;
struct timeval perfmon_tv_start;
struct timeval perfmon_tv_end;





static int dev_open();
static int dev_close();
static int dev_read();
static int dev_ioctl();


static int
noselect(dev, rw, p)
        dev_t dev;
        int rw;
        struct proc *p;
{
        printf("noselect(0x%x) called\n", dev);
         
        return (6 );
}


static struct cdevsw cdev = {
  dev_open,
  dev_close,
  dev_read,



  nowrite,

  dev_ioctl,



  nostop,




  noreset,




  nodevtotty,




  noselect,




  nommap,




  ((d_strategy_t *)0 ) ,

  "pciddc",
  (struct bdevsw *) 0 ,
  -1
};

# 189 "driver.c"


driver_stats_t driver_stats;

 
 

static int testslrv2cpt1 = 0;
static int testslrv2cpt2 = 0;

static void testslrv2callback(caddr_t param, int size)
{
  testslrv2cpt2++;

   
  (*(int *)param)++;
  wakeup(param);
}

 
 

static int testputbench_ident[16 ];
static int hsltestput_layer = -1;

static int hsl2tty_ident[4 ];
static int hsl2tty_size[4 ];

void
hsl2ttysend_cb(caddr_t param, int size)
{
  int s;
  int dest;

  dest = (int) param;

  s = splhigh();
  hsl2tty_size[dest] = size;
  hsl2tty_ident[dest]--;
  wakeup(&hsl2tty_ident[dest]);
  splx(s);
}

void
hsl2ttyrecv_cb(caddr_t param)
{
  psignal((struct proc *) param, (int) 23 );
}

 

int
hsl_bench_latence1(int minor, hsl_bench_latence_t *param_lat)
{
  int s;
  lpe_entry_t entry;
  mi_t mi;
  volatile char *target;
  int cpt;
  int global_cnt;
  int res;
  int wait_count;
  int count_limit;

  wait_count = param_lat->wait_count;

  entry.page_length = param_lat->size;
  entry.routing_part = param_lat->dist_node;
  entry.control = (1<<30) ;
  mi = put_get_mi_start(minor,
			hsltestput_layer);
  entry.control |= mi;
  entry.PRSA = (caddr_t) param_lat->dist_addr;
  entry.PLSA = (caddr_t) param_lat->local_addr;

  target = param_lat->local_addr_virt + param_lat->size - 1;

  count_limit = param_lat->count;

   

  s = splhigh();

  put_flush_lpe(minor);










  if (param_lat->first == 1 ) {
     

    for (global_cnt = 0; global_cnt < count_limit; global_cnt++) {


 

      *target = '\1';

       

      if (!(global_cnt % 1001))
	put_flush_lpe(minor);
      res = put_add_entry(minor, &entry);

      if (res != 0) {



	splx(s);
	param_lat->result = global_cnt;
	return res;
      }

      cpt = 0;
      while (*target == '\1') {
	cpt++;
	if (cpt == wait_count) {



	  splx(s);
	  param_lat->result = global_cnt;
	  return 11 ;
	}
      }
    }
  } else {
     

    *target = '\2';

    for (global_cnt = 0; global_cnt < count_limit; global_cnt++) {

 

      cpt = 0;
      while (*target == '\2') {
	cpt++;
	if (cpt == wait_count) {



	  splx(s);
	  param_lat->result = global_cnt;
	  return 11 ;
	}
      }

 

      *target = '\2';

      if (!(global_cnt % 1001))
	put_flush_lpe(minor);
      res = put_add_entry(minor, &entry);
      if (res != 0) {



	splx(s);
	param_lat->result = global_cnt;
	return res;
      }
    }
  }





  splx(s);

  param_lat->result = global_cnt;
  return 0;
}

 

int
hsl_bench_throughput1(int minor, hsl_bench_throughput_t *param_thr)
{
  int s;
  lpe_entry_t entry;
  mi_t mi;
  volatile char *target;
  int cpt;
  int global_cnt = 0;
  int res;
  int wait_count;
  int count_limit;

  wait_count = param_thr->wait_count;

  entry.routing_part = param_thr->dist_node;
  entry.control = (1<<30) ;
  mi = put_get_mi_start(minor,
			hsltestput_layer);
  entry.control |= mi;
  if (param_thr->first == 1 ) {
    entry.page_length = param_thr->size;
    entry.PRSA = (caddr_t) param_thr->dist_addr;
    entry.PLSA = (caddr_t) param_thr->local_addr;
  } else {
    entry.page_length = 1;
    entry.PRSA = (caddr_t) (param_thr->dist_addr + param_thr->size);
    entry.PLSA = (caddr_t) (param_thr->local_addr + param_thr->size);
  }

  target = param_thr->local_addr_virt + param_thr->size;

  count_limit = param_thr->count;

   

  s = splhigh();

  put_flush_lpe(minor);






  if (param_thr->first == 1 ) {
     

    for (global_cnt = 0; global_cnt < count_limit - 1; global_cnt++) {
       

      if (!(global_cnt % 1001))
	put_flush_lpe(minor);
      do {
	res = put_add_entry(minor, &entry);
	if (res == 35 )
	  put_flush_lpe(minor);
      } while (res == 35 );

      if (res != 0) {



	splx(s);
	param_thr->result = global_cnt;
	return res;
      }
    }

    entry.page_length++;
    *target = '\1';
    do {
      res = put_add_entry(minor, &entry);
      if (res == 35 )
	put_flush_lpe(minor);
    } while (res == 35 );
    if (res != 0) {



      splx(s);
      param_thr->result = global_cnt;
      return res;
    }

    cpt = 0;
    while (*target == '\1') {
      cpt++;
      if (cpt == wait_count) {



	splx(s);
	param_thr->result = global_cnt;
	return 11 ;
      }
    }

  } else {
     

    *target = '\2';
    cpt = 0;
    while (*target == '\2') {
      cpt++;
      if (cpt == wait_count) {



	splx(s);
	param_thr->result = global_cnt;
	return 11 ;
      }
    }

    *target = '\2';
    res = put_add_entry(minor, &entry);
    if (res != 0) {



      splx(s);
      param_thr->result = global_cnt;
      return res;
    }
  }





  splx(s);

  param_thr->result = global_cnt;
  return 0;
}


 









void
hsltestputrecv_irqsent(int minor,
		       mi_t mi,
		       lpe_entry_t entry)
{
  { strncpy(debug_stage,  ( "hsltestputrecv_irqsent" ), sizeof(debug_stage)  - 1); } 
# 539 "driver.c"

}


 










void
hsltestputrecv_irqreceived(int minor,
			   mi_t mi,
			   u_long data1,
			   u_long data2)
{
  int s;

  { strncpy(debug_stage,  ( "hsltestputrecv_irqreceived" ), sizeof(debug_stage)  - 1); } 

# 576 "driver.c"


  s = splhigh();

   


  testputbench_ident[mi - put_get_mi_start(minor, hsltestput_layer)]++;
  wakeup(testputbench_ident + (mi - put_get_mi_start(minor, hsltestput_layer)));

  splx(s);
}

 

static int txt_offset;
static char txt_info[10000];






























 









static char *
format(type, value)
       int  type;
       long value;
{
  char tmp[256];
  int i;
  static char strings[10][256];
  static int current = 0;

  if (current == 10)
    current = 0;

  for (i = 0;
       i < sizeof strings[current];
       i++)
    strings[current][i] = ' ';

  switch (type) {
  case 1 :
    sprintf(tmp,
	    "%p",
	    (caddr_t) value);
    strcpy(strings[current] + 10 - strlen(tmp),
	 tmp);
    break;

  case 2 :
    sprintf(tmp,
	    "%d",
	    (int) value);
    strcpy(strings[current] + 4 - strlen(tmp),
	 tmp);
    break;

  case 3 :
    sprintf(tmp,
	    "%d",
	    (int) value);
    strcpy(strings[current] + 8 - strlen(tmp),
	 tmp);
    break;

  default:
    sprintf(strings[current],
	    "ERROR");
  }

  return strings[current++];
}

 






























 





static void
driver_update()
{





  txt_info[0] = 0;

  sprintf(txt_info + strlen(txt_info),  "/dev/hsl : kernel drivers and protocols for MPC parallel computer\n\n" ); if (sizeof(txt_info) - strlen(txt_info) < 2 * 160) { sprintf(txt_info + strlen(txt_info), "--TRUNCATED--\n"); return; } ;



  sprintf(txt_info + strlen(txt_info),  "***** SLR/V - SCP/V\n" ); if (sizeof(txt_info) - strlen(txt_info) < 2 * 160) { sprintf(txt_info + strlen(txt_info), "--TRUNCATED--\n"); return; } ;
  sprintf(txt_info + strlen(txt_info),  "calls to send()           : %s\n" ,  
	      format(3 ,
		     driver_stats.calls_slrpv_send) ); if (sizeof(txt_info) - strlen(txt_info) < 2 * 160) { sprintf(txt_info + strlen(txt_info), "--TRUNCATED--\n"); return; } ;
  sprintf(txt_info + strlen(txt_info),  "calls to recv()           : %s\n" ,  
	      format(3 ,
		     driver_stats.calls_slrpv_recv) ); if (sizeof(txt_info) - strlen(txt_info) < 2 * 160) { sprintf(txt_info + strlen(txt_info), "--TRUNCATED--\n"); return; } ;
  sprintf(txt_info + strlen(txt_info),  "***** SLR/P - SCP/V\n" ); if (sizeof(txt_info) - strlen(txt_info) < 2 * 160) { sprintf(txt_info + strlen(txt_info), "--TRUNCATED--\n"); return; } ;
  sprintf(txt_info + strlen(txt_info),  "calls to send()           : %s\n" ,  
	      format(3 ,
		     driver_stats.calls_slrpp_send) ); if (sizeof(txt_info) - strlen(txt_info) < 2 * 160) { sprintf(txt_info + strlen(txt_info), "--TRUNCATED--\n"); return; } ;
  sprintf(txt_info + strlen(txt_info),  "calls to recv()           : %s\n" ,  
	      format(3 ,
		     driver_stats.calls_slrpp_recv) ); if (sizeof(txt_info) - strlen(txt_info) < 2 * 160) { sprintf(txt_info + strlen(txt_info), "--TRUNCATED--\n"); return; } ;
  sprintf(txt_info + strlen(txt_info),  "calls to ddslrp_timeout() : %s\n" ,  
	      format(3 ,
		     driver_stats.calls_timeout) ); if (sizeof(txt_info) - strlen(txt_info) < 2 * 160) { sprintf(txt_info + strlen(txt_info), "--TRUNCATED--\n"); return; } ;
  sprintf(txt_info + strlen(txt_info),  "timeouts from timeouts    : %s\n" ,  
	      format(3 ,
		     driver_stats.timeout_timeout) ); if (sizeof(txt_info) - strlen(txt_info) < 2 * 160) { sprintf(txt_info + strlen(txt_info), "--TRUNCATED--\n"); return; } ;
  sprintf(txt_info + strlen(txt_info),  "***** PUT\n" ); if (sizeof(txt_info) - strlen(txt_info) < 2 * 160) { sprintf(txt_info + strlen(txt_info), "--TRUNCATED--\n"); return; } ;
  sprintf(txt_info + strlen(txt_info),  "calls to put_add_entry()  : %s\n" ,  
	      format(3 ,
		     driver_stats.calls_put_add_entry) ); if (sizeof(txt_info) - strlen(txt_info) < 2 * 160) { sprintf(txt_info + strlen(txt_info), "--TRUNCATED--\n"); return; } ;
  sprintf(txt_info + strlen(txt_info),  "hardware interrupts       : %s\n" ,  
	      format(3 ,
		     driver_stats.calls_put_interrupt) ); if (sizeof(txt_info) - strlen(txt_info) < 2 * 160) { sprintf(txt_info + strlen(txt_info), "--TRUNCATED--\n"); return; } ;

  sprintf(txt_info + strlen(txt_info),  "unattended interrupts     : %s\n" ,  
	      format(3 ,
		     driver_stats.unattended_interrupts) ); if (sizeof(txt_info) - strlen(txt_info) < 2 * 160) { sprintf(txt_info + strlen(txt_info), "--TRUNCATED--\n"); return; } ;







}

 

int event_ident;

trace_var_t trace_var;

static int trace_events_low;
static int trace_events_high;
static int trace_events_index;

trace_event_t trace_events[10000 ];


 





void
init_events()
{
  trace_events_low = trace_events_high = trace_events_index = 0;
}


 








void
add_event(type, entry, event)
     trace_type_t       type;
     trace_type_entry_t entry;
     trace_t            event;
{
  int i ;

   

  i = ((trace_events_high + 1) == 10000 ) ? 0 : trace_events_high + 1;
  if (i == trace_events_low) {
    trace_events_index++;

     
    return;
  }

  trace_events[trace_events_high].type       = type;
  trace_events[trace_events_high].entry_type = entry;
  trace_events[trace_events_high].index      = trace_events_index;
  trace_events[trace_events_high].trace      = event;

  microtime(&trace_events[trace_events_high].tv);

  trace_events_high = i;
  trace_events_index++;

  wakeup(&event_ident);

   
}


 






void
add_event_string(str)
     char *str;
{
  trace_t event;
  int s;

  s = splhigh();

  if (perfmon_invalid) {
    perfmon_invalid = 0;
    add_event_string(">>>INVALID<<<");
  }

  strncpy(event.str, str, 80  - 1);
  event.str[80  - 1] = 0;
  add_event(TRACE_STR, TRACE_NO_TYPE, event);

  perfmon_invalid = 0;

  splx(s);
}


 






void
add_event_tv(comment)
             char *comment;
{
  u_int diff;
  char str[80 ];

  if (!perfmon_invalid) {

     





    diff = perfmon_tv_end.tv_sec * 1000000 + perfmon_tv_end.tv_usec -
      (perfmon_tv_start.tv_sec * 1000000 + perfmon_tv_start.tv_usec);
    sprintf(str, "DELTA TIME : %d microsec - %s", diff, comment);
    add_event_string(str);
  }
}


 







trace_event_t *
get_event()
{
  int s;
  trace_event_t *result;

  s = splhigh();

  if (trace_events_high == trace_events_low) {
    splx(s);
    return 0 ;
  }

  result = trace_events + trace_events_low;

  trace_events_low++;
  if (trace_events_low == 10000 ) trace_events_low = 0;

  splx(s);
  return result;
}


 

 







static int
dev_open()
{
  { strncpy(debug_stage,  ( "dev_open" ), sizeof(debug_stage)  - 1); } 






  txt_offset = 0;
  driver_update();

  open_count++;

  return 0;
}


 







static int
dev_close()
{
  { strncpy(debug_stage,  ( "dev_close" ), sizeof(debug_stage)  - 1); } 

  txt_offset = 0;






  open_count--;
  return 0;
}


 










static int
dev_read(dev, uio, flag)
         dev_t dev;
         struct uio *uio;
         int flag;
{
  int res;
  int len;






  len = uio->uio_iov->iov_len;
  len = ((( len )<(  strlen(txt_info) - txt_offset ))?( len ):(  strlen(txt_info) - txt_offset )) ;
  if (!len) return 0;

  res = uiomove(txt_info + txt_offset, len, uio);
  txt_offset += len;

  return res;
}


 

 







void vmflags2str(flagsstr, flags)
                 char *flagsstr;
                 u_short flags;
{
  flagsstr[0] = 0;



  if (flags & 0x0004 )       strcat(flagsstr, " ACTIVE");
  if (flags & 0x0008 )         strcat(flagsstr, " DEAD");
  if (flags & 0x0040 )       strcat(flagsstr, " PIPWNT");
  if (flags & 0x0080 )    strcat(flagsstr, " WRITEABLE");
  if (flags & 0x0100 ) strcat(flagsstr, " MIGHTBEDIRTY");
  if (flags & 0x0200 )     strcat(flagsstr, " CLEANING");




  if (flags & 0x0 )       strcat(flagsstr, " NORMAL");
  if (flags & 0x1 )   strcat(flagsstr, " SEQUENTIAL");
  if (flags & 0x2 )       strcat(flagsstr, " RANDOM");
}

 









 







# 1109 "driver.c"



 












static int
dev_ioctl(dev, com, data, flag, p)
     dev_t dev;
     int com;
     caddr_t data;
     int flag;
     struct proc *p;
{
  struct _virtualRegion *vz;
  struct _physicalWrite *pw2;
  physical_read_t *pr;
  physical_write_t *pw;
  pagedescr_t pagedescr[10];



  pcicfgregs probe;

  trace_event_t *event_p;
  int s, tmp, res, i, j;
  node_t node;
  int test2slrv_status;

  { strncpy(debug_stage,  ( "dev_ioctl" ), sizeof(debug_stage)  - 1); } 





  switch (com) {
  case ((unsigned long)( 0x80000000   | ((  sizeof(  caddr_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  40 ) )))   :
    { strncpy(debug_stage,  ( "HSLTESTSLR" ), sizeof(debug_stage)  - 1); } 





    pagedescr[0].addr = *(caddr_t *) data;
    pagedescr[0].size = 2;
    pagedescr[1].addr = 2 + *(caddr_t *) data;
    pagedescr[1].size = 1;
     
    res = slrpp_send(1, 3, pagedescr, 2, 0 , 0 , p);
    return res;
     
    break;

     

  case ((unsigned long)( 0x80000000   | ((  sizeof(  caddr_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  41 ) )))   :
    { strncpy(debug_stage,  ( "HSLTESTSLR2" ), sizeof(debug_stage)  - 1); } 





    pagedescr[0].addr = *(caddr_t *) data;
    pagedescr[0].size = 1;
    pagedescr[1].addr = 1 + *(caddr_t *) data;
    pagedescr[1].size = 2;
     
    res = slrpp_recv(0, 3, pagedescr, 2, 0 , 0 , p);
    return res;
     
    break;

     

  case ((unsigned long)( 0x80000000   | ((  sizeof(  caddr_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  42 ) )))   :
    { strncpy(debug_stage,  ( "HSLTESTSLRV" ), sizeof(debug_stage)  - 1); } 





     
    res = slrpv_send(1, 3, *(caddr_t *) data, 5000, 0 , 0 , p);
    return res;
     
    break;

     

  case ((unsigned long)( 0x80000000   | ((  sizeof(  caddr_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  43 ) )))   :
    { strncpy(debug_stage,  ( "HSLTESTSLRV2" ), sizeof(debug_stage)  - 1); } 





     
 
res = slrpv_recv(0, 3, *(caddr_t *) data, 5000, 0 , 0 , p);

    return res;
     
    break;

     

  case ((unsigned long)( 0x80000000   | ((  sizeof(  caddr_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  61 ) )))   :
    { strncpy(debug_stage,  ( "HSLTEST2SLRV" ), sizeof(debug_stage)  - 1); } 





     
    test2slrv_status = 0;
    testslrv2cpt1++;
    res = slrpv_send(1, 3, *(caddr_t *) data, 20000, testslrv2callback, (caddr_t) &test2slrv_status, p);
    if (res) {
      log(3 , "slrpv_send error\n");
      return res;
    }

    s = splhigh();
     
    while(!test2slrv_status) {
      res = tsleep(&test2slrv_status, (22  + 4) | 0x100 , "DATASENT", 0);
      if (res) {
	log(3 , "tsleep error");
	return res;
      }
    }
    test2slrv_status--;
    splx(s);
     
    return 0;
     
    break;

     

  case ((unsigned long)( 0x80000000   | ((  sizeof(  caddr_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  62 ) )))   :
    { strncpy(debug_stage,  ( "HSLTEST2SLRV2" ), sizeof(debug_stage)  - 1); } 





     
 
    test2slrv_status = 0;
    testslrv2cpt1++;
    res = slrpv_recv(0, 3, *(caddr_t *) data, 20000, (void (*)(caddr_t)) testslrv2callback, (caddr_t) &test2slrv_status, p);
    if (res) {
      log(7 ," slrpv_recv error\n");
      return res;
    }

    s = splhigh();
     
    while(!test2slrv_status) {
      res = tsleep(&test2slrv_status, (22  + 4) | 0x100 , "DATARECV", 0);
      if (res) {
	log(3 , "tsleep error");
	return res;
      }
    }
    test2slrv_status--;
    splx(s);
     
    return 0;
     
    break;

     

  case ((unsigned long)( 0x80000000   | ((  sizeof(  int )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  63 ) )))   :
    s = splhigh();
    for (i = 0; i < *(int *) data; i++);

    return 0;
     
    break;


     

  case ((unsigned long)( 0x80000000   | ((  sizeof(  struct _virtualRegion )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  50 ) )))   :
    { strncpy(debug_stage,  ( "HSLTESTSLRVPROT" ), sizeof(debug_stage)  - 1); } 





     
    res = slrpv_send_prot(1, 3,
			  (*(struct _virtualRegion *) data).start,
			  (*(struct _virtualRegion *) data).size,
			  0 , 0 , p);
    return res;
     
    break;

     

  case ((unsigned long)( 0x80000000   | ((  sizeof(  struct _virtualRegion )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  51 ) )))   :
    { strncpy(debug_stage,  ( "HSLTESTSLRV2PROT" ), sizeof(debug_stage)  - 1); } 





     
    res = slrpv_recv_prot(0, 3,
			  (*(struct _virtualRegion *) data).start,
			  (*(struct _virtualRegion *) data).size,
			  0 , 0 , p);
    return res;
     
    break;

     

  case ((unsigned long)( (0x80000000 | 0x40000000 )   | ((  sizeof(  int )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  1 ) )))   :
    { strncpy(debug_stage,  ( "HSLTEST" ), sizeof(debug_stage)  - 1); } 





    wakeup(&pending_send_free_ident);

     



     
    break;

     

  case ((unsigned long)( (0x80000000 | 0x40000000 )   | ((  sizeof(  struct _virtualRegion )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  2 ) )))   :
    { strncpy(debug_stage,  ( "HSLVIRT2PHYSTST" ), sizeof(debug_stage)  - 1); } 





    vz = (struct _virtualRegion *) data;

    log(7 , "0x%x\n", pmap_kextract(((vm_offset_t) ( vz->start ))) );

    break;

     

  case ((unsigned long)( 0x40000000   | ((  sizeof(  mi_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  44 ) )))   :
    { strncpy(debug_stage,  ( "HSLTESTPUTRECV1" ), sizeof(debug_stage)  - 1); } 





    {
      int res;

      if (hsltestput_layer != -1) {
	res = put_unregister_SAP(((int)(( dev )&0xffff00ff)) ,
				 hsltestput_layer);
	hsltestput_layer = -1;
	if (res)
	  return res;
      }

      hsltestput_layer = put_register_SAP(((int)(( dev )&0xffff00ff)) ,
					  hsltestputrecv_irqsent,
					  hsltestputrecv_irqreceived);

      if (hsltestput_layer < 0)
	return 12 ;

      res = put_attach_mi_range(((int)(( dev )&0xffff00ff)) ,
				hsltestput_layer,
				16 );

      if (res)
	return res;

      *((mi_t *) data) = put_get_mi_start(((int)(( dev )&0xffff00ff)) ,
					  hsltestput_layer);
      return 0;
    }
     
    break;

     

  case ((unsigned long)( 0x20000000   | ((  0  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  45 ) )))   :
    { strncpy(debug_stage,  ( "HSLTESTPUTRECV2" ), sizeof(debug_stage)  - 1); } 





    {
      int res;

      if (hsltestput_layer != -1) {
	res = put_unregister_SAP(((int)(( dev )&0xffff00ff)) ,
				 hsltestput_layer);
	hsltestput_layer = -1;
	if (res)
	  return res;
      }
    }
    break;

     

  case ((unsigned long)( 0x80000000   | ((  sizeof(  lpe_entry_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  46 ) )))   :
    { strncpy(debug_stage,  ( "HSLTESTPUTSEND0" ), sizeof(debug_stage)  - 1); } 





    {
      int res;
      mi_t mi;
      lpe_entry_t entry;

      entry = *(lpe_entry_t *) data;

      if (hsltestput_layer < 0)
	return 12 ;

      mi = put_get_mi_start(((int)(( dev )&0xffff00ff)) ,
			    hsltestput_layer);

      entry.control |= mi + 4;

       
       
























      res = put_add_entry(((int)(( dev )&0xffff00ff)) , &entry);

      if (res)
	return res;

      return 0;
    }
     
    break;

     

  case ((unsigned long)( 0x80000000   | ((  sizeof(  lpe_entry_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  52 ) )))   :
    { strncpy(debug_stage,  ( "HSLTESTPUTSEND1" ), sizeof(debug_stage)  - 1); } 





    {
      int res;
      lpe_entry_t entry;

      entry = *(lpe_entry_t *) data;

      if (hsltestput_layer < 0)
	return 12 ;

       
       
























      res = put_add_entry(((int)(( dev )&0xffff00ff)) , &entry);

      if (res)
	return res;

      return 0;
    }
     
    break;

     

  case ((unsigned long)( 0x20000000   | ((  0  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  47 ) )))   :
    { strncpy(debug_stage,  ( "HSLTESTPUTSEND2" ), sizeof(debug_stage)  - 1); } 





    {
    }
    break;

     

  case ((unsigned long)( 0x80000000   | ((  sizeof(  int )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  48 ) )))   :
    { strncpy(debug_stage,  ( "HSLTESTPUTBENCH1" ), sizeof(debug_stage)  - 1); } 





    s = splhigh();
  restart:
    if (!testputbench_ident[*(int *) data]) {



      res = tsleep(&testputbench_ident[*(int *) data],
		   (22  + 4)  | 0x100 , "BENCH1", 0 );
      if (!res)
	goto restart;
      splx(s);

       



      if (res == (-1) ) res = 4 ;

      return res;
    }
    testputbench_ident[*(int *) data]--;
    splx(s);
    break;

     

  case ((unsigned long)( (0x80000000 | 0x40000000 )   | ((  sizeof(  int )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  49 ) )))   :
    { strncpy(debug_stage,  ( "HSLTESTPUTBENCH2" ), sizeof(debug_stage)  - 1); } 





    testputbench_ident[*(int *) data] = 0;
    *(int * ) data = put_get_mi_start(((int)(( dev )&0xffff00ff)) ,
				      hsltestput_layer) + *(int *) data;
    break;

     

  case ((unsigned long)( 0x80000000   | ((  sizeof(  int )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  55 ) )))   :

    { strncpy(debug_stage,  ( "HSL2TTYINIT" ), sizeof(debug_stage)  - 1); } 




 

    hsl2tty_ident[*(int *) data] = 0;
    break;

     

  case ((unsigned long)( (0x80000000 | 0x40000000 )   | ((  sizeof(  hsl2tty_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  54 ) )))   :
    { strncpy(debug_stage,  ( "HSL2TTYSEND" ), sizeof(debug_stage)  - 1); } 




 

    s = splhigh();

# 1643 "driver.c"


    if (slrpv_cansend(((hsl2tty_t *) data)->dest, 0) == 0 ) {
      log(3 ,
	  "HSL2TTYSEND error 1\n");
      splx(s);
      return 35  ;
    }

    hsl2tty_ident[((hsl2tty_t *) data)->dest]++;

    splx(s);

    res = slrpv_send_prot(((hsl2tty_t *) data)->dest,
			  0,
			  ((hsl2tty_t *) data)->addr,
			  ((hsl2tty_t *) data)->size,
			  hsl2ttysend_cb,
			  (caddr_t) (int) (((hsl2tty_t *) data)->dest),
			  p);
    if (res) return res;

    s = splhigh();

  ttyrestart:

     



    if (hsl2tty_ident[((hsl2tty_t *) data)->dest] > 0) {
      res = tsleep(&hsl2tty_ident[((hsl2tty_t *) data)->dest],
		   (22  + 4)  , "HSL2TTY", 0 );
      if (!res ) goto ttyrestart;
      else {
	splx(s);
	return res;
      }
    }

    ((hsl2tty_t *) data)->size = hsl2tty_size[((hsl2tty_t *) data)->dest];
    splx(s);

    break;

     

  case ((unsigned long)( (0x80000000 | 0x40000000 )   | ((  sizeof(  int )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  56 ) )))   :
    { strncpy(debug_stage,  ( "HSL2TTYCANSEND" ), sizeof(debug_stage)  - 1); } 




 

    node = *(int *) data;










    *(int *) data = slrpv_cansend(node, 0);
    break;

     

  case ((unsigned long)( (0x80000000 | 0x40000000 )   | ((  sizeof(  hsl2tty_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  53 ) )))   :
    { strncpy(debug_stage,  ( "HSL2TTYRECV" ), sizeof(debug_stage)  - 1); } 




 

    res = slrpv_recv_prot(((hsl2tty_t *) data)->dest,
			  0,
			  ((hsl2tty_t *) data)->addr,
			  ((hsl2tty_t *) data)->size,
			  hsl2ttyrecv_cb,
			  (caddr_t) p,
			  p);
    if (res) return res;
    break;

     

# 1745 "driver.c"


     

  case ((unsigned long)( (0x80000000 | 0x40000000 )   | ((  sizeof(  caddr_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  8 ) )))   :
    { strncpy(debug_stage,  ( "HSLVIRT2PHYS" ), sizeof(debug_stage)  - 1); } 





    *(caddr_t *) data = (caddr_t) pmap_kextract(((vm_offset_t) ( *(caddr_t *) data ))) ;
    break;

     

  case ((unsigned long)( 0x80000000   | ((  sizeof(  lpe_entry_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  9 ) )))   :
    { strncpy(debug_stage,  ( "HSLADDLPE" ), sizeof(debug_stage)  - 1); } 





    res = put_add_entry(((int)(( dev )&0xffff00ff)) , (lpe_entry_t *) data);
    log(7 , "HSLADDLPE : res = %d\n", res);
    return res;
     
    break;

     

  case ((unsigned long)( 0x40000000   | ((  sizeof(  lpe_entry_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  10 ) )))   :
    { strncpy(debug_stage,  ( "HSLGETLPE" ), sizeof(debug_stage)  - 1); } 





    s = splhigh();

    for (;; ) {
      res = put_get_entry(((int)(( dev )&0xffff00ff)) , (lpe_entry_t *) data);
      if (res != 35 ) {
	splx(s);
	return res;
      }




      res = tsleep(&hslgetlpe_ident, (22  + 4)  | 0x100 , "GETLPE", 0 );
      if (res) {
	 



	if (res == (-1) ) res = 4 ;

	splx(s);
	return res;
      }
    }

     
    splx(s);
    break;

     

  case ((unsigned long)( 0x20000000   | ((  0  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  11 ) )))   :
    { strncpy(debug_stage,  ( "HSLFLUSHLPE" ), sizeof(debug_stage)  - 1); } 





    return put_flush_entry(((int)(( dev )&0xffff00ff)) );
     
    break;

     

  case ((unsigned long)( 0x80000000   | ((  sizeof(  lpe_entry_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  15 ) )))   :
    { strncpy(debug_stage,  ( "HSLSIMINT" ), sizeof(debug_stage)  - 1); } 





    return put_simulate_interrupt(((int)(( dev )&0xffff00ff)) , *(lpe_entry_t *) data);
     
    break;

     

  case ((unsigned long)( (0x80000000 | 0x40000000 )   | ((  sizeof(  slr_config_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  12 ) )))   :
    { strncpy(debug_stage,  ( "HSLSETCONFIG" ), sizeof(debug_stage)  - 1); } 





    return set_config_slr(*(slr_config_t *) data);
     
    break;

     

  case ((unsigned long)( (0x80000000 | 0x40000000 )   | ((  sizeof(  physical_read_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  13 ) )))   :
    { strncpy(debug_stage,  ( "HSLREADPHYS" ), sizeof(debug_stage)  - 1); } 





    s = splhigh();

    pr = (physical_read_t *) data;

    if (!pr->len) return 0;
    if ((( (unsigned) pr->src ) & ~((1<< 12 ) -1) )  != (( (unsigned) (pr->src + pr->len - 1) ) & ~((1<< 12 ) -1) ) ) {
      splx(s);
      return 22 ;
    }

    *(int *) CMAP1 = 0x001  | 0x002  | (( (unsigned) pr->src ) & ~((1<< 12 ) -1) ) ;

    tmp = *(int *) (CADDR1 + ((int) pr->src & ((1<< 12 ) -1) ));

    res = copyout((char *) (CADDR1 + ((int) pr->src & ((1<< 12 ) -1) )),
		  pr->data,
		  pr->len);

    *(int *) CMAP1 = 0;
    invltlb();

    splx(s);

    if (res)
      return res;
    break;

     

  case ((unsigned long)( (0x80000000 | 0x40000000 )   | ((  sizeof(  physical_write_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  14 ) )))   :
    { strncpy(debug_stage,  ( "HSLWRITEPHYS" ), sizeof(debug_stage)  - 1); } 





    s = splhigh();

    pw = (physical_write_t *) data;

    if (!pw->len) return 0;
    if ((( (unsigned) pw->dst ) & ~((1<< 12 ) -1) )  != (( (unsigned) (pw->dst + pw->len - 1) ) & ~((1<< 12 ) -1) ) ) {
      splx(s);
      return 22 ;
    }

    *(int *) CMAP1 = 0x001  | 0x002  | (( (unsigned) pw->dst ) & ~((1<< 12 ) -1) ) ;

    res = copyin(pw->data,
		 (char *) (CADDR1 + ((int) pw->dst & ((1<< 12 ) -1) )),
		 pw->len);

    *(int *) CMAP1 = 0;
    invltlb();

    splx(s);

    if (res)
      return res;
    break;

     

  case ((unsigned long)( (0x80000000 | 0x40000000 )   | ((  sizeof(  struct _physicalWrite )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  3 ) )))   :
    { strncpy(debug_stage,  ( "HSLWRITEPHYS2" ), sizeof(debug_stage)  - 1); } 





    s = splhigh();

    pw2 = (struct _physicalWrite *) data;

    *(int *) CMAP1 = 0x001  | 0x002  | (( (unsigned) pw2->addr ) & ~((1<< 12 ) -1) ) ;

    tmp = *(int *)(CADDR1 + ((int) pw2->addr & ((1<< 12 ) -1) ));
    pw2->value = tmp;

 

    *(int *) CMAP1 = 0;
    invltlb();

    splx(s);

    break;

     

  case ((unsigned long)( 0x80000000   | ((  sizeof(  hsl_put_init_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  4 ) )))   :
    { strncpy(debug_stage,  ( "HSLPUTINIT" ), sizeof(debug_stage)  - 1); } 





    res = put_init(((int)(( dev )&0xffff00ff)) ,
		   ((hsl_put_init_t *) data)->length,
		   ((hsl_put_init_t *) data)->node,
		   ((hsl_put_init_t *) data)->routing_table);




    return res;
     
    break;

     

  case ((unsigned long)( (0x80000000 | 0x40000000 )   | ((  sizeof(  caddr_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  5 ) )))   :
    { strncpy(debug_stage,  ( "HSLSLRPINIT" ), sizeof(debug_stage)  - 1); } 





    res = init_slr(((int)(( dev )&0xffff00ff)) );
    if (!res) *(caddr_t *) data = contig_space_phys;
    else *(caddr_t *) data = 0 ;





    return res;
     
    break;

     

  case ((unsigned long)( 0x20000000   | ((  0  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  6 ) )))   :
    { strncpy(debug_stage,  ( "HSLPUTEND" ), sizeof(debug_stage)  - 1); } 





    res = put_end(((int)(( dev )&0xffff00ff)) );
    return res;
     
    break;

     

  case ((unsigned long)( 0x20000000   | ((  0  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  7 ) )))   :
    { strncpy(debug_stage,  ( "HSLSLRPEND" ), sizeof(debug_stage)  - 1); } 





    end_slr();
    return 0;
     
    break;

     

  case ((unsigned long)( 0x40000000   | ((  sizeof(  int )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  17 ) )))   :
    { strncpy(debug_stage,  ( "HSLGETNODE" ), sizeof(debug_stage)  - 1); } 





    *(node_t *) data = put_get_node(((int)(( dev )&0xffff00ff)) );
    break;

     

  case ((unsigned long)( (0x80000000 | 0x40000000 )   | ((  sizeof(  opt_contig_mem_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  16 ) )))   :
    { strncpy(debug_stage,  ( "HSLGETOPTCONTIGMEM" ), sizeof(debug_stage)  - 1); } 





    ((opt_contig_mem_t *) data)->ptr  = (caddr_t) opt_contig_space;
    ((opt_contig_mem_t *) data)->size = (size_t)  opt_contig_size;
    ((opt_contig_mem_t *) data)->phys = (caddr_t) opt_contig_space_phys;
    return 0;
     
    break;

     

  case ((unsigned long)( (0x80000000 | 0x40000000 )   | ((  sizeof(  mpc_pci_conf_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  18 ) )))   :
    { strncpy(debug_stage,  ( "PCIREADCONF" ), sizeof(debug_stage)  - 1); } 










    s = splhigh();
# 2069 "driver.c"

    probe.bus  = ((mpc_pci_conf_t *) data)->bus;
    probe.slot = ((mpc_pci_conf_t *) data)->device;
    probe.func = ((mpc_pci_conf_t *) data)->func;

    ((mpc_pci_conf_t *) data)->data = 
      pci_cfgread(&probe, ((mpc_pci_conf_t *) data)->reg, 4);

    splx(s);

    return 0;
     
    break;

     

  case ((unsigned long)( 0x80000000   | ((  sizeof(  mpc_pci_conf_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  19 ) )))   :
    { strncpy(debug_stage,  ( "PCIWRITECONF" ), sizeof(debug_stage)  - 1); } 










    s = splhigh();
# 2106 "driver.c"

    probe.bus  = ((mpc_pci_conf_t *) data)->bus;
    probe.slot = ((mpc_pci_conf_t *) data)->device;
    probe.func = ((mpc_pci_conf_t *) data)->func;
    
    pci_cfgwrite(&probe, ((mpc_pci_conf_t *) data)->reg,
		 ((mpc_pci_conf_t *) data)->data, 4);

    splx(s);

    return 0;

     

  case ((unsigned long)( 0x40000000   | ((  sizeof(  trace_event_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  20 ) )))   :
    { strncpy(debug_stage,  ( "HSLGETEVENT" ), sizeof(debug_stage)  - 1); } 





    s = splhigh();

  try_event:
    event_p = get_event();
    if (!event_p) {



      res = tsleep(&event_ident,
		   (22  + 4)  | 0x100 ,
		   "GETEVENT",
		   0 );
      if (!res)
	goto try_event;  

      splx(s);

       



      if (res == (-1) ) res = 4 ;

      return res;
    }

    *(trace_event_t *) data = *event_p;

    return 0;
     
    break;

     

  case ((unsigned long)( 0x80000000   | ((  sizeof(  pid_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  21 ) )))   :
    { strncpy(debug_stage,  ( "HSLVMINFO" ), sizeof(debug_stage)  - 1); } 





    {
      int i = 0;
      pid_t pid;
      struct proc *pr;
      struct vm_map_entry *current_entry;
      char protstr[4], maxprotstr[4];
      char flagsstr[256];

      pid = *(pid_t *) data;
      pr = pfind(pid);
      log(6 , "Virtual memory map of process %d (proc : %p)\n", pid, pr);
      if (!pr) return 22 ;

      log(6 , "text %p (size: %d pages)\n", pr->p_vmspace->vm_taddr,
	  pr->p_vmspace->vm_tsize);
      log(6 , "data %p (size: %d pages)\n", pr->p_vmspace->vm_daddr,
	  pr->p_vmspace->vm_dsize);
      log(6 , "stack size: %d pages\n", pr->p_vmspace->vm_ssize);

       
      log(6 , "nentries in vm_map : %d\n",
	  pr->p_vmspace->vm_map.nentries);
      log(6 , "vm_map virtual size : 0x%x\n",
	  pr->p_vmspace->vm_map.size);

      current_entry = &pr->p_vmspace->vm_map.header;

      for (i = 0;
	   (i < 1 + pr->p_vmspace->vm_map.nentries) && current_entry;
	   i++, (current_entry = current_entry->next)) {
	protstr[0] = (current_entry->protection & ((vm_prot_t) 0x01) ) ?
	  'R' : ' ';
	protstr[1] = (current_entry->protection & ((vm_prot_t) 0x02) ) ?
	  'W' : ' ';
	protstr[2] = (current_entry->protection & ((vm_prot_t) 0x04) ) ?
	  'X' : ' ';
	protstr[3] = 0;
	maxprotstr[0] = (current_entry->max_protection & ((vm_prot_t) 0x01) ) ?
	  'R' : ' ';
	maxprotstr[1] = (current_entry->max_protection & ((vm_prot_t) 0x02) ) ?
	  'W' : ' ';
	maxprotstr[2] = (current_entry->max_protection & ((vm_prot_t) 0x04) ) ?
	  'X' : ' ';
	maxprotstr[3] = 0;
	log(6 , "---\n");
	log(6 , "vm_map_entry %d : %p -> %p / prot[%s] - max[%s]\n", i,
	    (caddr_t) current_entry->start,
	    (caddr_t) current_entry->end, protstr, maxprotstr);

	flagsstr[0] = 0;
	if (current_entry->eflags & 0x1 )
	  strcpy(flagsstr, " IS_A_MAP");
	if (current_entry->eflags & 0x2 )
	  strcat(flagsstr, " IS_SUBMAP");
	if (current_entry->eflags & 0x2 )
	  strcat(flagsstr, " COPY_ON_WRITE");
	if (current_entry->eflags & 0x8 )
	  strcat(flagsstr, " NEEDS_COPY");
	if (current_entry->eflags & 0x10 )
	  strcat(flagsstr, " NO_FAULT");
	if (current_entry->eflags & 0x20 )
	  strcat(flagsstr, " USER_WIRED");
	 





	log(6 , "%s inher:%d wired_cnt:%d offset:0x%lx\n", flagsstr,
	    current_entry->inheritance, current_entry->wired_count,
	    (u_long) current_entry->offset);


	 
	if (!(current_entry->eflags &
	      (0x1  | 0x2 )) &&
	    current_entry->object.vm_object) {
 

	  vmflags2str(flagsstr, current_entry->object.vm_object->flags);
# 2256 "driver.c"

	  log(6 , " OBJECT:%p size:%d refcnt:%d flags:%s pager:XXX off:0x%x [old: shadow:XXX off:XXX copy:XXX]\n",
	      current_entry->object.vm_object,
	      current_entry->object.vm_object->size,
	      current_entry->object.vm_object->ref_count,
	      flagsstr,
	       
	      (u_int) current_entry->object.vm_object->paging_offset);


	   


 
 
 

 


















	}
	
      }

    }

    return 0;
     
    break;


     

  case ((unsigned long)( 0x40000000   | ((  sizeof(  vm_object_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  22 ) )))   :
    { strncpy(debug_stage,  ( "MPCOSGETOBJ" ), sizeof(debug_stage)  - 1); } 









    return 0;
     
    break;

     

  case ((unsigned long)( (0x80000000 | 0x40000000 )   | ((  sizeof(  set_mode_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  26 ) )))   :
    { strncpy(debug_stage,  ( "HSLSETMODEREAD" ), sizeof(debug_stage)  - 1); } 





    return set_mode_read(((int)(( dev )&0xffff00ff)) ,
			 (set_mode_t *) data);
     
    break;

     

  case ((unsigned long)( 0x80000000   | ((  sizeof(  set_mode_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  27 ) )))   :
    { strncpy(debug_stage,  ( "HSLSETMODEWRITE" ), sizeof(debug_stage)  - 1); } 





    return set_mode_write(((int)(( dev )&0xffff00ff)) ,
			  (set_mode_t *) data);
     
    break;

     

  case ((unsigned long)( 0x80000000   | ((  sizeof(  struct linker_set * )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  23 ) )))   :
    { strncpy(debug_stage,  ( "PCISETBUS" ), sizeof(debug_stage)  - 1); } 













    return 0;

     

  case ((unsigned long)( 0x20000000   | ((  0  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  33 ) )))   :
    { strncpy(debug_stage,  ( "DUMPCHANNELSTATE" ), sizeof(debug_stage)  - 1); } 





    log(7 , "-------------- -------------------------\n");
    log(7 , "-------------- DUMPING channel_state[][]\n");

    s = splhigh();

    for (i = 0; i < (1<< 4 )  - 1; i++)
      for (j = 4 ; j < 8 ; j++) {
	if (channel_protocol[i][j] > HSL_PROTO_MDCP) {
	  log(3 ,
	      "channel_protocol[%d][%d] invalid\n",
	      i, j);
	  continue;
	}

	switch (channel_state[i][j]) {
	case CHAN_OPEN:








	  log(7 ,
	      "channel[%d][%d] state OPENED - proto %s - classname 0x%x (uid:%d)\n",
	      i, j,
	      protocol_names[channel_protocol[i][j]],
	      (int) channel_classname[i][j],
	      (int) (classname2uid[channel_classname[i][j]]));

	  break;

	case CHAN_SHUTDOWN_0:








	  log(7 ,
	      "channel[%d][%d] state SHUTDOWN 0 - proto %s - classname 0x%x (uid:%d)\n",
	      i, j,
	      protocol_names[channel_protocol[i][j]],
	      (int) channel_classname[i][j],
	      (int) classname2uid[channel_classname[i][j]]);

	  break;

	case CHAN_SHUTDOWN_1:








	  log(7 ,
	      "channel[%d][%d] state SHUTDOWN 1 - proto %s - classname 0x%x (uid:%d)\n",
	      i, j,
	      protocol_names[channel_protocol[i][j]],
	      (int) channel_classname[i][j],
	      (int) classname2uid[channel_classname[i][j]]);

	  break;

	case CHAN_SHUTDOWN_2:








	  log(7 ,
	      "channel[%d][%d] state SHUTDOWN 2 - proto %s - classname 0x%x (uid:%d)\n",
	      i, j,
	      protocol_names[channel_protocol[i][j]],
	      (int) channel_classname[i][j],
	      (int) classname2uid[channel_classname[i][j]]);

	  break;

	case CHAN_CLOSE:
	  break;

	default:








	  log(4 ,
	      "channel[%d][%d] invalid state - proto %s - classname 0x%x (uid:%d)\n",
	      i, j,
	      protocol_names[channel_protocol[i][j]],
	      (int) channel_classname[i][j],
	      (int) classname2uid[channel_classname[i][j]]);

	  break;
	}
      }

    splx(s);
    break;

     

  case ((unsigned long)( 0x20000000   | ((  0  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  28 ) )))   :
    { strncpy(debug_stage,  ( "DUMPINTERNALTABLES" ), sizeof(debug_stage)  - 1); } 





    s = splhigh();

    log(7 , "-------------- -------------------------\n");
    log(7 , "-------------- DUMPING pending_receive[]\n");

    for (i = 0; i < 10 ; i++)
      if ((*pending_receive)[i].valid == VALID) {
	log(7 ,
	    "pending_receive[%d]: mi=0x%x chn=%d seq=%d stage=%d micp_seq=%d dst=%d\n",
	    i,

	    (int)

	    (*pending_receive)[i].mi,
	    (*pending_receive)[i].channel,
	    (*pending_receive)[i].seq,




	    0,
	    0,

	    (*pending_receive)[i].sender);
      }

    log(7 , "-------------- DUMPING pending_send[]\n");

    for (i = 0; i < 10 ; i++)
      if (pending_send[i].valid != INVALID) {
	log(7 ,
	    "pending_send[%d]: valid=%d mi=0x%x chn=%d seq=%d dst=%d\n",
	    i,
	    pending_send[i].valid,

	    (int)

	    pending_send[i].mi,
	    pending_send[i].channel,
	    pending_send[i].seq,
	    pending_send[i].dest);
      }

    log(7 , "-------------- DUMPING received_receive[][]\n");

    for (i = 0; i < (1<<8)  ; i++)
      for (j = 0; j < (1<< 4 )  - 1; j++)
	if (recv_valid[i][j].general.valid == VALID) {
	  log(7 ,
	      "received_receive[%d][%d]: mi=0x%x chn=%d seq=%d dst=%d\n",
	      i, j,

	      (int)

	      (*received_receive)[i][j].general.mi,
	      (*received_receive)[i][j].general.channel,
	      (*received_receive)[i][j].general.seq,
	      (*received_receive)[i][j].general.dest);
	}

    splx(s);
    break;

     

  case ((unsigned long)( 0x80000000   | ((  sizeof(  struct _virtualRegion )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  29 ) )))   :
    { strncpy(debug_stage,  ( "HSLPROTECTWIRE" ), sizeof(debug_stage)  - 1); } 





    return slrpv_prot_wire((vm_offset_t) ((struct _virtualRegion *) data)->start,
			   ((struct _virtualRegion *) data)->size,
			   0 ,
			   0 ,
			   p);
     
    break;

     

  case ((unsigned long)( 0x20000000   | ((  0  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  30 ) )))   :
    { strncpy(debug_stage,  ( "HSLPROTECTDUMP" ), sizeof(debug_stage)  - 1); } 





    slrpv_prot_dump();

    return 0;
     
    break;

     

  case ((unsigned long)( 0x20000000   | ((  0  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  31 ) )))   :
    { strncpy(debug_stage,  ( "SLRVGARBAGECOLLECT" ), sizeof(debug_stage)  - 1); } 





    slrpv_prot_garbage_collection();

    return 0;
     
    break;

     

  case ((unsigned long)( 0x80000000   | ((  sizeof(  manager_open_channel_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  32 ) )))   :
    { strncpy(debug_stage,  ( "MANAGEROPENCHANNEL" ), sizeof(debug_stage)  - 1); } 





    switch (((manager_open_channel_t *) data)->protocol) {
    case HSL_PROTO_SLRP_P:
    case HSL_PROTO_SCP_P:
      res = slrpp_open_channel(((manager_open_channel_t *) data)->node,
			       ((manager_open_channel_t *) data)->chan0,
			       ((manager_open_channel_t *) data)->classname,
			       p);
      if (res) return res;

      res = slrpp_open_channel(((manager_open_channel_t *) data)->node,
			       ((manager_open_channel_t *) data)->chan1,
			       ((manager_open_channel_t *) data)->classname,
			       p);
      if (res) return res;

      channel_protocol[((( ((manager_open_channel_t *) data)->node ) < ( 
				      my_node )) ? ( ((manager_open_channel_t *) data)->node ) : (( ((manager_open_channel_t *) data)->node ) - 1)) ]
                      [((manager_open_channel_t *) data)->chan0] =
	((manager_open_channel_t *) data)->protocol;

      channel_protocol[((( ((manager_open_channel_t *) data)->node ) < ( 
				      my_node )) ? ( ((manager_open_channel_t *) data)->node ) : (( ((manager_open_channel_t *) data)->node ) - 1)) ]
                      [((manager_open_channel_t *) data)->chan1] =
	((manager_open_channel_t *) data)->protocol;
       
      break;

    case HSL_PROTO_SLRP_V:
    case HSL_PROTO_SCP_V:
      res = slrpv_open_channel(((manager_open_channel_t *) data)->node,
			       ((manager_open_channel_t *) data)->chan0,
			       ((manager_open_channel_t *) data)->classname,
			       p);
      if (res) return res;

      res = slrpv_open_channel(((manager_open_channel_t *) data)->node,
			       ((manager_open_channel_t *) data)->chan1,
			       ((manager_open_channel_t *) data)->classname,
			       p);
      if (res) return res;

      channel_protocol[((( ((manager_open_channel_t *) data)->node ) < ( 
				      my_node )) ? ( ((manager_open_channel_t *) data)->node ) : (( ((manager_open_channel_t *) data)->node ) - 1)) ]
                      [((manager_open_channel_t *) data)->chan0] =
	((manager_open_channel_t *) data)->protocol;

      channel_protocol[((( ((manager_open_channel_t *) data)->node ) < ( 
				      my_node )) ? ( ((manager_open_channel_t *) data)->node ) : (( ((manager_open_channel_t *) data)->node ) - 1)) ]
                      [((manager_open_channel_t *) data)->chan1] =
	((manager_open_channel_t *) data)->protocol;
       
      break;

    case HSL_PROTO_MDCP:
      res = mdcp_init_com(((manager_open_channel_t *) data)->node,
			  ((manager_open_channel_t *) data)->chan0,
			  ((manager_open_channel_t *) data)->chan1,
			  ((manager_open_channel_t *) data)->classname,
			  p);
      if (res) return res;

      channel_protocol[((( ((manager_open_channel_t *) data)->node ) < ( 
				      my_node )) ? ( ((manager_open_channel_t *) data)->node ) : (( ((manager_open_channel_t *) data)->node ) - 1)) ]
                      [((manager_open_channel_t *) data)->chan0] =
	((manager_open_channel_t *) data)->protocol;

      channel_protocol[((( ((manager_open_channel_t *) data)->node ) < ( 
				      my_node )) ? ( ((manager_open_channel_t *) data)->node ) : (( ((manager_open_channel_t *) data)->node ) - 1)) ]
                      [((manager_open_channel_t *) data)->chan1] =
	((manager_open_channel_t *) data)->protocol;
       
      break;

    default:
      return 43 ;
       
      break;
    }

    return 0;
     
    break;

     

  case ((unsigned long)( (0x80000000 | 0x40000000 )   | ((  sizeof(  manager_shutdown_1stStep_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  34 ) )))   :
    { strncpy(debug_stage,  ( "MANAGERSHUTDOWN1stSTEP" ), sizeof(debug_stage)  - 1); } 





    switch (((manager_shutdown_1stStep_t *) data)->protocol) {
    case HSL_PROTO_SLRP_P:
    case HSL_PROTO_SCP_P:
      res = slrpp_shutdown0_channel(((manager_shutdown_1stStep_t *) data)->node,
				    ((manager_shutdown_1stStep_t *) data)->chan0,
				    &((manager_shutdown_1stStep_t *) data)->ret_seq_send_0,
				    &((manager_shutdown_1stStep_t *) data)->ret_seq_recv_0,
				    p);
      if (res) return res;

      res = slrpp_shutdown0_channel(((manager_shutdown_1stStep_t *) data)->node,
				    ((manager_shutdown_1stStep_t *) data)->chan1,
				    &((manager_shutdown_1stStep_t *) data)->ret_seq_send_1,
				    &((manager_shutdown_1stStep_t *) data)->ret_seq_recv_1,
				    p);
      return res;
       
      break;

    case HSL_PROTO_SLRP_V:
    case HSL_PROTO_SCP_V:
      res = slrpv_shutdown0_channel(((manager_shutdown_1stStep_t *) data)->node,
				    ((manager_shutdown_1stStep_t *) data)->chan0,
				    &((manager_shutdown_1stStep_t *) data)->ret_seq_send_0,
				    &((manager_shutdown_1stStep_t *) data)->ret_seq_recv_0,
				    p);
      if (res) return res;

      res = slrpv_shutdown0_channel(((manager_shutdown_1stStep_t *) data)->node,
				    ((manager_shutdown_1stStep_t *) data)->chan1,
				    &((manager_shutdown_1stStep_t *) data)->ret_seq_send_1,
				    &((manager_shutdown_1stStep_t *) data)->ret_seq_recv_1,
				    p);
      return res;
       
      break;

    case HSL_PROTO_MDCP:
      res = mdcp_shutdown0_com(((manager_shutdown_1stStep_t *) data)->node,
			       ((manager_shutdown_1stStep_t *) data)->chan0,
			       &((manager_shutdown_1stStep_t *) data)->ret_seq_send_0,
			       &((manager_shutdown_1stStep_t *) data)->ret_seq_recv_0,
			       &((manager_shutdown_1stStep_t *) data)->ret_seq_send_1,
			       &((manager_shutdown_1stStep_t *) data)->ret_seq_recv_1,
			       p);
      return res;
       
      break;

    default:
      return 43 ;
       
      break;
    }

    return 0;
     
    break;

     

  case ((unsigned long)( 0x80000000   | ((  sizeof(  manager_shutdown_2ndStep_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  36 ) )))   :
    { strncpy(debug_stage,  ( "MANAGERSHUTDOWN2ndSTEP" ), sizeof(debug_stage)  - 1); } 





    switch (((manager_shutdown_2ndStep_t *) data)->protocol) {
    case HSL_PROTO_SLRP_P:
    case HSL_PROTO_SCP_P:
      res = slrpp_shutdown1_channel(((manager_shutdown_2ndStep_t *) data)->node,
				    ((manager_shutdown_2ndStep_t *) data)->chan0,
				    ((manager_shutdown_2ndStep_t *) data)->seq_send_0,
				    ((manager_shutdown_2ndStep_t *) data)->seq_recv_0,
				    p);
      if (res) return res;

      res = slrpp_shutdown1_channel(((manager_shutdown_2ndStep_t *) data)->node,
				    ((manager_shutdown_2ndStep_t *) data)->chan1,
				    ((manager_shutdown_2ndStep_t *) data)->seq_send_1,
				    ((manager_shutdown_2ndStep_t *) data)->seq_recv_1,
				    p);
      return res;
       
      break;

    case HSL_PROTO_SLRP_V:
    case HSL_PROTO_SCP_V:
      res = slrpv_shutdown1_channel(((manager_shutdown_2ndStep_t *) data)->node,
				    ((manager_shutdown_2ndStep_t *) data)->chan0,
				    ((manager_shutdown_2ndStep_t *) data)->seq_send_0,
				    ((manager_shutdown_2ndStep_t *) data)->seq_recv_0,
				    p);
      if (res) return res;

      res = slrpv_shutdown1_channel(((manager_shutdown_2ndStep_t *) data)->node,
				    ((manager_shutdown_2ndStep_t *) data)->chan1,
				    ((manager_shutdown_2ndStep_t *) data)->seq_send_1,
				    ((manager_shutdown_2ndStep_t *) data)->seq_recv_1,
				    p);
      return res;
       
      break;

    case HSL_PROTO_MDCP:
      res = mdcp_shutdown1_com(((manager_shutdown_2ndStep_t *) data)->node,
			       ((manager_shutdown_2ndStep_t *) data)->chan0,
			       ((manager_shutdown_2ndStep_t *) data)->seq_send_0,
			       ((manager_shutdown_2ndStep_t *) data)->seq_recv_0,
			       ((manager_shutdown_2ndStep_t *) data)->seq_send_1,
			       ((manager_shutdown_2ndStep_t *) data)->seq_recv_1,
			       p);
      return res;
       
      break;

    default:
      return 43 ;
       
      break;
    }

    return 0;
     
    break;

     

  case ((unsigned long)( 0x80000000   | ((  sizeof(  manager_shutdown_3rdStep_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  37 ) )))   :
    { strncpy(debug_stage,  ( "MANAGERSHUTDOWN3rdSTEP" ), sizeof(debug_stage)  - 1); } 





    switch (((manager_shutdown_3rdStep_t *) data)->protocol) {
    case HSL_PROTO_SLRP_P:
    case HSL_PROTO_SCP_P:
      res = slrpp_close_channel(((manager_shutdown_3rdStep_t *) data)->node,
				((manager_shutdown_3rdStep_t *) data)->chan0,
				p);
      if (res) return res;

      res = slrpp_close_channel(((manager_shutdown_3rdStep_t *) data)->node,
				((manager_shutdown_3rdStep_t *) data)->chan1,
				p);
      return res;
       
      break;

    case HSL_PROTO_SLRP_V:
    case HSL_PROTO_SCP_V:
      res = slrpv_close_channel(((manager_shutdown_3rdStep_t *) data)->node,
				((manager_shutdown_3rdStep_t *) data)->chan0,
				p);
      if (res) return res;

      res = slrpv_close_channel(((manager_shutdown_3rdStep_t *) data)->node,
				((manager_shutdown_3rdStep_t *) data)->chan1,
				p);
      return res;
       
      break;

    case HSL_PROTO_MDCP:
      res = mdcp_end_com(((manager_shutdown_3rdStep_t *) data)->node,
			 ((manager_shutdown_3rdStep_t *) data)->chan0,
			 p);
      if (res && res != 35  && res != 57  && res != 2 )
	log(3 , "error: mdcp_end_com() -> %d\n", res);

      return res;
       
      break;

    default:
      return 43 ;
       
      break;
    }

    return 0;
     
    break;

     

  case ((unsigned long)( 0x80000000   | ((  sizeof(  manager_set_appclassname_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  35 ) )))   :




    slrpp_set_appclassname(((manager_set_appclassname_t *) data)->cn,
			   ((manager_set_appclassname_t *) data)->uid);

    return 0;
     
    break;

     

  case ((unsigned long)( (0x80000000 | 0x40000000 )   | ((  sizeof(  libmpc_read_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  38 ) )))   :




    if (((libmpc_read_t *) data)->channel < 4 )
      return 1 ;

     



    if (slrpp_channel_check_rights(((libmpc_read_t *) data)->node,
				   ((libmpc_read_t *) data)->channel,
				   p) == 0 )
      return 1 ;

     


    if (channel_state[((( ((libmpc_read_t *) data)->node ) < (  my_node )) ? ( ((libmpc_read_t *) data)->node ) : (( ((libmpc_read_t *) data)->node ) - 1)) ]
                     [((libmpc_read_t *) data)->channel] != CHAN_OPEN) {
      ((libmpc_read_t *) data)->nbytes = 0;
       



      return   0;
    }

    if (channel_protocol[((( ((libmpc_read_t *) data)->node ) < (  my_node )) ? ( ((libmpc_read_t *) data)->node ) : (( ((libmpc_read_t *) data)->node ) - 1)) ]
                        [((libmpc_read_t *) data)->channel] != HSL_PROTO_MDCP)
      return 43 ;

    res = mdcp_read(((libmpc_read_t *) data)->node,
		    ((libmpc_read_t *) data)->channel,
		    ((libmpc_read_t *) data)->buf,
		    ((libmpc_read_t *) data)->nbytes,
		    &((libmpc_read_t *) data)->nbytes,
		    p);

    return res;
     
    break;

     

  case ((unsigned long)( (0x80000000 | 0x40000000 )   | ((  sizeof(  libmpc_write_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  39 ) )))   :




    if (((libmpc_write_t *) data)->channel < 4 )
      return 1 ;

     



    if (slrpp_channel_check_rights(((libmpc_write_t *) data)->node,
				   ((libmpc_write_t *) data)->channel,
				   p) == 0 )
      return 1 ;

     


    if (channel_state[((( ((libmpc_write_t *) data)->node ) < (  my_node )) ? ( ((libmpc_write_t *) data)->node ) : (( ((libmpc_write_t *) data)->node ) - 1)) ]
                     [((libmpc_write_t *) data)->channel] != CHAN_OPEN)
      return 57 ;

    if (channel_protocol[((( ((libmpc_write_t *) data)->node ) < (  my_node )) ? ( ((libmpc_write_t *) data)->node ) : (( ((libmpc_write_t *) data)->node ) - 1)) ]
                        [((libmpc_read_t *) data)->channel] != HSL_PROTO_MDCP)
      return 43 ;

    res = mdcp_write(((libmpc_write_t *) data)->node,
		    ((libmpc_write_t *) data)->channel,
		    (caddr_t) ((libmpc_write_t *) data)->buf,
		    ((libmpc_write_t *) data)->nbytes,
		    &((libmpc_write_t *) data)->nbytes,
		    p);

    return res;
     
    break;

     

  case ((unsigned long)( (0x80000000 | 0x40000000 )   | ((  sizeof(  libmpc_select_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  58 ) )))   :




     

    res = hsl_select(p, (libmpc_select_t *) data,
		     &((libmpc_select_t *) data)->retval);

    return res;
     
    break;

     

  case ((unsigned long)( (0x80000000 | 0x40000000 )   | ((  sizeof(  hsl_bench_latence_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  59 ) )))   :




    res = hsl_bench_latence1(((int)(( dev )&0xffff00ff)) , (hsl_bench_latence_t *) data);

    return res;
     
    break;

     

  case ((unsigned long)( (0x80000000 | 0x40000000 )   | ((  sizeof(  hsl_bench_throughput_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  60 ) )))   :




    res = hsl_bench_throughput1(((int)(( dev )&0xffff00ff)) , (hsl_bench_throughput_t *) data);

    return res;
     
    break;

     

  case ((unsigned long)( 0x20000000   | ((  0  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  57 ) )))   :

    do {
      res = tsleep(&selwait, 24  | 0x100 , "SeLeCt", 0 );
      if (!res)
	log(3 , "GLOBAL SELECT WAKEUP\n");
    } while (!res);

    return 0;
     
    break;

     

  case ((unsigned long)( 0x40000000   | ((  sizeof(  tables_info_t )  & 0x1fff ) << 16) | (( 	( 'H' ) ) << 8) | (  (  25 ) )))   :




    get_tables_info(((int)(( dev )&0xffff00ff)) , (tables_info_t *) data);

    return 0;
     
    break;
  }

  return 0;
}


 









static int

hsldriver_load(lkmtp, cmd)



     struct lkm_table *lkmtp;
     int cmd;
{

  struct lkm_table lkmt;
  struct lkm_any lkma;




  pcicfgregs probe;

  int device;
  u_long val;
  int s;
  boolean_t notfound = 1 ;
  int current_minor = 0;

  { strncpy(debug_stage,  ( "hsldriver_load" ), sizeof(debug_stage)  - 1); } 







   
  lkmt.private.lkm_any = &lkma;
  lkma.lkm_name = "cmemdriver_mod";
  if (!lkmexists(&lkmt)) {
    log(3 ,
	"Can't load hsldriver while cmemdriver not loaded.\n");
    return -1;
  }


  bzero(&driver_stats,
	sizeof driver_stats);

  init_events();

  { int s = splhigh(); add_event(TRACE_VAR, ( TRACE_INIT_DRIVER ), (trace_t) trace_var); splx(s); }; ;

  slr_clear_config();
  put_init_SAP();

   
  for (device = 0 ;
       device < 32 ;
       device++) {
    s = splhigh();







    probe.bus  = 0 ;
    probe.slot = device;
    probe.func = 0 ;
    val = pci_cfgread(&probe, 0x00 , 4);

    splx(s);

    if (val == 0x0 ) {
       
      notfound = 0 ;
      put_set_device(current_minor++,



                     probe);

      log(7 ,
	  "\nHSL board found : device %d\n", device);





      printf("\nVersion : PCIDDC 2nd Run\n");

    }
  }

  if (notfound == 1 )
    log(7 ,
	"\nHSL board not found\n");

  printf("\nHSL driver loaded with success\n(c) Laboratoire LIP6 / Departement ASIM\n    Universite Pierre et Marie Curie (Paris 6)\n(feel free to contact fenyo@asim.lip6.fr for informations about this driver)\n\n");

  return 0;
}


 









static int

hsldriver_unload(lkmtp, cmd)



     struct lkm_table *lkmtp;
     int cmd;
{
  { strncpy(debug_stage,  ( "hsldriver_unload" ), sizeof(debug_stage)  - 1); } 






 
 

  close_slr();
  put_end_SAP();

  return 0;
}


 









static int

hsldriver_stat(lkmtp, cmd)



        struct lkm_table *lkmtp;
        int cmd;
{
  return 0;
}


 












int
hslinit(lkmtp, cmd, ver)
        struct lkm_table *lkmtp;
        int cmd;
        int ver;
{
  { strncpy(debug_stage,  ( "hslinit" ), sizeof(debug_stage)  - 1); } 

# 3271 "driver.c"

  if (  
	   ver   != 1 )	return 22 ;	switch (  
	   cmd  ) {	int	error;	case 1 :	  
	   lkmtp  ->private.lkm_any =	(struct lkm_any *)&    hsldriver_mod_struct  ;	if (lkmexists(   	   lkmtp  )) return 17 ;	if ((error =   
	   hsldriver_load  (   	   lkmtp  ,    	   cmd  )))	return error;	break;	case 2 :	if ((error =   
	   hsldriver_unload  (   	   lkmtp  ,    	   cmd  )))	return error;	break;	case 3 :	if ((error =   
	   hsldriver_stat  (   	   lkmtp  ,    	   cmd  )))	return error;	break;	}	return lkmdispatch(   	   lkmtp  ,    	   cmd  );  

}

# 3313 "driver.c"

