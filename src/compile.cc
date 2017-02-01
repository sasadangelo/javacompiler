/*
 * file compile.cc
 * 
 * descrizione: questo file implementa la classe fondamentale dell'intero
 *              compilatore, ossia la classe in cui saranno memorizzate
 *              tutte le informazioni inerenti a una compilation unit:
 *              parse-tree, symbol-table, classpath, etc.
 *
 * Questo file e' parte del Compilatore Java.
 *
 * Maggio 1997, scritto da Salvatore D'Angelo e-mail xc0261@xcom.it.
 */

#include <stdarg.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <globals.h>
#include <cstring.h>
#include <descriptor.h>
#include <hash.h>
#include <cphash.h>
#include <table.h>
#include <jvm.h>
#include <compile.h>
#include <tree.h>
#include <errors.h>
#include <err_messages.h>
#include <environment.h>
#include <backpatch.h>
#include <access.h>

#define PARSE_ERROR_MSG "parse error"

extern int yylineno;
extern int yychar;
extern TreeNode *Dummy;
extern char *msg_errors[];
extern int Nest_lev;
extern int LastIndex;
extern String DebugDirectory;
extern String ObjectName;
extern String init_name;
extern FILE *fParseTree;
extern FILE *fSTout;
extern STNode *current_class;

String CloneableName;
String clinit_name;

/*
 * Il numero di bucket della hash table e' fisso, pero' si puo' pensare di
 * migliorare la gestione utilizzando tabelle dinamiche o euristiche sul
 * file sorgente da cui estrarre la taglia giusta per i bucket.
 */

/*
 * costruttore/distruttore classe CompileUnit
 */

CompileUnit::CompileUnit(char *name_file)
{
  syntax_tree=NULL;
  table=new STable(NUM_BUCKET);
  env=new Environment;
  errors=new ListErrors;
  Class=new Class_File();

  name_source=name_file;
  package_name="";

  numlabel=0;
  sdefault=FALSE;

  intf_public=FALSE;
}

CompileUnit::~CompileUnit()
{
  delete syntax_tree;
  delete errors;
  delete table;
  delete env;
  // delete Class;
}

/*
 * Alcuni metodi per l'accesso ai campi della classe.
 */

Environment *CompileUnit::GetEnvironment()          { return env;          }
String& CompileUnit::GetPackageName()               { return package_name; }
void CompileUnit::SetPackageName(String& _name)     { package_name=_name;  }
STable *CompileUnit::GetTable()                     { return table;        } 
ListErrors *CompileUnit::GetErrors()                { return errors;       }
CPHash *CompileUnit::GetCPHash()                    { return cphash;       }
Class_File *CompileUnit::GetClass()                 { return Class;        }

Tree *CompileUnit::make_syntax_tree(TreeNode *t)
{
  return syntax_tree=new Tree(t);
}

int CompileUnit::is_in_package() { return package_name!=""; }

/*
 * CompileUnit::Parse
 *
 * Metodo che inizializza il processo di visita e analisi del parse-tree.
 */

void CompileUnit::Parse()
{
  if ((javafile=fopen(name_source.to_char(),"r+"))==NULL) 
    {
      MsgErrors(0,msg_errors[ERR_INVALID_ARG],name_source.to_char());
      yyin=stdin;
    }
  else
    {
      yyin=javafile;

      /*
       * Analisi sintattica.
       */

      if (yyparse()!=0)
	MsgErrors(yylineno,msg_errors[ERR_GENERIC],PARSE_ERROR_MSG);
      else
	{

	  /*
	   * Analisi parse-tree (analisi semantica) e eventuale generazione di
	   * codice.
	   */

	  Nest_lev=FIRST_LVL;

	  ParseTree();

	  /*
	   * Stampo il parse-tree e la symbol table dopo l'analisi.
	   */
	  
	  syntax_tree->PrintTree(fParseTree);
	  table->PrintTable(fSTout);

	  GenCode();
	}

      if (errors->GetNumErrors()>0)
	errors->Print();

      fclose(javafile);
    }
}

/*
 * CompileUnit::MsgErrors
 *
 * Invia un messaggio di errore alla lista di errori che il compilatore con-
 * serva durante la fase di compilazione, per poi visualizzarla alla fine
 * ordinata per numero di linea.
 */

