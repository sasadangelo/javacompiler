/*
 * file parse.cc
 *
 * descrizione: questo file contiene routine che visitano il parse-tree cos-
 *              struito con Yacc e gestiscono:
 *
 *                         - errori semantici;
 *                         - nomi non risolti;
 *                         - ottimizzazioni constant-folding;
 *                         - semplici ottimizzazioni aritmetiche;
 *                         - conversioni implicite.
 *
 * Questo file e' parte del Compilatore Java.
 *
 * Dicembre 1997, scritto da Salvatore D'Angelo e-mail xc0261@xcom.it.
 */

#include <stdio.h>
#include <globals.h>
#include <access.h>
#include <errors.h>
#include <cstring.h>
#include <descriptor.h>
#include <local.h>
#include <environment.h>
#include <tree.h>
#include <hash.h>
#include <table.h>
#include <compile.h>

/*
 * dichiarazioni esterne.
 */

extern STNode *current_class;
extern STNode *current_meth;
extern String ObjectName, RuntimeExceptionName, ErrorName, ThrowableName;
extern int Nest_lev;
extern int LastIndex;
extern char *msg_errors[];
extern TreeNode *Dummy;
extern Descriptor DesThrowable;
extern String init_name;

/*
 * Dichiarazioni globali.
 */

/******************************************************************************
 * classe CompileUnit (implementazione metodi inerenti al parsing)            *
 ******************************************************************************

/*
 * CompileUnit::ParseTree
 *
 * Inizia da questo metodo l'analisi del parse-tree, il cui compito e' quello
 * di individuare errori-semantici e risolvere nomi "unresolved".
 */

void CompileUnit::ParseTree()
{
  ParseCompUnit(syntax_tree->GetRoot());
}

/*
 * CompileUnit::ParseCompUnit
 * 
 * Metodi di supporto per il parsing di classi e interfacce dichiarate nel
 * file .java.
 */

void CompileUnit::ParseCompUnit(TreeNode *root)
{
  if (!root->IsDummy())
    {
      ParseCompUnit(root->GetLeftC());
      if (((EXPNode *)root->GetRightC())->GetNodeOp()==ClassOp)
	ParseClassDecl(root->GetRightC());
      else
	ParseInterfaceDecl(root->GetRightC());
    }
}

/*
 * CompileUnit::ParseClassDecl
 *
 * Analisi del parse-tree che dichiara una classe.
 */

void CompileUnit::ParseClassDecl(TreeNode *root)
{
  /*
   * NON GESTITO!!!
   * 
   * Se la classe corrente eredita metodi astratti da superinterfacce o 
   * superclassi e questi non vengono implementati, allora la classe va
   * dichiarata "abstract".
   */

  /*
   * I passi che e' necessario eseguire per effettuare il parsing di una clas-
   * se, sono:
   *
   * - risolvere, se necessario il nome della superclasse;
   * - gestire eventuali errori semantici legati alla superclasse;
   * - gestire eventuali errori semantici legati alle interfacce da implementa-
   *   re;
   * - fare il parsing del corpo della classe.
   */

  env->openContext(IN_CLASS);
  Nest_lev++;
  current_class=((IDNode *)root->GetLeftC()->GetLeftC())->GetIden();

  /*
   * dichiaro variabili ausiliare che contengono rispettivamente i seguenti 
   * valori:
   *
   * etree ----> punta al nodo EXPNode etichettato ExtendsOp;
   * itree ----> punta al nodo EXPNode etichettato ImplementsOp;
   * intftree -> punta alla radice dell'albero dell'interfaccia da implementa-
   *             re (usato solo in una circostanza);
   * snode ----> nodo della symbol-table che eventualmente conterra' la
   *             superclasse;
   * inode ----> nodo della symbol-table che eventualmente conterra' un'in-
   *             terfaccia implementata dalla classe corrente;
   * cname ----> nome della classe corrente;
   * sname ----> stringa che conterra' il nome della superclasse;
   * iname ----> conterra' i nomi delle interfacce implementate dalla classe
   *             corrente;
   * oname ----> nome "java/lang/Object".
   */

  TreeNode *etree=root->GetLeftC()->GetRightC()->GetLeftC();
  TreeNode *itree=root->GetLeftC()->GetRightC()->GetRightC();  
  String   cname;
  String   sname;
  String   iname;
  STNode   *snode;
  STNode   *inode;

  /*
   * parsing della superclasse.
   */

  cname=current_class->getname();

  if (etree->GetRightC()->is_UNAMENode())
    {
      /*
       * il nome della superclasse e' "unresolved", per cui e' necessario
       * risolverlo.
       */
      
      sname=((UNAMENode *)etree->GetRightC())->GetName();
      /* if ((snode=LoadClass(sname,++LastIndex))!=NULL)
	{
	  delete etree->GetRightC();
	  etree->SetRightC(new IDNode(snode));  
	}*/
      snode=LoadClass(sname,++LastIndex);
    }
  else
    snode=((IDNode *)etree->GetRightC())->GetIden();

  /*
   * Errore di compilazione se la superclasse non esiste, oppure esiste ma
   * e' un'interfaccia.
   */

  if (!snode)
    MsgErrors(etree->GetLine(),msg_errors[ERR_SUPER_NOT_FOUND],sname.to_char(),
	      current_class->getname().to_char());
  else
    if (!snode->is_class())
      MsgErrors(snode->getline(),msg_errors[ERR_INTF_SUPER_CLASS],
		cname.to_char(),sname.to_char());

  /*
   * Se la superclasse non esiste o e' un'interfaccia, allora la reale
   * superclasse, sara' "java/lang/Object".
   */

  if (!snode || !snode->is_class())
    snode=LoadClass(ObjectName,++LastIndex);

  delete etree->GetRightC();
  etree->SetRightC(new IDNode(snode));  
  
  /*
   * A questo punto se snode!=NULL, esso conterra' il nodo della effettiva
   * superclasse: "java/lang/Object" o quella definita dall'utente se
   * questi non ha commesso errori.
   */

  if (snode)
    {
      /*
       * NON GESTITO!!!
       *
       * Controllo di cicli nella definizione di superclassi.
       */

      if (IS_FINAL(snode->getaccess()))
	MsgErrors(current_class->getline(),msg_errors[ERR_SUPER_IS_FINAL],
		  snode->getname().to_char());
    }

  /*
   * parsing delle interfacce da implementare.
   */

  for (; !itree->IsDummy(); itree=itree->GetLeftC())
    {
      /*
       * Per ogni interfaccia:
       *
       * - risolvo il nome in caso in cui sia "unresolved" e eventualmente
       *   eseguo il LoadClass;
       * - controllo che la classe sia PUBLIC o definita nel pacchetto cor-
       *   rente;
       * - controllo che lo stesso nome non venga ripetuto piu' volte.
       */ 
      
      if (itree->GetRightC()->is_UNAMENode())
	{
	  /*
	   * Il nome e' "unresolved" e' necessario risolverlo.
	   */

	  iname=((UNAMENode *)itree->GetRightC())->GetName();
	  if ((inode=LoadClass(iname,++LastIndex))!=NULL)
	    {
	      delete itree->GetRightC();
	      itree->SetRightC(new IDNode(inode));
	    }
	}
      else
	{
	  inode=((IDNode *)itree->GetRightC())->GetIden();
	  iname=inode->getname();
	}

      if (!inode)
	MsgErrors(itree->GetLine(),msg_errors[ERR_INTF_NOT_FOUND],
		  iname.to_char(),current_class->getname().to_char());
      else
	if (!inode->is_interface())
	  MsgErrors(inode->getline(),msg_errors[ERR_NOT_INTF],iname.to_char());

      /*
       * Controllo ora che in precedenza non sia stata definita
       * un' implementazione della stessa interfaccia.
       */
      
      for (TreeNode *t=itree->GetLeftC(); !t->IsDummy(); t=t->GetLeftC())
	{
	  String s;

	  if (t->GetRightC()->is_UNAMENode())
	    s=((UNAMENode *)t->GetRightC())->GetName();
	  else
	    s=((IDNode *)t->GetRightC())->GetIden()->getname();
	  
	  if (iname==s)
	    MsgErrors(t->GetLine(),msg_errors[ERR_INTF_REPEATED],
		      iname.to_char());
	}

      /*
       * Se l'interfaccia da implementare e' "resolved", e' necessario
       * controllare che essa sia definita nel pacchetto corrente o
       * sia definita PUBLIC.
       */

      if (itree->GetRightC()->is_IDNode())
	if (!IS_PUBLIC(inode->getaccess()) && 
	    (inode->getdescriptor().to_packagename()==GetPackageName()))
	  MsgErrors(inode->getline(),msg_errors[ERR_CANT_ACCESS_CLASS],
		    iname.to_char());
    }

  /* 
   * effettuo il parsing del corpo della classe.
   */

  Nest_lev++;
  ParseClassBody(root->GetRightC());
  Nest_lev--;

  int caccess=current_class->getaccess();
  int line=current_class->getline();
  
  if (IS_PRIVATE(caccess))
    MsgErrors(line,msg_errors[ERR_PRIVATE_CLASS]);

  if (IS_ABSTRACT(caccess) && IS_FINAL(caccess))
    MsgErrors(line,msg_errors[ERR_FINAL_ABSTRACT],cname.to_char());

  current_class=NULL;
  Nest_lev--;
  env->closeContext();
}

