/*
 * file gen.cc
 *
 * descrizione: questo file implementa la fase di generazione del codice,
 *              nella quale un parse-tree di una classe o interfaccia corretta
 *              viene codificata in codice JVM.
 *
 * Questo file e' parte del Compilatore Java.
 *
 * Maggio 1997, scritto da Michele Risi, e-mail micris@zoo.diaedu.unisa.it
 *                         Salvatore D'Angelo, e-mail xc0261@xcom.it
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <globals.h>
#include <cstring.h>
#include <descriptor.h>
#include <hash.h>
#include <cphash.h>
#include <table.h>
#include <cstring.h>
#include <tree.h>
#include <jvm.h>
#include <compile.h>
#include <opcodes.h>
#include <access.h>
#include <../stack.cc>
#include <local.h>
#include <backpatch.h>

extern TreeNode *Dummy;
extern STNode *current_class;
extern int max_local;   // taglia dell' area riservata alle var. locali e ai
                        // parametri sullo stack frame.

/*
 * CompileUnit::GenCode
 *
 * Metodo dal quale inizia la generazione del codice JVM.
 */

void CompileUnit::GenCode() 
{ 
  GenCompUnit(syntax_tree->GetRoot()); 
}

/*
 * CompileUnit::GenCompUnit
 *
 * Da qui si controlla se una classe o interfaccia e' priva di errori e puo' 
 * essere generata.
 */

void CompileUnit::GenCompUnit(TreeNode *root)
{
  if (!root->IsDummy())
    {
      String Name;
      
      GenCompUnit(root->GetLeftC());

      /*
       * Genero la classe o l'interfaccia a patto che sia corretta.
       */

      STNode *Id=
	((IDNode *)root->GetRightC()->GetLeftC()->GetLeftC())->GetIden();

      if (Id->is_generable())
	{

	  Class = new Class_File();
	  cphash= new CPHash(NUM_BUCKET);
      
	  if (((EXPNode *)root->GetRightC())->GetNodeOp()==ClassOp)
	    GenClass(root->GetRightC());
	  else
	    GenInterface(root->GetRightC());

	  Name=Id->getname();

	  Class->WriteClass(Name);

	  delete cphash;
	  delete Class;
	}
    }
}

/*****************************************************************************
 * Codice per la generazione di una classe.                                  *
 *****************************************************************************/

/*
 * CompileUnit::GenClass
 *
 * Se e' attivato questo metodo, significa che la classe e' priva di errori,
 * per cui puo' essere generata.
 */

void CompileUnit::GenClass(TreeNode *root)
{
  _u2 attr_name_index, name_index;
  String SourceFile;
  String Name;
  int class_index;
  STNode *Id;

  current_class=((IDNode *)root->GetLeftC()->GetLeftC())->GetIden();

  /*
   * Classe This.
   */


  Class->Setaccessflags(current_class->getaccess());
  Name=current_class->getfullname();

  Class->Setthisclass(Class->Load_Constant_Class(Name));

  /*
   * Superclasse.
   */

  Id=((IDNode *)
      root->GetLeftC()->GetRightC()->GetLeftC()->GetRightC())->GetIden();

  Class->Setsuperclass(Class->Load_Constant_Class(Id->getfullname()));  

  /*
   * Interfacce Implementate.
   */

  GenSuperIntf(root->GetLeftC()->GetRightC()->GetRightC());

  /*
   * Generazione corpo di una classe: metodi, campi, costruttori e inizializ-
   * zatore statico.
   */

  GenClassBody(root->GetRightC());

  /*
   * Imposto l'attributo SourceFile.
   */

  SourceFile="SourceFile";

  attr_name_index=Class->Load_Constant_Utf8(SourceFile);
  name_index=Class->Load_Constant_Utf8(name_source);

  Class->SetAttributes(new Source_File_Attribute(name_index,attr_name_index));
  Class->Setconstantpoolcount(Class->Getconstantpool()->GetLen());
  Class->Setattributescount(1);  
  current_class=NULL;
}

/*
 * CompileUnit::GenSuperIntf
 *
 * Genera i riferimenti alle interfacce implementate dalla classe corrente o
 * alle superinterfacce di un'interfaccia.
 * Se a chiamare questo metodo e' GenClass, allora un albero ImplementsOp gli
 * verra' trasmesso, altrimenti un ExtendsOp.
 */

void CompileUnit::GenSuperIntf(TreeNode *root)
{
  if (!root->IsDummy())
    {
      GenSuperIntf(root->GetLeftC());

      STNode *Id=((IDNode *) root->GetRightC())->GetIden();
      String Name;

      Name=Id->getfullname();

      Class->Setinterfaces(Class->Load_Constant_Class(Name));
      Class->Setinterfacescount((Class->Getinterfacescount())+1);      
    }
}

/*
 * CompileUnit::GenClassBody
 *
 * Passiamo ora alla generazione del corpo di una classe. In esso troveremo
 * dichiarazioni di campi, metodi, costruttori e inizializzatore statico.
 * Se questo metodo verra' invocato da un'interfaccia, allora solo metodi e
 * campi saranno generati.
 */

void CompileUnit::GenClassBody(TreeNode *root)
{
  if (!root->IsDummy())
    {
      GenClassBody(root->GetLeftC());

      switch (((EXPNode *)root->GetRightC())->GetNodeOp())
	{
	case FieldDeclOp : GenFieldDecl(root->GetRightC());  break;
	case MethodDeclOp: GenMethodDecl(root->GetRightC()); break;
	case ConstrDeclOp: GenMethodDecl(root->GetRightC()); break;
	case StaticInitOp: GenStaticInit(root->GetRightC()); break;
	}
    }
}

/*
 * CompileUnit::GenFieldDecl
 *
 * Generazione di campi per una classe.
 */

