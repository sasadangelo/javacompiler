%{

/*
 * file java.y
 *
 * descrizione: parser del compilatore. Esso controlla la correttezza sintat-
 *              tica e provvede alla costruzione del parse tree.
 *
 * Questo file e' parte del Compilatore Java.
 *
 * Maggio 1997, scritto da Salvatore D'Angelo  e-mail xc0261@xcom.it.
 */

#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <stdarg.h>
#include <globals.h>
#include <cstring.h>
#include <descriptor.h>
#include <hash.h>
#include <table.h>
#include <local.h>
#include <compile.h>
#include <errors.h>
#include <access.h>
#include <tree.h>
#include <environment.h>
#include <lex.h>
#include <string.h>

void yyerror(char *, ...) { } 
int  yylex();
  
/*
 * Stiamo qui dichiarando alcune variabili globali di fondamentale importanza:
 *
 * - current_class: da il nodo della classe attualmente in compilazione;
 * - current_meth : da il nodo dell'eventuale metodo che si sta compilando;
 * - Nest_lev     : indica il livello di scoping corrente in una certa classe;
 * - Index        : identifica univocamente una classe o interfaccia.
 * - LastIndex    : utilizzata affinche' non si abbia due classi con lo
 *                  stesso valore Index.
 */

STNode *current_class=NULL;
STNode *current_meth=NULL;
int Nest_lev=2;
int Index=-1;
int LastIndex=-1;

String init_name; 
String ObjectName;
String ErrorName;
String RuntimeExceptionName;
String ThrowableName;


/*
 * dichiarazioni esterne.
 */
  
extern int  yylineno;
extern char *yytext;
extern TreeNode *Dummy;
extern char *msg_errors[];
extern CompileUnit *Unit;
extern FILE *fTree;

extern Descriptor DesVoid;
extern Descriptor DesDouble;
extern Descriptor DesLong;
extern Descriptor DesFloat;
  
%}

%union {
  String *str;
  int    valint;
  float  valfloat;
  char   ascii;
  TreeNode *tree;
  Descriptor *descriptor;
  STNode *sym;
 }

/*
 * token delle keywords (e parole riservate)
 */

%token ABSTRACT BOOLEAN BREAK BYTE BYVALUE CASE CAST CATCH CHAR CLASS
%token CONST CONTINUE DEFAULT DO DOUBLE ELSE EXTENDS FINAL FINALLY FLOAT
%token FOR FUTURE GENERIC GOTO IF IMPLEMENTS IMPORT INNER INT INTERFACE
%token INSTANCEOF LONG NATIVE NEW NULLTOKEN OPERATOR OUTER PACKAGE PRIVATE
%token PROTECTED PUBLIC REST RETURN SHORT STATIC SUPER SWITCH SYNCHRONIZED
%token THIS THROW THROWS TRANSIENT TRY VAR VOID VOLATILE WHILE

%token <str> IDEN

/*
 * Costanti
 */

%token <valint>   BLITERAL ILITERAL CLITERAL
%token <valfloat> FLITERAL
%token <str>      SLITERAL

%type <valint>   Mod Mods OptMods

%type <descriptor>      FPType IntegralType PrimitiveType Type NumericType
%type <descriptor>      OptDims Dims ReferenceType ArrayType 

%type <tree>     ClassDecl OptSuper Super ClassBody OptClassBodyDecls 
%type <tree>     ClassBodyDecls ClassBodyDecl ClassMemberDecl FieldDecl
%type <tree>     VarDecltors VarDecltor MethodDecltor MethodDecl MethodHeader
%type <tree>     MethodBody OptThrows Throws ClassTypeList Block OptBlockStmts
%type <tree>     BlockStmts BlockStmt LocVarDeclStmt Stmt LocVarDecl IfThenStmt
%type <tree>     IfThenElseStmt OptTypeDecls TypeDecls TypeDecl WhileStmt
%type <tree>     OptParameterList ParameterList AssignExpr Expr CondOrExpr 
%type <tree>     CondExpr CondAndExpr IorExpr XorExpr AndExpr EqualityExpr 
%type <tree>     RelopExpr ShiftExpr AddExpr MultExpr UnaryExpr PreIncDecExpr
%type <tree>     UnaryExprNotPlusMinus PostFixExpr Primary PrimaryNoNewArray
%type <tree>     Literal PostIncDecExpr CastExpr StmtNoShortIf EmptyStmt
%type <tree>     StmtWithoutTrailSubStmt DoStmt ForStmt OptExpr OptForUpdate
%type <tree>     OptForInit ForInit ForUpdate SwitchStmt SwitchBlock
%type <tree>     LabeledStmt InterfaceDecl VarInit StaticInit StmtExprList
%type <tree>     LabeledStmtNoShortIf IfThenElseStmtNoShortIf 
%type <tree>     WhileStmtNoShortIf ForStmtNoShortIf BreakStmt ContinueStmt 
%type <tree>     ReturnStmt SyncStmt ThrowStmt TryStmt ArrayInit
%type <tree>     OptSwitchBlockStmtGroups SwitchBlockStmtGroups 
%type <tree>     SwitchBlockStmtGroup OptSwitchLabels SwitchLabels SwitchLabel
%type <tree>     ConstExpr OptCatches Catches CatchClause Finally ExprStmt
%type <tree>     StmtExpr Assignment OptExtInterfaces ExtInterfaces
%type <tree>     InterfaceBody OptInterfaceMemberDecls InterfaceMemberDecls 
%type <tree>     InterfaceMemberDecl ConstDecl AbstractMethodDecl
%type <tree>     FieldAccess MethodInv ArrayAccess LeftHandSide ArgList
%type <tree>     OptArgList InterfaceTypeList Interfaces OptInterfaces
%type <tree>     ConstrDecl ConstrBody ExplConstrInv
%type <tree>     ConstrDecltor ClassInstCreationExpr ArrayCreationExpr
%type <tree>     OptVarInits VarInits DimExprs DimExpr

%type <str>      Name SimpleName QualifiedName  

%type <sym>      VarDecltorId Parameter OptIden ClassType InterfaceType
%type <sym>      ClassOrInterfaceType

/*
 * Operatori
 */

%right <valint> EQUAL ASSIGNOP
%right '?' ':'
%left  OROR
%left  ANDAND
%left  '|'
%left  '^'
%left  '&'
%left  <valint> EQUOP
%left  <valint> RELOP
%left  <valint> SHIFTOP
%left  '+' '-'
%left  '*' 
%left  <valint> DIVOP
%right <valint> INCOP UNOP

/* 
 * Separatori
 */

 %token '('  ')'  '{'  '}' '['  ']' ';'  '.'  ','

%expect 6

%start Goal

%%

Goal:   
        { 
	  String packagelang;
          char *p;

	  p=new char [PATH_MAX];
	  getwd(p);
	  packagelang="java/lang";
	  Unit->import_on_demand(ENVIRONMENT->GetClassPath(),packagelang);
	  chdir(p);
	  delete p;
        }
      CompUnit 
    ;

Literal: ILITERAL  
           {
	     $$=new INUMNode($1);

	     if (yytext[strlen(yytext)-1]=='l' || 
		 yytext[strlen(yytext)-1]=='L')
	       $$->SetDescriptor(DesLong);
	   }
       | FLITERAL  
           {
	     /*
	      * Osservando il compilatore Java Sun, mi sono reso conto che
	      * un numero in virgola mobile per default e' double.
	      */

	     $$=new FNUMNode($1);

	     if (yytext[strlen(yytext)-1]=='f' || 
		 yytext[strlen(yytext)-1]=='F')
	       $$->SetDescriptor(DesFloat);
	     else
	       $$->SetDescriptor(DesDouble);
	   }
       | BLITERAL  { $$=new BOOLEANode($1);  }
       | CLITERAL  { $$=new CHARNode($1);    }
       | SLITERAL  { $$=new STRINGNode(*$1); }
       | NULLTOKEN { $$=new NULLNode();      }
       ;

