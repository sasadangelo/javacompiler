/*
 * file compile.h
 *
 * descrizione: definizione della classe CompileUnit, classe principale del
 *              compilatore.
 *
 * Questo file e' parte del Compilatore Java.
 *
 * Maggio 1997, scritto da Salvatore D'Angelo e-mail xc0261@xcom.it.
 */

#ifndef COMPILE_H
#define COMPILE_H

#include <stack.h>

#define MAX_LABEL_IN_SWITCH 100

extern int yyparse();
extern FILE *yyin;

class Tree;
class STable;
class Environment;
class STNode;
class TreeNode;
class ListErrors;
class Descriptor;
class Class_File;
class Code_Attribute;
class BPList;
class CPHash;
class Local_Count;
class Method_Info;

/*
 * Il nuovo tipo di dato brk e' il record degli stack: s_brk e s_con; utilizza-
 * ti per la gestione delle istruzioni break e continue.
 */

typedef struct {
  String label;
  BPList *bplist;
} brk_rec;

typedef struct {
  String label;
  int    index;
} con_rec;

typedef struct {
  _u4 label;
  int is_default;
  _u4 jump;
} switch_rec;

class CompileUnit 
{
private:

  Tree        *syntax_tree;       // syntax-tree
  STable      *table;             // symbol-table.
  Environment *env;               // ambiente di esecuzione.
  Class_File  *Class;             // JVM.
  ListErrors  *errors;            // lista degli errori.

  String      name_source;        // nome file .java
  FILE        *javafile;          // stream contenente il source code.
  String      package_name;       // nome eventuale pacchetto. 

  /*
   * Campi per la gestione della Java Virtual Machine.
   *
   * - tabella hash utilizzata per evitare duplicazione di nomi nella 
   *   Constant Pool;
   * - stack variabili locali;
   * - program counter;
   * - altri;
   */

  CPHash *cphash;				     

  Stack<Local_Count *> *stk;        // stack variabili locali e parametri.

  int not_return;                   // utilizzato per inserire un return di 
                                    // default se il metodo non lo contiene.   

  Stack<brk_rec *> *s_brk;          // stack break
  Stack<con_rec *> *s_con;          // stack continue

  switch_rec *table_switch;         // tabella per la generazione di un'istru-
                                    // zione switch

  /*
   * Campi utilizzati nel parsing dell'istruzione switch.
   */

  int  sdefault;
  long label[MAX_LABEL_IN_SWITCH];
  long numlabel;

  /*
   * intf_public vale TRUE se nel file .java c'e' piu' di un'interfaccia 
   * public, FALSE altrimenti.
   */
  
  int  intf_public;

  /*
   * Metodi privati utilizzati per l'analisi del syntax-tree.
   */

  void ParseTree();
  void ParseCompUnit(TreeNode *);
  void ParseClassDecl(TreeNode *);
  void ParseClassBody(TreeNode *);
  void ParseFieldDecl(TreeNode *);
  void ParseMethodDecl(TreeNode *);
  void ParseMethodHeader(TreeNode *);
  void ParseThrows(TreeNode *);
  void ParseBlock(TreeNode *);
  void ParseStmt(TreeNode *);
  void ParseLocVarDecl(TreeNode *);
  void ParseIfThenElse(TreeNode *);
  void ParseLoop(TreeNode *);
  void ParseWhile(TreeNode *);
  void ParseDoWhile(TreeNode *);
  void ParseLabelStmt(TreeNode *);
  void ParseForStmt(TreeNode *);
  void ParseSwitchStmt(TreeNode *);
  void ParseSwitchBody(TreeNode *, TreeNode *);
  void ParseBodySwitchLabel(TreeNode *);
  void ParseBreakStmt(TreeNode *);
  void ParseContinueStmt(TreeNode *);
  void ParseReturnStmt(TreeNode *);
  void ParseSyncStmt(TreeNode *);
  void ParseThrowStmt(TreeNode *);
  void ParseTryStmt(TreeNode *);
  void ParseTryBody(TreeNode *);
  TreeNode *ParseExplConstrInv(TreeNode *);
  void ParseInterfaceDecl(TreeNode *);
  void ParseInterfaceBody(TreeNode *);
  void ParseIntfFieldDecl(TreeNode *);
  void ParseIntfMethodDecl(TreeNode *);
  void ParseConstrDecl(TreeNode *);
  void ParseStaticInit(TreeNode *);

