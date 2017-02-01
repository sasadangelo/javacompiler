/*
 * file table.h
 *
 * descrizione: in questo file sono definite le classi STNode e STable.
 *
 * Questo file e' parte del Compilatore Java.
 *
 * Maggio 1997, scritto da Salvatore D'Angelo e-mail xc0261@xcom.it.
 */

#ifndef TABLE_H
#define TABLE_H

class TreeNode;
class HTNode;
class Hashtable;

class STNode : public HTNode
{

private :

  TreeNode  *myheader;
  int        access;
  Descriptor descriptor;
  String     full_name;
  int        level;
  int        index;
  int        unresolved;
  STNode    *myclass;
  int        line; 
  int        used;

  int        local_index;  // indice stack frame, per variabili locali e 
			   // parametri formali.
  
  int        generable;    // se il nodo denota una classe, allora questo campo
		           // dice se essa e' oppure no generabile.

public :

  STNode();
  STNode(String&,int,int,Descriptor&);
  ~STNode();

  TreeNode *getmyheader();
  void  setmyheader(TreeNode *);
  int   getaccess();
  void  setaccess(int);
  Descriptor& getdescriptor();
  void  setdescriptor(Descriptor& );
  String& getfullname();
  void  setfullname(String& );
  int   getlevel();
  void  setlevel(int);
  int   getindex();
  void  setindex(int);
  void  setunresolved();
  void  setresolved();
  STNode *getmyclass();
  void    setmyclass(STNode*);

  int   getline(); 
  void  setline(int);

  void  set_unused();
  int   is_used();

  _u1   getlocalindex();
  void  setlocalindex(_u1);

  int    is_local();
  int    is_resolved();
  int    is_class();
  int    is_method();
  int    is_field();
  int    is_interface();
  int    is_subclass(STNode*);
  int    implements(STNode*);
  int    is_subinterface(STNode*);
  STNode *get_superclass();
  int    is_local_on_stackframe();
  int    is_generable();

  void  set_not_generable();

  void  PrintNode(FILE *);
};

/*
 * This is the definition of Symbol Table class.
 */

// #define MAX_CLASS_DEFINED 255

class STable : public Hashtable
{
public:

  STable(int);
  ~STable();
  STNode *Install_Id(String&,int,int,Descriptor&);
  void    Discard(STNode *);
  void    Recover(STNode *);
  STNode* LookUpHere(String&);
  STNode* LookUpHere(String&,int);
  STNode* LookUpHere(String&,int,int);
  STNode* LookUpHere(String&,int,int,Descriptor&);
  STNode* LookUp(String&,int);
  STNode* LookUp(String&,int,int);
  STNode* NextSym(STNode *);
  STNode* NextSym(STNode *, int, int);
  void    PrintTable(FILE *);
};

#endif