#define MSG_ERROR_LENGTH  120

void CompileUnit::MsgErrors(int line, char *fmt, ...)
{
  va_list args;
  va_start(args,fmt);
  char *msg=new char[MSG_ERROR_LENGTH];

  if (current_class) current_class->set_not_generable();

  vsprintf(msg,fmt,args);
  errors->InsertMsg(line,msg);

  va_end(args);
  delete msg;
}

/*
 * CompileUnit::mask_access
 *
 * Questo metodo e' utilizzato durante la fase di analisi sintattica, per
 * combinare tra loro accessi riferiti a una stessa entita', fornendo 
 * eventuali messaggi di errore nel caso in cui un accesso possa essere
 * ripetuto piu' volte. Esempio:
 *
 *          class dino 
 *          {
 *             public public dino;  errore!!!
 *              ...
 *          }
 */

int CompileUnit::mask_access(int access1, int access2)
{
  if ((access1 & access2)!=0)
    MsgErrors(yylineno,msg_errors[ERR_REPEATED_MODIFIER]);
  else
    access1|=access2;
  if (IS_PUBLIC(access1))
    {
      if (IS_PROTECTED(access1) || (IS_PRIVATE(access1)))
	access1&=~(ACC_PRIVATE|ACC_PROTECTED);
    }
  else
    if (IS_PROTECTED(access1) && IS_PRIVATE(access1))
      access1&=~(ACC_PRIVATE);
  return access1;
}

/*
 * CompileUnit::import_on_demand
 *
 * Quando in un file .java si specifica:
 *
 *             import java.awt.*;
 *
 * si caricano a livello di scoping FIRST_LVL, i nomi delle classi del pacchet-
 * to java/awt. Nel momento in cui si utilizza un di queste classi, questa 
 * verra' effettivamente caricata.
 * Quando utilizzate questa funzione e' bene conservare il pwd corrente con
 * getwd(), per poi ripristinarlo nel momento in cui questo metodo termina.
 */

void CompileUnit::import_on_demand(String& class_path, String& package)
{
  struct stat statbuf;
  struct dirent * d;
  DIR * directory;
  String path;

  path=class_path+"/"+package;
  if ((directory=opendir(path.to_char()))==NULL)
    MsgErrors(yylineno,msg_errors[ERR_PACKAGE_NOT_FOUND],path.to_char(),
	      "import");
  else
    {
      chdir(path.to_char());
      while((d=readdir(directory))!=0)
	{
	  String s;

	  s=d->d_name;
	  stat(d->d_name,&statbuf);
	  if (!S_ISLNK(statbuf.st_mode))
	    {
	      if (s.cut(s.getlength()-5,s.getlength())==".class")
		{
		  String fname;
		  String nameclass;
		  Descriptor descriptor;

		  nameclass=s.cut(1,s.getlength()-6);
		  fname=package+"/"+nameclass;
		  descriptor.build_link(fname);

		  STNode *id=table->Install_Id(nameclass,FIRST_LVL,0,
					       descriptor);
		  id->setfullname(fname);
		}
	    }
	  else
	    if (S_ISDIR(statbuf.st_mode))
	      if (s!="." && s!=".." && d->d_ino!=0)
		import_on_demand(class_path,package+"/"+s);
	}
    }
  closedir(directory);
}

/*
 * CompileUnit::discard_parameters
 *
 * Elimina i parametri formali dalla symbol-table, ponendoli nel cestino di
 * quest'ultima.
 */

void CompileUnit::discard_parameters(TreeNode *root)
{
  for (TreeNode *t=root;!t->IsDummy();t=t->GetRightC())
    {
      STNode *n=((IDNode*)t->GetLeftC())->GetIden();
      table->Discard(n);
    }
}

/*
 * CompileUnit::discard_locals
 *
 * Scarta le variabili locali a un blocco dalla symbol-table, ponendole
 * nel cestino di quest'ultima.
 */

void CompileUnit::discard_locals(TreeNode* root)
{
  for (TreeNode *stmt=root;!stmt->IsDummy();stmt=stmt->GetLeftC())
    if (stmt->GetRightC()->is_EXPNode() &&
	((EXPNode*)stmt->GetRightC())->GetNodeOp()==LocVarDeclOp)
      for (TreeNode *t=stmt->GetRightC();!t->IsDummy();t=t->GetLeftC())
	{
	  STNode *node=((IDNode *)t->GetRightC()->GetLeftC())->GetIden();
	  table->Discard(node);
	}
}