/*
 * CompileUnit::ParseClassBody
 *
 * effettua il parsing del corpo si una classe.
 */

void CompileUnit::ParseClassBody(TreeNode *root)
{
  if (!root->IsDummy())
    {
      ParseClassBody(root->GetLeftC());
      switch (((EXPNode *)root->GetRightC())->GetNodeOp())
	{
	case FieldDeclOp : ParseFieldDecl(root->GetRightC());  break;
	case MethodDeclOp: ParseMethodDecl(root->GetRightC()); break;
	case StaticInitOp: ParseStaticInit(root->GetRightC()); break;
	case ConstrDeclOp: ParseConstrDecl(root->GetRightC()); break;
	}
    }
}

/*
 * CompileUnit::ParseFieldDecl
 *
 * Faccio il parsing della dichiarazione di un campo per una classe.
 */

void CompileUnit::ParseFieldDecl(TreeNode *root)
{
  /*
   * Per ogni identificatore dichiarato come campo, controllo che:
   *
   * - non sia di tipo "void";
   * - non sia transient e allo stesso tempo static o final;
   * - non sia synchronized, native o abstract;
   * - se il campo e' final esso deve essere inizializzato con un'espressio-
   *   ne costante e assegnabile al campo secondo l'"assign conversion".
   */

  if (!root->IsDummy())
    {
      ParseFieldDecl(root->GetLeftC());

      TreeNode *old_etree;
      TreeNode *etree=root->GetRightC()->GetRightC();
      STNode *idnode=((IDNode *)root->GetRightC()->GetLeftC())->GetIden();
      Descriptor iddes;
      String idname;
      int    idaccess=idnode->getaccess();

      iddes=idnode->getdescriptor();
      idname=idnode->getname();

      if (iddes==DES_VOID)
	MsgErrors(idnode->getline(),msg_errors[ERR_VOID_INST_VAR],
		  idname.to_char());
      
      if (IS_TRANSIENT(idaccess) && 
	  (IS_STATIC(idaccess) || IS_FINAL(idaccess)))
	MsgErrors(idnode->getline(),msg_errors[ERR_TRANSIENT_MODIFIER],
		  idname.to_char());
      
      if (IS_SYNCHRONIZED(idaccess) || IS_NATIVE(idaccess) || 
	  IS_ABSTRACT(idaccess))
	MsgErrors(idnode->getline(),msg_errors[ERR_VAR_MODIFIER],
		  idname.to_char());
      
      if (IS_FINAL(idaccess) && etree->IsDummy())
	MsgErrors(idnode->getline(),msg_errors[ERR_INITIALIZER_NEEDED],
		  idname.to_char());

      /*
       * NON GESTITO!!!
       *
       * Se un identificatore non ha alcuna espressione associata, allora in 
       * base al suo descrittore, gli verra' assegnato il valore di default.
       * In questo modo, alla fine, etree sara' sicuramente non Dummy.
       */

      if (!etree->IsDummy())
	{
	  /*
	   * Faccio il Parsing dell'espressione prima di utilizzarla.
	   */

	  etree=ParseVarInit(etree);

	  if (iddes.is_reference() && etree->is_NULLNode())
	    etree->SetDescriptor(iddes);

	  root->GetRightC()->SetRightC(etree);
	  
	  if (IS_FINAL(idaccess) && !IsConstExpr(etree))
	    MsgErrors(etree->GetLine(),msg_errors[ERR_CONST_EXPR_REQUIRED]);
	  
	  if (!is_Assign_Conversion(iddes,etree->GetDescriptor()))
	    MsgErrors(etree->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"=",
		      etree->GetDescriptor().to_typename(),
		      iddes.to_typename());
		      
	  else
	    if (iddes!=etree->GetDescriptor())
	      {
		etree->GetRightC()->SetRightC(new EXPNode(CastOp,
							   Dummy,etree));
		etree->GetRightC()->GetRightC()->SetDescriptor(iddes);
	      }

	  /*
	   * Aggiungo l'inizializzazione del campo a tutti i costruttori, se
	   * il campo non e' static, altrimenti lo si inserisce in <clinit>
	   * (inizializzatore statico). Infatti, l'attributo 
	   * ConstantValue_Attribute e' utilizzabile a patto che il campo sia
	   * static e l'espressione costant sia un valore (grazie al constant 
	   * folding) numerico (long, int, float e double) o una stringa.
	   *
	   * Attualmente ConstantValue_Attribute e' usato per campi static e 
	   * final, in realta', pero', le specifiche della JVM richiedono solo
	   * la staticita' del campo.
	   */

	  
	  if (!(IS_STATIC(idaccess) && IS_FINAL(idaccess) && 
		IsConstExpr(etree)))
	    if (IS_STATIC(idaccess))
	      InitFieldStatic(idnode, etree);
	    else
	      InitFieldNotStatic(idnode, etree);
	}
    }
}