Type: PrimitiveType 
    | ReferenceType
    ;

PrimitiveType: NumericType
             | BOOLEAN      { $$=new Descriptor(DES_BOOLEAN); }
             ;

NumericType: IntegralType
           | FPType
           ;

IntegralType: BYTE          { $$=new Descriptor(DES_BYTE);    }
            | SHORT         { $$=new Descriptor(DES_SHORT);   }
            | INT           { $$=new Descriptor(DES_INT);     }
            | LONG          { $$=new Descriptor(DES_LONG);    }
            | CHAR          { $$=new Descriptor(DES_CHAR);    }
            ;

FPType: FLOAT               { $$=new Descriptor(DES_FLOAT);   } 
      | DOUBLE              { $$=new Descriptor(DES_DOUBLE);  }
      ;

ReferenceType: ClassOrInterfaceType 
                 {
		   $$=new Descriptor();

		   $$->build_link($1->getfullname());
		 }
             | ArrayType
             ;

ClassOrInterfaceType: Name          
                        {
			  $$=STABLE->LookUp(*$1,SECOND_LVL);
	       
			  if ($$)
			    {
			      
			      if ($$->getlevel()==FIRST_LVL)
				$$=Unit->LoadClass($$->getfullname(),
						   ++LastIndex);
			    }
			  else
			    {
			      /*
			       * E' necessario installare la classe come 
			       * "unresolved".
			       */
		   
			      Descriptor d;

			      d.build_link(*$1);
			      $$=STABLE->Install_Id(*$1,SECOND_LVL,Index,d);
			      $$->setfullname(*$1);
			      STABLE->Discard($$);
			      $$->setunresolved();
			    }
			}
                    ;

ClassType: ClassOrInterfaceType
         ;

InterfaceType: ClassOrInterfaceType
             ;

ArrayType: PrimitiveType '[' ']'
             { 
	       $1->build_array();
	       $$=$1;
	     }
         | Name '[' ']'          
             {
	       STNode *n=STABLE->LookUp(*$1,SECOND_LVL);
	       $$=new Descriptor ();   
	       
	       if (n)
		 {
		   if (n->getlevel()==FIRST_LVL)
		     n=Unit->LoadClass(n->getfullname(),++LastIndex);
		 }
	       else
		 {
		   /*
		    * Attenzione!!!
		    *
		    * La gestione che qui ho previsto mi lascia perplesso
		    * nel momento in cui, una classe che installero' come
		    * "unresolved", non verra' poi mai piu' definita.
		    */
		   
		   n=STABLE->Install_Id(*$1,SECOND_LVL,Index,*$$);
		   n->setfullname(*$1);
		   STABLE->Discard(n);
		   n->setunresolved();
		 }

	       $$->build_link(n->getfullname());
	       $$->build_array();
	     }
         | ArrayType '[' ']' 
             {
	       $1->build_array();
	       $$=$1;
	     }
         ;

Name: SimpleName
    | QualifiedName
    ;

SimpleName: IDEN
          ;

QualifiedName: Name '.' IDEN   { *$$=*$1+"/"+*$3; }
             ;

CompUnit: OptPackDecl OptImpDecls OptTypeDecls
            {
	      Unit->make_syntax_tree($3)->PrintTree(fTree);
	    }
        ;

OptPackDecl:
           | PackDecl
           ;

OptImpDecls:
           | ImpDecls
           ;

OptTypeDecls: /* empty */ { $$=Dummy; } 
            | TypeDecls 
            ;

ImpDecls: ImpDecl
        | ImpDecls ImpDecl
        ;

TypeDecls: TypeDecl           
             { 
	       if (!$1->IsDummy())
		 $$=new EXPNode(CompUnitOp,Dummy,$1);
	       else
		 $$=Dummy;
	     }
         | TypeDecls TypeDecl 
             {  
	       if ($1->IsDummy()) 
		 $$=$1;
	       else
		 if ($2->IsDummy()) 
		   $$=$1;
		 else 
		   $$=new EXPNode(CompUnitOp,$1,$2); 
	     }
         ;

PackDecl: PACKAGE Name ';'
            {
	      Unit->SetPackageName(*$2);
	    }
        ;

ImpDecl: SingleTypeImpDecl
       | TypeImpOnDemDecl
       ;

SingleTypeImpDecl: IMPORT Name ';'
                     {
		       Unit->LoadClass(*$2,++LastIndex);
		     }
                 ;

TypeImpOnDemDecl: IMPORT Name '.' '*' ';' 
                    {
		      String s;

		      s=ENVIRONMENT->GetClassPath();

		      /*
		       * al contrario di quanto si possa pensare, nonostante
		       * il filesystem mi garantisce che non ci sono duplic.
		       * di nomi, potrei avere:
		       *               import java.awt.*;
		       *               import java.awt.*;
		       */

		      char *p=new char[PATH_MAX];
		      getwd(p);
		      Unit->import_on_demand(s,*$2);
		      chdir(p);
		      delete p;
		    }
                ;

TypeDecl: ClassDecl
        | InterfaceDecl
        | ';'           { $$=Dummy; }
        ;

Mods: Mod
    | Mods Mod { $$=Unit->mask_access($1,$2); }
    ;

Mod: PUBLIC        { $$=ACC_PUBLIC;       }
   | PROTECTED     { $$=ACC_PROTECTED;    }
   | PRIVATE       { $$=ACC_PRIVATE;      }
   | STATIC        { $$=ACC_STATIC;       }
   | ABSTRACT      { $$=ACC_ABSTRACT;     }
   | FINAL         { $$=ACC_FINAL;        }
   | NATIVE        { $$=ACC_NATIVE;       }
   | SYNCHRONIZED  { $$=ACC_SYNCHRONIZED; }
   | TRANSIENT
       {
         $$=ACC_TRANSIENT;
         Unit->MsgErrors(yylineno,msg_errors[ERR_NOT_SUPPORTED],"transient"); 
       }
   | VOLATILE
       {
         $$=ACC_VOLATILE;
         Unit->MsgErrors(yylineno,msg_errors[ERR_NOT_SUPPORTED],"volatile");
       }
   ;

ClassOrError: CLASS
            | /* empty */
                {
		  /*
		   * Questa e' una regola fittizia, introdotta per mostrare
		   * come poter rilevare un errore sintattico.
		   * Nella versione di Dicembre, cosi' come in quella di Maggio
		   * non sono gestiti gli errori sintattici, sia per lo scarso
		   * interesse, sia perche' posono essere introdotti facilmen-
		   * te.
		   */
		  
		  Unit->MsgErrors(yylineno,msg_errors[ERR_CLASS_EXPECTED]);
		}
            ;