void CompileUnit::GenFieldDecl(TreeNode *root)
{
  if (!root->IsDummy())
    {
      GenFieldDecl(root->GetLeftC());

      /*
       * Genero la definizione del campo.
       */

      Field_Info *field=new Field_Info();
      STNode *Id=((IDNode *) root->GetRightC()->GetLeftC())->GetIden();
      String Name, Des_Utf8;
      Constant_Value_Attribute *cv;

      /*
       * Riempiamo l'entry field info riservata al campo.innanzittutto setto
       * gli accessi.
       */

      field->SetAccessFlags(Id->getaccess());      
      Name=Id->getfullname();

      field->SetNameIndex(Class->Load_Constant_Utf8(Name));

      /*
       * Settiamo descriptor index.
       */

      Des_Utf8=Id->getdescriptor().to_char();

      field->SetDescriptorIndex(Class->Load_Constant_Utf8(Des_Utf8));

      /*
       * Imposto l'eventuale attributo ConstantValue. Le specifiche della JVM
       * sostengono che tale attributo va usato solo per campi statici. Speri-
       * mentalmente, pero', mi sono reso conto che esso e' generato quando il
       * campo e' static e final.
       */

      if (!root->GetRightC()->GetRightC()->IsDummy())
	if (IS_STATIC(Id->getaccess()) && IS_FINAL(Id->getaccess()))
	  {
	    _u2 attr_name_index;
	    
	    field->SetAttributesCount(1);
	    field->SetArray(1);
	    Name="ConstantValue";

	    attr_name_index=Class->Load_Constant_Utf8(Name);

	    /*
	     * L'espressione associata al campo, per poter essere generata,
	     * deve essere costante (altrimenti va calcolata a run-time).
	     * Poiche' per ora il constant-folding non e' implementato alla
	     * perfezione, dovremo supporre di avere uno dei seguenti tipi
	     * di nodi: INUMNode, CHARNode, FNUMNode, STRINGNode e BOOLEANNode;
	     * cosa che sara' sempre vera quando il costant-folding funzionera'
	     * al meglio.
	     */

	    TreeNode *expr=root->GetRightC()->GetRightC();
	    int index;

	    long valint, valbool;
	    double valfloat;
	    
	    switch (expr->GetNodeKind())
	      {
	      case _CHARNode  :
	      case _INUMNode  :
		valint=((INUMNode *)expr)->GetVal();

		if (expr->GetDescriptor()==DES_LONG)
		  index=Class->Load_Constant_Long(valint);
		else
		  index=Class->Load_Constant_Integer(valint);
		break;

	      case _FNUMNode  :
		valfloat=((FNUMNode *)expr)->GetVal();

		if (expr->GetDescriptor()==DES_DOUBLE)
		  index=Class->Load_Constant_Double(valfloat);
		else
		  index=Class->Load_Constant_Float(valfloat);
		break;

	      case _BOOLEANode:
		valbool=((BOOLEANode *)expr)->GetBoolean();

		index=Class->Load_Constant_Integer(valbool);
		break;

	      case _STRINGNode:
		String str;

		str=((STRINGNode *)expr)->GetString();
		index=Class->Load_Constant_String(str);
		break;
	      }
	    
	    cv=new Constant_Value_Attribute(index,attr_name_index);
	    field->SetAttribute(cv,0);
	  }
	else
	  field->SetAttributesCount(0);
      else
	field->SetAttributesCount(0);

      Class->Setfieldinfo(field);
      Class->Setfieldcount((Class->Getfieldcount())+1);
    }
}

/*
 * CompileUnit::GenMethodDecl
 *
 * Generazione sia dell'intestazione che del codice di un metodo.
 */

void CompileUnit::GenMethodDecl(TreeNode *root)
{
  Local_Count *local;

  STNode *Id=((IDNode *)root->GetLeftC()->GetLeftC()->GetLeftC())->GetIden();
  Method_Info *minfo=new Method_Info();

  String Name, Des_Utf8;

  /*
   * Allochiamo lo stack utilizzato per riservare spazio alle varibili locali
   * e ai parametri sullo stack frame.
   */

  stk = new Stack<Local_Count *>(MAX_LOCAL);
  
  if (IS_STATIC(Id->getaccess()))
    local = new Local_Count(-1);
  else
    local = new Local_Count(0);

  stk->Push(local);

  /*
   * Allochiamo un method_info nella tabella dei metodi e impostiamo i valori
   * dei vari campi.
   */

  minfo->SetAccessFlags(Id->getaccess());  

  Name=Id->getfullname();
  minfo->SetNameIndex(Class->Load_Constant_Utf8(Name));

  Des_Utf8=Id->getdescriptor().to_char();
  minfo->SetDescriptorIndex(Class->Load_Constant_Utf8(Des_Utf8));

  /*
   * Generiamo la parte di codice inerente ai parametri formali. Piu' che
   * generare del codice noi non facciamo altro che riservare uno spazio
   * nello stack frame per ciascun parametro, come se fosse una variabile
   * locale.
   */

  if (IS_STATIC(Id->getaccess()))
    max_local=0;
  else
    max_local=1;

  GenMethodParameters(root->GetLeftC()->GetLeftC()->GetRightC());

  /*
   * Generiamo le definizioni della sezione Throws.
   */

  GenMethodThrows(root->GetLeftC()->GetRightC(),minfo);

  /*
   * Genero il codice JVM per il metodo, ovverosia la sezione CodeAttribute.
   */

  GenMethodBody(root->GetRightC(),minfo);

  Class->Setmethodinfo(minfo);

  Class->Setmethodcount((Class->Getmethodcount())+1);

  delete stk;
}

/*
 * CompileUnit::GenMethodParameters
 *
 * Questo metodo piu' che generare codice, riserva lo spazio necessario ai pa-
 * rametri nello stack frame del metodo. Ricordiamo brevemente che lo stack
 * frame e' una struttura dati fondamentale in teoria dei compilatori (vedi
 * Cap. 7 "Compilers, Principles Techniques and tools" Ullman) sul quale
 * noi riserviamo spazi per parametri e variabili locali, mentre la sua reale
 * implementazione sta nel back-end del compilatore (interprete Java).
 */

void CompileUnit::GenMethodParameters(TreeNode *root)
{
  for (TreeNode *t=root; !t->IsDummy(); t=t->GetRightC())
    {
      STNode *Id=((IDNode *)t->GetLeftC())->GetIden();
      Descriptor descriptor;
      int index;

      descriptor=Id->getdescriptor();

      if (descriptor.is_reference())
	index=stk->Top()->inc(1);
      else
	if (descriptor.is_primitive())
	  if (descriptor.is_integral())
	    if (descriptor==DES_LONG)
	      {
		index=stk->Top()->inc(2);
		index--;
	      }
	    else
	      index=stk->Top()->inc(1);
	  else
	    if (descriptor==DES_DOUBLE)
	      {
		index=stk->Top()->inc(2);
		index--;
	      }
	    else
	      index=stk->Top()->inc(1);
	else
	  if (descriptor==DES_BOOLEAN)
	    index=stk->Top()->inc(1);
      
      Id->setlocalindex(index);
    }
}

/*
 * CompileUnit::GenMethodThrows
 *
 * Generiamo le definizione delle classi situate nella clausola throws del 
 * metodo.
 */

void CompileUnit::GenMethodThrows(TreeNode *root, Method_Info *minfo)
{
  if (! root->IsDummy())
    {
      int len=0;
      Exceptions_Attribute *ea = new Exceptions_Attribute();          
      _u2 attr_name_index;
      
      for (TreeNode *t=root; !t->IsDummy(); t=t->GetLeftC())
	{
	  String str;
	  
	  str="Exceptions";
	  attr_name_index=Class->Load_Constant_Utf8(str);
	  
	  STNode *Id=((IDNode *)t->GetRightC())->GetIden();
	  str=Id->getfullname();
	  
	  int class_index=Class->Load_Constant_Class(str);
	  
	  ea->SetException(class_index);
	  len++;
	}
      
      ea->SetNumberOfExceptions(len);
      ea->SetAttributeNameIndexLength(attr_name_index,(sizeof(_u2)*(len+1)));
      minfo->SetArray(2);
      minfo->SetAttribute(ea,minfo->GetAttributesCount());
      minfo->SetAttributesCount(minfo->GetAttributesCount()+1);
    }
}

/*
 * CompileUnit::GenStaticInit
 *
 * L'inizializzatore statico e' equivalente a un metodo statico senza parametri
 * e senza la clausola throws. Il nome di questo metodo all'interno di JVM
 * e' <clinit>. L'idea da noi impostata e' quella di gestire l'inizializzatore
 * statico proprio come un metodo.
 */

void CompileUnit::GenStaticInit(TreeNode *root)
{
  // per ora trascuriamo, perche' penso di dover modificare un po' il parser.
}

/*****************************************************************************
 * Codice per la generazione di un' interfaccia.                             *
 *****************************************************************************/

/*
 * CompileUnit::GenInterface
 *
 * Genera un file .class per un'interfaccia.
 */

