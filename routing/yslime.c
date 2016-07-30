#ifndef lint
static char const yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93";
#endif
#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYLEX yylex()
#define YYEMPTY -1
#define yyclearin (yychar=(YYEMPTY))
#define yyerrok (yyerrflag=0)
#define YYRECOVERING (yyerrflag!=0)
#if defined(c_plusplus) || defined(__cplusplus)
#include <stdlib.h>
#else
extern char *getenv();
extern void *realloc();
#endif
static int yygrowstack();
#define YYPREFIX "yy"
#line 2 "yslime.y"
typedef union {
	long i;
	char *s;
} YYSTYPE;
#line 26 "y.tab.c"
#define NUM 257
#define ID 258
#define IDT 259
#define STRING 260
#define DO 261
#define SET 262
#define NETWORK 263
#define NODE 264
#define OF 265
#define AT 266
#define TO 267
#define router 268
#define processor 269
#define generator 270
#define WITH 271
#define CONNECT 272
#define END 273
#define YYERRCODE 256
const short yylhs[] = {                                        -1,
    0,    0,    0,    5,    6,    5,    3,    3,    4,    4,
    9,   10,   12,    8,   13,    8,   16,    2,    2,    2,
    2,    2,   17,   15,   15,   15,   15,    7,    7,   11,
   11,    1,    1,    1,   14,   14,   14,
};
const short yylen[] = {                                         2,
    2,    5,    4,    1,    0,    4,    4,    6,    2,    0,
    0,    0,    0,    8,    0,    9,    0,    3,    1,    1,
    1,    1,    0,    5,    3,    6,    3,    1,    0,    2,
    0,    1,    1,    1,    1,    1,    1,
};
const short yydefred[] = {                                      0,
    0,    0,    0,    0,    0,   28,    0,    0,    0,    0,
    1,    0,    0,   15,   11,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    3,    9,    0,    0,    0,    0,
    7,    2,   19,   17,   22,   21,   20,    0,   12,   24,
    0,    0,   32,   33,   34,    0,    0,    0,    0,   26,
    6,    8,   18,    0,    0,    0,   13,    0,    0,   35,
   36,   37,    0,    0,   14,   16,   30,
};
const short yydgoto[] = {                                       3,
   46,   38,    4,   16,   21,   29,    5,   17,   24,   49,
   65,   59,   23,   63,    6,   47,   27,
};
const short yysindex[] = {                                    -88,
 -227, -244,    0,  -88, -258,    0, -253, -255,  -66,  -64,
    0, -219, -255,    0,    0, -233, -255,    0,  -50,    0,
  -57, -231, -249, -249,    0,    0,  -48, -213,    1, -232,
    0,    0,    0,    0,    0,    0,    0,    6,    0,    0,
  -46, -219,    0,    0,    0,  -10,  -48, -249, -218,    0,
    0,    0,    0,   -8, -249,   -9,    0, -234, -217,    0,
    0,    0,   12, -203,    0,    0,    0,
};
const short yyrindex[] = {                                   -208,
    0,    0,    0, -208,    0,    0,    0, -216,    0,    0,
    0,    0, -216,    0,    0,    0, -216,  -40,  -28,  -42,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0, -240,    0,
    0,    0,    0,    0,    0,    0,    0,
};
const short yygindex[] = {                                     54,
    0,  -20,    0,   -2,   17,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  -22,    0,    0,
};
#define YYTABLESIZE 245
const short yytable[] = {                                      25,
   31,    5,    2,   39,   40,   12,   14,   13,   33,   34,
   22,   27,    9,   10,   26,    4,   15,   25,   35,   36,
   37,   31,   60,   61,   53,   62,   18,   54,   19,   27,
    7,   31,   31,    8,   57,   43,   44,   45,   20,   25,
   28,   32,    2,   41,   42,   48,   50,   52,   55,   56,
   23,   58,   66,   64,   67,   29,   10,   11,   51,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    1,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   30,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   25,    4,   25,    0,    0,   25,    0,    0,    0,
   25,   25,   25,   27,    0,   27,    0,    0,   27,    0,
    0,    0,   27,   27,   27,
};
const short yycheck[] = {                                      40,
   58,   44,   91,   24,   27,  264,  262,  261,  258,  259,
   13,   40,  257,  258,   17,   58,  272,   58,  268,  269,
  270,  262,  257,  258,   47,  260,   93,   48,   93,   58,
  258,  272,  273,  261,   55,  268,  269,  270,  258,  273,
   91,  273,   91,  257,   44,   40,   93,   58,  267,   58,
   91,   61,   41,  271,  258,  264,  273,    4,   42,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  263,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  265,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  262,  265,  264,   -1,   -1,  267,   -1,   -1,   -1,
  271,  272,  273,  262,   -1,  264,   -1,   -1,  267,   -1,
   -1,   -1,  271,  272,  273,
};
#define YYFINAL 3
#ifndef YYDEBUG
#define YYDEBUG 0
#elif YYDEBUG
#include <stdio.h>
#endif
#define YYMAXTOKEN 273
#if YYDEBUG
const char * const yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,"'('","')'",0,0,"','",0,0,0,0,0,0,0,0,0,0,0,0,0,"':'",0,0,"'='",0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"'['",0,"']'",0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"NUM",
"ID","IDT","STRING","DO","SET","NETWORK","NODE","OF","AT","TO","router",
"processor","generator","WITH","CONNECT","END",
};
const char * const yyrule[] = {
"$accept : depart",
"depart : declarations depart",
"depart : NETWORK ID DO line_list END",
"depart : NETWORK DO line_list END",
"fab_liste_identificateurs : ID",
"$$1 :",
"fab_liste_identificateurs : ID $$1 ',' fab_liste_identificateurs",
"declarations : optional_tableau NODE fab_liste_identificateurs ':'",
"declarations : optional_tableau NODE fab_liste_identificateurs OF type ':'",
"line_list : realstart line_list",
"line_list :",
"$$2 :",
"$$3 :",
"$$4 :",
"realstart : CONNECT $$2 free_ID $$3 TO free_ID $$4 optional_with",
"$$5 :",
"realstart : SET $$5 free_ID '(' free_ID ':' '=' some_types ')'",
"$$6 :",
"free_ID : IDT $$6 tableau",
"free_ID : ID",
"free_ID : generator",
"free_ID : processor",
"free_ID : router",
"$$7 :",
"tableau : '[' NUM ']' $$7 tableau",
"tableau : '[' NUM ']'",
"tableau : '[' ID ']' '[' NUM ']'",
"tableau : '[' ID ']'",
"optional_tableau : tableau",
"optional_tableau :",
"optional_with : WITH ID",
"optional_with :",
"type : router",
"type : processor",
"type : generator",
"some_types : NUM",
"some_types : ID",
"some_types : STRING",
};
#endif
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 500
#define YYMAXDEPTH 500
#endif
#endif
int yydebug;
int yynerrs;
int yyerrflag;
int yychar;
short *yyssp;
YYSTYPE *yyvsp;
YYSTYPE yyval;
YYSTYPE yylval;
short yyss[YYSTACKSIZE];
YYSTYPE yyvs[YYSTACKSIZE];
#define yystacksize YYSTACKSIZE
#line 191 "yslime.y"
#include <string.h>