ClassDecl: OptMods ClassOrError IDEN
             {
               Descriptor des;

	       Index=++LastIndex;

	       if (Unit->is_in_package())
		 des.build_link(Unit->GetPackageName()+"/"+*$3);
	       else
		 des.build_link(*$3);
	       if ((current_class=STABLE->LookUp(*$3,SECOND_LVL))!=NULL)
		 {
		   if (current_class->getlevel()==FIRST_LVL)
		     Unit->MsgErrors(yylineno,
				     msg_errors[ERR_CLASS_MULTIDEF_IMPORT],
				     $3->to_char(),
				     current_class->getfullname().to_char());
		   else
		     Unit->MsgErrors(yylineno,msg_errors[ERR_CLASS_MULTIDEF],
				     $3->to_char(),"type declaration");
		   current_class=STABLE->Install_Id(*$3,SECOND_LVL,Index,des);
		   STABLE->Discard(current_class);
		 }
	       else
		 current_class=STABLE->Install_Id(*$3,SECOND_LVL,Index,des);
	       if (Unit->is_in_package())
		 current_class->setfullname(Unit->GetPackageName()+"/"+*$3);
	       else
		 current_class->setfullname(*$3);
	       current_class->setaccess($1);
               $<sym>$=current_class;
	     }
           OptSuper OptInterfaces ClassBody
             {
	       /*
		* 
		* In ogni caso $5 riportera' sempre un parse-tree con riferi-
		* mento a una superclasse, perche' anche se l'utente non 
		* definisce una superclasse per la classe corrente, allora si
		* assume java/lang/Object come superclasse di default.
		*/
	       
	       STNode *cnode;
	       TreeNode *t, *t1, *t2, *t3, *t4;

	       if ((cnode=STABLE->LookUpHere(init_name,THIRD_LVL,
						  $<sym>4->getindex()))==NULL)
		 {
		   
		   /*
		    * Non e' stato definito un costruttore per la classe cor-
		    * rente. Si procede alla costruzione del parse-tree di un
		    * costruttore di default cosi' fatto:
		    *
		    *             void <nome classe> ()
		    *             {
		    *               super();
		    *             }
		    */
		   
		   
		   Descriptor descriptor;

		   descriptor="()V";
		   STNode *constrnode=STABLE->Install_Id(init_name,THIRD_LVL,
							 $<sym>4->getindex(),
							 descriptor);

		   if (IS_PUBLIC($1))
		     constrnode->setaccess(ACC_PUBLIC);

		   t1=new IDNode(constrnode);
		   t2=new EXPNode(CommaOp,t1,Dummy);
		   t3=new EXPNode(ConstrHeaderOp,t2,Dummy);
		   t4=new EXPNode(MethodCallOp,
				  new EXPNode(CommaOp,new SUPERNode(),
					      new UNAMENode(init_name)),
				  Dummy);
		   t4=new EXPNode(BlockOp,Dummy,
				  new EXPNode(StmtOp,Dummy,t4));
		   t=new EXPNode(ConstrDeclOp,t3,t4);
		   $7=new EXPNode(ClassBodyOp,$7,t);
		   constrnode->setmyheader(t);
		   constrnode->setmyclass(current_class);
		 }

	       t1=new EXPNode(CommaOp,$5,$6);
	       t2=new IDNode($<sym>4);
	       t3=new EXPNode(ClassHeaderOp,t2,t1);
	       t=new EXPNode(ClassOp,t3,$7);
	       $<sym>4->setmyheader(t);
	       // Index=++LastIndex;
	       $$=t;
	       current_class=NULL;
	     }
        ;

OptMods: /* empty */ { $$=0; }
       | Mods
       ;

OptSuper: /* empty */
            {
	      /*
	       * NON GESTITO!!!
	       *
	       * Caso in cui si compila java/lang/Object.
	       */
	      STNode *ObjectNode=Unit->LoadClass(ObjectName,++LastIndex);
	      $$=new EXPNode(ExtendsOp,Dummy,new IDNode(ObjectNode));
  	    }
        | Super
        ;

OptInterfaces: /* empty */ { $$=Dummy; }
             | Interfaces
             ;

Super: EXTENDS ClassType
         {
	   /*
	    * NON GESTITO!!!
	    *
	    * Caso in cui si compila java/lang/Object.
	    */

	   if ($2->is_resolved())
	     {
	       if ($2->is_interface())
		 {
		   Unit->MsgErrors(yylineno,msg_errors[ERR_INTF_SUPER_CLASS],
				   current_class->getname().to_char(),
				   $2->getname().to_char());

		   /*
		    * la superclasse sara' java/lang/Object.
		    */
		   
		   $2=Unit->LoadClass(ObjectName,++LastIndex);
		 }

	       $$=new EXPNode(ExtendsOp,Dummy,new IDNode($2));
	     }
	   else
	     $$=new EXPNode(ExtendsOp,Dummy,new UNAMENode($2->getname()));
	 }
     ;

Interfaces: IMPLEMENTS InterfaceTypeList { $$=$2; }
          ;

InterfaceTypeList: InterfaceType 
                     {
		       if ($1->is_resolved())
			 $$=new EXPNode(ImplementsOp,Dummy,new IDNode($1));
		       else
			 $$=new EXPNode(ImplementsOp,Dummy,
					new UNAMENode($1->getname()));

		     }
                 | InterfaceTypeList ',' InterfaceType
                     {
		       if ($3->is_resolved())
			 $$=new EXPNode(ImplementsOp,$1,new IDNode($3));
		       else
			 $$=new EXPNode(ImplementsOp,$1,
					new UNAMENode($3->getname()));
		     }
                 ;

ClassBody: '{' OptClassBodyDecls '}' 
             { 
	       $$=$2; 
	     }
         ;

OptClassBodyDecls: /* empty */
                     {
		       $$=Dummy;
		     }
                 |   { Nest_lev++; }
                   ClassBodyDecls
                     {
		       Nest_lev--;
		       $$=$2;
		     }
                 ;

ClassBodyDecls: ClassBodyDecl
                  {
                    $$=new EXPNode(ClassBodyOp,Dummy,$1);
                  }
              | ClassBodyDecls ClassBodyDecl
                  {
                    $$=new EXPNode(ClassBodyOp,$1,$2);
                  }
              ;

ClassBodyDecl: ClassMemberDecl
             | StaticInit
             | ConstrDecl
             ;

ClassMemberDecl: FieldDecl
               | MethodDecl 
               ;

FieldDecl: OptMods Type VarDecltors ';'
             {
	       /*
		* Per ogni campo dichiarato, aggiungo gli accessi e il
		* descrittore.
		*/

	       for (TreeNode *t=$3; !t->IsDummy(); t=t->GetLeftC())
		 {
		   STNode *n=((IDNode*)t->GetRightC()->GetLeftC())->GetIden();
		   n->setdescriptor(n->getdescriptor()+*$2);
		   n->setaccess($1);
		   n->setmyclass(current_class);
		 }

	       $$=$3;
	       ((EXPNode *)$$)->SetTreeOp(FieldDeclOp);
             }
         ;

VarDecltors: VarDecltor                 
               { 
		 $$=new EXPNode(-1,Dummy,$1);
	       }
           | VarDecltors ',' VarDecltor 
               { 
		 $$=new EXPNode(-1,$1,$3);
	       }
           ;

VarDecltor: VarDecltorId
              {
                $$=new EXPNode(CommaOp, new IDNode($1), Dummy);
              }
          | VarDecltorId '=' VarInit
              {
                $$=new EXPNode(CommaOp, new IDNode($1), $3);
              }
          ;

VarDecltorId: IDEN 
               {
		 STNode *node;
		 Descriptor des;

		 des=DES_NULL;
                 if ((node=STABLE->LookUpHere(*$1,Nest_lev,Index))!=NULL
		     && (!node->is_method()))
		   {
		     if (node->is_field())
		       {
			 Unit->MsgErrors(yylineno,msg_errors[ERR_VAR_MULTIDEF],
					 node->getname().to_char(),
					 "...");
			 node=STABLE->Install_Id(*$1,Nest_lev,Index,des);
			 STABLE->Discard(node);
		       }
		     else
		       {

			 /*
			  * l'identificatore denota una variabile locale o
			  * un parametro formale.
			  */

			 if (ENVIRONMENT->is_inContext(IN_PARAM))
			   Unit->MsgErrors(yylineno,
					   msg_errors[ERR_DUPLICATE_ARGUMENT],
					   node->getname().to_char());
			 else
			   Unit->MsgErrors(yylineno,
					   msg_errors[ERR_LOCAL_REDEFINED],
					   node->getname().to_char());
			 
			 node=STABLE->Install_Id(*$1,Nest_lev,Index,des);
			 STABLE->Discard(node);
			 node->set_unused();
		       }
		   }
		 else
		   node=STABLE->Install_Id(*$1,Nest_lev,Index,des);
		 node->setmyclass(current_class);
		 $$=node;
	       }
            | VarDecltorId '[' ']'
                {
		  Descriptor des;

		  des=$1->getdescriptor();		  
		  des.build_array();
		  $1->setdescriptor(des);
		}
            ;