void CompileUnit::GenInterface(TreeNode *root)
{
  String Name;
  _u2 attr_name_index, name_index;

  current_class=((IDNode *)root->GetLeftC()->GetLeftC()->GetLeftC())->GetIden();

  /*
   * Setto gli accessi dell'interfaccia. Si ricordi che un'interfaccia si
   * distingue da una classe per il fatto che ACC_INTERFACE e' settato.
   */

  Class->Setaccessflags(current_class->getaccess());

  /*
   * Impostiamo il puntatore this_index del file .class.
   */

  Name=current_class->getfullname();

  Class->Setthisclass(Class->Load_Constant_Class(Name));

  /*
   * Impostiamo il puntatore super_index del file .class. L'indice super_index
   * per un'interfaccia fa riferimento sempre a un item Constant_Class conte-
   * nente la definizione di java/lang/Object. 
   */

  Name="java/lang/Object";

  Class->Setsuperclass(Class->Load_Constant_Class(Name));  

  /*
   * Generiamo i riferimenti alle superinterfacce dell'interfaccia corrente.
   */

  GenSuperIntf(root->GetLeftC()->GetRightC()->GetRightC());

  /*
   * Generiamo il corpo dell'inetrfaccia, ossia i riferimenti a metodi e campi.
   * Il metodo invocato e' lo stesso che genera il corpo di una classe, la 
   * differenza, pero', e' che quest'ultimo non invochera' mai un costruttore
   * o inizializzatore statico.
   */

  GenClassBody(root->GetRightC());

  /*
   * Impostiamo l'attributo SourceFile per il file .class
   */

  Name="SourceFile";

  attr_name_index=Class->Load_Constant_Utf8(Name);
  name_index=Class->Load_Constant_Utf8(name_source);

  Class->SetAttributes(new Source_File_Attribute(attr_name_index,name_index));
  Class->Setconstantpoolcount(Class->Getconstantpool()->GetLen());
  Class->Setattributescount(1);
  current_class=NULL;
}

/*****************************************************************************
 * Codice per la generazione del corpo di un metodo in codice JVM.           *
 *****************************************************************************/

/*
 * CompileUnit::GenMethodBody
 *
 * Genero il corpo del metodo, ossia le effettive istruzioni JVM, che saranno
 * poste nella sezione CodeAttribute del metodo.
 */

void CompileUnit::GenMethodBody(TreeNode *root, Method_Info *minfo)
{
  String str;

  Constant_Utf8 *C_Utf8;

  char *name;
  _u2 cvalue;
  _u2 ind;
  
  Code_Attribute *ca = new Code_Attribute();
   
  str="Code";
  ca->SetAttributeNameIndex(Class->Load_Constant_Utf8(str));

  stk->Dup();          // duplico il top dello stack per var. locali.
  
  not_return=TRUE;     // usato per stabilire se nel metodo e' usato return.
  
  pc_count=0;          // reset program counter.

  /*
   * Allochiamo gli stack: s_brk e s_con; per la gestione delle istruzioni 
   * break e continue.
   */

  s_brk = new Stack<brk_rec *>(S_BRK_SIZE);
  s_con = new Stack<con_rec *>(S_CON_SIZE);

  GenBlock(root,ca);

  delete s_brk;
  delete s_con;
  
  if (not_return)
    ca->Gen(RETURN);

  ca->SetMaxStack(20);  // parametro da determinare
  ca->SetMaxLocals(max_local);
  ca->SetCodeLength(pc_count);
  
  ca->SetAttributeLength(8+pc_count+2+8*ca->GetExceptionTableLength()+2);
  
  if(!minfo->GetAttributesCount())
    minfo->SetArray(1);

  minfo->SetAttribute(ca,minfo->GetAttributesCount());
  minfo->SetAttributesCount(minfo->GetAttributesCount()+1);  

  stk->Pop();
}

/*
 * CompileUnit::GenStmts
 *
 * Genera istruzioni JVM.
 */

void CompileUnit::GenStmts(TreeNode *root, Code_Attribute *ca)
{
  if (!root->IsDummy())
    {
      GenStmts(root->GetLeftC(),ca);

      if (!root->GetRightC()->IsDummy())
	GenStmt(root->GetRightC(),ca);
    }
}

/*
 * CompileUnit::GenStmt
 *
 * Genera una singola istruzione JVM. Le istruzioni possono essere:
 *
 * - dichiarazione variabili locali;
 * - if .. then .. else;
 * - while .. do;
 * - do .. while;
 * - for;
 * - blocco di istruzioni;
 * - nulla;
 * - assegnazione semplice;
 * - assegnazione composta;
 * - pre/post-incremento/decremento;
 * - invocazione di metodi;
 * - creazione di istanze di classi;
 * - switch;
 * - break;
 * - continue;
 * - return;
 * - synchronized;
 * - throw;
 * - try .. catch .. finally;
 * - istruzioni etichettate.
 */
 
void CompileUnit::GenStmt(TreeNode *root,Code_Attribute *ca)
{
  if (!root->IsDummy())
    {
      switch (((EXPNode *)root)->GetNodeOp())
	{
	case LocVarDeclOp   : GenLocalVarDecl(root,ca);     break;
	case IfThenElseOp   : GenIf(root,ca);               break;
	case LoopOp         : GenLoop(root,ca);             break;
	case BlockOp        : GenBlock(root,ca);            break;
	case AssignOp       : GenAssignment(root,ca);       break;
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
	case AssignURShiftOp: GenCompAssignment(root,ca);   break;
	case MinusMinusOp   :
	case PlusPlusOp     : 
	  if (root->GetLeftC()->IsDummy())
	    GenIncDecStmt(root,ca);
	  else
	    GenIncDecStmt(root,ca);
	  break;
	case MethodCallOp   : GenMethodCall(root,ca,FALSE); break;
	case NewOp          : GenNewClassInstance(root,ca,FALSE); break;
	case SwitchStmtOp   : GenSwitch(root,ca);           break;
	case BreakStmtOp    : GenBreak(root,ca);            break;
	case ContinueStmtOp : GenContinue(root,ca);         break;
	case ReturnStmtOp   : GenReturn(root,ca);           break;
	case SyncStmtOp     : GenSynchronized(root,ca);     break;
	case ThrowStmtOp    : GenThrow(root,ca);            break;
	case TryStmtOp      : GenTry(root,ca);              break;
	case LabelStmtOp    : GenLabeledStmt(root,ca);      break;
	}
    }
}

/*
 * CompileUnit::GenBlock
 *
 * Genera un blocco di codice.
 */

void CompileUnit::GenBlock(TreeNode *root, Code_Attribute *ca)
{
  stk->Dup();

  /*
  String NameBlock;

  NameBlock="";
  
  brk_rec *brec=new brk_rec;
  brec->label=NameBlock;
  brec->bplist=NULL;
  
  con_rec *crec=new con_rec;
  crec->label=NameBlock;
  crec->index=pc_count;
  
  s_brk->Push(brec);
  s_con->Push(crec);
  */
  if (((EXPNode *)root->GetRightC())->GetNodeOp()==BlockOp)
    GenBlock(root->GetRightC(),ca);
  else
    GenStmts(root->GetRightC(),ca);
  /*
  backpatch(ca, s_brk->Top()->bplist,pc_count);
  */

  /*
   * NON GESTITO!!!
   *
   * Devo distruggere i top dei 3 stack.
   */
 
  stk->Pop();
  /* s_brk->Pop();
  s_con->Pop();*/
}