/*
 * CompileUnit::is_Cast_Conversion
 *
 * Restituisce TRUE se e' ammessa la conversione dal descrittore source a 
 * quello dest, FALSE altrimenti.
 */

int CompileUnit::is_Cast_Conversion(Descriptor& dest, Descriptor& source)
{

  int i,j;

  int primitive_table[9][9]={/* b s i l f d c b v              */
    
                               {1,1,1,1,1,1,1,1,0}, /* byte    */
			       {1,1,1,1,1,1,1,1,0}, /* short   */
			       {1,1,1,1,1,1,1,1,0}, /* int     */
			       {0,1,1,1,1,1,1,1,0}, /* long    */
			       {1,1,1,1,1,1,1,1,0}, /* float   */
			       {1,1,1,1,1,1,1,1,0}, /* double  */
			       {1,1,1,1,1,1,1,1,0}, /* char    */
			       {1,1,1,1,1,1,1,1,0}, /* boolean */
			       {0,0,0,0,0,0,0,0,1}  /* void    */
			     };
  
  int reference_table[4][4]={{1,5,3,9},
			     {2,6,8,0},
			     {3,7,3,11},
			     {4,0,0,10}
			   };
  
  char *value[]={ DES_BYTE  ,
		  DES_SHORT ,
		  DES_INT   ,
		  DES_LONG  ,
		  DES_FLOAT ,
		  DES_DOUBLE,
		  DES_CHAR  ,
		  DES_VOID  
		  };
  
  if ((source.is_primitive() || source==DES_VOID) &&
      (dest.is_primitive()   || dest==DES_VOID))
    {      
      for (i=0; i<8; i++)
	if (source==value[i]) break;
      for (j=0; j<8; j++)
	if (source==value[j]) break;
      return primitive_table[i][j];
    }

  if (source.is_reference() && dest.is_reference())
    {
      int row, col;
      STNode *node_source, *node_dest, *node;

      if (source.is_array()) 
	  col=3;
      else
	{

	  /*
	   * Controlliamo se la classe (o interfaccia) source e' definita
	   * nel pacchetto corrente e ricerchiamo il nodo nella symbol-table.
	   */

	  if (source.to_packagename()==package_name)
	    node_source=LoadClass(source.to_singlename(),++LastIndex);
	  else
	    node_source=LoadClass(source.to_fullname(),++LastIndex);

	  if (node_source->is_class())
	    {
	      if (IS_FINAL(node_source->getaccess())) 
		col=1;
	      else
		col=0;
	    }
	  else
	    if (node_source->is_interface())
	      col=2;
	}

      if (dest.is_array())
	row=3;
      else
	{
	  if (dest.to_packagename()==package_name)
	    node_dest=LoadClass(dest.to_singlename(),++LastIndex);
	  else
	    node_dest=LoadClass(dest.to_fullname(),++LastIndex);

	  if (node_dest->is_class())
	    {
	      if (!IS_FINAL(node_dest->getaccess())) 
		row=0;
	      else
		row=1;
	    }
	  else
	    if (node_dest->is_interface())
	      row=2;
	}
      
      switch(reference_table[row][col])
	{
	case 1:

	  /*
	   * Source deve essere una sottoclasse di dest, viceversa o la stessa
	   * classe, altrimenti la conversione non e' ammessa.
	   */
 
	  if (node_source==node_dest || node_source->is_subclass(node_dest) ||
	      node_dest->is_subclass(node_source))
	    return TRUE;
	  return FALSE;

	case 2:

	  /*
	   * Dest deve essere una sottoclasse di source, altrimenti la 
	   * conversione non e' ammessa.
	   */
 
	  if (node_dest==node_source || node_dest->is_subclass(node_source))
	    return TRUE;
	  return FALSE;

	case 3:

	    return TRUE;

	case 4:

	  /*
	   * Source deve essere Object.
	   */
	  
	  if ((node=LoadClass(ObjectName,++LastIndex))==NULL)
	    return FALSE;
	  else
	    if (node_source==node)
	      return TRUE;
	    else
	      return FALSE;

	case 5:

	  /*
	   * Source deve essere una sottoclasse di dest.
	   */

	  if (node_source==node_dest || node_source->is_subclass(node_dest))
	    return TRUE;
	  return FALSE;

	case 6:

	  /*
	   * Source deve essere la stessa classe di dest.
	   */

	  if (node_source==node_dest)
	    return TRUE;
	  return FALSE;

	case 7:

	  /*
	   * Source deve implementare l'interfaccia dest
	   */

	  if (node_source->implements(node_dest))
	    return TRUE;
	  return FALSE;

	case 8:

	  /*
	   * Dest deve implementare l'interfaccia dest.
	   */

	  if (node_dest->implements(node_source))
	    return TRUE;
	  return FALSE;

	case 9:

	  /*
	   * Dest deve essere Object.
	   */

	  if (node_dest->getname()=="java/lang/Object")
	    return TRUE;
	  return FALSE;

	case 10:

	  /*
	   * Source e dest sono lo stesso tipo primitivo, o dest e' un 
	   * reference con source che gli puo' essere assegnato.
	   */

	  if (source.array_comp_type().is_primitive() && 
	      dest.array_comp_type().is_primitive()   &&
	      source.array_comp_type()==dest.array_comp_type())
	    return TRUE;
	  else
	    if (dest.is_reference())  
	      if (is_Cast_Conversion(dest.array_comp_type(),
				     source.array_comp_type()))
		return TRUE;
	  return FALSE;

	case 11:
	  
	  /*
	   * Source e' un array e Dest e' un'interfaccia, secondo le specifiche
	   * di Java, solo Cloneable e' l'interfaccia implementata da array.
	   */

	  if ((node=LoadClass(CloneableName,++LastIndex))==NULL)
	    return FALSE;
	  else
	    if (node==node_dest)
	      return TRUE;
	    else
	      return FALSE;
	}
    }
  return FALSE;
}

