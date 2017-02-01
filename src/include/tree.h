/*
 * file tree.h
 *
 * descrizione: in questo file sono definite tutte le classi necessarie per
 *              implementare il parse-tree del compilatore.
 *
 * Questo file e' parte del Compilatore Java.
 *
 * Maggio 1997, scritto da Magliano Marco e-mail marmag@zoo.diaedu.unisa.it.
 */

#ifndef TREE_H
#define TREE_H

#define _NULLNode    0
#define _THISNode    1
#define _SUPERNode   2
#define _EXPNode     3
#define _STRINGNode  4
#define _CHARNode    5
#define _INUMNode    6
#define _FNUMNode    7
#define _IDNode      8
#define _BOOLEANode  9
#define _UNAMENode   10

#define CompUnitOp         0
#define ClassOp            1
#define ClassHeaderOp      2
#define CommaOp            3
#define ClassBodyOp        4
#define ExtendOp           5
#define ImplementsOp       6
#define FieldDeclOp        7
#define MethodDeclOp       8
#define MethodHeaderOp     9
#define BlockOp            10
#define ThrowsOp           11
#define ParameterOp        12
#define StmtOp             13
#define LocVarDeclOp       14
#define IfThenElseOp       15
#define LoopOp             16
#define SwitchStmtOp       17
#define BodySwitchOp       18
#define BreakStmtOp        19
#define ContinueStmtOp     20
#define ReturnStmtOp       21
#define ThrowStmtOp        22
#define SyncStmtOp         23
#define TryStmtOp          24
#define StaticInitOp       25
#define ConstrDeclOp       26
#define ExplConstrInvOp    27
#define ArgsOp             28
#define InterfaceOp        29
#define InterfaceHeaderOp  30
#define InterfaceBodyOp    31
#define ExtendsOp          32
#define OrOrOp             33
#define AndAndOp           34
#define OrOp               35
#define XorOp              36
#define AndOp              37
#define EqOp               38
#define NeOp               39
#define LtOp               40
#define GtOp               41
#define LeOp               42
#define GeOp               43
#define LShiftOp           44
#define RShiftOp           45
#define UrShiftOp          46
#define AddOp              47
#define SubOp              48
#define MultOp             49
#define ModOp              50
#define DivOp              51
#define CondExprOp         52
#define UniPlusOp          53
#define UniMinusOp         54
#define PlusPlusOp         55
#define MinusMinusOp       56
#define NewOp              57
#define CastOp             58
#define FieldAccessOp      59
#define MethodCallOp       60
#define ArrayAccessOp      61
#define NewArrayOp         62
#define DimExprOp          63
#define AssignOp           64
#define LogicalNotOp       65
#define BitwiseNotOp       66
#define TryBodyOp          67
// probabilmente va eliminato. #define CheckCastOp        68
#define ConstrHeaderOp     69
#define ThisOp             70
#define SuperOp            71
#define LabelStmtOp        72
// #define ConstrCallOp       73
#define BodySwitchLabelOp  74
#define AssignMultOp       75
#define AssignDivOp        76
#define AssignModOp        77
#define AssignPlusOp       78
#define AssignMinusOp      79
#define AssignAndOp        80
#define AssignOrOp         81
#define AssignXorOp        82
#define AssignLShiftOp     83
#define AssignRShiftOp     84
#define AssignURShiftOp    85
#define ArrayComponentOp   86
#define InstanceOfOp       87

class STNode;
class BPList;

static int crosses[300];

/*****************************************************************************
 * classe TreeNode.                                                          *
 *****************************************************************************/

class TreeNode {
protected:
  int        NodeKind;
  TreeNode  *LeftC;
  TreeNode  *RightC;
  Descriptor descriptor;
  int        line;        /*
			   * linea di codice a cui la parte di albero fa
			   * riferimento.
			   */

  BPList *truelist;       // lista di backpatching per le espressioni booleane 
  BPList *falselist;      // lista di backpatching per le espressioni booleane 

 protected:

  void zerocrosses();
  void indent(FILE *,int);

public:

  TreeNode(); 
  virtual ~TreeNode();
  TreeNode *GetLeftC();
  void      SetLeftC(TreeNode*);
  TreeNode *GetRightC();
  void      SetRightC(TreeNode*);
  int       IsDummy();
  Descriptor& GetDescriptor();
  void      SetDescriptor(Descriptor&);
  int       GetLine();
  void      SetLine(int);
  int       GetNodeKind();

  BPList *GetTrueList();
  void    SetTrueList(BPList*);
  BPList *GetFalseList();
  void    SetFalseList(BPList*);

  int       is_BOOLEANode();
  int       is_NULLNode();
  int       is_THISNode();
  int       is_SUPERNode();
  int       is_EXPNode();
  int       is_IDNode();
  int       is_STRINGNode();
  int       is_CHARNode();
  int       is_INUMNode();
  int       is_FNUMNode();
  int       is_UNAMENode();
  
  virtual void PrintNode(FILE *, int);
};

/*
 * The NULLNode class definition.
 */

class NULLNode : public TreeNode {
public:

  NULLNode();
  ~NULLNode();
  void PrintNode(FILE *, int);
};

/*
 * The THISNode class definition.
 */

class THISNode : public TreeNode {
public:

  THISNode(); 
  ~THISNode();
  void PrintNode(FILE *, int);
};

/*
 * The SUPERNode class definition.
 */

class SUPERNode : public TreeNode {
public:

  SUPERNode(); 
  ~SUPERNode();
  void PrintNode(FILE *, int);
};

/*
 * The EXPNode class definition.
 */

class EXPNode : public TreeNode {

private:

  int NodeOp;

public:

  EXPNode(int, TreeNode *, TreeNode *); 
  ~EXPNode();

  int GetNodeOp();
  void SetNodeOp(int);

  void SetTreeOp(int);

  void PrintNode(FILE *, int);
};

/*
 * The STRINGNode class definition.
 */

class STRINGNode : public TreeNode {
private:
  String str;
public:

  STRINGNode(char *);
  STRINGNode(String&);
  ~STRINGNode();
  String& GetString();
  void SetString(String&);
  void PrintNode(FILE *,int);
};

/*
 * The CHARNode class definition.
 */

class CHARNode : public TreeNode {
 private: 
  int character;
 public:

  CHARNode(int); 
  ~CHARNode();
  int GetCharacter();
  void SetCharacter(int);
  void PrintNode(FILE *, int);
};

/*
 * The INUMNode class definition.
 */

class INUMNode : public TreeNode {
 private: 
  long valint;
 public:

  INUMNode(long); 
  ~INUMNode();
  long GetVal();
  void SetVal(long);
  void PrintNode(FILE *, int);
};

/*
 * The FNUMNode class definition.
 */

class FNUMNode : public TreeNode {
 private: 
  double valfloat;
 public:

  FNUMNode(double); 
  ~FNUMNode();
  double GetVal();
  void SetVal(double);
  void PrintNode(FILE *, int);
};

/*
 * The IDNode class definition.
 */

class IDNode : public TreeNode {
 private: 
  STNode *iden;
 public:

  IDNode(STNode *); 
  ~IDNode();
  STNode *GetIden();
  void SetIden(STNode *);
  void PrintNode(FILE *, int);
};

/*
 * The BOOLEANode class definition.
 */

class BOOLEANode : public TreeNode {
 private: 
  int valbool;
 public:

  BOOLEANode(int); 
  ~BOOLEANode();
  int GetBoolean();
  void SetBoolean(int);
  void PrintNode(FILE *, int);
};

/*
 * The Tree Class Definition.
 */

class Tree {
private:
  TreeNode *Root;
public:
  Tree(TreeNode *);
  ~Tree();
  int       IsDummy();
  TreeNode *GetRoot();
  void      SetRoot(TreeNode *);
  void      PrintTree(FILE *);
};

/*
 * The UNAMENode class definition.
 */

class UNAMENode : public TreeNode {
private:
  String name;
public:

  UNAMENode(char *);
  UNAMENode(String&);
  ~UNAMENode();
  String& GetName();
  void SetName(String&);
  void PrintNode(FILE *,int);
};

#endif