/*
 * CompileUnit::GenLocalVarDecl
 *
 * Trovare la dichiarazione di una variabile locale, non comporta la genera-
 * zione di un particolare codice, bensi' si riserva solo uno spazio alla
 * variabile locale sullo stack frame.
 */

void CompileUnit::GenLocalVarDecl(TreeNode *root, Code_Attribute *ca)
{
  if (!root->IsDummy())
    {
      GenLocalVarDecl(root->GetLeftC(),ca);

      /*
       * Assegno alla variabile locale corrente un posto nello stack frame.
       * In futuro, quando sara' disponibile la rimozione di codice morto,
       * tale assegnazione di posto viene fatta solo se la variabile e'
       * effettivamente utilizzata.
       */

      STNode *Id=((IDNode *)root->GetRightC()->GetLeftC())->GetIden();
      Descriptor descriptor;
      _u1 local;
      int offset;
      
      descriptor=Id->getdescriptor();
      
      if (descriptor.is_reference()) 
	{
	  local=stk->Top()->inc(1);
	  offset=4;
	}
      else
	if (descriptor.is_integral() || descriptor==DES_BOOLEAN)
	  if (descriptor==DES_LONG)
	    {
	      local=stk->Top()->inc(2);
	      local--;
	      offset=1;
	    }
	  else
	    {
	      local=stk->Top()->inc(1);
	      offset=0;
	    }
	else
	  if (descriptor==DES_FLOAT)
	    {
	      local=stk->Top()->inc(1);
	      offset=2;
	    }
	  else
	    {
	      local=stk->Top()->inc(2);
	      local--;
	      offset=3;
	    }
      
      Id->setlocalindex(local);

      if (!root->GetRightC()->GetRightC()->IsDummy())
	{
	  /*
	   * A questo punto, lo spazio e' stato riservato. Ora se esiste una
	   * espressione associata alla dichiarazione, bisognera' generare 
	   * quest'ultima e il risultato bisognera' inserirlo nella variabile 
	   * locale con una operazione di store.
	   */

	  GenExpr(root->GetRightC()->GetRightC(),ca);
	  
	  switch (local)
	    {
	    case 0:
	    case 1:
	    case 2:
	    case 3:

	      /*
	       * Sfrutto una proprieta' semplice di sequenzialita' dei codici
	       * operativi per scegliere l'operatore di store in base a local
	       * e al descrittore.
	       */

	      ca->Gen(INIT_STORE+offset*4+local);
	      break;

	    default:

	      /*
	       * Sfrutto una proprieta' semplice di sequenzialita' dei codici
	       * operativi per scegliere l'operatore di store in base a local
	       * e al descrittore.
	       */

	      ca->Gen(INIT_STORE_INDEX+offset,local);
	      break;	      
	    }
	}
    }
}

/*
 * CompileUnit::GenIf
 *
 * Genera le istruzioni if..then..else o if..then. A seconda che la parte else
 * esista, il metodo corrente, invochera': GenIfThenElse oppure GenIfThen
 */

void CompileUnit::GenIf(TreeNode *root, Code_Attribute *ca)
{
  if (root->GetRightC()->IsDummy())
    GenIfThen(root,ca);
  else
    GenIfThenElse(root,ca);
}

/*
 * CompileUnit::GenIfThenElse
 *
 * Generiamo l'istruzione if..then..else. Per tale istruzione noi negheremo
 * l'espressione cosi' da eseguire l'else se l'espressione negata e' vera, il
 * then in caso constrario. Anche Java Sun lavora con quest'ottica. l'espres-
 * sione dopo che e' stata generata puo' avere una truelist e una falselist,
 * di cui bisognera' poi fare il backpatching.
 */

void CompileUnit::GenIfThenElse(TreeNode *root, Code_Attribute *ca)
{
  BPList *truelist, *falselist;

  TreeNode *expr=new EXPNode(LogicalNotOp,Dummy,root->GetLeftC()->GetLeftC());
  TreeNode *s1=root->GetLeftC()->GetRightC();
  TreeNode *s2=root->GetRightC();
  root->GetLeftC()->SetLeftC(expr);

  GenExpr(expr,ca);

  /*
   * Se la truelist e la falselist sono entrambe NULL, significa che sull'ope-
   * rand-stack avremo gia' un valore logico 1 o 0 da valutare.
   */

  _u4 label_1, label_2, dest_1, dest_2;

  if (!expr->GetTrueList() && !expr->GetFalseList())
    {
      /*
       * ATTENZIONE!!!
       *
       * Penso che questo pezzo di codice verra' eseguito solo nel caso in
       * cui l'espressione e' un valore booleano (true o false). Quando in 
       * futuro, a tempo di compilazione, il constant folding funzionera'
       * per qualsiasi caso, allora gestiremo l'if affinche' a tempo di
       * compilazione vega direttamente selezionata la sezione THEN e la
       * sezione ELSE.
       */

      label_1=pc_count;

      ca->Gen(IFEQ,(_u2) 0);

      if (((EXPNode *)s2)->GetNodeOp()==BlockOp)
	GenBlock(s2,ca);
      else
	GenStmt(s2,ca);

      label_2=pc_count;

      ca->Gen(GOTO,(_u2) 0);

      dest_1=pc_count;

      if (((EXPNode *)s1)->GetNodeOp()==BlockOp)
	GenBlock(s1,ca);
      else
	GenStmts(s1,ca);

      ca->Gen_to(label_1,IFEQ,dest_1);
      ca->Gen_to(label_2,GOTO,pc_count);
    }
  else
    {
      dest_1=pc_count;

      if (((EXPNode *)s1)->GetNodeOp()==BlockOp)
	GenBlock(s1,ca);
      else
	GenStmts(s1,ca);

      label_1=pc_count;

      ca->Gen(GOTO,(_u2) 0);

      dest_2=pc_count;

      if (((EXPNode *)s2)->GetNodeOp()==BlockOp)
	GenBlock(s2,ca);
      else
	GenStmts(s2,ca);

      ca->Gen_to(label_1,GOTO,(_u2) pc_count);

      backpatch(ca,expr->GetTrueList(),dest_2);
      backpatch(ca,expr->GetFalseList(),dest_1);

      delete expr->GetTrueList();
      delete expr->GetFalseList();

      expr->SetTrueList(NULL);
      expr->SetFalseList(NULL);
    }
}

/*
 * CompileUnit::GenIfThen
 *
 * Generiamo l'istruzione if..then. La gestione e' analoga a quella vista 
 * sopra.
 */

void CompileUnit::GenIfThen(TreeNode *root, Code_Attribute *ca)
{
  BPList *truelist, *falselist;

  TreeNode *expr=new EXPNode(LogicalNotOp,Dummy,root->GetLeftC()->GetLeftC());
  TreeNode *s1=root->GetLeftC()->GetRightC();
  root->GetLeftC()->SetLeftC(expr);

  GenExpr(expr,ca);

  /*
   * Se la truelist o la falselist sono NULL, significa che sull'operand-stack
   * avremo gia' un valore logico 1 o 0 da valutare.
   */

  _u4 label_1, dest_1, dest_2;

  if (!expr->GetTrueList() && !expr->GetFalseList())
    {
      label_1=pc_count;

      ca->Gen(IFNE,(_u2) 0);

      dest_1=pc_count;

      if (((EXPNode *)s1)->GetNodeOp()==BlockOp)
	GenBlock(s1,ca);
      else
	GenStmts(s1,ca);

      ca->Gen_to(label_1,IFNE,(_u2) pc_count);
    }
  else
    {
      dest_1=pc_count;

      if (((EXPNode *)s1)->GetNodeOp()==BlockOp)
	GenBlock(s1,ca);
      else
	GenStmts(s1,ca);

      dest_2=pc_count;

      backpatch(ca,expr->GetTrueList(),dest_2);
      backpatch(ca,expr->GetFalseList(),dest_1);

      delete expr->GetTrueList();
      delete expr->GetFalseList();

      expr->SetTrueList(NULL);
      expr->SetFalseList(NULL);
    }
}
 