VarInit: Expr
       | ArrayInit
       ;

MethodDecl: MethodHeader MethodBody
              {
		STNode *node=((IDNode*)$1->GetLeftC()->GetLeftC())->GetIden();
		$$=new EXPNode(MethodDeclOp,$1,$2);
		node->setmyheader($$);

		/*
		 * E' necessario eliminare i parametri formali dalla symbol-
		 * table. Eventuali parametri locali sono gia' stati elimi-
		 * nati achiusura del blocco.
		 */

		Unit->discard_parameters($1->GetLeftC()->GetRightC());
	      }
          ;

MethodHeader: OptMods Type MethodDecltor OptThrows
                {
		  STNode *node=((IDNode*)$3->GetLeftC())->GetIden();
		  
		  /*
		   * Se stiamo in una class final, allora il metodo e' final.
		   */
		  
		  if (IS_FINAL(current_class->getaccess()))
		    $1|=ACC_FINAL;
	
		  /*
		   * I metodi static o private sono implicitamente final.
		   */
		   
		  if (IS_STATIC($1))  $1|=ACC_FINAL;
		  if (IS_PRIVATE($1)) $1|=ACC_FINAL;
 
		  node->setaccess($1);
		  node->setdescriptor(node->getdescriptor()+*$2);
		  $$=new EXPNode(MethodHeaderOp,$3,$4);
		}
            | OptMods VOID MethodDecltor OptThrows
                {
		  STNode *node=((IDNode*)$3->GetLeftC())->GetIden();

		  /*
		   * Se stiamo in una class final, allora il metodo e' final.
		   */
		  
		  if (IS_FINAL(current_class->getaccess()))
		    $1|=ACC_FINAL;

		  if (IS_STATIC($1))  $1|=ACC_FINAL;
		  if (IS_PRIVATE($1)) $1|=ACC_FINAL;

		  node->setaccess($1);
		  node->setdescriptor(node->getdescriptor()+DES_VOID);
		  $$=new EXPNode(MethodHeaderOp,$3,$4);
		}
            ;

MethodDecltor: IDEN '(' OptParameterList ')' 
                 {
		   Descriptor des;
		   STNode *node;

		   des=$3->GetDescriptor();
		   des.build_signature();
       
		   for (node=STABLE->LookUpHere(*$1,THIRD_LVL,Index); 
			node!=NULL; node=STABLE->NextSym(node,THIRD_LVL,Index))
		     if (node->is_method() && 
			 node->getdescriptor().to_signature()==des)
		       break;
		   
		   if (node!=NULL)
		     {
		       Unit->MsgErrors(yylineno,msg_errors[ERR_METH_MULTIDEF],
				       $1->to_char());
		       node=STABLE->Install_Id(*$1,Nest_lev,Index,des);
		       STABLE->Discard(node);
		     }
		   else
		     node=STABLE->Install_Id(*$1,Nest_lev,Index,des);
		   node->setmyclass(current_class);
		   $$=new EXPNode(CommaOp,new IDNode(node),$3);
                 }
             | MethodDecltor '[' ']'
                 {
		   STNode *node=((IDNode*)$1->GetLeftC())->GetIden();
		   node->setdescriptor(node->getdescriptor()+DES_DIM);
		 }
             ;

OptParameterList: /* empty */ { $$=Dummy; }
                |   
                    { 
                      Nest_lev++;
		      ENVIRONMENT->openContext(IN_PARAM);
		    } 
                  ParameterList 
                    { 
		      Nest_lev--;
		      $$=$2;
		      ENVIRONMENT->closeContext();
		    }
                ;

ParameterList: Parameter
                 {
		   $$=new EXPNode(ParameterOp, new IDNode($1),Dummy);
		   $$->SetDescriptor($1->getdescriptor());
		 }
             | ParameterList ',' Parameter
                 {
		   TreeNode *p=$1;
		   while(!p->GetRightC()->IsDummy()) p=p->GetRightC();
		   p->SetRightC(new EXPNode(ParameterOp,new IDNode($3),Dummy));
		   $$=$1;
       	           $$->SetDescriptor($1->GetDescriptor()+$3->getdescriptor());
		 }
             ;

Parameter: Type VarDecltorId
             {
	       if (*$1==DES_VOID)
		 Unit->MsgErrors(yylineno,msg_errors[ERR_VOID_ARGUMENT],
				 $2->getname().to_char());
	       $2->setdescriptor(*$1);
	       $$=$2;
	     }
         ;

OptThrows: /* empty */  
             { 
	       $$=Dummy; 
	     }
         | Throws
         ;

Throws: THROWS ClassTypeList 
          { 
	    $$=$2; 
	  }
      ;

ClassTypeList: ClassType
                 {
		   if ($1->is_resolved())
		     $$=new EXPNode(ThrowsOp,Dummy,new IDNode($1));
		   else
		     $$=new EXPNode(ThrowsOp,Dummy,
				    new UNAMENode($1->getname()));
		 }
             | ClassTypeList ',' ClassType
                 {
		   TreeNode *t=$1;
		   
		   for (; !t->GetLeftC()->IsDummy(); t=t->GetLeftC());

		   if ($3->is_resolved())
		     t->SetLeftC(new EXPNode(ThrowsOp,Dummy,new IDNode($3)));
		   else
		     t->SetLeftC(new EXPNode(ThrowsOp,Dummy,
					     new UNAMENode($3->getname())));
		   $$=$1;
		 }
             ;

MethodBody:   { 
	        if (Nest_lev < FOURTH_LVL) Nest_lev++; 
	      }
            Block
              {
	        Nest_lev--;
	        $$=$2;
	      }
          | ';'   
              { 
		$$=Dummy; 
	      }
          ;

StaticInit: STATIC Block
              {
		$$=new EXPNode(StaticInitOp,Dummy,$2);
	      }
          ;

ConstrDecl: OptMods ConstrDecltor OptThrows ConstrBody
              {
		STNode *cnode=((IDNode*)$2->GetLeftC())->GetIden();

		cnode->setaccess($1);

		$$=new EXPNode (ConstrDeclOp,new EXPNode(ConstrHeaderOp,$2,$3),
				$4);
		cnode->setmyheader($$);
		Unit->discard_parameters($2->GetRightC());
	      }
          ;

ConstrDecltor: SimpleName '(' OptParameterList ')'  
                 {
		   Descriptor des;
		   STNode *cnode;

		   des.build_method($3->GetDescriptor(),DesVoid);

		   if (*$1!=current_class->getname())
		     {
		       Unit->MsgErrors(yylineno,
				       msg_errors[ERR_INVALID_METHOD_DECL]);

		       cnode=STABLE->Install_Id(*$1,THIRD_LVL,
					       current_class->getindex(),
					       des);
		       STABLE->Discard(cnode);
		     }
		   else
		     if ((cnode=STABLE->LookUpHere(init_name,THIRD_LVL,
						   current_class->getindex(),
						   des))!=NULL)
		       {
			 Unit->MsgErrors(yylineno,
					 msg_errors[ERR_METH_MULTIDEF],
					 $1->to_char());
			 cnode=STABLE->Install_Id(init_name,THIRD_LVL,
						  current_class->getindex(),
						  des);
			 STABLE->Discard(cnode);
		       }
		     else
		       cnode=STABLE->Install_Id(init_name,THIRD_LVL,
						current_class->getindex(),des);

		   cnode->setmyclass(current_class);

		   $$=new EXPNode(CommaOp,new IDNode(cnode),$3);
		 }
             ;