#include "autorouting.h"
#include "reseau.h"
#include "globals.h"
#include "misc.h"

int nb_interv=0,nb_id=0,nb_element=0,found_num,first_num,second_num;
char indic=0,tempo,part=0;
int interv[10];
char *id[10];
char s[50],*found,*for_first,*for_second;

Noeud *Select(char *name)
{ Noeud *first,*n;
  char name_o[100];	
  int i=Inconnu,j=0;
  NameOnly(name,name_o);
  if ((first=Chercher(Desc,name))==NULL
      || (first->N.Type==Inconnu)
     )
  {	
	n=Premier(Desc);
	while (n!=NULL)
	{
	  if (n->N.Nom[0]=='@'
	      && strcasecmp(&n->N.Nom[1],name_o)==0
	     ) { i=n->N.Type;
		 /*printf(" %s %d*",n->N.Nom,n->N.Type);*/
                 n=NULL;	
               } else n=Suivant(Desc);
	}
	if (i==Inconnu)
	    { char s[80];
	      sprintf(s,"\nComponent %s hasn't been set yet.\n",name);
	      Erreur(s,-1);	
	    }			
	if (first==NULL) Desc=Ajouter(Desc,(first=Nouveau(name,i,Configuration[i].nb_ports)));
	else first->N.Type=i;

	if (Prime==NULL && Configuration[i].types==Routeur) Prime=first;
	/*printf("%s %d\n",first->N.Nom,first->N.Type);*/
  } 
  return first;
}				