/*
 * CompileUnit::is_Assign_Conversion
 *
 * Controlla se e' ammesso la conversione dal descrittore source a dest secondo
 * i criteri dell'assign conversion.
 */

int CompileUnit::is_Assign_Conversion(Descriptor& dest, Descriptor& source)
{
  if (source==DES_INT && (dest==DES_BYTE || dest==DES_SHORT || dest==DES_CHAR))
    {
      /*
       * NON GESTITO!!!
       *
       * L'unica cosa non gestita in tale algoritmo e' la possibilita' di nar-
       * rowing da int a byte, short, char.
       * Cioe' se una variabile intera e' rappresentabile in byte, short, char,
       * allora puo' essere convertita in tali tipi.
       *
       * Questo e' l'unico aspetto che differisce assign conversion da method
       * conversion. 
       * In effetti questo tipo di conversione non sara' mai attivata e per
       * ora ammettiamo che assign conversion lavori come method conversion.
       */
    }
  
  return is_Method_Conversion(dest,source);
}

/*
 * CompileUnit::is_Method_Conversion
 *
 * restituisce TRUE se e' ammessa la conversione di descrittore da source a
 * destsecondo l'algoritmo di method conversion, FALSE altrimenti.
 */

int CompileUnit::is_Method_Conversion(Descriptor& dest, Descriptor& source)
{
  int i,j;

  int primitive_table[8][8]={/* b s i l f d c              */
    
                               {1,1,1,1,1,1,0}, /* byte    */
			       {0,1,1,1,1,1,0}, /* short   */
			       {0,0,1,1,1,1,0}, /* int     */
			       {0,0,0,1,1,1,0}, /* long    */
			       {0,0,0,0,1,1,0}, /* float   */
			       {0,0,0,0,0,1,0}, /* double  */
			       {0,0,1,1,1,1,1}  /* char    */
			     };

  int reference_table[4][4]={{1,1,2,2},
			     {3,3,2,0},
			     {4,4,5,6},
			     {0,0,0,7}
			   };
  
  char *value[]={ DES_BYTE  ,
		  DES_SHORT ,
		  DES_INT   ,
		  DES_LONG  ,
		  DES_FLOAT ,
		  DES_DOUBLE,
		  DES_CHAR
		};
  
  if (source.is_primitive() && dest.is_primitive())
    {      
      for (i=0; i<7; i++)
	if (source==value[i])
	  break;
      for (j=0; j<7; j++)
	if (dest==value[j])
	  break;
      return primitive_table[i][j];
    }
  else
    if (source==DES_BOOLEAN && dest==DES_BOOLEAN)
      return TRUE;
    else
      if (source==DES_VOID && dest==DES_VOID)
	return TRUE;
      else
	if (source.is_reference() && dest.is_reference())
	  {
	    int row, col;
	    STNode *node_source, *node_dest, *node;
	    
	    if (source.is_array()) 
	      col=3;
	    else
	      {
		if (source=="Lgimnasium/agamennone;") source="Lgimnasium/agamennone;";
		if (source.to_packagename()==package_name)
		  node_source=LoadClass(source.to_singlename(),++LastIndex);
		else
		  node_source=LoadClass(source.to_fullname(),++LastIndex);
		
		if (node_source->is_class())
		  {
		    if (IS_FINAL(node_source->getaccess())) 
		      col=1;
		    else
		      col=0;
		  }
		else
		  if (node_source->is_interface())
		    col=2;
	      }
	    
	    if (dest.is_array())
	      row=3;
	    else
	      {
		if (dest.to_packagename()==package_name)
		  node_dest=LoadClass(dest.to_singlename(),++LastIndex);
		else
		  node_dest=LoadClass(dest.to_fullname(),++LastIndex);
		
		if (node_dest->is_class())
		  {
		    if (!IS_FINAL(node_dest->getaccess())) 
		      row=0;
		    else
		      row=1;
		  }
		else
		  if (node_dest->is_interface())
		    row=2;
	      }
	    
	    switch(reference_table[row][col])
	      {
	      case 1:
		
		/*
		 * Source deve essere una sottoclasse di dest oppure 
		 * source==dest.
		 */
		
		if (node_source->is_subclass(node_dest) || 
		    (node_source==node_dest))
		  return TRUE;
		return FALSE;
	      case 2:
		
		/*
		 * dest deve essere Object
		 */

		if ((node=LoadClass(ObjectName,++LastIndex))==NULL)
		  return FALSE;
		else
		  if (node==node_dest)
		    return TRUE;
		  else
		    return FALSE;

	      case 3:
		
		/*
		 * Source deve essere la stessa classe di dest.
		 */

		if (node_source==node_dest)
		  return TRUE;
		return FALSE;
	      case 4:

		/*
		 * Source deve implementare l'interfaccia dest.
		 */

		if (node_source->implements(node_dest))
		  return TRUE;
		return FALSE;
	      case 5:
		
		/*
		 * Source deve essere una sottointerfaccia di dest.
		 */

		if (node_source->is_subinterface(node_dest) || 
		    node_source==node_dest)
		  return TRUE;
		return FALSE;
	      case 6:
		
		/*
		 * Dest e' un'interfaccia, mentre Source e' un array, secondo
		 * le specifiche di Java (JLS pag. 61) l'assegnazione e' 
		 * corretta se Source==Cloneable, l'unica interfaccia imple-
		 * mentata da array.
		 */

		if ((node=LoadClass(CloneableName,++LastIndex))==NULL)
		  return FALSE;
		else
		  if (node_dest==node)
		    return TRUE;
		  else
		    return FALSE;
		      
	      case 7:
		
		/*
		 * Source e dest sono lo stesso tipo primitivo o dest e'
		 * un reference type e source e' assegnabile a dest.
		 */
		
		if (source.array_comp_type().is_primitive() &&
		    dest.array_comp_type().is_primitive()   &&
		    source.array_comp_type()==dest.array_comp_type())
		  return TRUE;
		else
		  if (dest.array_comp_type().is_reference() &&
		      is_Assign_Conversion(dest.array_comp_type(),
					   source.array_comp_type()))
		    return TRUE;
		return FALSE;
	      }
	  }
  return FALSE;
}