/*
 * CompileUnit::ParseMethodDecl
 *
 * Effettuo il parsing della dichiarazione di un metodo.
 */

void CompileUnit::ParseMethodDecl(TreeNode *root)
{
  env->openContext(IN_METHOD);

  /*
   * Per un metodo si attivano i seguenti controlli:
   *
   * - se il metodo non ha corpo, allora va dichiarato "abstract", in caso
   *   contrario no;
   * - allocare i parametri formali a livello (FOURTH_LVL,Index);
   * - il metodo non puo' essere abstract e static;
   * - il metodo non puo' essere transient;
   * - il metodo non puo' essere volatile;
   * - se il metodo e' static e non final, allora va settato anche l'accesso
   *   final;
   * - se il metodo e' private e non final, allora va settato anche l'accesso
   *   final;
   * - se la classe corrente e' final allora anche il metodo e' final.
   */

  TreeNode *ptree=root->GetLeftC()->GetLeftC()->GetRightC();
  current_meth=((IDNode *)root->GetLeftC()->GetLeftC()->GetLeftC())->GetIden();
  String   methname;
  int      methaccess=current_meth->getaccess();
  int      methline=current_meth->getline();

  methname=current_meth->getname();

  if (root->GetRightC()->IsDummy())
    {
      if (!IS_ABSTRACT(methaccess))
	MsgErrors(methline,msg_errors[ERR_NO_METH_BODY],methname.to_char());
    }
  else
    if (IS_ABSTRACT(methaccess))
      MsgErrors(methline,msg_errors[ERR_INVALID_METH_BODY],methname.to_char());

  /*
   * NON GESTITO!!!
   *
   * In ogni caso, se il metodo e' abstract informare la classe corrente che al
   * suo interno c'e' una classe abstract.
   */

  ParseMethodHeader(root);

  ParseThrows(root->GetLeftC()->GetRightC());

  /*
   * Le variabili locali sono eliminati da ParseBlock.
   */

  if (!root->GetRightC()->IsDummy())
    {
      Nest_lev++;
      ParseBlock(root->GetRightC());
      Nest_lev--;
    }

  discard_parameters(ptree);
  current_meth=NULL;

  env->closeContext();
}

/*
 * CompileUnit::ParseMethodHeader
 *
 * Invocato per il parsing di metodi, sia nella definizione di classi che di 
 * interfacce.
 */

void CompileUnit::ParseMethodHeader(TreeNode *root)
{ 
  TreeNode *ptree=root->GetLeftC()->GetLeftC()->GetRightC();  
  String   methname;
  int      methaccess=current_meth->getaccess();
  int      methline=current_meth->getline();

  methname=current_meth->getname();

  /*
   * Per ogni parametro formale, allocarlo nella symbol-table a livello di 
   * scoping (FOURTH_LVL,Index).
   */

  env->openContext(IN_PARAM);

  for (TreeNode *t=ptree; !t->IsDummy(); t=t->GetRightC())
    table->Recover(((IDNode *)t->GetLeftC())->GetIden());

  env->closeContext();

  if (IS_STATIC(methaccess) && IS_ABSTRACT(methaccess))
    MsgErrors(methline,msg_errors[ERR_STATIC_MODIFIER],
	      methname.to_char());
  if (IS_TRANSIENT(methaccess))
    MsgErrors(methline,msg_errors[ERR_TRANSIENT_METH],methname.to_char());
  if (IS_VOLATILE(methaccess))
    MsgErrors(methline,msg_errors[ERR_VOLATILE_METH],methname.to_char());

  /*
   * Controllo che non stiamo facendo overriding di metodi static o final.
   */

  for (STNode *n=table->LookUpHere(current_meth->getname()); n!=NULL; 
       n=table->NextSym(n))
    {
      Descriptor signature_n, signature_meth;

      signature_n=n->getdescriptor().to_signature();
      signature_meth=current_meth->getdescriptor().to_signature();

      if (n->is_method() && n!=current_meth && signature_n==signature_meth &&
	  current_class->is_subclass(n->getmyclass()))
	{
	  /*
	   * Stiamo facendo l'overriding di un metodo. Bisogna controllare
	   * fondamentalmente due cose:
	   *
	   * a) il metodo sovrascritto non deve essere final;
	   * b) il metodo sovrascritto non deve essere static;
	   *
	   * Nel caso b) poiche' un metodo static e anche final, allora e'
	   * preferibile inviare un messaggio di errore.
	   */
	  
	  if (IS_STATIC(n->getaccess()))
	    MsgErrors(methline,msg_errors[ERR_OVERRIDE_STATIC_METH],
		      n->getname().to_char(),
		      n->getmyclass()->getname().to_char());
	  else
	    if (IS_FINAL(n->getaccess()))
	      MsgErrors(methline,msg_errors[ERR_FINAL_METH_OVERRIDE],
			n->getname().to_char(),
			n->getmyclass()->getname().to_char());
	}
    }
}

/*
 * CompileUnit::ParseThrows
 *
 * Questa routine gestisce la clausola Throws nella dichiarazione di un metodo.
 */

void CompileUnit::ParseThrows(TreeNode *root)
{
  for (TreeNode *t=root; !t->IsDummy(); t=t->GetLeftC())
    {
      String tname;
      STNode *tnode;

      /*
       * Per ogni classe definita in questa clausola, attivo i seguenti con-
       * trolli:
       *
       * - controllo che il nome sia risolto, in caso contrario, lo risolvo.
       * - e' necessario che tale classe sia assegnabile a java/lang/Throwable
       *   secondo l'algoritmo di "assign conversion".
       */

      if (t->GetRightC()->is_UNAMENode())
	{
	  tname=((UNAMENode *)t->GetRightC())->GetName();
	  if ((tnode=LoadClass(tname,++LastIndex))!=NULL)
	    {
	      delete t->GetRightC();
	      t->SetRightC(new IDNode(tnode));
	    }
	}
      else
	tnode=((IDNode *)t->GetRightC())->GetIden();

      if (!tnode)
	MsgErrors(t->GetLine(),msg_errors[ERR_CLASS_NOT_FOUND],
		  tname.to_char(),"throws");
      else
	if (!is_Assign_Conversion(DesThrowable,tnode->getdescriptor()))
	  MsgErrors(t->GetLine(),msg_errors[ERR_THROWS_NOT_THROWABLE],
		    tnode->getname().to_char());
    }
}

/*
 * CompileUnit::ParseBlock
 *
 * Effettua il parsing di un blocco di istruzioni. Un blocco puo' contenere
 * le seguenti istruzioni:
 *
 * - istruzione vuota;
 * - dichiarazione di variabili locali;
 * - istruzione if..then..else;
 * - istruzione while;
 * - istruzione do..while;
 * - istruzione for;
 * - istruzione switch;
 * - istruzione break;
 * - istruzione continue;
 * - istruzione return;
 * - istruzione synchronized;
 * - istruzione throw;
 * - istruzione try;
 * - blocco di istruzioni;
 * - blocco di istruzioni etichettato;
 * - assegnazione semplice (es. a=b);
 * - assegnazione composta (es. a&=b);
 * - operzioni di pre/post-incremento/decremento;
 * - invocazione di metodi;
 * - creazione oggetto;
 */