  /*
   * Metodi privati usati nell'analisi sematica per gestire le espressioni.
   */

  int IsConstExpr(TreeNode*);
  int IsVariable(TreeNode*);

  TreeNode *ParseExpr(TreeNode *);
  TreeNode *ParseConstExpr(TreeNode *);
  TreeNode *ParseCondExpr(TreeNode *);
  TreeNode *ParseCondOrExpr(TreeNode *);
  TreeNode *ParseCondAndExpr(TreeNode *);
  TreeNode *ParseIorExpr(TreeNode *);
  TreeNode *ParseXorExpr(TreeNode *);
  TreeNode *ParseAndExpr(TreeNode *);
  TreeNode *ParseEqualityExpr(TreeNode *);
  TreeNode *ParseRelopInstanceExpr(TreeNode *);
  TreeNode *ParseRelopExpr(TreeNode *);
  TreeNode *ParseInstanceOfExpr(TreeNode *);
  TreeNode *ParseShiftExpr(TreeNode *);
  TreeNode *ParseAddSubExpr(TreeNode *);
  TreeNode *ParseAddExpr(TreeNode *);
  TreeNode *ParseSubExpr(TreeNode *);
  TreeNode *ParseMultDivExpr(TreeNode *);
  TreeNode *ParseMultExpr(TreeNode *);
  TreeNode *ParseDivExpr(TreeNode *);
  TreeNode *ParseUnaryExpr(TreeNode *);
  TreeNode *ParsePreIncDecExpr(TreeNode *);
  TreeNode *ParseUnaryAddExpr(TreeNode *);
  TreeNode *ParseUnarySubExpr(TreeNode *);
  TreeNode *ParseUnaryNotPlusMinusExpr(TreeNode *);
  TreeNode *ParseLogicalNotExpr(TreeNode *);
  TreeNode *ParseBitwiseNotExpr(TreeNode *);
  TreeNode *ParseCastExpr(TreeNode *);
  TreeNode *ParsePostFixExpr(TreeNode *);
  TreeNode *ParseNameExpr(TreeNode *);
  TreeNode *ParsePostIncDecExpr(TreeNode *);
  TreeNode *ParsePrimaryExpr(TreeNode *);
  TreeNode *ParseArrayAccess(TreeNode *);
  TreeNode *ParseArrayCreation(TreeNode *);

  TreeNode *ParseFieldAccess(TreeNode *);
  STNode   *SearchField(STNode*, String&, int*, int);
  STNode   *SearchFieldInSuperInterfaces(STNode *, STNode *, String&, int, 
					 int*);
  STNode   *SearchFieldInSuperClass(STNode *, STNode *, String&, int*, int);
  int      IsAccessibleField(STNode *, STNode *);

  TreeNode *ParseMethodCall(TreeNode*);
  TreeNode *ParseMethodCall2(TreeNode*);
  TreeNode *ParseArgsExpr(TreeNode*);
  STNode *SelectMethod(STNode*,String&,TreeNode*);
  STNode *SearchMethodInLink(STNode *,STNode *, String&, TreeNode*, int *);
  STNode *SearchMethodInSuperClass(STNode*,String&,TreeNode*,int*,int*,int);
  STNode *SearchMethodInSuperInterfaces(STNode*,STNode*,String&,TreeNode*,int*,
					int*,int);
  int IsApplicable(TreeNode*,TreeNode*);
  int IsApplicable2(TreeNode*,TreeNode*,int*);
  int IsAccessible(STNode*,STNode*);
  TreeNode *ConvertArgs(TreeNode*,TreeNode*);
  int IsMostSpecific(STNode*,STNode*);