/*
 * CompileUnit::BArithmetic_Promotion
 *
 * Controlla la promozione di descrittori di tipo primitive type.
 * Secondo le specifiche Java, (vedi pag. 74 di Java Language Specification),
 * sono applicati i seguenti criteri. Dati due descrittori primitive type:
 *
 *    - se uno dei due e' double l'altro sara' promosso a double;
 *    - altrimenti, se uno dei due e' float, l'altro sara' promosso a float;
 *    - altrimenti, se uno dei due e' long, l'altro sara' promosso a long;
 *    - altrimenti sono promossi entrambi a int.
 */

Descriptor& CompileUnit::BArithmetic_Promotion(Descriptor& des1, 
					       Descriptor& des2)
{
  Descriptor *des;

  if (des1.is_primitive() && des2.is_primitive())
    if (des1==DES_DOUBLE || des2==DES_DOUBLE)
      des=new Descriptor(DES_DOUBLE);
    else
      if (des1==DES_FLOAT || des2==DES_FLOAT)
	des=new Descriptor(DES_FLOAT);
      else
	if (des1==DES_LONG || des2==DES_LONG)
	  des=new Descriptor(DES_LONG);
	else
	  des=new Descriptor(DES_INT);
  else
    des=new Descriptor(DES_NULL);

  return *des;

/*
  int i,j;
  char *primitive_descriptor[]={DES_BYTE,DES_CHAR,DES_SHORT,DES_INT,DES_LONG,
				DES_FLOAT,DES_DOUBLE};
  Descriptor *des;

  for (i=0; i<SIZE_DES_PRIMITIVE; i++)
    if (des1==primitive_descriptor[i])
      break;
  for (j=0; j<SIZE_DES_PRIMITIVE; j++)
    if (des2==primitive_descriptor[j])
      break;
  if (i<SIZE_DES_PRIMITIVE && j<SIZE_DES_PRIMITIVE)
    if (i<j)
      des=new Descriptor(primitive_descriptor[j]);
    else
      des=new Descriptor(primitive_descriptor[i]);
  else
    des=new Descriptor(DES_NULL);
  return *des;*/
}