/*
 * CompileUnit::GenLoopOp
 *
 * Generiamo le istruzioni di ciclo: while, do e for
 */

void CompileUnit::GenLoop(TreeNode *root, Code_Attribute *ca)
{
  _u4 offset=pc_count;

  String NameBlock;

  NameBlock="";
  
  brk_rec *brec=new brk_rec;
  brec->label=NameBlock;
  brec->bplist=NULL;
  
  con_rec *crec=new con_rec;
  crec->label=NameBlock;
  crec->index=pc_count;
  
  s_brk->Push(brec);
  s_con->Push(crec);
  
  if (((EXPNode *)root->GetLeftC())->GetNodeOp()==CommaOp)
    GenFor(root,ca);
  else
    if (((EXPNode *)root->GetRightC())->GetNodeOp()==StmtOp  ||
	((EXPNode *)root->GetRightC())->GetNodeOp()==BlockOp)
      GenWhile(root,ca);
    else
      GenDoWhile(root,ca);
  
  backpatch(ca, s_brk->Top()->bplist,pc_count);
  s_brk->Pop();
  s_con->Pop();
  
}

/*
 * CompileUnit::GenWhile
 *
 * Generiamo il codice JVM per l'istruzione while. La gestione, in un certo
 * senso, e' analoga a quella dell' if.
 */

void CompileUnit::GenWhile(TreeNode *root, Code_Attribute *ca)
{
  _u2 dest;
  _u4 label;

  EXPNode *expr=(EXPNode *)root->GetLeftC();
  EXPNode *stmt=(EXPNode *)root->GetRightC();

  label=pc_count;

  ca->Gen(GOTO,(_u2) 0);

  dest=pc_count;

  if (stmt->GetNodeOp()==StmtOp)
    GenStmts(stmt,ca);
  else
    GenBlock(stmt,ca);

  ca->Gen_to(label,GOTO,pc_count);

  GenExpr(expr,ca);

  if (!expr->GetTrueList() && !expr->GetFalseList())
    ca->Gen(IFNE,(_u2) (dest-pc_count));
  else
    {
      backpatch(ca,expr->GetTrueList(),dest);
      backpatch(ca,expr->GetFalseList(),pc_count);

      delete expr->GetTrueList();
      delete expr->GetFalseList();

      expr->SetTrueList(NULL);
      expr->SetFalseList(NULL);
    }

}

/*
 * CompileUnit::GenDoWhile
 *
 * Genera l'istruzione do..while. La gestione e' analoga a quella del while.
 */

void CompileUnit::GenDoWhile(TreeNode *root, Code_Attribute *ca)
{
  _u2 dest;

  TreeNode *stmt=(EXPNode *)root->GetLeftC();
  TreeNode *expr=(EXPNode *)root->GetRightC();

  dest=pc_count;

  if (((EXPNode *)stmt)->GetNodeOp()==StmtOp)
    GenStmt(stmt,ca);
  else
    GenBlock(stmt,ca);

  GenExpr(expr,ca);

  if (!expr->GetTrueList() && !expr->GetFalseList())
    ca->Gen(IFNE,(_u2) (dest-pc_count));
  else
    {
      backpatch(ca,expr->GetTrueList(),dest);
      backpatch(ca,expr->GetFalseList(),pc_count);

      delete expr->GetTrueList();
      delete expr->GetFalseList();

      expr->SetTrueList(NULL);
      expr->SetFalseList(NULL);
    }
}

/*
 * CompileUnit::GenFor
 *
 * Genero l'istruzione for.
 */

void CompileUnit::GenFor(TreeNode *root, Code_Attribute *ca)
{
  _u2 dest;
  _u4 label;

  TreeNode *init  =root->GetLeftC()->GetLeftC();
  TreeNode *expr  =root->GetLeftC()->GetRightC();
  TreeNode *stmts =root->GetRightC()->GetLeftC();
  TreeNode *update=root->GetRightC()->GetRightC();

  GenStmts(init,ca);

  label=pc_count;

  ca->Gen(GOTO,(_u2) 0);

  dest=pc_count;

  if (((EXPNode *)stmts)->GetNodeOp()==StmtOp)
    GenStmts(stmts,ca);
  else
    GenBlock(stmts,ca);

  GenStmts(update,ca);

  ca->Gen_to(label,GOTO,pc_count);

  GenExpr(expr,ca);
  
  if (!expr->GetTrueList() && !expr->GetFalseList())
    ca->Gen(IFNE,(_u2) (dest-pc_count));
  else
    {
      backpatch(ca,expr->GetTrueList(),dest);
      backpatch(ca,expr->GetFalseList(),pc_count);

      delete expr->GetTrueList();
      delete expr->GetFalseList();

      expr->SetTrueList(NULL);
      expr->SetFalseList(NULL);
    }  
}

/*
 * CompileUnit::GenBreak
 *
 * Genera l'istruzione break con o senza etichetta. Per implementare l'istru-
 * zione break, utilizzeremo uno stack che ad ogni apertura di blocco
 * memorizza il nome di quest'ultimo e la lista di backpatching delle istru-
 * zioni break che ad esso fanno riferimento.
 */

void CompileUnit::GenBreak(TreeNode *root, Code_Attribute *ca)
{
  if (root->GetRightC()->IsDummy())
    {
      /*
       * L'istruzione break e' senza etichetta.
       */
      
      if (s_brk->Top()->bplist)
	s_brk->Top()->bplist->Add(pc_count);
      else
	{
	  s_brk->Top()->bplist=new BPList();
	  s_brk->Top()->bplist->Add(pc_count);
	}
    }
  else
    {
      /*
       * L'istruzione break e' con etichetta.
       */

      String LabelName;

      LabelName=((IDNode *)root->GetRightC())->GetIden()->getname();

      for (int i=s_brk->StackSize()-1; i >=0; i--)
	if ((*s_brk)[i]->label==LabelName)
	  if ((*s_brk)[i]->bplist)
	    (*s_brk)[i]->bplist->Add(pc_count);
	  else
	    {
	      (*s_brk)[i]->bplist=new BPList();
	      (*s_brk)[i]->bplist->Add(pc_count);
	    }
    }

  ca->Gen(GOTO,(_u2)0);
}

/*
 * CompileUnit::GenContinue
 *
 * Genera l'istruzione continue con o senza etichetta. La gestione e' simile a 
 * quella dell'istruzione break.
 */

void CompileUnit::GenContinue(TreeNode *root, Code_Attribute *ca)
{
  if (root->GetRightC()->IsDummy())
    {
      /*
       * L'istruzione continue e' senza etichetta.
       */
      
      ca->Gen(GOTO,(_u2)(s_con->Top()->index-pc_count));
    }
  else
    {
      /*
       * L'istruzione continue e' con etichetta.
       */

      String LabelName;

      LabelName=((IDNode *)root->GetRightC())->GetIden()->getname();

      for (int i=s_con->StackSize()-1; i>=0; i--)
	if ((*s_con)[i]->label==LabelName)
	  ca->Gen(GOTO,(_u2)((*s_con)[i]->index-pc_count));
    }
}