  TreeNode *ParseAssignment(TreeNode*);
  TreeNode *ParseCompAssignment(TreeNode*);
  TreeNode *ParseCreateClassInstance(TreeNode*);
  TreeNode *ParseConstrCall(TreeNode*);

  TreeNode *ParseArrayInit(TreeNode *);
  TreeNode *ParseVarInit(TreeNode *);

  void InitFieldStatic(STNode *,TreeNode *);
  void InitFieldNotStatic(STNode *,TreeNode *);

  /*
   * Metodi privati per la generazione del codice JVM.
   */

  void GenCompUnit(TreeNode*);
  void GenClass(TreeNode*);
  void GenSuperIntf(TreeNode*);
  void GenClassBody(TreeNode *root);
  void GenFieldDecl(TreeNode *);
  void GenMethodDecl(TreeNode*);
  void GenMethodParameters(TreeNode*);
  void GenMethodThrows(TreeNode *, Method_Info *);
  void GenStaticInit(TreeNode *);
  void GenInterface(TreeNode *);

  /*
   * Metodi per la generazione di codice JVM per le istruzioni.
   */
  
  void GenMethodBody(TreeNode*, Method_Info*);
  void GenStmts(TreeNode *, Code_Attribute *);
  void GenStmt(TreeNode *,Code_Attribute *);
  void GenBlock(TreeNode *, Code_Attribute *);
  void GenLocalVarDecl(TreeNode *,Code_Attribute *);
  void GenIf(TreeNode *, Code_Attribute *);
  void GenIfThen(TreeNode *, Code_Attribute *);
  void GenIfThenElse(TreeNode *, Code_Attribute *);
  void GenLoop(TreeNode *, Code_Attribute *);
  void GenWhile(TreeNode *, Code_Attribute *);
  void GenDoWhile(TreeNode *, Code_Attribute *);
  void GenFor(TreeNode *, Code_Attribute *);
  void GenBreak(TreeNode *, Code_Attribute *);
  void GenContinue(TreeNode *, Code_Attribute *);
  void GenReturn(TreeNode *, Code_Attribute *);
  void GenSynchronized(TreeNode *, Code_Attribute *);
  void GenThrow(TreeNode *, Code_Attribute *);
  void GenTry(TreeNode *, Code_Attribute *);
  void GenCatchFinally(TreeNode *, Code_Attribute *, int, int, _u1, _u1, 
		       BPList *, BPList *, BPList *);
  void GenLabeledStmt(TreeNode *, Code_Attribute *);
  void GenSwitch(TreeNode *, Code_Attribute *);
  void GenIncDecStmt(TreeNode *, Code_Attribute *);

  /*
   * Metodi per la generazione di codice JVM per le espressioni rvalue.
   */