int ChercherDansConfig(char *name) 
{char s[42];
 int i,tempo;
 /*printf("/%s/ %d \n",name,nb_config);*/
 i=0;
 while (i<nb_config)
  { 
	/*printf(";%s\n",Configuration[i].nom);*/
	if (strcasecmp(name,Configuration[i].nom)==0) break;
	i++;
  }
/*printf("*%d*",i);*/
 if (i<nb_config) tempo=i;
 else tempo=Inconnu;

 return tempo;
}













#line 304 "y.tab.c"
#define YYABORT goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab

int
yyparse()
{
    register int yym, yyn, yystate;
#if YYDEBUG
    register const char *yys;

    if ((yys = getenv("YYDEBUG")))
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = (-1);

    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if ((yyn = yydefred[yystate])) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yyssp >= yyss + yystacksize - 1)
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
        yychar = (-1);
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;
#if defined(lint) || defined(__GNUC__)
    goto yynewerror;
#endif
yynewerror:
    yyerror("syntax error");
#if defined(lint) || defined(__GNUC__)
    goto yyerrlab;
#endif
yyerrlab:
    ++yynerrs;
yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yyss + yystacksize - 1)
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = (-1);
        goto yyloop;
    }
yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    yyval = yyvsp[1-yym];
    switch (yyn)
    {
case 2:
#line 17 "yslime.y"
{ Go=0; }
break;
case 3:
#line 18 "yslime.y"
{ Go=0; }
break;
case 4:
#line 21 "yslime.y"
{ id[nb_id++]=yyvsp[0].s; }
break;
case 5:
#line 22 "yslime.y"
{ id[nb_id++]=yyvsp[0].s; }
break;
case 7:
#line 27 "yslime.y"
{
		 nb_interv=0;
		 nb_id=0;
			
		}
break;
case 8:
#line 33 "yslime.y"
{ int i,tempo;
		  char nom[100];
		  Noeud *n;	
                  for (i=0;i<nb_id;i++)
		  { sprintf(nom,"@%s",id[i]);
			/*printf("\n%s",id[i]);*/
			/* ajout d'une classe routeur/processeur */
		    if (yyvsp[-1].i==Routeur) tempo=ChercherDansConfig("\"router\"");
		    else tempo=ChercherDansConfig("\"generator\"");
		    if (tempo==Inconnu)
			Erreur("Type par defaut non trouvable.\n",-1);	
		    Desc=Ajouter(Desc,(n=Nouveau(nom,tempo,Configuration[tempo].nb_ports)));
		  }
		  nb_interv=0;
		  nb_id=0;
		}
break;
case 11:
#line 56 "yslime.y"
{ 	part=0;
			found=NULL;
			for_second=NULL;
		    }
break;
case 12:
#line 60 "yslime.y"
{
			for_first=found;
			first_num=found_num;
			found=NULL;
		    }
break;
case 13:
#line 66 "yslime.y"
{
			for_second=found;
			second_num=found_num;
		    }
break;
case 14:
#line 71 "yslime.y"
{
	 	Noeud *first,* second;
		struct Champ *o;
		int i,first_pos,second_pos;
		if (for_first==NULL || for_second==NULL)
			Erreur("A port name was expected.\n",-1);
	  	first=Select(yyvsp[-5].s);
	  	i=first->N.Type;
		o=FindIn(Configuration[i].Interface,for_first,&first_pos);
		if (first_num<o->First || first_num>o->Last)
			Erreur("Indice hors interval.\n",-1);
		first_pos+=first_num-o->First;

	  	second=Select(yyvsp[-2].s);
	  	i=second->N.Type;
		o=FindIn(Configuration[i].Interface,for_second,&second_pos);
		if (second_num<o->First || second_num>o->Last)
			Erreur("Indice hors interval.\n",-1);
		second_pos+=second_num-o->First;
		/*printf(" %d %d\n",first_pos,second_pos);*/
	 	Ajouter_Connexion(first_pos,first,second_pos,second);
	}
break;
case 15:
#line 93 "yslime.y"
{ part=1; }
break;
case 16:
#line 94 "yslime.y"
{ if (strcasecmp(yyvsp[-4].s,"type")==0)
	  { if ( tempo == Inconnu )
	    { char s[40];
	      sprintf(s,"\nComponent %s has an unknown type.\n",yyvsp[-6].s);
	      Erreur(s,-1);
	    } else { Noeud *n;
		     /*printf(".%s.",$2);*/	
	  	     if ((n=Chercher(Desc,yyvsp[-6].s))==NULL)
			Desc=Ajouter(Desc,(n=Nouveau(yyvsp[-6].s,tempo,Configuration[tempo].nb_ports)));
		     else n->N.Type=tempo;
		     if (Prime==NULL) Prime=n;
		   }
	  }
	  else {	int i;
			Noeud *n;
			sprintf(s,"\"%s\"",yyvsp[-6].s);
			i=ChercherDansConfig(s);
			if (keep_probes && i!=Inconnu)
			{
			  /*if (i==Inconnu) Erreur("????\n",-1);*/
			  NameOnly(yyvsp[-4].s,s);	
			  if (FindIn(Configuration[i].Probes,s,&i)!=NULL)
				 Go=0;
			}
  	     	        n=Chercher(Desc,yyvsp[-6].s);
			NameOnly(yyvsp[-4].s,s);
			if (keep_config)
			{ 	
	  	     	  if (n!=NULL)
			  {
			     if (FindIn(Configuration[n->N.Type].CompoCfg,s,&i)!=NULL)
				 Go=0;	
			  }
			}

	  	     	if (n!=NULL)
			{	
			    if (FindIn(Configuration[n->N.Type].ToUpdate,s,&i)!=NULL)
				 Go=0;
			}

		}

	}
break;
case 17:
#line 140 "yslime.y"
{	
			strcpy(s,yyvsp[0].s);
		}
break;
case 18:
#line 143 "yslime.y"
{ 
			/*sprintf(s,"%s%s",$1,s);*/
			yyval.s = strdup(s);
		 }
break;
case 19:
#line 147 "yslime.y"
{ yyval.s = yyvsp[0].s; }
break;
case 20:
#line 148 "yslime.y"
{ yyval.s = "generator"; }
break;
case 21:
#line 149 "yslime.y"
{ yyval.s = "generator"; }
break;
case 22:
#line 150 "yslime.y"
{ yyval.s = "router"; }
break;
case 23:
#line 153 "yslime.y"
{ sprintf(s,"%s[%d]",s,yyvsp[-1].i); }
break;
case 25:
#line 154 "yslime.y"
{ sprintf(s,"%s[%d]",s,yyvsp[-1].i); }
break;
case 26:
#line 156 "yslime.y"
{	
		/*sprintf(s,"%s[%s][%d]",s,$2,$5);*/
		found=yyvsp[-4].s;
		found_num=yyvsp[-1].i;
	 }
break;
case 27:
#line 162 "yslime.y"
{
		/*sprintf(s,"%s[%s]",s,$2);*/
		found=yyvsp[-1].s;
		found_num=0;
	 }
break;
case 32:
#line 178 "yslime.y"
{ yyval.i=Routeur; }
break;
case 33:
#line 179 "yslime.y"
{ yyval.i = Processeur; }
break;
case 34:
#line 180 "yslime.y"
{ yyval.i = Processeur; }
break;
case 37:
#line 186 "yslime.y"
{ tempo=ChercherDansConfig(yyvsp[0].s);
	  }
break;
#line 656 "y.tab.c"
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yyssp, yystate);
#endif
    if (yyssp >= yyss + yystacksize - 1)
    {
        goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;
yyoverflow:
    yyerror("yacc stack overflow");
yyabort:
    return (1);
yyaccept:
    return (0);
}