void CompileUnit::ParseBlock(TreeNode *root)
{
  if (!root->GetRightC()->IsDummy())
    {
      ParseStmt(root->GetRightC());
      discard_locals(root->GetRightC());
    }
}

/*
 * CompileUnit::ParseStmt
 *
 * Effettua il parsing di un set di istruzioni magari racchiuse in un blocco.
 */

void CompileUnit::ParseStmt(TreeNode *root)
{
  if (!root->IsDummy())
    {
      ParseStmt(root->GetLeftC());

      if (!root->GetRightC()->IsDummy())
	switch (((EXPNode *) root->GetRightC())->GetNodeOp())
	  {
	  case LocVarDeclOp   : ParseLocVarDecl(root->GetRightC());     break;
	  case IfThenElseOp   : ParseIfThenElse(root->GetRightC());    break; 
	  case LoopOp         : ParseLoop(root->GetRightC());           break;
	  case BlockOp        : ParseBlock(root->GetRightC());          break;
	  case AssignOp       : ParseAssignment(root->GetRightC());     break;
	  case AssignMultOp   : 
	  case AssignDivOp    : 
	  case AssignModOp    : 
	  case AssignPlusOp   : 
	  case AssignMinusOp  : 
	  case AssignAndOp    : 
	  case AssignOrOp     : 
	  case AssignXorOp    : 
	  case AssignLShiftOp : 
	  case AssignRShiftOp : 
	  case AssignURShiftOp: ParseCompAssignment(root->GetRightC()); break;
	  case PlusPlusOp     :
	    if (!root->GetRightC()->GetLeftC()->IsDummy())
	      ParsePostIncDecExpr(root->GetRightC());
	    else
	      ParsePreIncDecExpr(root->GetRightC());
	    break;
	  case MinusMinusOp   : 
	    if (!root->GetRightC()->GetLeftC()->IsDummy())
	      ParsePostIncDecExpr(root->GetRightC());
	    else
	      ParsePreIncDecExpr(root->GetRightC());
	    break;
	  case MethodCallOp   : ParseMethodCall(root->GetRightC());     break; 
	  case SwitchStmtOp   : ParseSwitchStmt(root->GetRightC());     break;
	  case BreakStmtOp    : ParseBreakStmt(root->GetRightC());      break;
	  case ContinueStmtOp : ParseContinueStmt(root->GetRightC());   break;
	  case ReturnStmtOp   : ParseReturnStmt(root->GetRightC());     break;
	  case SyncStmtOp     : ParseSyncStmt(root->GetRightC());       break;
	  case ThrowStmtOp    : ParseThrowStmt(root->GetRightC());      break;
	  case TryStmtOp      : ParseTryStmt(root->GetRightC());        break;
	  case LabelStmtOp    : ParseLabelStmt(root->GetRightC());      break;
	    
	    /*
	     * Questi ultimi due casi possono verificarsi solo se si sta
	     * effettuando il parsing di un costruttore.
	     */

	  case ThisOp         : 
	    root->SetRightC(ParseExplConstrInv(root->GetRightC())); 
	    break;
	  case SuperOp        : 
	    root->SetRightC(ParseExplConstrInv(root->GetRightC())); 
	    break;
	  }
    }
}

/*
 * CompileUnit::ParseLocVarDecl
 *
 * Gestisco la dichiarazione di variabili locali.
 */

void CompileUnit::ParseLocVarDecl(TreeNode *root)
{
  /*
   * Per ogni variabile locale dichiarata bisogna:
   *
   * - allocare l'identificatore nella symbol-table a livello 
   *   (FOURTH_LVL,Index);
   * - fare il parsing dell'eventuale istruzione associata;
   * - controllare che il tipo dell'espressione sia assegnabile a tipo della
   *   variabile secondo l'algoritmo di assign conversion (eventualmente fare
   *   il casting implicito);
   */
  
  if (!root->IsDummy())
    {
      ParseLocVarDecl(root->GetLeftC());

      STNode    *locnode=((IDNode *) root->GetRightC()->GetLeftC())->GetIden();
      TreeNode  *etree=root->GetRightC()->GetRightC();

      table->Recover(locnode);
      if (!etree->IsDummy())
	{
	  Descriptor ldes, edes;

	  ldes=locnode->getdescriptor();

	  etree=ParseVarInit(etree);

	  if (ldes.is_reference() && etree->is_NULLNode())
	    etree->SetDescriptor(ldes);

	  root->GetRightC()->SetRightC(etree);
	  edes=etree->GetDescriptor();
      
	  if (!is_Assign_Conversion(ldes,edes))
	    {
	      if (ldes.is_primitive() && edes.is_primitive())
		MsgErrors(locnode->getline(),
			  msg_errors[ERR_EXPLICIT_CAST_NEEDED],"=",
			  edes.to_typename(),ldes.to_typename());
		else
		  MsgErrors(locnode->getline(),
			    msg_errors[ERR_INCOMPATIBLE_TYPE],"=",
			    edes.to_typename(),ldes.to_typename());
	    }
	  else
	    if (ldes!=edes)
	      {
		TreeNode *ctree=new EXPNode(CastOp,Dummy,etree);
		ctree->SetDescriptor(ldes);
		root->GetRightC()->SetRightC(ctree);
	      }
	}
    }
}

/*
 * CompileUnit::ParseIfThenElse
 *
 * Effettua il parsing di un'istruzione if..then..else.
 */

void CompileUnit::ParseIfThenElse(TreeNode *root)
{
  TreeNode *etree=root->GetLeftC()->GetLeftC();

  etree=ParseExpr(etree);

  root->GetLeftC()->SetLeftC(etree);

  if (etree->GetDescriptor()!=DES_BOOLEAN)
    MsgErrors(etree->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"if",
	      etree->GetDescriptor().to_typename(),"boolean");

  if (!root->GetLeftC()->GetRightC()->IsDummy())
    if (((EXPNode *)root->GetLeftC()->GetRightC())->GetNodeOp()==BlockOp)
      ParseBlock(root->GetLeftC()->GetRightC());
    else
      ParseStmt(root->GetLeftC()->GetRightC());
  if (!root->GetRightC()->IsDummy())
    if (((EXPNode *)root->GetRightC())->GetNodeOp()==BlockOp)
      ParseBlock(root->GetRightC());
    else
      ParseStmt(root->GetRightC());
}

/*
 * CompileUnit::ParseLoop
 *
 * Attiva i parsing delle istruzioni while..do, do..while e for.
 */

void CompileUnit::ParseLoop(TreeNode *root)
{
  if (root->GetLeftC()->is_EXPNode() && 
      ((EXPNode *)root->GetLeftC())->GetNodeOp()==CommaOp)
    ParseForStmt(root);
  else
    if (root->GetLeftC()->is_EXPNode() && 
	(((EXPNode *)root->GetLeftC())->GetNodeOp()==StmtOp ||
	 ((EXPNode *)root->GetLeftC())->GetNodeOp()==BlockOp))
      ParseDoWhile(root);
    else
      ParseWhile(root);
}