  void GenExpr(TreeNode *, Code_Attribute *);
  void GenCondExpr(TreeNode *, Code_Attribute *);
  void GenCondOrExpr(TreeNode *, Code_Attribute *);
  void GenCondAndExpr(TreeNode *, Code_Attribute *);
  void GenIorExpr(TreeNode *, Code_Attribute *);
  void GenXorExpr(TreeNode *, Code_Attribute *);
  void GenAndExpr(TreeNode *, Code_Attribute *);
  void GenEqualityExpr(TreeNode *, Code_Attribute *);
  void GenRelopInstanceExpr(TreeNode *, Code_Attribute *);
  void GenRelopExpr(TreeNode *, Code_Attribute *);
  void GenInstanceExpr(TreeNode *, Code_Attribute *);
  void GenShiftExpr(TreeNode *, Code_Attribute *);
  void GenAddSubExpr(TreeNode *, Code_Attribute *);
  void GenAddExpr(TreeNode *, Code_Attribute *);
  void GenAddString(TreeNode *, Code_Attribute *);
  void GenSubExpr(TreeNode *, Code_Attribute *);
  void GenMultDivExpr(TreeNode *, Code_Attribute *);
  void GenMultExpr(TreeNode *, Code_Attribute *);
  void GenDivExpr(TreeNode *, Code_Attribute *);
  void GenUnaryExpr(TreeNode *, Code_Attribute *);
  void GenPreIncDecExpr(TreeNode *, Code_Attribute *);
  void GenUnaryAddExpr(TreeNode *, Code_Attribute *);
  void GenUnarySubExpr(TreeNode *, Code_Attribute *);
  void GenUnaryExprNotPlusMinus(TreeNode *, Code_Attribute *);
  void GenBitwiseNotExpr(TreeNode *, Code_Attribute *);
  void GenCastExpr(TreeNode *, Code_Attribute *);
  void GenLogicalNotExpr(TreeNode *, Code_Attribute *);
  void GenPostFixExpr(TreeNode *, Code_Attribute *);
  void GenPostIncDecExpr(TreeNode *, Code_Attribute *);
  void GenArrayAccess(TreeNode *, Code_Attribute *);
  void GenFieldAccess(TreeNode *, Code_Attribute *, int);
  void GenMethodCall(TreeNode *, Code_Attribute *, int);
  void GenNewClassInstance(TreeNode *, Code_Attribute *, int);
  void GenNewArray(TreeNode *, Code_Attribute *);
  void GenArrayDimension(TreeNode *, Code_Attribute *);
  void GenLoadLocVar(TreeNode *, Code_Attribute *);
  void GenConstantInteger(TreeNode *, Code_Attribute *);
  void GenConstantFloat(TreeNode *, Code_Attribute *);
  void GenBoolean(TreeNode *, Code_Attribute *);
  void GenChar(TreeNode *, Code_Attribute *);
  void GenString(TreeNode *, Code_Attribute *);
  void GenAssignment(TreeNode *, Code_Attribute *);
  void GenCompAssignment(TreeNode *, Code_Attribute *);

  /*
   * Metodi per la generazione di codice JVM per le espressioni lvalue.
   */

  void GenLValue(TreeNode *, Code_Attribute *);
  void GenLValueLocal(TreeNode *, Code_Attribute *);
  void GenLValueField(TreeNode *, Code_Attribute *);
  void GenLValueArray(TreeNode *, Code_Attribute *);

public:

  _u4 pc_count;                     // program counter.

  CompileUnit(char *);
  ~CompileUnit();
  Environment *GetEnvironment();

  String& GetPackageName();
  void SetPackageName(String&);

  Class_File *GetClass();

  CPHash *GetCPHash();

  Tree *make_syntax_tree(TreeNode*);

  ListErrors  *GetErrors();

  STable *GetTable();

  void Parse();
  void GenCode();

  void MsgErrors(int, char *,...);

  int  is_in_package();
  int  mask_access(int, int);
  void import_on_demand(String&,String&);
  void discard_parameters(TreeNode *);
  void discard_locals(TreeNode*);

  STNode *LoadClass(String&,int);

  /*
   * Metodi che controllano i 5 tipi di casting ammessi da Java:
   *
   * - assign conversion;
   * - method convertion;
   * - cast convertion;
   * - binary arithmetic promotion;
   * - unary arithmetic promotion;
   *
   * Dalle specifiche di Java risulta che noi non utilizziamo i seguenti tipi 
   * di conversione:
   *
   * - widening primitive e reference conversions;
   * - narrowing primitive e reference conversions;
   *
   * Il motivo di questa rinuncia e' principalmente dovuta al fatto che non
   * abbiamo capito in quali circostanze (probabilmente poche) questi tipi
   * di conversioni vanno applicate.
   *
   * Si rende noto che implicitamente, negli altri tipi di conversione, sono 
   * rispettate le clausole del Forbidden Conversion (Java Language Specifica-
   * tion pag. 60).
   * 
   */

  int  is_Assign_Conversion(Descriptor&, Descriptor&);
  int  is_Method_Conversion(Descriptor&, Descriptor&);
  int  is_Cast_Conversion(Descriptor&, Descriptor&);
  Descriptor& BArithmetic_Promotion(Descriptor&, Descriptor&);
  Descriptor& UArithmetic_Promotion(Descriptor&);

  void backpatch(Code_Attribute *, BPList *, _u4);
};

#endif