ConstrBody: '{' OptBlockStmts '}'
            {
	      /*
	       * Poiche' non c'e' una invocazione esplicita ad un altro cos-
	       * truttore della classe corrente o a un costruttore della sua
	       * superclasse, allora, per default, si invochera' in costruttore
	       * void <init>() della superclasse.
	       */

	      if ($2->IsDummy())
		$$=new EXPNode(BlockOp,Dummy,Dummy);
	      else
		{
		  $2->SetLeftC(new EXPNode(StmtOp,$2->GetLeftC(),Dummy));
		  $$=$2;
		}
	    }
          | '{' ExplConstrInv OptBlockStmts '}'
            {
	      if ($3->IsDummy())
		$$=new EXPNode(BlockOp,Dummy,new EXPNode(StmtOp,Dummy,$2));
	      else
		{
		  $3->SetLeftC(new EXPNode(StmtOp,$3->GetLeftC(),$2));
		  $$=$3;
		}
	    }
          ;
/*
ConstrBody: '{' OptExplConstrInv OptBlockStmts '}'
              {
		if ($3->IsDummy())
		  if ($2->IsDummy())
		    $$=new EXPNode(BlockOp,Dummy,Dummy);
		  else
		    $$=new EXPNode(BlockOp,Dummy,new EXPNode(StmtOp,Dummy,$2));
		else
		  $3->SetLeftC(new EXPNode(StmtOp,$3->GetLeftC(),$2));
	      }
          ;
	  
OptExplConstrInv: // empty
                    { 
		      $$=Dummy; 
		    }
                | ExplConstrInv
                ;
		*/

ExplConstrInv: THIS '(' OptArgList ')' ';'
                 {
		   $$=new EXPNode(ThisOp,Dummy,$3);
		 }
             | SUPER '(' OptArgList ')' ';'
                 {
		   $$=new EXPNode(SuperOp,Dummy,$3);
		 }
             ;

OptArgList: /* empty */ 
              { 
		$$=Dummy; 
	      }
          | ArgList
          ;

InterfaceDecl: OptMods INTERFACE IDEN
                 {
		   STNode *node;
		   Descriptor des;

		   Index=++LastIndex;

		   if (Unit->is_in_package())
		     des.build_link(Unit->GetPackageName()+"/"+*$3);
		   else
		     des.build_link(*$3);
		   if ((node=STABLE->LookUpHere(*$3,SECOND_LVL))!=NULL)
		     {
		       Unit->MsgErrors(yylineno,msg_errors[ERR_CLASS_MULTIDEF],
				       $3->to_char(),"java file");
		       node=STABLE->Install_Id(*$3,SECOND_LVL,Index,des);
		       STABLE->Discard(node);
		     }
		   else
		     node=STABLE->Install_Id(*$3,SECOND_LVL,Index,des);
		   if (Unit->is_in_package())
		     node->setfullname(Unit->GetPackageName()+"/"+*$3);
		   else
		     node->setfullname(*$3);
		   node->setaccess($1|ACC_ABSTRACT|ACC_INTERFACE);
		   current_class=node;
		   $<sym>$=node;
		 }
               OptExtInterfaces InterfaceBody
                 {
		   TreeNode *t=new EXPNode(InterfaceHeaderOp,
					   new IDNode($<sym>4),$5);
		   $$=new EXPNode(InterfaceOp,t,$6);
		   $<sym>4->setmyheader($$);
		   current_class=NULL;
		   // Index=++LastIndex;
		 }
             ;

OptExtInterfaces: /* empty */ 
                    { 
		      $$=Dummy; 
		    }
                | ExtInterfaces
                ;

ExtInterfaces: EXTENDS InterfaceType
                 {
		   if ($2->is_resolved())
		     if (!IS_INTERFACE($2->getaccess()))
		       {
			 Unit->MsgErrors(yylineno,msg_errors[ERR_NOT_INTF],
					 $2->getname().to_char());
			 $$=Dummy;
		       }
		     else
		       $$=new EXPNode(ExtendsOp,Dummy,new IDNode($2));
		   else
		     $$=new EXPNode(ExtendsOp,Dummy,
				    new UNAMENode($2->getname()));
		 }
             | ExtInterfaces ',' InterfaceType
                 {
		   /*
		    * Secondo le specifiche Java, la duplicazione di un nome
		    * di interfaccia nella lista delle superinterfacce, non
		    * determina un errore di compilazione. In tal caso il
		    * compilatore evitera' di memorizzare di nuovo lo stesso
		    * nome nel parse-tree.
		    */

		   if ($3->is_resolved())
		     {
		       if (!IS_INTERFACE($3->getaccess()))
			 {
			   Unit->MsgErrors(yylineno,msg_errors[ERR_NOT_INTF],
					   $3->getname().to_char());
			   $$=$1;
			 }
		       else
			 {
			   TreeNode *t;
			   for (t=$1; !t->IsDummy(); t=t->GetLeftC())
			     {
			       STNode *node;
			     
			       node=((IDNode*)t->GetRightC())->GetIden();
			       if (node->getname()==$3->getname())
				 break;
			     }
			   if (t->IsDummy())
			     $$=new EXPNode(ExtendsOp,$1,new IDNode($3));
			   else
			     $$=$1;
			 }
		     }
		   else
		     $$=new EXPNode(ExtendsOp,$1,
				    new UNAMENode($3->getname()));
		 }
             ;

InterfaceBody: '{' { Nest_lev++; } OptInterfaceMemberDecls '}'
                 {  
		   $$=$3;
		   Nest_lev--;
		 }
             ; 

OptInterfaceMemberDecls: /* empty */ 
                           { 
			     $$=Dummy; 
			   }
                       | InterfaceMemberDecls
                       ;

InterfaceMemberDecls: InterfaceMemberDecl
                        {
			  $$=new EXPNode(InterfaceBodyOp,Dummy,$1);
			}
                    | InterfaceMemberDecls InterfaceMemberDecl 
                        {
			  $$=new EXPNode(InterfaceBodyOp,$1,$2);
			} 
                    ;

InterfaceMemberDecl: ConstDecl
                   | AbstractMethodDecl
                   ;

ConstDecl: FieldDecl
         ;

AbstractMethodDecl: MethodHeader ';'
                      {
			STNode *node=((IDNode *)
				      $1->GetLeftC()->GetLeftC())->GetIden();
			$$=new EXPNode(MethodDeclOp,$1,Dummy);
			node->setmyheader($$);			
			Unit->discard_parameters($1->GetLeftC()->GetRightC());
		      }
                  ;

ArrayInit: '{' OptVarInits ',' '}' { $$=$2; }
         | '{' OptVarInits '}'     { $$=$2; }
         ;

OptVarInits: /* empty */ { $$=Dummy; }
           | VarInits    
           ;

VarInits: VarInit 
            { 
	      $$=new EXPNode(ArrayComponentOp,Dummy,$1); 
	    }
        | VarInits ',' VarInit
            {
	      $$=new EXPNode(ArrayComponentOp,$1,$3); 
	    }
        ;

Block: '{' OptBlockStmts '}' 
         {
	   $$=new EXPNode(BlockOp,Dummy,$2);

           /*
	    * E' necessario scartare le variabili locali al blocco.
	    */

	   Unit->discard_locals($2);
	 }
     ;

OptBlockStmts: /* empty */ { $$=Dummy; }
             | BlockStmts 
             ;

BlockStmts: BlockStmt
          | BlockStmts BlockStmt 
              {
		$2->SetLeftC($1);
		$$=$2;
	      }
          ;

BlockStmt: LocVarDeclStmt
         | Stmt
         ;

LocVarDeclStmt: LocVarDecl ';'
                  {
		    $$=new EXPNode(StmtOp,Dummy,$1);
		  }
              ;