/*
 * CompileUnit::ParseWhile
 * 
 * Effettua il parsing dell'istruzione while.
 */

void CompileUnit::ParseWhile(TreeNode *root)
{
  env->openContext(IN_WHILE);
  
  TreeNode *etree=root->GetLeftC();

  etree=ParseExpr(etree);

  root->SetLeftC(etree);

  if (etree->GetDescriptor()!=DES_BOOLEAN)
    MsgErrors(etree->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"while",
	      etree->GetDescriptor().to_typename(),"boolean");

  int op=((EXPNode *)root->GetRightC())->GetNodeOp();

  if (op==BlockOp)
    ParseBlock(root->GetRightC());
  else
    ParseStmt(root->GetRightC());

  env->closeContext();
}

/*
 * CompileUnit::ParseDoWhile
 * 
 * Effettua il parsing dell'istruzione do..while.
 */

void CompileUnit::ParseDoWhile(TreeNode *root)
{
  env->openContext(IN_WHILE);
  
  TreeNode *etree=root->GetRightC();

  int op=((EXPNode *)root->GetLeftC())->GetNodeOp();

  if (op==BlockOp)
    ParseBlock(root->GetLeftC());
  else
    ParseStmt(root->GetLeftC());

  etree=ParseExpr(etree);
  root->SetRightC(etree);
  if (etree->GetDescriptor()!=DES_BOOLEAN)
    MsgErrors(etree->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"do",
	      etree->GetDescriptor().to_typename(),"boolean");
  
  env->closeContext();
}

/*
 * CompileUnit::ParseForStmt
 *
 * Effettua il parsing dell'istruzione for.
 */

void CompileUnit::ParseForStmt(TreeNode *root)
{
  env->openContext(IN_FOR);

  TreeNode *etree=root->GetLeftC()->GetRightC();

  /*
   * Rialloco nella symbol-table le variabili locali di inizializzazione del 
   * for.
   */

  ParseStmt(root->GetLeftC()->GetLeftC());

  /*
   * Effettuo il parsing dell'espressione di test.
   */

  etree=ParseExpr(etree);

  root->GetLeftC()->SetRightC(etree);

  if (etree->GetDescriptor()!=DES_BOOLEAN)
    MsgErrors(etree->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"for",
	      etree->GetDescriptor().to_typename(),"boolean");

  ParseStmt(root->GetRightC()->GetRightC());

  if (((EXPNode *)root->GetRightC()->GetLeftC())->GetNodeOp()==BlockOp)
    ParseBlock(root->GetRightC()->GetLeftC());
  else
    ParseStmt(root->GetRightC()->GetLeftC());

  discard_locals(root->GetLeftC()->GetLeftC());
  discard_locals(root->GetRightC()->GetLeftC());
  discard_locals(root->GetRightC()->GetRightC());

  env->closeContext();
}

/*
 * CompileUnit::ParseSwitchStmt
 *
 * Effettua il parsing di un'istruzione switch.
 */

void CompileUnit::ParseSwitchStmt(TreeNode *root)
{
  env->openContext(IN_SWITCH);
  sdefault=FALSE;

  TreeNode *etree=root->GetLeftC();

  etree=ParseExpr(etree);
  root->SetLeftC(etree);
  if (!etree->GetDescriptor().is_integral() && 
      etree->GetDescriptor()!=DES_LONG)
    if (etree->GetDescriptor().is_primitive())
      MsgErrors(etree->GetLine(),msg_errors[ERR_EXPLICIT_CAST_NEEDED],
		"switch",etree->GetDescriptor().to_typename(),"int");
    else
      MsgErrors(etree->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"switch",
		etree->GetDescriptor().to_typename(),"int");

  if (!root->GetRightC()->IsDummy())
    ParseSwitchBody(root,root->GetRightC());

  sdefault=FALSE;
  numlabel=0;
  env->closeContext();
}

/*
 * CompileUnit::ParseSwitchBody
 *
 * Questo metodo analizza ricorsivamente il corpo di uno switch. Ogni chiamata
 * a questo metodo, analizza, in effetti, una singola clausola "case".
 */

void CompileUnit::ParseSwitchBody(TreeNode *root, TreeNode *t)
{
  if (!t->IsDummy())
    {
      ParseSwitchBody(root,t->GetLeftC());
      
      ParseBodySwitchLabel(t->GetRightC()->GetLeftC());
      ParseStmt(t->GetRightC()->GetRightC());
    }
}

/*
 * CompileUnit::ParseBodySwitchLabel
 *
 * Effettua il parsing delle clausole "case" e "default" di un'istruzione 
 * switch.
 */

void CompileUnit::ParseBodySwitchLabel(TreeNode *root)
{
  if (!root->IsDummy())
    {
      ParseBodySwitchLabel(root->GetLeftC());
      
      if (root->GetRightC()->IsDummy())
	if (sdefault)
	  MsgErrors(root->GetLine(),msg_errors[ERR_DUPLICATE_DEFAULT]);
	else
	  sdefault=TRUE;
      else
	{
	  root->SetRightC(ParseConstExpr(root->GetRightC()));

	  TreeNode *Label=root->GetRightC();
	  
	  if (!Label->GetDescriptor().is_integral() && 
	      Label->GetDescriptor()!=DES_LONG)
	    if (Label->GetDescriptor().is_primitive())
	      MsgErrors(Label->GetLine(),msg_errors[ERR_EXPLICIT_CAST_NEEDED],
			"switch",Label->GetDescriptor().to_typename(),"int");
	    else
	      MsgErrors(Label->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],
			"switch",Label->GetDescriptor().to_typename(),"int");
	  else
	    {
	      /*
	       * L'espressione e' un INUMNode, controlliamo solo che la label 
	       * non sia stata gia' definita.
	       */
	      
	      int duplicated=0;

	      for (int i=0; i<numlabel; i++)
		if (((INUMNode *)root->GetRightC())->GetVal()==label[i])
		  {
		    MsgErrors(root->GetLine(),msg_errors[ERR_DUPLICATE_LABEL],
			      "...");
		    duplicated=1;
		    break;
		  }
	      
	      if (!duplicated)
		label[numlabel]=((INUMNode *)root->GetRightC())->GetVal();
	      numlabel++;
	    }
	}
    }
}

/*
 * CompileUnit::ParseBreakStmt
 *
 * Effettua il parsing di un'istruzione break.
 */

void CompileUnit::ParseBreakStmt(TreeNode *root)
{
  if (!env->is_inContext(IN_LOOP) && !env->is_inContext(IN_SWITCH))
    MsgErrors(root->GetLine(),msg_errors[ERR_INVALID_BREAK]);
}

/*
 * CompileUnit::ParseContinueStmt
 *
 * Effettua il parsing di un'istruzione continue.
 */

void CompileUnit::ParseContinueStmt(TreeNode *root)
{
  if (!env->is_inContext(IN_LOOP))
    MsgErrors(root->GetLine(),msg_errors[ERR_INVALID_CONTINUE]);
}

/*
 * CompileUnit::ParseReturnStmt
 *
 * Effettua il parsing di un'istruzione return.
 */

