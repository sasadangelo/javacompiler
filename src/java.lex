%{

/*  
 * file Java.lex
 * 
 * descrizione: file scritto in lex che implementa lo scanner del compilatore
 *              Java.
 *
 * Questo file e' parte del Compilatore Java.
 *
 * Maggio 1997, scritto da Salvatore D'Angelo e-mail xc0261.xcom.it
 * Dicembre 1997, modificato da Salvatore D'Angelo
 *
 */

#include <cstring.h>
#include <stdlib.h>
#include <globals.h>
#include <java.tab.h>
#include <compile.h>
#include <errors.h>
#include <lex.h>

#define ENDFILE       0                  /* token end of file  */
#define NUM_KEYWORDS 59                  /* number of keywords */   

#define TRACE fTrace

extern FILE *fTrace;

//int yylineno=1;

extern CompileUnit *Unit;
extern char *msg_errors[];

int Id_or_Keyword(char *);

%}

WhiteSpace    ([ \t\f\r\n]|(\r\n))
Letter        [A-Za-z_$]
Digit         [0-9]
HexDigit      [0-9a-fA-F]
OctDigit      [0-7]
ZeroThree     [0-3]
Identifier    {Letter}({Letter}|{Digit})*
ExponentPart  [eE]([+-]?{Digit}+)?
OctalEscape \\({OctDigit}|{OctDigit}{OctDigit}|{ZeroThree}{OctDigit}{OctDigit})
CLiteral      ('[^'\\]')|'([\b\t\n\f\r\"\'\\]|{OctalEscape})'
StringChar    ([^\"\\\r\n]|([\b\t\n\f\r\"\'\\]{OctalEscape}))

%%

{WhiteSpace}*        { 
                       for (char *p=yytext; *p!='\0'; p++)
                           if (*p=='\n') yylineno++; 
                     }

"//"[^\r\n]* { 
                       fprintf(TRACE,"LineComment linea %d\n",yylineno); 
                     }

"/*"           { 
                 register int c;
                 int doc,line;
		 
                 line=yylineno; doc=yyinput(); unput(doc);
		 for(;;) 
		   {
                     while((c=yyinput())!='*' && c!=EOF)
		       if (c=='\n') yylineno++;
                     if (c=='*') {
		       while ((c=yyinput())=='*');
		       if (c=='/') 
			 {
			   if (doc=='*') 
			     fprintf(TRACE,"DocComment linea %d\n",line);
			   else
			     fprintf(TRACE,"Comment linea %d\n",line);
			   break;    
			 }
		       else
			 if (c==EOF)
			   {
			     Unit->MsgErrors(yylineno,
					     msg_errors[ERR_EOF_IN_COMMENT]);
         		     break;
			   }
			 else
			   if (c=='\n') yylineno++;
		     }
		     else 
		       if (c==EOF)
			 {
		           Unit->MsgErrors(yylineno,
					   msg_errors[ERR_EOF_IN_COMMENT]);
			   break;
			 }
		   } 
	       } 


{Identifier}   { 
                 yylval.str=new String(yytext);
                 return Id_or_Keyword(yytext); 
               }

{Digit}+\.{Digit}*([eE]([+-]?{Digit}+)?)?[fFdD]? |
\.{Digit}+([eE]([-+]?{Digit}+)?)?[fFdD]? |
{Digit}+([eE]([+-]?{Digit}+)?)[fFdD]? { sscanf(yytext,"%f",
                                               &(yylval.valfloat));
                                   fprintf(TRACE,"Numero reale %f linea %d\n",
                                                   yylval.valfloat,yylineno);
                                   return FLITERAL; }

[1-9]{Digit}*[lL]? { 
                     yylval.valint=atoi(yytext);
                     fprintf(TRACE,"Numero intero %d  linea %d\n",
                                                      yylval.valint,yylineno);
                     return ILITERAL; 
                   }

0[xX]{HexDigit}+[lL]?  { 
                         sscanf(yytext,"%x",&(yylval.valint));
                         fprintf(TRACE,"Numero intero esad. %d  linea %d\n",
                                                      yylval.valint,yylineno);
                         return ILITERAL; 
                       }

0{OctDigit}*[lL]?  { 
                     sscanf(yytext,"%o",&(yylval.valint));
                     fprintf(TRACE,"Numero intero ottale %d  linea %d\n",
                                                      yylval.valint,yylineno);
                     return ILITERAL; 
                   } 

{CLiteral}    { 
                char *lexeme;

                if (yytext[1]=='\\' && yytext[2]!='\'' ) {
                    lexeme=new char [yyleng-3];
                    strcpy(lexeme,yytext+2);
                    sscanf(lexeme,"%o",&(yylval.valint));           
		  }
                else 
                    yylval.valint=*(yytext+1);
                switch (yylval.valint) {
		    case '\b':
                        fprintf(TRACE,"Carat. \\ b linea %d\n",yylineno);
                        break;
		    case '\t':
                        fprintf(TRACE,"Carat. \\ t linea %d\n",yylineno);
                        break;
		    case '\n':
                        fprintf(TRACE,"Carat. \\ n linea %d\n",yylineno);
                        yylineno++;
                        break;
		    case '\f':
                        fprintf(TRACE,"Carat. \\ f linea %d\n",yylineno);
                        break;
		    case '\r':
                        fprintf(TRACE,"Carat. \\ r linea %d\n",yylineno);
                         break;
		    default:
                        fprintf(TRACE,"Carat. %c linea %d\n",
                                                     yylval.valint,yylineno); 
                 }
                return CLITERAL; 
              }


\"{StringChar}*\n { 
                    yylval.str=new String(yytext+1,yyleng-2);
                    fprintf(TRACE,"Stringa %s linea %d\n",
			    yylval.str->to_char(),yylineno);
		    Unit->MsgErrors(yylineno,
				    msg_errors[ERR_NEWLINE_IN_STRING]);
		    yylineno++;
                    return SLITERAL;
                  }

\"{StringChar}*\" { 
                    yylval.str=new String(yytext+1,yyleng-2);
		    fprintf(TRACE,"Stringa %s linea %d\n",
			    yylval.str->to_char(),yylineno);
		    return SLITERAL; 
                  }

\"{StringChar}* { 
                  yylval.str=new String(yytext+1,yyleng-2);
		  fprintf(TRACE,"Stringa %s linea %d\n",
                          yylval.str->to_char(),yylineno);
		  Unit->MsgErrors(yylineno,msg_errors[ERR_EOF_IN_STRING]);
		  return SLITERAL;
                } 

"("        { 
             fprintf(TRACE,"Simbolo (  linea %d\n",yylineno);
             return '(' ; 
           }

")"        { 
             fprintf(TRACE,"Simbolo )  linea %d\n",yylineno);
             return ')' ; 
           }

"{"        { 
             fprintf(TRACE,"Simbolo {  linea %d\n",yylineno);
             return '{' ; 
           }

"}"        { 
             fprintf(TRACE,"Simbolo }  linea %d\n",yylineno);
             return '}' ; 
           }

"["        { 
             fprintf(TRACE,"Simbolo [  linea %d\n",yylineno);
             return '[' ; 
           }

"]"        { 
             fprintf(TRACE,"Simbolo ]  linea %d\n",yylineno);
             return ']' ; 
           }

";"        { 
             fprintf(TRACE,"Simbolo ;  linea %d\n",yylineno);
             return ';' ; 
           }

","        { 
             fprintf(TRACE,"Simbolo ,  linea %d\n",yylineno);
             return ',' ; 
           } 

"."        { 
             fprintf(TRACE,"Simbolo .  linea %d\n",yylineno);
             return '.' ; 
           }

"++"       |
"--"       { 
             fprintf(TRACE,"Simbolo %s linea %d\n",yytext,yylineno);
             yylval.valint=(yytext[0]=='+' ? PLUSPLUS : MINUSMINUS);
             return INCOP; 
           }

[~!]       { 
             fprintf(TRACE,"Simbolo %s linea %d\n",yytext,yylineno);
             yylval.valint=(yytext[0]=='~' ? TILDE : ESCLAMATION);
             return UNOP; 
           }

"*"        { 
             fprintf(TRACE,"Simbolo * linea %d\n",yylineno);
             return '*' ; 
           }

[/%]       { 
             fprintf(TRACE,"Simbolo %s linea %d\n",yytext,yylineno);
             yylval.valint=(yytext[0]=='/' ? DIVIDE : MOD);
             return DIVOP; 
           }

"+"        { 
             fprintf(TRACE,"Simbolo + linea %d\n",yylineno);
             return '+' ; 
           }

"-"        { 
             fprintf(TRACE,"Simbolo - linea %d\n",yylineno);
             return '-' ; 
           }

"<<"|">>"|">>>"  { 
                   fprintf(TRACE,"Simbolo %s linea %d\n",yytext,yylineno);
                   yylval.valint=yytext[2] ? URSHIFT 
                                           : (yytext[0]=='<' ? LSHIFT : RSHIFT)
                                           ;
                   return SHIFTOP; 
                 }

[<>]=?     { 
             yylval.valint=yytext[1] ? (yytext[0]=='>' ? GE : LE) 
                                     : (yytext[0]=='>' ? GT : LT)
                                     ;
             fprintf(TRACE,"Simbolo %s linea %d\n",yytext,yylineno);
             return RELOP ; 
           }

[!=]=      { 
             fprintf(TRACE,"Simbolo %s linea %d\n",yytext,yylineno);
             yylval.valint=(yytext[0]=='!' ? NE : EQ);
             return EQUOP; 
           }

[*/%+\-&|^]=        |
("<<"|">>"|">>>")=  { 
                      fprintf(TRACE,"Simbolo %s linea %d\n",yytext,yylineno);
                      yylval.ascii=(yytext[0]=='*' ? STAR_ASSIGN   :
                             (yytext[0]=='/' ? DIVIDE_ASSIGN  :
                             (yytext[0]=='%' ? MOD_ASSIGN     :
                             (yytext[0]=='+' ? PLUS_ASSIGN    :
                             (yytext[0]=='-' ? MINUS_ASSIGN   :
                             (yytext[0]=='&' ? AND_ASSIGN     :
                             (yytext[0]=='|' ? OR_ASSIGN      :
                             (yytext[0]=='^' ? XOR_ASSIGN     :
                             (yytext[0]=='<' ? LSHIFT_ASSIGN  :
                             (yytext[2]      ? URSHIFT_ASSIGN :
                                               RSHIFT_ASSIGN ))))))))))
                             ;
                      return ASSIGNOP; 
	             }

"="           { 
                fprintf(TRACE,"Simbolo = linea %d\n",yylineno);
                return '=' ; 
              }

"&"           { 
                fprintf(TRACE,"Simbolo & linea %d\n",yylineno);
                return '&' ; 
              }

"^"           { 
                fprintf(TRACE,"Simbolo ^ linea %d\n",yylineno);
                return '^' ; 
              }

"|"           { fprintf(TRACE,"Simbolo | linea %d\n",yylineno);
                return '|' ; }

"&&"          { fprintf(TRACE,"Simbolo && linea %d\n",yylineno);
                return ANDAND; }

"||"          { fprintf(TRACE,"Simbolo || linea %d\n",yylineno);
                return OROR; }

"?"           { fprintf(TRACE,"Simbolo ? linea %d\n",yylineno);
                return '?'; }

":"           { fprintf(TRACE,"Simbolo : linea %d\n",yylineno);
                return ':'; }

.             { fprintf(stderr,"Errore lessicale linea:%d\n",yylineno);
                return(ENDFILE); }

%%

typedef struct {
    const char *lexeme; 
    int  token;
} TKWord;

TKWord    Keytab[]={{"abstract",    ABSTRACT},
                    {"boolean",     BOOLEAN},
                    {"break",       BREAK},
                    {"byte",        BYTE},
                    {"byvalue",     BYVALUE},
                    {"case",        CASE},
                    {"cast",        CAST},
                    {"catch",       CATCH},
                    {"char",        CHAR},
                    {"class",       CLASS},
                    {"const",       CONST},
                    {"continue",    CONTINUE},
                    {"default",     DEFAULT},
                    {"do",          DO},
                    {"double",      DOUBLE},
                    {"else",        ELSE},
                    {"extends",     EXTENDS},
                    {"false",       BLITERAL},
                    {"final",       FINAL},
                    {"finally",     FINALLY},
                    {"float",       FLOAT},
                    {"for",         FOR},
                    {"future",      FUTURE}, 
                    {"generic",     GENERIC},
                    {"goto",        GOTO},
                    {"if",          IF},
                    {"implements",  IMPLEMENTS},
                    {"import",      IMPORT},
                    {"inner",       INNER}, 
                    {"int",         INT},
                    {"interface",   INTERFACE},
                    {"istanceof",   INSTANCEOF},
                    {"long",        LONG},
                    {"native",      NATIVE},
                    {"new",         NEW}, 
                    {"null",        NULLTOKEN},
                    {"operator",    OPERATOR},
                    {"outer",       OUTER},
                    {"package",     PACKAGE},
                    {"private",     PRIVATE},
                    {"protected",   PROTECTED}, 
                    {"public",      PUBLIC},
                    {"rest",        REST},
                    {"return",      RETURN},
                    {"short",       SHORT},
                    {"static",      STATIC}, 
                    {"super",       SUPER},
                    {"switch",      SWITCH},
                    {"synchronized",SYNCHRONIZED},
                    {"this",        THIS},
                    {"throw",       THROW},
                    {"throws",      THROWS}, 
                    {"transient",   TRANSIENT},
                    {"true",        BLITERAL},
                    {"try",         TRY},
                    {"var",         VAR},
                    {"void",        VOID}, 
                    {"volatile",    VOLATILE},
                    {"while",       WHILE}};

/*
 * Funzione cmp           
 * 
 * confronta due oggetti TKWord. utilizzata nel controllo che verifica se
 * un identificatore e' una keyword.
 */

static int cmp(const void *a, const void *b)
{
    const TKWord *_a=(TKWord *)a;
    const TKWord *_b=(TKWord *)b;
    return strcmp(_a->lexeme,_b->lexeme);
}


/*
 * Funzione Id_or_Keyword() 
 * 
 * controlla se un identificatore e' oppure no una keyword.
 */

int Id_or_Keyword(char *lexeme)
{
    TKWord *p,dummy;
    
    dummy.lexeme=lexeme;
    p=(TKWord *) bsearch(&dummy,Keytab,NUM_KEYWORDS,sizeof(TKWord),cmp);
    if (p) {
        switch (p->token) {
        case BOOLEAN:
            fprintf(TRACE,"Iden. di tipo %s - linea:%d\n",yytext,yylineno);
            break;
	case BYTE:
            fprintf(TRACE,"Iden. di tipo %s - linea:%d\n",yytext,yylineno);
            break;
	case CHAR:
            fprintf(TRACE,"Iden. di tipo %s - linea:%d\n",yytext,yylineno);
            break;
        case DOUBLE:
            fprintf(TRACE,"Iden. di tipo %s - linea:%d\n",yytext,yylineno);
            break;
        case FLOAT:
            fprintf(TRACE,"Iden. di tipo %s - linea:%d\n",yytext,yylineno);
            break;
        case INT:
            fprintf(TRACE,"Iden. di tipo %s - linea:%d\n",yytext,yylineno);
            break;
        case LONG:
            fprintf(TRACE,"Iden. di tipo %s - linea:%d\n",yytext,yylineno);
            break;
        case SHORT:
            fprintf(TRACE,"Iden. di tipo %s - linea:%d\n",yytext,yylineno);
            break;
        case VOID:
            fprintf(TRACE,"Iden. di tipo %s - linea:%d\n",yytext,yylineno);
            break;
        case BLITERAL:
	    if (strcmp(yytext,"true")==0)
	      yylval.valint=1;
	    else
	      yylval.valint=0;
            fprintf(TRACE,"Costante booleana %s - linea:%d\n",yytext, 
                                                               yylineno);
            break;
        default:
            fprintf(TRACE,"Keyword %s - linea:%d\n",yytext,yylineno);
            break;
	 }
        return p->token;
     }
    else {
        fprintf(TRACE,"Identificatore %s - linea:%d\n",yytext,yylineno);
        return IDEN;
     }
 }