/*
 * CompileUnit::GenReturn
 *
 * Genera l'istruzione return.
 */

void CompileUnit::GenReturn(TreeNode *root, Code_Attribute *ca)
{
  _u4 dest_1, dest_2;

  EXPNode *expr=(EXPNode *)root->GetRightC();
  Descriptor des;

  if (expr->IsDummy())
    ca->Gen(RETURN);
  else
    {
      des=expr->GetDescriptor();
      GenExpr(expr,ca);
      
      if (des==DES_BOOLEAN && (expr->GetTrueList() || expr->GetFalseList()))
	{
	  dest_1=pc_count;
	  
	  ca->Gen(ICONST_0);
	  ca->Gen(GOTO,(_u2) 4);
	  dest_2=pc_count;
	  ca->Gen(ICONST_1);
	  backpatch(ca,expr->GetFalseList(),dest_1);
	  backpatch(ca,expr->GetTrueList(),dest_2);

	  delete expr->GetTrueList();
	  delete expr->GetFalseList();
	  
	  expr->SetTrueList(NULL);
	  expr->SetFalseList(NULL);
	}


      if (des.is_reference())
	ca->Gen(ARETURN);
      else
	if ((des.is_integral() && des!=DES_LONG) || des==DES_BOOLEAN)
	  ca->Gen(IRETURN);
	else
	  if (des==DES_LONG)
	    ca->Gen(LRETURN);
	  else
	    if (des==DES_FLOAT)
	      ca->Gen(FRETURN);
	    else
	      ca->Gen(DRETURN);
    }

  not_return=FALSE;

}

/*
 * CompileUnit::GenSynchronized
 *
 * Genero l'istruzione synchronized. Quest'istruzione e' abbastanza semplice, 
 * infatti i passi da seguire sono i seguenti:
 *
 * - genero l'espressione grazie alla quale avro' un rif. a oggetto sul top
 *   dell'operand stack;
 * - genero MONITORENTER;
 * - genero il blocco di istruzioni riservate a un thread;
 * - genero MONITOREXIT;
 */

void CompileUnit::GenSynchronized(TreeNode *root, Code_Attribute *ca)
{
  EXPNode *stmts=(EXPNode *)root->GetRightC();

  GenExpr(root->GetLeftC(),ca);
  ca->Gen(MONITORENTER);
  
  if (stmts->GetNodeOp()==StmtOp)
    GenStmt(stmts,ca);
  else
    GenBlock(stmts,ca);

  ca->Gen(MONITOREXIT);
}

/*
 * CompileUnit::GenThrow
 *
 * Istruzione throw. Anche la gestione di quest'istruzione e' molto semplice.
 */

void CompileUnit::GenThrow(TreeNode *root, Code_Attribute *ca)
{
  GenExpr(root->GetLeftC(),ca);
  ca->Gen(ATHROW);
}

/*
 * CompileUnit::GenTry
 *
 * Genero l'istruzione try..catch..finally.
 */

void CompileUnit::GenTry(TreeNode *root, Code_Attribute *ca)
{
  _u2 start_pc, end_pc;

  TreeNode *stmts=root->GetLeftC();

  int finally=FALSE;
  int catches=FALSE;

  BPList *list_jsr=new BPList();
  BPList *list_goto=new BPList();
  BPList *list_exit=new BPList();

  start_pc=pc_count;

  /*
   * Controllo se c'e' nell'istruzione try la clausola finally e almeno una
   * clausola catch. In caso affermativo, attivo i rispettivi flag finally e
   * catches.
   */

  if (root->GetRightC()->GetRightC()->GetLeftC()->IsDummy())
    finally=TRUE;

  if (root->GetRightC()->GetLeftC()->IsDummy())
    catches=TRUE;

  _u1 finally_index;
  _u1 any_index;

  /*
   * Riserviamo un posto sullo stack frame a due variabili locali fittizie, 
   * utilizzate per la gestione della clasola finally.
   */
  
  if (root->GetRightC()->GetRightC()->GetLeftC()->IsDummy())
    {
      any_index=stk->Top()->inc(1);
      finally_index=stk->Top()->inc(1);
    }

  if (((EXPNode *)stmts)->GetNodeOp()==BlockOp)
    GenBlock(stmts, ca);
  else
    GenStmts(stmts, ca);

  if (finally && catches)
    {
      list_goto->Add(pc_count);
      end_pc=pc_count;
      ca->Gen(GOTO,(_u2) 0);
    }
  else
    if (finally)
      {
	list_jsr->Add(pc_count);
	ca->Gen(JSR,(_u2) 0);
	list_exit->Add(pc_count);
	end_pc=pc_count;
	ca->Gen(GOTO, (_u2) 0);
      }
    else
      {
	list_exit->Add(pc_count);
	end_pc=pc_count;
	ca->Gen(GOTO, (_u2) 0);
      }

  GenCatchFinally(root->GetRightC(), ca, catches, finally, any_index, 
		  finally_index, list_jsr, list_goto, list_exit);

  /*
   * Per ogni entry della exceptions table, imposto i valori di start_pc e 
   * end_pc. Per la clausola finally, imposto solo start_pc.
   */

  backpatch(ca, list_exit, pc_count);

  ca->SetExceptionTableLength();
  _u2 exceptiontablelength=ca->GetExceptionTableLength();
  Exception_Parameter *ep;

  for (int i=0; i < exceptiontablelength; i++)
    {
      ep=ca->GetExceptionParameter(i);
      ep->SetStartPc(start_pc);
      if (ep->GetCatchType()!=0)
	ep->SetEndPc(end_pc);
      ca->SetExceptionParameter(ep,i);
    }
}