void CompileUnit::ParseReturnStmt(TreeNode *root)
{
  Descriptor rdes, mdes;

  if (root->GetRightC()->IsDummy())
    rdes=DES_VOID;
  else
    {
      /*
       * L'istruzione "return" e' dotata di espressione, per cui si procede al
       * parsing di questa per poi prelevarne il descrittore.
       */

      root->SetRightC(ParseExpr(root->GetRightC()));
      rdes=root->GetRightC()->GetDescriptor();
    }

  /*
   * L'istruzione "return" sintatticamente puo' stare in:
   *
   * - metodi;
   * - costruttori;
   * - inizializzatore statico.
   *
   * E' necessario inviare un messaggio di errore nel caso in cui esso compare
   * nell'inizializzatore statico, perche' semanticamente non e' ammesso.
   */

  if (env->is_inContext(IN_STATIC))
    MsgErrors(root->GetLine(),
	      msg_errors[ERR_RETURN_INSIDE_STATIC_INITIALIZER]);
  else
    {
      mdes=current_meth->getdescriptor().return_type();
      if (!is_Assign_Conversion(mdes,rdes))
	{
	  /*
	   * Il descrittore dell'espressione di "return" e quello del valore
	   * di ritorno del metodo, sono incompatibili, per cui e' necessario
	   * inviare un opportuno messaggio di errore.
	   */

	  if (mdes.is_primitive() && rdes.is_primitive())
	    MsgErrors(root->GetLine(),msg_errors[ERR_LOSE_PRECISION],
		      rdes.to_typename(),mdes.to_typename());
	  else
	    if (mdes.is_reference() && rdes.is_reference())
	      MsgErrors(root->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],
			"return",rdes.to_typename(),mdes.to_typename());
	    else
	      if (rdes==DES_VOID && mdes!=DES_VOID)
		MsgErrors(root->GetLine(),msg_errors[ERR_RETURN_WITHOUT_VALUE],
			  current_meth->getname().to_char());
	      else
		if (mdes==DES_VOID && rdes!=DES_VOID)
		  MsgErrors(root->GetLine(),msg_errors[ERR_RETURN_WITH_VALUE],
			    current_meth->getname().to_char());
		else
		  MsgErrors(root->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],
			    "return",rdes.to_typename(),mdes.to_typename());
	}
      else
	if (rdes!=mdes)
	  {
	    TreeNode *ctree=new EXPNode(CastOp,Dummy,root->GetRightC());
	    ctree->SetDescriptor(mdes);
	    root->SetRightC(ctree);
	  }
    }
}

/*
 * CompileUnit::ParseSyncStmt
 *
 * Effettua il parsing di un'istruzione "synchronized".
 */

void CompileUnit::ParseSyncStmt(TreeNode *root)
{
  TreeNode *etree=root->GetLeftC();
  etree=ParseExpr(etree);
  root->SetLeftC(etree);
  ParseBlock(root->GetRightC());
  if (!etree->GetDescriptor().is_reference())
    MsgErrors(etree->GetLine(),msg_errors[ERR_INVALID_TYPE_EXPR]);
}

/*
 * CompileUnit::ParseThrowStmt
 *
 * Effettua il parsing dell'istruzione "throw".
 */

void CompileUnit::ParseThrowStmt(TreeNode *root)
{
  /*
   * Il parsing di quest'istruzione puo' apparire ad un primo impatto molto
   * complessa, in questo commento sintetizzeremo in breve i passi che verranno
   * compiuti.
   * 
   * 1) Caricare la classe Throwable se per caso non lo si e' fatto in
   *    precedenza;
   * 2) fare il parser dell'espressione e controllare che essa sia assegnabile,
   *    secondo l'algoritmo di Assign Conversion, altrimenti si avra' un
   *    errore di compilazione;
   * 3) Una delle seguenti 3 condizioni deve essere vera o ci sara' un errore 
   *    di compilazione:
   *
   *    a) l'espressione denota la classe RuntimeException o una sua sotto-
   *       classe; oppure l'espressione denota la classe Error o una sua 
   *       sottoclasse;
   *    b) l'istruzione "throw" e' invocata all'interno di un'istruzione
   *       "try" e l'espressione e' assegnabile a uno dei parametri delle
   *       clausole "catch";
   *    c) "throw" e' lanciato in un metodo o costruttore e l'espressione e'
   *       assegnabile a una delle classi disposte nella clausola "throws"
   *       di questi.
   */

  TreeNode *etree=root->GetLeftC();        // parse-tree espressione.

  STNode *tnode=LoadClass(ThrowableName,++LastIndex);

  etree=ParseExpr(etree);
  root->SetLeftC(etree);

  if (!tnode || !is_Assign_Conversion(DesThrowable,etree->GetDescriptor()))
    MsgErrors(etree->GetLine(),msg_errors[ERR_THROW_NOT_THROWABLE],
	      etree->GetDescriptor().to_typename());
  else
    {
      /*
       * Carico in:
       *
       * - cnode, la classe dell'espressione dell'istruzione throw;
       * - enode, la classe "java/lang/Error";
       * - rnode, la classe "java/lang/RuntimeException".
       */

      String cname;

      cname=etree->GetDescriptor().to_fullname();

      STNode *cnode=LoadClass(cname,++LastIndex);
      STNode *rnode=LoadClass(RuntimeExceptionName,++LastIndex);
      STNode *enode=LoadClass(ErrorName,++LastIndex);

      /*
       * L'Espressione e' assegnabile a java/lang/Throwable, per cui
       * bisogna controllare che una delle 3 condizioni sopra descritte
       * sia vera.
       */
      
      /*
       * Condizione a).
       */

      if (cnode && ((cnode==rnode || cnode->is_subclass(rnode)) ||
		    cnode==enode || cnode->is_subclass(enode)))
	return;
	
      /*
       * Condizione b).
       *
       * NON GESTITO!!!
       * 
       * Il return e' eseguito solo se l'espressione e' assegnabile a una 
       * delle classi specificate in catch.
       */

      if (env->is_inContext(IN_TRY))
	return;

      /*
       * Condizione c).
       */   

      if (env->is_inContext(IN_METHOD))
	for (TreeNode *t=current_meth->getmyheader()->GetLeftC()->GetRightC(); 
	     !t->IsDummy(); t=t->GetLeftC())
	  if (cnode->is_subclass(((IDNode *)t->GetRightC())->GetIden()) ||
	      cnode==((IDNode *)t->GetRightC())->GetIden())
	    return;

      /*
       * Nessuna delle 3 condizioni e' vera, per cui si invia un messaggio di
       * errore.
       */

      MsgErrors(cnode->getline(),msg_errors[ERR_UNCAUGHT_EXCEPTION],
		cnode->getfullname().to_char());
    }
}

/*
 * CompileUnit::ParseTryStmt
 * CompileUnit::ParseTryBody
 *
 * Questi due metodi, combinati insieme effettuano il parsing di un'istruzione
 * "try".
 */