/*
 * CompileUnit::UArithmetic_Promotion
 *
 * Applica l'Arithmetic_Promotion unario, cioe' un descrittore di tipo short o
 * byte lo converte ad int.
 */

Descriptor& CompileUnit::UArithmetic_Promotion(Descriptor& descriptor)
{
  Descriptor *des;

  des=new Descriptor;
  if (descriptor==DES_BYTE || descriptor==DES_SHORT)
    *des=DES_INT;
  else 
    *des=descriptor;
  return *des;
}

/*
 * CompileUnit::LoadClass
 *
 * Dato il nome di una classe, presente informato .class su file system,
 * il metodo corrente la carica, crea per essa un parse-tree da aggiungere
 * a quello di sistema, alloca eventuali simboli nella symbol-table e 
 * restituisce il nodo della symbol-table dove e' definita la classe in
 * questione.
 */

STNode *CompileUnit::LoadClass(String& Name, int Index)
{
  Class_File *ClassLoad=new Class_File;
  STNode *node=ClassLoad->LoadClass(Name,Index);

  delete ClassLoad;
  return node;
}

/*
 * CompileUnit::backpatch
 *
 * Effettua il backpatching di espressioni booleane, per break e continue.
 */

void CompileUnit::backpatch(Code_Attribute *ca, BPList *list, _u4 label)
{
  if (list)
    for (BPNode *p=list->GetHeadlist(); p!=NULL; p=p->getnext())
      {
	_u1 opcode=ca->GetCode((_u2)p->getaddress());
	ca->Gen_to(p->getaddress(),opcode,(_u2)label);
      }
}

/*
 * CompileUnit::InitFieldNotStatic
 *
 * Questo metodo e' utilizzato nella dichiarazione di campi, quando nel file 
 * sorgente si tenta di inizializzare un campo non statico.
 * in effetti questo metodo modifica il syntax tree facendo in modo che 
 * all'interno di tutti i costruttori vengano aggiunte le istruzioni per 
 * inizializzare il campo.
 * Infatti, decompilando un file java in cui si inizializzano campi non statici
 * si puo' facilmente constatare come il codice venga aggiunto a tutti i 
 * costruttori della classe.
 * In ogni classe esiste almeno un costruttore.
 */