void CompileUnit::GenCatchFinally(TreeNode *root, Code_Attribute *ca, 
				  int catches, int finally,
				  _u1 any_index, _u1 finally_index,
				  BPList *list_goto, BPList *list_jsr,
				  BPList *list_exit)
{
  if (!root->IsDummy())
    {
      GenCatchFinally(root->GetLeftC(),ca, catches, finally, any_index,
		      finally_index, list_goto, list_jsr, list_exit);

      if (!root->GetRightC()->GetLeftC()->IsDummy())
	{

	  /*
	   * Stiamo processando una clausola catch, per cui i passi che segui-
	   * ranno, sono:
	   *
	   * a) alloco un'entry nella tabella exception_table;
	   * b) imposto l'indirizzo di handler_pc;
	   * c) imposto la classe da catturare;
	   * d) carico il valore del parametro sull'operand stack;
	   * e) genero il blocco codice allegato;
	   * f) genero un exit se non c'e' finally, altrimenti un salto 
	   *    all'ultimo catch, dove ci sara' un'istruzione JSR seguita da 
	   *    exit.
	   */
	  
	  STNode *Id=((IDNode *)root->GetRightC()->GetLeftC())->GetIden();
	  String Name;
	  
	  Exception_Parameter *ep = new Exception_Parameter();
	  
	  Name=Id->getdescriptor().to_fullname();
	  ep->SetHandlerPc(pc_count);
	  ep->SetCatchType(Class->Load_Constant_Class(Name));

	  // ca->Gen(POP);

	  _u1 index=stk->Top()->inc(1);

	  ((IDNode *)
	   root->GetRightC()->GetLeftC())->GetIden()->setlocalindex(index);

	  /*
	   * L'eccezione lanciata va caricata nel parametro della clausola
	   * catch. Java Sun esegue questa operazione solo se il parametro
	   * e' poi realmente utilizzato, altrimenti viene generato una istru-
	   * zione POP al suo posto.
	   */

	  switch (index)
	    {
	    case 0 : 
	    case 1 :
	    case 2 :
	    case 3 : ca->Gen(ASTORE_0+index); break;
	    default: ca->Gen(ASTORE,index);   break;
	    }

	  GenBlock(root->GetRightC()->GetRightC(),ca);
	  stk->Top()->dec(1);

	  if (finally && catches)
	    {
	      list_goto->Add(pc_count);
	      ca->Gen(GOTO,(_u2) 0);
	    }
	  else
	    if (catches)
	      {
		list_exit->Add(pc_count);
		ca->Gen(GOTO,(_u2) 0);
	      }

	  ca->AddException(ep);
	}
      else
	{
	  /*
	   * Stiamo processando una clausola finally, per cui i passi che se-
	   * guiranno, sono:
	   *
	   * a) alloco un'entry nella tabella exception_table;
	   * b) imposto l'indirizzo di handler_pc pari a pc_count;
	   * d) imposto la classe da catturare a 0 (any class);
	   * e) genero
	   *         ASTORE any_index
	   *         JSR <inizio finally block>
	   *         ALOAD any_index
	   *         ATHROW
	   *         
	   * f) genero il blocco codice allegato;
	   */

	  Exception_Parameter *ep= new Exception_Parameter();
	  
	  ep->SetHandlerPc(pc_count);
	  ep->SetCatchType(0);
	  ep->SetEndPc(pc_count);

	  switch (any_index)
	    {
	    case 0 :
	    case 1 :
	    case 2 :
	    case 3 : ca->Gen(ASTORE_0+any_index); break;
	    default: ca->Gen(ASTORE, any_index);
	    }

	  list_jsr->Add(pc_count);
	  backpatch(ca, list_goto, pc_count);

	  ca->Gen(JSR, (_u2) 0);

	  switch (any_index)
	    {
	    case 0 :
	    case 1 :
	    case 2 :
	    case 3 : ca->Gen(ALOAD_0+any_index); break;
	    default: ca->Gen(ALOAD, any_index);
	    }

	  ca->Gen(ATHROW);

	  backpatch(ca, list_jsr, pc_count);

	  switch (finally_index)
	    {
	    case 0 :
	    case 1 :
	    case 2 :
	    case 3 : ca->Gen(ASTORE_0+finally_index); break;
	    default: ca->Gen(ASTORE, finally_index);
	    }

	  GenBlock(root->GetRightC()->GetRightC(), ca);

	  ca->Gen(RET,finally_index);

	  ca->AddException(ep);
	}
    }
} 


/*
 * CompileUnit::GenLabeledStmt
 *
 * Genero un blocco di istruzioni etichettato. In effetti questo metodo
 * lavora come GenBlock, la differenza sta nel modo con cui vengono gestiti
 * i due stack: s_brk e s_con;
 */

void CompileUnit::GenLabeledStmt(TreeNode *root, Code_Attribute *ca)
{
  stk->Dup();

  String NameBlock;

  NameBlock=((IDNode *)root->GetLeftC())->GetIden()->getname();
  
  brk_rec *brec=new brk_rec;
  brec->label=NameBlock;
  brec->bplist=NULL;
  
  con_rec *crec=new con_rec;
  crec->label=NameBlock;
  
  s_brk->Push(brec);
  s_con->Push(crec);

  if (((EXPNode *)root->GetRightC())->GetNodeOp()==BlockOp)
    GenBlock(root->GetRightC(),ca);
  else
    GenStmts(root->GetRightC(),ca);
  
  /*
   * NON GESTITO!!!
   *
   * Devo distruggere i top dei 3 stack.
   */

  stk->Pop();
  s_brk->Pop();
  s_con->Pop();
}

/*
 * CompileUnit::GenSwitch
 *
 * Genera l'istruzione switch. La Java Virtual Machine utilizza due istruzioni
 * macchina per la gestione dello switch: LOOKUPSWITCH e TABLESWITCH. La dif-
 * ferenza tra di esse e' che la prima e' utilizzata quando le label hanno
 * ordine sparso, mentre la seconda quando le label sono consecutive.
 * Per ulteriori dettagli, fare riferimento al libro "The Java Virtual Machine
 * Specification" di Lindholm-Yellin.
 *
 * Nota:
 *
 * la funzione cmp_sort e' utilizzata da qsort all'interno del metodo, mentre
 * cmp_search e' utilizzata da bsearch.
 */

int cmp_sort(switch_rec *a, switch_rec* b)
{
  return (a->label > b->label);
}

int cmp_search(switch_rec *a, switch_rec* b)
{
  return a->label > b->label ? +1 : a->label < b->label ? -1 : 0;
}