void CompileUnit::ParseTryStmt(TreeNode *root)
{
  /*
   * E' necessario sapere se ci troviamo in un'istruzione "try" nel caso in
   * cui si effettua il parsing di un'istruzione "throw", per cui e' bene 
   * memorizzare tale informazione nello stack ENVIRONMENT.
   */

  env->openContext(IN_TRY);

  ParseBlock(root->GetLeftC());
  ParseTryBody(root->GetRightC());

  env->closeContext();
}

void CompileUnit::ParseTryBody(TreeNode *root)
{
  if (!root->IsDummy())
    {
      STNode *pnode;

      ParseTryBody(root->GetLeftC());
      
      if (!root->GetRightC()->GetLeftC()->IsDummy())
	{
	  /*
	   * Siamo in una clausola Catch.
	   */

	  STNode *cnode;
	  String Name;

	  pnode=((IDNode *)root->GetRightC()->GetLeftC())->GetIden();

	  table->Recover(pnode);

	  Name=pnode->getdescriptor().to_fullname();

	  if ((cnode=LoadClass(Name,++LastIndex))==NULL)
	    MsgErrors(pnode->getline(),msg_errors[ERR_CLASS_NOT_FOUND],
		      Name.to_char(),"catch");
	  else
	    if (!cnode->is_class() && !cnode->is_interface())
	      MsgErrors(pnode->getline(),msg_errors[ERR_CATCH_NOT_THROWABLE],
			cnode->getdescriptor().to_typename());
	    else
	      if (!cnode->is_subclass(LoadClass(ThrowableName,++LastIndex)))
		MsgErrors(pnode->getline(),msg_errors[ERR_CATCH_NOT_THROWABLE],
			  cnode->getdescriptor().to_typename());
	}

      /*
       * Sia che stiamo in una clausola Catch che Finally e' necessario fare
       * il parsing del blocco di codice allegato.
       */
      
      ParseBlock(root->GetRightC()->GetRightC());
      
      if (!root->GetRightC()->GetLeftC()->IsDummy())
	table->Discard(pnode);
    }  
}

/*
 * CompileUnit::ParseExplConstrInv
 *
 * Invocazione esplicita a costruttori della classe corrente o superclasse.
 * Cio' e' ammesso solo all'inizio di un costruttore.
 */

TreeNode *CompileUnit::ParseExplConstrInv(TreeNode *root)
{
  /*
   * root e' un EXPNode di tipo: ThisOp o SuperOp
   */
  
  TreeNode *ctree;
  TreeNode *args=root->GetRightC();

  root->SetRightC(Dummy);

  if (((EXPNode *)root)->GetNodeOp()==ThisOp)
    {
      ctree=new THISNode();
      ctree->SetDescriptor(current_class->getdescriptor());
    }
  else
    {
      /*
       * NON GESTITO!!!
       *
       * Stiamo lavorando con l'ipotesi che non compilaremo mai il sorgente
       * di java/lang/Object, quindi ogni classe compilata e' dotata di 
       * superclasse.
       */

      ctree=new SUPERNode();
      ctree->SetDescriptor(current_class->get_superclass()->getdescriptor());
    }

  delete root;

  TreeNode *tree=new EXPNode(MethodCallOp,
		     new EXPNode(CommaOp,ctree,new UNAMENode(init_name)),
			     args);

  return ParseConstrCall(tree);
}

/*
 * CompileUnit::ParseLabelStmt
 *
 * effettua il parsing di una o piu' istruzioni etichettate.
 */

void CompileUnit::ParseLabelStmt(TreeNode *root)
{
  table->Recover(((IDNode *)root->GetLeftC())->GetIden());
  ParseStmt(root->GetRightC());
}

/*
 * CompileUnit::ParseInterfaceDecl
 *
 * Da questo metodo inizia il parsing di un'interfaccia.
 */

void CompileUnit::ParseInterfaceDecl(TreeNode *root)
{
  env->openContext(IN_INTERFACE);
  Nest_lev++;
  STNode *inode=((IDNode *)root->GetLeftC()->GetLeftC())->GetIden();
  int    iaccess=inode->getaccess();

  current_class=inode;

  /*
   * Un'interfaccia non puo' essere: final, private, synchronized, native,
   * static e  protected.
   */

  if (IS_FINAL(iaccess))
    MsgErrors(inode->getline(),msg_errors[ERR_FINAL_INTF]);

  if (IS_FINAL(iaccess))
    MsgErrors(inode->getline(),msg_errors[ERR_PRIVATE_CLASS]);

  if (IS_PRIVATE(iaccess))
    MsgErrors(inode->getline(),msg_errors[ERR_PRIVATE_CLASS]);

  if (IS_SYNCHRONIZED(iaccess) || IS_NATIVE(iaccess) || IS_STATIC(iaccess) ||
      IS_PROTECTED(iaccess))
    MsgErrors(inode->getline(),msg_errors[ERR_TOPLEVEL_EXPECTED]);

  if (IS_PUBLIC(iaccess) && intf_public)
    MsgErrors(inode->getline(),msg_errors[ERR_PUBLIC_CLASS_FILE],
	      inode->getname().to_char(),(inode->getname()+".java").to_char());
  
  if (IS_PUBLIC(iaccess)) intf_public++;

  /*
   * Analizziamo ora le superinterfacce, cercando, se e' il caso, di risolvere
   * i nomi, controllare che siano interfacce, eliminare nomi ripetuti e,
   * infine, controllare la presenza di definizioni cicliche nell'ereditarie-
   * ta'.
   */

  TreeNode *prectree=NULL;

  for (TreeNode *etree=root->GetLeftC()->GetRightC(); !etree->IsDummy();
       prectree=etree, etree=etree->GetLeftC())
    {
      String cname;
      STNode *cnode;

      if (etree->GetRightC()->is_UNAMENode())
	{
	  /*
	   * Il nome e' "unresolved", e' necessario risolverlo.
	   */

	  cname=((UNAMENode *)root->GetRightC())->GetName();
	  if ((cnode=LoadClass(cname,++LastIndex))!=NULL)
	    {
	      delete etree->GetRightC();
	      etree->SetRightC(new IDNode(cnode));
	    }
	}
      else
	{
	  cnode=((IDNode *)root->GetRightC())->GetIden();
	  cname=cnode->getname();
	}

      /*
       * Se il nome e' stato risolto, controllare che sia un'interfaccia.
       */

      if (cnode && !cnode->is_interface())
	MsgErrors(cnode->getline(),msg_errors[ERR_NOT_INTF],cname.to_char());

      /*
       * Sia che il nome e' stato risolto oppure che non lo sia e' necessario
       * controllare che il nome corrente non sia duplicato
       */

      for (TreeNode *t=etree->GetLeftC(); t->IsDummy(); t=t->GetLeftC())
	{
	  String name;

	  if (t->is_UNAMENode())
	    name=((UNAMENode *)t)->GetName();
	  else
	    name=((IDNode *)t)->GetIden()->getname();

	  if (cname==name)

	    /*
	     * Il nome di questa interfaccia va trascurato.
	     */
	    
	    if (prectree)
	      {
		prectree->SetLeftC(etree->GetLeftC());
		etree->SetLeftC(Dummy);
		delete etree;
	      }
	    else
	      {
		root->GetLeftC()->SetRightC(etree->GetLeftC());
		etree->SetLeftC(Dummy);
		delete etree;
	      }
	}
    }
  
  Nest_lev++;
  ParseInterfaceBody(root->GetRightC());
  Nest_lev--;

  current_class=NULL;
  Nest_lev--;
  env->closeContext();
}