LocVarDecl: Type VarDecltors 
              {
                /*
		 * Per ogni identificatore, aggiungo il descrittore.
		 */

                for (TreeNode *t=$2; !t->IsDummy(); t=t->GetLeftC())
                  {
                    STNode *n=((IDNode*)t->GetRightC()->GetLeftC())->GetIden();
		    n->setdescriptor(n->getdescriptor()+*$1);
                  }
		((EXPNode*)$2)->SetTreeOp(LocVarDeclOp);
                $$=$2;
              }
          ;

Stmt: StmtWithoutTrailSubStmt
    | LabeledStmt             
    | IfThenStmt              { $$=new EXPNode(StmtOp,Dummy,$1); }
    | IfThenElseStmt          { $$=new EXPNode(StmtOp,Dummy,$1); }
    | WhileStmt               { $$=new EXPNode(StmtOp,Dummy,$1); }
    | ForStmt                 { $$=new EXPNode(StmtOp,Dummy,$1); }
    ; 

StmtNoShortIf: StmtWithoutTrailSubStmt
             | LabeledStmtNoShortIf
             | IfThenElseStmtNoShortIf { $$=new EXPNode(StmtOp,Dummy,$1); }
             | WhileStmtNoShortIf      { $$=new EXPNode(StmtOp,Dummy,$1); }
             | ForStmtNoShortIf        { $$=new EXPNode(StmtOp,Dummy,$1); }
             ;

StmtWithoutTrailSubStmt: Block
                       | EmptyStmt
                       | ExprStmt     { $$=new EXPNode(StmtOp,Dummy,$1); }
                       | SwitchStmt   { $$=new EXPNode(StmtOp,Dummy,$1); }
                       | DoStmt       { $$=new EXPNode(StmtOp,Dummy,$1); }
                       | BreakStmt    { $$=new EXPNode(StmtOp,Dummy,$1); }
                       | ContinueStmt { $$=new EXPNode(StmtOp,Dummy,$1); }
                       | ReturnStmt   { $$=new EXPNode(StmtOp,Dummy,$1); }
                       | SyncStmt     { $$=new EXPNode(StmtOp,Dummy,$1); }
                       | ThrowStmt    { $$=new EXPNode(StmtOp,Dummy,$1); }
                       | TryStmt      { $$=new EXPNode(StmtOp,Dummy,$1); }
                       ;

EmptyStmt: ';' 
             { 
               $$=Dummy; 
             }
         ;

LabeledStmt: IDEN ':'
               {
                 Descriptor des;

		 des=DES_LABEL;
                 if (($<sym>$=STABLE->LookUpHere(*$1,Nest_lev,Index))!=NULL)
		   Unit->MsgErrors(yylineno,msg_errors[ERR_INVALID_LABEL]);

		 $<sym>$=STABLE->Install_Id(*$1,Nest_lev,Index,des);
		 STABLE->Discard($<sym>$);
	       }
             Stmt 
               { 
                 $$=new EXPNode(LabelStmtOp,new IDNode($<sym>3),$4);
	       }
           ;


LabeledStmtNoShortIf: IDEN
                        {
			  Descriptor des;

			  des=DES_LABEL;
			  if (($<sym>$=STABLE->LookUpHere(*$1,Nest_lev,
							  Index))!=NULL)
			    Unit->MsgErrors(yylineno,
					    msg_errors[ERR_INVALID_LABEL]);

			  $<sym>$=STABLE->Install_Id(*$1,Nest_lev,Index,des);
			  STABLE->Discard($<sym>$);
			}
                      ':' StmtNoShortIf 
                        { 
			  $$=new EXPNode(LabelStmtOp,new IDNode($<sym>3),$4);
			}
           ;

ExprStmt: StmtExpr ';'
        ;

StmtExpr: Assignment
        | PreIncDecExpr
        | PostIncDecExpr
        | MethodInv
        | ClassInstCreationExpr
        ;

IfThenStmt: IF '(' Expr ')' Stmt 
              { 
		$$=new EXPNode(IfThenElseOp,new EXPNode(CommaOp,$3,$5),Dummy);
	      }
          ;

IfThenElseStmt: IF '(' Expr ')' StmtNoShortIf ELSE Stmt
                  {
		    $$=new EXPNode(IfThenElseOp,new EXPNode(CommaOp,$3,$5),$7);
		  }
              ;

IfThenElseStmtNoShortIf: IF '(' Expr ')' StmtNoShortIf ELSE StmtNoShortIf
                           {
			     $$=new EXPNode(IfThenElseOp,
					    new EXPNode(CommaOp,$3,$5),$7);
			   }
                       ;

SwitchStmt: SWITCH '(' Expr ')' SwitchBlock
              {
		$$=new EXPNode(SwitchStmtOp,$3,$5);
	      } 
          ;

SwitchBlock: '{' OptSwitchBlockStmtGroups OptSwitchLabels '}' 
               {
                 /*
		  * Durante la fase di analisi, non siamo riusciti a capire
		  * a cosa serve $3.
		  */

		 $$=$2;
	     }
           ;

OptSwitchBlockStmtGroups: /* empty */ { $$=Dummy; }
                        | SwitchBlockStmtGroups
                        ;

OptSwitchLabels: /* empty */ { $$=Dummy; }
               | SwitchLabels
               ;

SwitchBlockStmtGroups: SwitchBlockStmtGroup
                         {
			   $$=new EXPNode(BodySwitchOp,Dummy,$1);
			 }
                     | SwitchBlockStmtGroups SwitchBlockStmtGroup
                         {
			   TreeNode *t=$1;

			   while(!t->GetLeftC()->IsDummy()) t=t->GetLeftC();
			   t->SetLeftC(new EXPNode(BodySwitchOp,Dummy,$2));
                         }   
                     ;

SwitchBlockStmtGroup: SwitchLabels BlockStmts
                        {
			  $$=new EXPNode(CommaOp,$1,$2);
			}
                    ;

SwitchLabels: SwitchLabel
                {
		  $$=new EXPNode(BodySwitchLabelOp,Dummy,$1);
		}
            | SwitchLabels SwitchLabel
                {
		  TreeNode *t=$1;
		  
		  while (!t->GetLeftC()->IsDummy()) t=t->GetLeftC();
		  t->SetLeftC(new EXPNode(BodySwitchLabelOp,Dummy,$2));
		}
            ;

SwitchLabel: CASE ConstExpr ':'
               {
		 $$=$2;
               }
           | DEFAULT ':' 
               { 
                 $$=Dummy;
               }
           ;

WhileStmt: WHILE '(' Expr ')' Stmt
             {
	       $$=new EXPNode(LoopOp,$3,$5);
	     }
         ;

WhileStmtNoShortIf: WHILE '(' Expr ')' StmtNoShortIf
                      {
			$$=new EXPNode(LoopOp,$3,$5);
		      }

DoStmt: DO Stmt WHILE '(' Expr ')' ';'
          {
            $$=new EXPNode(LoopOp,$2,$5);
	  }
      ;

ForStmt: FOR '(' OptForInit ';' OptExpr ';' OptForUpdate ')' Stmt
           {
	     /*
	      * scarto delle variabili locali.
	      */

	     Unit->discard_locals($3);
	     Unit->discard_locals($7);
	     Unit->discard_locals($9);

	     if ($5->IsDummy())
	       $5=new BOOLEANode(true);

	     $$=new EXPNode(LoopOp,new EXPNode(CommaOp,$3,$5),
			           new EXPNode(CommaOp,$9,$7));
	   }
       ;

OptForInit: /* empty */ 
              { 
                $$=Dummy; 
              }
          | { if (++Nest_lev > FOURTH_LVL) Nest_lev=FOURTH_LVL; } 
              ForInit 
            { $$=$2; }
          ;

OptExpr: /* empty */ 
           { 
	     $$=Dummy; 
	   }
       | Expr
       ;