void CompileUnit::GenSwitch(TreeNode *root, Code_Attribute *ca)
{
  switch_rec *default_rec=NULL;
  int i, index=0, label, label_default, dest;
  _u4 offset, num_label=0;

  String NameBlock;

  NameBlock="";
  
  brk_rec *brec=new brk_rec;
  brec->label=NameBlock;
  brec->bplist=NULL;
  
  s_brk->Push(brec);

  GenExpr(root->GetLeftC(),ca);

  /*
   * Calcoliamo innanzittutto il numero di label presenti all'interno di una
   * istruzione switch.
   */
  
  for (TreeNode *t1=root->GetRightC(); !t1->IsDummy(); t1=t1->GetLeftC())
    for (TreeNode *t2=t1->GetRightC()->GetLeftC(); !t2->IsDummy(); 
	 t2=t2->GetLeftC())
      if (!t2->GetRightC()->IsDummy())
	num_label++;

  /*
   * Allochiamo una tabella di num_label elementi.
   */

  table_switch=new switch_rec[num_label];

  /*
   * Assegniamo ad ogni entry della tabella una label e ordiniamola in base ad
   * essa.
   */

  for (TreeNode *t1=root->GetRightC();!t1->IsDummy();t1=t1->GetLeftC())
    for (TreeNode *t2=t1->GetRightC()->GetLeftC(); !t2->IsDummy(); 
	 t2=t2->GetLeftC(), index++)
      if (!t2->GetRightC()->IsDummy())
	{
	  table_switch[index].label=((INUMNode *)t2->GetRightC())->GetVal();
	  table_switch[index].is_default=FALSE;
	}

  qsort(table_switch,num_label,sizeof(switch_rec),cmp_sort);

  /*
   * Controlliamo se le label sono consecutive, in caso affermativo, utilizzare
   * l'istruzione TABLESWITCH, altrimenti LOOKUPSWITCH.
   */

  int j=0;

  for (int l=table_switch[0].label; j<num_label; j++)
    if (!table_switch[j].is_default)
      if (l++!=table_switch[j].label)
	break;

  if (j < num_label)
    {
      /*
       * Le label non sono consecutive, bisogna utilizzare l'istruzione
       * LOOKUPSWITCH
       */

      offset=pc_count;

      ca->Gen(LOOKUPSWITCH);

      int padds=pc_count%4==0 ? pc_count%4 : 4-pc_count%4;

      for (int k=0; k<padds; k++) ca->Gen(0);

      /*
       * Lasciamo lo spazio necessario per memorizzare le coppie match-offset e
       * per default.
       */
      
      label=pc_count;

      for (i=0; i<num_label*8+8; i++) ca->Gen(0);
      
      /*
       * Generiamo ora i blocchi di codice.
       */

      for (TreeNode *t1=root->GetRightC();!t1->IsDummy();t1=t1->GetLeftC())
	{
	  EXPNode *stmts=(EXPNode *)t1->GetRightC()->GetRightC();
	  dest=pc_count;
	  
	  if (stmts->GetNodeOp()==BlockOp)
	    GenBlock(stmts,ca);
	  else
	    GenStmts(stmts,ca);
	  
	  for (TreeNode *t2=t1->GetRightC()->GetLeftC(); !t2->IsDummy(); 
	       t2=t2->GetLeftC())
	    {
	      switch_rec *rec;

	      if (t2->GetRightC()->IsDummy())
		{
		  default_rec=new switch_rec;
		  default_rec->jump=dest;
		}
	      else
		{
		  switch_rec dummy;

		  dummy.label=((INUMNode *)t2->GetRightC())->GetVal();
		  rec=(switch_rec*)bsearch(&dummy,table_switch,num_label,
					   sizeof(switch_rec),cmp_search);
		  rec->jump=dest;
		}
	    }
	}
      
      /*
       * Generiamo ora le coppie match-offset.
       */

      if (default_rec)
	{
	  ca->Gen_to(label,default_rec->jump-offset);
	  ca->Gen_to(label+4,num_label);
	}
      else
	{
	  ca->Gen_to(label,pc_count-offset);
	  ca->Gen_to(label+4,num_label);
	}

      label+=8;

      for (j=0; j < num_label; j++)
	{
	  ca->Gen_to(label,table_switch[j].label);
	  ca->Gen_to(label+4,table_switch[j].jump-offset);
	  label+=8;
	}
    }
  else
    {
      /*
       * Le label sono consecutive, bisogna utilizzare l'istruzione TABLESWITCH
       */

      _u4 high, low;

      offset=pc_count;

      ca->Gen(TABLESWITCH);

      int padds=pc_count%4==0 ? pc_count%4 : 4-pc_count%4;

      for (int k=0; k<padds; k++) ca->Gen(0);

      /*
       * Bisogna lasciare 4 byte di spazio per il jump di default.
       */

      label_default=pc_count;

      ca->Gen(0);
      ca->Gen(0);
      ca->Gen(0);
      ca->Gen(0);

      low=table_switch[0].label;
      high=table_switch[num_label-1].label;

      ca->Gen(low >> 24);                     // limite inferiore.
      ca->Gen((low >> 16) & 0x000000FF);
      ca->Gen((low >> 8) & 0x000000FF);
      ca->Gen(low & 0x000000FF);

      ca->Gen(high >> 24);                    // limite superiore. 
      ca->Gen((high >> 16) & 0x000000FF);
      ca->Gen((high >> 8) & 0x000000FF);
      ca->Gen(high & 0x000000FF);

      /*
       * Bisogna lasciare (high-low+1)*4 byte di spazio per il jump delle 
       * label.
       */

      label=pc_count;

      for (int k=0; k<(high-low+1)*4; k++) ca->Gen(0);

      /*
       * Generiamo ora i blocchi codice.
       */

      for (TreeNode *t1=root->GetRightC();!t1->IsDummy();t1=t1->GetLeftC())
	{
	  EXPNode *stmts=(EXPNode *)t1->GetRightC()->GetRightC();
	  dest=pc_count;
	  
	  if (stmts->GetNodeOp()==BlockOp)
	    GenBlock(stmts,ca);
	  else
	    GenStmts(stmts,ca);
	  
	  for (TreeNode *t2=t1->GetRightC()->GetLeftC(); !t2->IsDummy(); 
	       t2=t2->GetLeftC())
	    {
	      switch_rec *rec;

	      if (t2->GetRightC()->IsDummy())
		{
		  default_rec=new switch_rec;
		  default_rec->jump=dest;
		}
	      else
		{
		  switch_rec dummy;

		  dummy.label=((INUMNode *)t2->GetRightC())->GetVal();
		  rec=(switch_rec *)bsearch(&dummy,table_switch,num_label,
					    sizeof(switch_rec),cmp_search);
		  rec->jump=dest;
		}
	    }
	}
      
      /*
       * Ora bisogna generare i jump alle istruzioni appena generate.
       */

      if (default_rec)
	ca->Gen_to(label_default,default_rec->jump-offset);
      else
	ca->Gen_to(label_default,pc_count-offset);
      
      for (i=0; i < num_label; i++)
	{
	  ca->Gen_to(label,table_switch[i].jump-offset);
	  label+=4;
	}
    }

  backpatch(ca, s_brk->Top()->bplist,pc_count);

  s_brk->Pop();
  s_con->Pop();

}

/*
 * CompileUnit::GenIncDecStmt
 *
 * Tratta il pre-incremento e pre-decremento come istruzioni.
 */

void CompileUnit::GenIncDecStmt(TreeNode *root, Code_Attribute *ca)
{
  TreeNode *Exp0;
  STNode *variable;
  int op=((EXPNode *)root)->GetNodeOp();
  Descriptor DesExp0;

  if (root->GetLeftC()->IsDummy())
    Exp0=root->GetRightC();
  else
    Exp0=root->GetLeftC();

  DesExp0=Exp0->GetDescriptor();

  if (Exp0->is_IDNode())
    variable=((IDNode *) Exp0)->GetIden();

  if (Exp0->is_IDNode() && variable->is_local() && DesExp0==DES_INT)
    if (op==PlusPlusOp)
      ca->Gen(IINC,variable->getlocalindex(),(char)1);
    else
      ca->Gen(IINC,variable->getlocalindex(),(char)-1);
  else
    {
      /*
       * La variabile da incrementare, puo' essere un campo, una variabile 
       * locale o un accesso a un componente di un array.
       */

      if (Exp0->is_EXPNode() && ((EXPNode *)Exp0)->GetNodeOp()==FieldAccessOp)
	GenFieldAccess(Exp0,ca,TRUE);
      else
	GenUnaryExpr(Exp0,ca);

      if (DesExp0==DES_INT)
	if (op==PlusPlusOp)
	  {
	    ca->Gen(ICONST_1);
	    ca->Gen(IADD);
	  }
	else
	  {
	    ca->Gen(ICONST_1);
	    ca->Gen(ISUB);
	  }
      else
	if (DesExp0==DES_LONG)
	  if (op==PlusPlusOp)
	    {
	      ca->Gen(LCONST_1);
	      ca->Gen(LADD);
	    }
	  else
	    {
	      ca->Gen(LCONST_1);
	      ca->Gen(LSUB);
	    }
	else
	  if (DesExp0==DES_FLOAT)
	    if (op==PlusPlusOp)
	      {
		ca->Gen(FCONST_1);
		ca->Gen(FADD);
	      }
	    else
	      {
		ca->Gen(FCONST_1);
		ca->Gen(FSUB);
	      }      
	  else
	    if (DesExp0==DES_DOUBLE)
	      if (op==PlusPlusOp)
		{
		  ca->Gen(DCONST_1);
		  ca->Gen(DADD);
		}
	      else
		{
		  ca->Gen(DCONST_1);
		  ca->Gen(DSUB);
		}
      GenLValue(Exp0,ca);
    }
}