/*
 * CompileUnit::ParseInterfaceBody
 *
 * Analizza il corpo di un'interfaccia.
 */

void CompileUnit::ParseInterfaceBody(TreeNode *root)
{
  /*
   * Nel corpo di un'interfaccia sintatticamente e' possibile trovare solo:
   *
   * - metodi astratti;
   * - campi inizializzati;
   */

  if (!root->IsDummy())
    {
      ParseInterfaceBody(root->GetLeftC());

      if (((EXPNode *)root->GetRightC())->GetNodeOp()==FieldDeclOp)
	ParseIntfFieldDecl(root->GetRightC());
      else
	ParseIntfMethodDecl(root->GetRightC());
    }
}

/*
 * CompileUnit::ParseIntfFieldDecl
 *
 * In effetti questo metodo richiama ParseFieldDecl che operera' sulla radice,
 * in piu' questo metodo effettuera' alcuni accorgimenti ausiliari che possia-
 * mo osservare leggendo il codice relativo.
 */

void CompileUnit::ParseIntfFieldDecl(TreeNode *root)
{
  ParseFieldDecl(root);
  
  /*
   * Per ogni identificatore:
   *
   * - setto il campo STATIC e FINAL;
   * - se l'interfaccia corrente e' PUBLIC allora anche il campo lo sara';
   * - l'espressione associata deve essere costante;
   * - se il campo e' definito PRIVATE o PROTECTED ci sara' un errore di
   *   compilazione.
   */

  for (TreeNode *ftree=root; !ftree->IsDummy(); ftree=ftree->GetLeftC())
    {
      STNode *idnode=((IDNode *)ftree->GetRightC()->GetLeftC())->GetIden();

      idnode->setaccess(idnode->getaccess() | ACC_STATIC | ACC_FINAL);

      if (IS_PUBLIC(current_class->getaccess()))
	idnode->setaccess(idnode->getaccess() | ACC_PUBLIC);

      TreeNode *etree=ftree->GetRightC()->GetRightC();

      if (!IsConstExpr(etree))
	MsgErrors(etree->GetLine(),msg_errors[ERR_CONST_EXPR_REQUIRED]);

      if (IS_PRIVATE(idnode->getaccess()) || IS_PROTECTED(idnode->getaccess()))
	MsgErrors(idnode->getline(),msg_errors[ERR_INTF_MODIFIER_FIELD]);
    }  
}

/*
 * CompileUnit::ParseIntfMethodDecl
 *
 * Anche qui l' analisi e' simile alla dichiarazione di metodi nelle classi,
 * solo che ci sono alcune diversita' che possiamo apprendere leggendo il 
 * metodo seguente.
 */

void CompileUnit::ParseIntfMethodDecl(TreeNode *root)
{
  /*
   * NON GESTITO!!!
   *
   * Non so se cio' serve, pero' l'interfaccia deve sapere quanti metodi as-
   * tratti sono ivi definiti?
   */

  STNode  *current_meth=((IDNode *)
			 root->GetLeftC()->GetLeftC()->GetLeftC())->GetIden();
  int     maccess=current_meth->getaccess();

  ParseMethodHeader(root);

  maccess=maccess | ACC_ABSTRACT;

  if (IS_PUBLIC(current_class->getaccess()))
    maccess=maccess | ACC_PUBLIC;
  
  if (IS_PROTECTED(maccess) || IS_FINAL(maccess) || IS_NATIVE(maccess) || 
      IS_STATIC(maccess) || IS_SYNCHRONIZED(maccess) || IS_PRIVATE(maccess))
    MsgErrors(current_meth->getline(),msg_errors[ERR_INTF_MODIFIER_METHOD],
	      current_meth->getname().to_char());

  current_meth->setaccess(maccess);
  
  discard_parameters(root->GetLeftC()->GetLeftC()->GetRightC());
  current_meth=NULL;
}

/*
 * CompileUnit::ParseConstrDecl
 *
 * Parsing di un costruttore.
 */

void CompileUnit::ParseConstrDecl(TreeNode *root)
{
  env->openContext(IN_CONSTRUCTOR);

  STNode *cnode=((IDNode*)root->GetLeftC()->GetLeftC()->GetLeftC())->GetIden();
  int    caccess=cnode->getaccess();

  if (IS_NATIVE(caccess) || IS_SYNCHRONIZED(caccess) || IS_ABSTRACT(caccess) ||
      IS_STATIC(caccess) || IS_FINAL(caccess))
    MsgErrors(cnode->getline(),msg_errors[ERR_CONSTR_MODIFIER],
	      current_class->getname().to_char());

  /*
   * Rimettiamo i parametri formali nella symbol-table.
   */
  
  TreeNode *ptree=root->GetLeftC()->GetLeftC()->GetRightC();

  for (TreeNode *t=ptree; !t->IsDummy(); t=t->GetRightC())
    table->Recover(((IDNode *)t->GetLeftC())->GetIden());

  /*
   * Eseguiamo il parsing della clausola throws e del corpo del metodo.
   */
  
  ParseThrows(root->GetLeftC()->GetRightC());

  if (!root->GetRightC()->IsDummy())
    {
      Nest_lev++;
      ParseBlock(root->GetRightC());
      Nest_lev--;
    }
  
  discard_parameters(root->GetLeftC()->GetLeftC()->GetRightC());

  env->closeContext();
}

/*
 * CompileUnit::ParseStaticInit
 *
 * Effettua il parsing dell'inizializzatore statico.
 */

void CompileUnit::ParseStaticInit(TreeNode *root)
{
  env->openContext(IN_STATIC);
  ParseBlock(root->GetRightC());
  env->closeContext();
}

/*
 * CompileUnit::ParseVarInit
 * 
 * Utilizzato quando si inizializza un campo o una variabile locale.
 */

TreeNode *CompileUnit::ParseVarInit(TreeNode *root)
{
  if (((EXPNode *)root)->GetNodeOp()==ArrayComponentOp)
    return ParseArrayInit(root);
  else
    return ParseExpr(root);
}

/*
 * CompileUnit::ParseArrayInit
 *
 * Analizza l'inizializzazione di un campo o var. locale di tipo array.
 */

TreeNode *CompileUnit::ParseArrayInit(TreeNode *root)
{
  if (!root->IsDummy())
    {
      Descriptor descriptor;

      root->SetLeftC(ParseArrayInit(root->GetLeftC()));
      
      descriptor=root->GetLeftC()->GetDescriptor();

      if (((EXPNode *)root->GetRightC())->GetNodeOp()==ArrayComponentOp)
	root->SetRightC(ParseArrayInit(root->GetRightC()));
      else
	root->SetRightC(ParseExpr(root->GetRightC()));

      descriptor+=root->GetRightC()->GetDescriptor();

      root->SetDescriptor(descriptor);

      return root;
    }
} 