OptForUpdate: /* empty */ 
                { 
		  $$=Dummy; 
		}
            | ForUpdate
            ;

ForInit: StmtExprList
       | LocVarDecl   { $$=new EXPNode(StmtOp,Dummy,$1); }
       ;

ForUpdate: StmtExprList
         ;

ForStmtNoShortIf: FOR '(' OptForInit ';' OptExpr ';' OptForUpdate ')' 
                  StmtNoShortIf
                    {
		      /*
		       * scarto delle variabili locali.
		       */
		      
		      Unit->discard_locals($3);
		      Unit->discard_locals($7);
		      Unit->discard_locals($9);

		      if ($5->IsDummy())
			$5=new BOOLEANode(true);
		      
		      $$=new EXPNode(LoopOp,new EXPNode(CommaOp,$3,$5),
				            new EXPNode(CommaOp,$9,$7));
		    }
                ;

StmtExprList: StmtExpr                  { $$=new EXPNode(StmtOp,Dummy,$1); }
            | StmtExprList ',' StmtExpr { $$=new EXPNode(StmtOp,$1,$3);    } 
            ;

BreakStmt: BREAK OptIden ';'
             {
               if ($2==NULL)
		 $$=new EXPNode(BreakStmtOp,Dummy,Dummy);
	       else
		 $$=new EXPNode(BreakStmtOp,Dummy,new IDNode($2));
             }
         ;

OptIden: /* empty */ 
           { 
             $$=NULL; 
           }
       | IDEN 
           { 
	     STNode *node;
             int index=current_class->getindex();

	     if ((node=STABLE->LookUp(*$1,Nest_lev,index))==NULL)
               Unit->MsgErrors(yylineno,
			       msg_errors[ERR_LABEL_NOT_FOUND],$1->to_char());
	     $$=node;
	   }
       ;

ContinueStmt: CONTINUE OptIden ';'
                {
		  if ($2==NULL)
		    $$=new EXPNode(ContinueStmtOp,Dummy,Dummy);
		  else
		    $$=new EXPNode(ContinueStmtOp,Dummy,new IDNode($2)); 
		}
            ;

ReturnStmt: RETURN OptExpr ';'
              {
                $$=new EXPNode(ReturnStmtOp,Dummy,$2);
	      }
          ;

ThrowStmt: THROW Expr ';'
             {
               $$=new EXPNode(ThrowStmtOp,$2,Dummy);
             }
         ;

SyncStmt: SYNCHRONIZED '(' Expr ')' Block
            {
	      $$=new EXPNode(SyncStmtOp,$3,$5);
	    }
        ;

TryStmt: TRY Block Catches    
           { 
	     $$=new EXPNode(TryStmtOp,$2,$3);
	   }
       | TRY Block OptCatches Finally 
           { 
	     $$=new EXPNode(TryStmtOp,$2,new EXPNode(TryBodyOp,$3,$4));
	   }
       ;

OptCatches: /* empty */ 
              { 
                $$=Dummy; 
              }
          | Catches
          ;

Catches: CatchClause         { $$=new EXPNode(TryBodyOp,Dummy,$1); }
       | Catches CatchClause { $$=new EXPNode(TryBodyOp,$1,$2);    }
       ;

CatchClause: CATCH '(' Parameter ')' Block
               {
                 $$=new EXPNode(CommaOp,new IDNode($3),$5);
		 STABLE->Discard($3);
               }
           ;

Finally: FINALLY Block 
               { 
                 $$=new EXPNode(CommaOp,Dummy,$2); 
               }
       ;

Primary: PrimaryNoNewArray
       | ArrayCreationExpr
       ;

PrimaryNoNewArray: Literal
                 | THIS 
                     {
		       $$=new THISNode();
		       $$->SetDescriptor(current_class->getdescriptor());
		     }
                 | '(' Expr ')'    
		     { 
		       $$=$2; 
		     }
                 | ClassInstCreationExpr
                 | FieldAccess
                 | MethodInv
                 | ArrayAccess
                 ;

ClassInstCreationExpr: NEW ClassType '(' OptArgList ')'
                         {
			   if ($2->is_resolved())
			     $$=new EXPNode(NewOp,
					new EXPNode(CommaOp,
						    new IDNode($2),
						    new UNAMENode(init_name)),
					    $4);
			   else
			     $$=new EXPNode(NewOp,
					    new UNAMENode($2->getname()),$4);
			 }
                     ;

ArgList: Expr  
           { 
	     $$=new EXPNode(ArgsOp,$1,Dummy);
	   }
       | ArgList ',' Expr
           {
	     TreeNode *p=$1;
	     while(!p->GetRightC()->IsDummy()) p=p->GetRightC();
	     p->SetRightC(new EXPNode(ArgsOp,$3,Dummy));
	     $$=$1;
           }
       ;

ArrayCreationExpr: NEW PrimitiveType DimExprs OptDims
                     {
		       Descriptor descriptor;

		       descriptor=*$4;
		       for (TreeNode *t=$3; !t->IsDummy(); t=t->GetLeftC())
			 descriptor+=DES_DIM;
		       descriptor+=*$2;
		       $$=new EXPNode(NewArrayOp,Dummy,$3);
		       $$->SetDescriptor(descriptor);
		     }
                 | NEW ClassOrInterfaceType DimExprs OptDims
                     {
		       Descriptor descriptor;
		       Descriptor ref;

		       descriptor=*$4;
		       for (TreeNode *t=$3; !t->IsDummy(); t=t->GetLeftC())
			 descriptor+=DES_DIM;

		       ref.build_link($2->getfullname());
		       descriptor+=ref;

		       if ($2->is_resolved())
			 $$=new EXPNode(NewArrayOp,new IDNode($2),$3);
		       else
			 $$=new EXPNode(NewArrayOp,
					new UNAMENode($2->getfullname()),
					$3);
		       
		       $$->SetDescriptor(descriptor);
		     }
                 ;

OptDims: /* empty */ 
           { 
	     $$=new Descriptor(DES_NULL); 
	   }
       | Dims        
       ;

DimExprs: DimExpr 
            {
	      $$=new EXPNode(CommaOp,Dummy,$1);
	    }
        | DimExprs DimExpr
            {
               $$=new EXPNode(CommaOp,$1,$2);
	    }
        ;

DimExpr: '[' Expr ']' { $$=$2; }
       ;

Dims: '[' ']'      
        { 
	  $$=new Descriptor(DES_DIM); 
	}
    | Dims '[' ']' 
        { 
	  *$$=*$1+DES_DIM;
	}
    ;

FieldAccess: Primary '.' IDEN 
               { 
		 $$=new EXPNode(FieldAccessOp,$1,new UNAMENode(*$3));
	       }
           | SUPER '.' IDEN 
               {
		 /*
		  * Bisogna tener presente che qui non possiamo stabilire
		  * il descrittore della superclasse della classe corrente.
		  * Cio' lo si puo' fare in fase di parsing quando siamo
		  * sicuri che se la classe corrente ha una superclasse,
		  * allora questa e' gia' inserita nella symbol-table.
		  */

		 $$=new EXPNode(FieldAccessOp,new SUPERNode(),
				new UNAMENode(*$3));
	       }
           ;

MethodInv: Name '(' OptArgList ')' 
             {
	       $$=new EXPNode(MethodCallOp,new UNAMENode(*$1),$3);
	     }
         | Primary '.' IDEN '(' OptArgList ')' 
             {
	       TreeNode *tree=new EXPNode(CommaOp,$1,new UNAMENode(*$3));
	       $$=new EXPNode(MethodCallOp,tree,$5);
	     }
         | SUPER '.' IDEN '(' OptArgList ')' 
             { 
	       // STNode *super_class=current_class->get_superclass();
	       TreeNode *tree=
		 new EXPNode(CommaOp,new SUPERNode(),new UNAMENode(*$3));
	       $$=new EXPNode(MethodCallOp,tree,$5);
             }
         ;