void CompileUnit::InitFieldNotStatic(STNode *fnode, TreeNode *etree)
{
  /*
   * Per ogni costruttore definito nella classe si aggiunge il pezzo di
   * syntax tree che sintetizza l'assegnazione:
   *
   *                        field=<expr>;
   */

  STNode *first=table->LookUpHere(init_name,THIRD_LVL,
				  current_class->getindex());

  for (STNode *cnode=first; cnode; 
       cnode=table->NextSym(cnode,THIRD_LVL,current_class->getindex()))
    {
      TreeNode *ctree=cnode->getmyheader();

      TreeNode *tree_1=new THISNode();
      TreeNode *tree_2=new IDNode(fnode);

      tree_1->SetDescriptor(current_class->getdescriptor());

      tree_1=new EXPNode(FieldAccessOp, tree_1, tree_2);
      tree_2=new EXPNode(AssignOp, tree_1, etree);

      tree_1=new EXPNode(StmtOp,Dummy,tree_2);

      if (!ctree->GetRightC()->GetRightC()->IsDummy())
	if (((EXPNode *)ctree->GetRightC()->GetRightC())->GetNodeOp()==StmtOp)
	  tree_1->SetLeftC(ctree->GetRightC()->GetRightC());
	else
	  {
	    tree_2=new EXPNode(StmtOp,Dummy,ctree->GetRightC()->GetRightC());
	    tree_1->SetLeftC(tree_2);
	  }
      ctree->GetRightC()->SetRightC(tree_1);
    } 
}

/*
 * CompileUnit::InitFieldStatic
 *
 * Questo metodo e' utilizzato nella dichiarazione di campi, quando nel file 
 * sorgente si tenta di inizializzare un campo statico con valori diversi
 * da costanti numeriche, caratteri o stringhe.
 * In effetti questo metodo modifica il syntax tree facendo in modo che 
 * all'interno dell'inizializzatore statico (<clinit>) vengano aggiunte le 
 * istruzioni per inizializzare il campo.
 * Se <clinit> non e' stato precedentemente definito, allora bisognera' 
 * dichiararlo implicitamente in questa sede. 
 */

void CompileUnit::InitFieldStatic(STNode *fnode, TreeNode *etree)
{
  /*
   * Per ogni costruttore definito nella classe si aggiunge il pezzo di
   * syntax tree che sintetizza l'assegnazione:
   *
   *                        field=<expr>;
   */

  TreeNode *tree_1, *tree_2;

  STNode *clinit=table->LookUpHere(clinit_name,THIRD_LVL,
				   current_class->getindex());

  if (!clinit)
    {
      /*
       * Bisogna dichiarare implicitamente <clinit>.
       */

      TreeNode *class_tree=current_class->getmyheader();

      Descriptor descriptor;

      descriptor="()V";

      clinit=table->Install_Id(clinit_name, THIRD_LVL, 
			       current_class->getindex(), descriptor);

      clinit->setaccess(ACC_STATIC);

      tree_1=new IDNode(clinit);
      tree_2=new EXPNode(BlockOp,Dummy,Dummy);
      
      tree_1=new EXPNode(CommaOp,tree_1,Dummy);
      tree_1=new EXPNode(MethodHeaderOp,tree_1,Dummy);
      tree_1=new EXPNode(MethodDeclOp,tree_1,tree_2);

      clinit->setmyheader(tree_1);

      tree_1=new EXPNode(ClassBodyOp,class_tree->GetRightC(),tree_1);
      class_tree->SetRightC(tree_1);
    }

  /*
   * A questo punto siamo certi che, comunque vada, un inizializzatore statico
   * e' presente nel sistema per la classe in esame.
   */

  TreeNode *clinit_tree=clinit->getmyheader();

  tree_1=new THISNode();
  tree_2=new IDNode(fnode);

  tree_1->SetDescriptor(current_class->getdescriptor());

  tree_1=new EXPNode(FieldAccessOp, tree_1, tree_2);
  tree_2=new EXPNode(AssignOp,tree_1,etree);

  tree_1=new EXPNode(StmtOp,Dummy,tree_2);

  if (!clinit_tree->GetRightC()->GetRightC()->IsDummy())
    if (((EXPNode *)
	 clinit_tree->GetRightC()->GetRightC())->GetNodeOp()==StmtOp)
      tree_1->SetLeftC(clinit_tree->GetRightC()->GetRightC());
    else
      {
	tree_2=new EXPNode(StmtOp,Dummy,clinit_tree->GetRightC()->GetRightC());
	tree_1->SetLeftC(tree_2);
      }
  clinit_tree->GetRightC()->SetRightC(tree_1);
} 
