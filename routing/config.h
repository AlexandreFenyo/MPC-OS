#ifndef __CONFIG
#define __CONFIG

struct Champ
{
  char *Nom;
  int First;
  int Last;
  struct Champ *next;
};

struct Config
{
  char *nom;
  char types;
  char *name;
  char used;
  struct Champ *Interface;
  struct Champ *NodeCfg;
  struct Champ *CompoCfg;
  struct Champ *Probes;
  struct Champ *ToUpdate;
  char *path;

  int nb_ports;
};

struct SML
{ char *path;
  char *name;
  struct SML *next;
};

#define MAX_IN_CONFIG 100

typedef struct Config CFG;
typedef struct SML ML;

struct Libr
{
  char *nom;
  int nbconf;	
  CFG Conf[150];
};

typedef struct Libr Library;


extern int LoadConfigurationFile(char *nom,char *libra);
extern struct Champ *FindIn(struct Champ *name,char *what,int *);

#endif

