ArrayAccess: Name '[' Expr ']' 
               {
		 $$=new EXPNode(ArrayAccessOp,new UNAMENode(*$1),$3);
	       }
           | PrimaryNoNewArray '[' Expr ']' 
               {
		 $$=new EXPNode(ArrayAccessOp,$1,$3);
	       }
           ;

PostFixExpr: Primary
           | Name          { $$=new UNAMENode(*$1); }
           | PostIncDecExpr  
           ;

PostIncDecExpr: PostFixExpr INCOP 
                  { 
		    /*
		     * La costruzione del parse-tree qui e' simile a 
		     * PreIncDecExpr. La differenza sta solo nell'ordine con 
		     * cui vengono disposti i figli.
		     */

		    if ($2==PLUSPLUS)
		      $$=new EXPNode(PlusPlusOp,$1,Dummy);
		    else
		      $$=new EXPNode(MinusMinusOp,$1,Dummy);
		  }
              ;

UnaryExpr: PreIncDecExpr 
         | UnaryExprNotPlusMinus
         | '+' UnaryExpr
             { 
	       $$=new EXPNode(UniPlusOp,Dummy,$2);
	     }
         | '-' UnaryExpr
             { 
	       $$=new EXPNode(UniMinusOp,Dummy,$2); 
	     }
         ;

PreIncDecExpr: INCOP UnaryExpr 
                 { 
		   if ($1==PLUSPLUS)
		     $$=new EXPNode(PlusPlusOp,Dummy,$2);
		   else
		     $$=new EXPNode(MinusMinusOp,Dummy,$2);		   
		 }
             ;

UnaryExprNotPlusMinus: PostFixExpr
                     | UNOP UnaryExpr 
                         {
			   if ($1==ESCLAMATION)
			     $$=new EXPNode(LogicalNotOp,Dummy,$2);
			   else
			     $$=new EXPNode(BitwiseNotOp,Dummy,$2);
			 }
                     | CastExpr
                     ;

CastExpr: '(' PrimitiveType OptDims ')' UnaryExpr
            {
	      Descriptor des;

	      des=*$3+*$2;
	      $$=new EXPNode(CastOp,Dummy,$5);
	      $$->SetDescriptor(des);
	    }
        | '(' Expr ')' UnaryExprNotPlusMinus
            {
	      $$=new EXPNode(CastOp,$2,$4);
	    }	
        | '(' Name Dims ')' UnaryExprNotPlusMinus
            {
	      Descriptor des;

	      des.build_link(*$2);
	      des=*$3+des;
	      $$=new EXPNode(CastOp,Dummy,$5);
	      $$->SetDescriptor(des);
	    }
        ;

MultExpr: UnaryExpr
        | MultExpr '*' UnaryExpr
            { 
	     $$=new EXPNode(MultOp,$1,$3);
	    }
        | MultExpr DIVOP UnaryExpr
            { 
	      if ($2==DIVIDE)
		$$=new EXPNode(DivOp,$1,$3);
	      else
		$$=new EXPNode(ModOp,$1,$3);
	    }
        ;

AddExpr: MultExpr
       | AddExpr '+' MultExpr
           { 
	     $$=new EXPNode(AddOp,$1,$3);
	   }
       | AddExpr '-' MultExpr
           {
	     $$=new EXPNode(SubOp,$1,$3);
	   }
       ;

ShiftExpr: AddExpr
         | ShiftExpr SHIFTOP AddExpr
             {
	       switch ($2) 
		 {
		 case LSHIFT : $$=new EXPNode(LShiftOp,$1,$3);  break;
		 case RSHIFT : $$=new EXPNode(RShiftOp,$1,$3);  break;
		 case URSHIFT: $$=new EXPNode(UrShiftOp,$1,$3); break;
		 }
	     }
         ;

RelopExpr: ShiftExpr
         | RelopExpr RELOP ShiftExpr
             { 
	       switch ($2) 
		 {
		 case LE: $$=new EXPNode(LeOp,$1,$3); break;
		 case GE: $$=new EXPNode(GeOp,$1,$3); break;
		 case LT: $$=new EXPNode(LtOp,$1,$3); break;
		 case GT: $$=new EXPNode(GtOp,$1,$3); break;
		 }
	     }
         | RelopExpr INSTANCEOF ReferenceType
             { 
	       $$=new EXPNode(InstanceOfOp,$1,Dummy);
	       $$->SetDescriptor(*$3);
	     }
         ;

EqualityExpr: RelopExpr
            | EqualityExpr EQUOP RelopExpr
                {
		  if ($2==EQ)
		    $$=new EXPNode(EqOp,$1,$3);
		  else
		    $$=new EXPNode(NeOp,$1,$3);
		}
            ;

AndExpr: EqualityExpr 
       | AndExpr '&' EqualityExpr
           {
	     $$=new EXPNode(AndOp,$1,$3);
	   }
       ;

XorExpr: AndExpr
       | XorExpr '^' AndExpr
           { 
	     $$=new EXPNode(XorOp,$1,$3);
	   }
       ;

IorExpr: XorExpr
       | IorExpr '|' XorExpr
           {
	     $$=new EXPNode(OrOp,$1,$3);
	   }
       ;

CondAndExpr: IorExpr
           | CondAndExpr ANDAND IorExpr
              { 
		 $$=new EXPNode(AndAndOp,$1,$3);
	      }
           ;

CondOrExpr: CondAndExpr
          | CondOrExpr OROR CondAndExpr
              {
		$$=new EXPNode(OrOrOp,$1,$3);
	      }
          ;

CondExpr: CondOrExpr
        | CondOrExpr '?' Expr ':' CondExpr 
            {
	      $$=new EXPNode(CondExprOp,new EXPNode(CommaOp,$1,$3),$5);
	    }
        ;

AssignExpr: CondExpr   
          | Assignment
          ;

Assignment: LeftHandSide ASSIGNOP AssignExpr 
              { 
		switch ($2)
		  {
		  case STAR_ASSIGN  :
		    $$=new EXPNode(AssignMultOp,$1,$3);
		    break;
		  case DIVIDE_ASSIGN:
		    $$=new EXPNode(AssignDivOp,$1,$3);
		    break;
		  case MOD_ASSIGN   :
		    $$=new EXPNode(AssignModOp,$1,$3);
		    break;
		  case PLUS_ASSIGN  :
		    $$=new EXPNode(AssignPlusOp,$1,$3);
		    break;
		  case MINUS_ASSIGN :
		    $$=new EXPNode(AssignMinusOp,$1,$3);
		    break;
		  case AND_ASSIGN   :
		    $$=new EXPNode(AssignAndOp,$1,$3);
		    break;
		  case OR_ASSIGN    :
		    $$=new EXPNode(AssignOrOp,$1,$3);
		    break;
		  case XOR_ASSIGN   :
		    $$=new EXPNode(AssignXorOp,$1,$3);
		    break;
		  case LSHIFT_ASSIGN:
		    $$=new EXPNode(AssignLShiftOp,$1,$3);
		    break;
		  case RSHIFT_ASSIGN:
		    $$=new EXPNode(AssignRShiftOp,$1,$3);
		    break;
		  case URSHIFT_ASSIGN:
		    $$=new EXPNode(AssignURShiftOp,$1,$3);
		    break;
		  }
	      }
          | LeftHandSide '=' AssignExpr    
              { 
		$$=new EXPNode(AssignOp,$1,$3);
	      }
          ;

LeftHandSide: Name
                {
		  $$=new UNAMENode(*$1);
                } 
            | FieldAccess
            | ArrayAccess
            ;

Expr: AssignExpr
    ;

ConstExpr: Expr 
         ;

%%






