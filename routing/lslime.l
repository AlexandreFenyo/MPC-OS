%{
#include "yslime.h"
#include "globals.h"
#include <stdlib.h>
#include <stdio.h>

extern int ligne;
%}
lettre [a-zA-Z]
chiffre [0-9]
nombre {chiffre}+
ident {lettre}({lettre}|{chiffre}|"."|"_")*
%%
\n(" ")*":"  {strcpy(dummy,"");
	      ligne ++;
	      return END;
	     }
\n           { if (Go) 
		 {
		   fprintf(sor,"%s\n",dummy);
		 }
	       Go=1;
	       strcpy(dummy,"");
 	       ligne ++;
	     }

-?{nombre}   { yylval.i=atol(yytext);
	       sprintf(dummy,"%s%s",dummy,yytext);
	       return NUM;
	     }  
[\[\]\.\:\=\(\)]  { sprintf(dummy,"%s%c",dummy,*yytext);
		    return *yytext;
		  }
[" "\t]      { sprintf(dummy,"%s%c",dummy,*yytext);
	     }

NODE         { sprintf(dummy,"%s%s",dummy,yytext);return NODE; }
OF           { sprintf(dummy,"%s%s",dummy,yytext);return OF; }
NETWORK      { sprintf(dummy,"%s%s",dummy,yytext);return NETWORK; }
DO           { sprintf(dummy,"%s%s",dummy,yytext);return DO; }
CONNECT      { sprintf(dummy,"%s%s",dummy,yytext);return CONNECT; }
AT           { sprintf(dummy,"%s%s",dummy,yytext);return AT; }
TO           { sprintf(dummy,"%s%s",dummy,yytext);return TO; }
SET          { sprintf(dummy,"%s%s",dummy,yytext);return SET; }
"--"         { char e;
	       sprintf(dummy,"%s--",dummy);
	       do { e=input();
		    if (e=='\n') break;
		    sprintf(dummy,"%s%c",dummy,e);
	          } while (1);
	       unput(e);
	     }
DISPLAY      { char e;
	       sprintf(dummy,"%sDISPLAY",dummy);
	       do { e=input();
		    if (e=='\n') break;
		    sprintf(dummy,"%s%c",dummy,e);
	          } while (1);
	       unput(e);
	     }
WITH         { sprintf(dummy,"%s%s",dummy,yytext);return WITH; }

"router"     { sprintf(dummy,"%s%s",dummy,yytext);return router; }	
"generator"  { sprintf(dummy,"%s%s",dummy,yytext);return generator; }	
"processor"  { sprintf(dummy,"%s%s",dummy,yytext);return processor; }	

{ident}      { if ((yylval.s=malloc(strlen(yytext)+2))==NULL) yyerror("erreur d'allocation");
		strcpy(yylval.s,yytext);
/*
	      printf("<id:%s>\n",yylval.s);
*/
	       sprintf(dummy,"%s%s",dummy,yytext);
	      return ID; 
	     }
{ident}/(" "*"[")    { if ((yylval.s=malloc(strlen(yytext)+2))==NULL) yyerror("erreur d'allocation");
		strcpy(yylval.s,yytext);
/*
	      printf("<idt:%s>\n",yylval.s);
*/
		 sprintf(dummy,"%s%s",dummy,yytext);
	      return IDT; 
	     }
\"[^\"]*.    { if ((yylval.s=malloc(strlen(yytext)+2))==NULL) yyerror("erreur d'allocation");
		strcpy(yylval.s,yytext);
/*
	       printf("<ch:%s>\n",yylval.s);
*/
	       sprintf(dummy,"%s%s",dummy,yytext);
               return(STRING);
             }
%%

yywrap()
{
/*"link"|"LINK"	     { return LINK; }*/
        return 1;
}


























