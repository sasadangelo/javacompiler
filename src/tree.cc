/*
 * file tree.cc
 * 
 * descrizione: questo file contiene l'implementazione delle classi necessarie
 *              alla costruzione del parse-tree.
 *
 * Questo file e' parte del Compilatore Java.
 *
 * Maggio 1997, scritto da Marco Magliano e-mail marmag@zoo.diaedu.unisa.it.
 */

#include <stdio.h>
#include <globals.h>
#include <cstring.h>
#include <descriptor.h>
#include <hash.h>    
#include <table.h>
#include <backpatch.h>
#include <tree.h>

extern int yylineno;

const char *NameNodeOp[]={"CompUnitOp",
		    "ClassOp",
		    "ClassHeaderOp",
		    "CommaOp",
		    "ClassBodyOp",
		    "ExtendOp",
		    "ImplementsOp",
		    "FieldDeclOp",
		    "MethodDeclOp",
		    "MethodHeaderOp",
		    "BlockOp",
		    "ThrowsOp",
		    "ParameterOp",
		    "StmtOp",
		    "LocVarDeclOp",
		    "IfThenElseOp",
		    "LoopOp",
		    "SwitchStmtOp",
		    "BodySwitchOp",
		    "BreakStmtOp",
		    "ContinueStmtOp",
		    "ReturnStmtOp",
		    "ThrowStmtOp",
		    "SyncStmtOp",
		    "TryStmtOp",
		    "StaticInitOp",
		    "ConstrDeclOp",
		    "ExplConstrInvOp",
		    "ArgsOp",
		    "InterfaceOp",
		    "InterfaceHeaderOp",
		    "InterfaceBodyOp",
		    "ExtendsOp",
		    "OrOrOp",
		    "AndAndOp",
		    "OrOp",
		    "XorOp",
		    "AndOp",
		    "EqOp",
		    "NeOp",
		    "LtOp",
		    "GtOp",
		    "LeOp",
		    "GeOp",
		    "LShiftOp",
		    "RShiftOp",
		    "UrShiftOp",
		    "AddOp",
		    "SubOp",
		    "MultOp",
		    "ModOp",
		    "DivOp",
		    "CondExprOp",
		    "UniPlusOp",
		    "UniMinusOp",
		    "PlusPlusOp",
		    "MinusMinusOp",
		    "NewOp",
		    "CastOp",
		    "FieldAccessOp",
		    "MethodCallOp",
                    "ArrayAccessOp",
                    "NewArrayOp",
                    "DimExprOp",
                    "AssignOp",
		    "LogicalNotOp",
		    "BitwiseNotOp",
		    "TryBodyOp",
		    "CheckCastOp",
		    "ConstrHeaderOp",
		    "ThisOp",
		    "SuperOp",
                    "LabelStmtOp",
                    "ConstrCallOp",         // da eliminare
                    "BodySwitchLabelOp",
                    "AssignMultOp",
                    "AssignDivOp",
		    "AssignModOp",
		    "AssignPlusOp",
		    "AssignMinusOp",
		    "AssignAndOp",
		    "AssignOrOp",
		    "AssignXorOp",
		    "AssignLShiftOp",
		    "AssignRShiftOp",
		    "AssignURShiftOp",
		    "ArrayComponentOp",
		    "InstanceOfOp"};

TreeNode *Dummy=new TreeNode();

/*****************************************************************************
 * classe TreeNode                                                           *
 *****************************************************************************/

/*
 * TreeNode class implementation.
 */

/*
 * costruttore classe TreeNode
 */

TreeNode::TreeNode() 
{
  LeftC=Dummy;
  RightC=Dummy;
  descriptor=DES_NULL;
  line=yylineno;
  truelist=NULL;
  falselist=NULL;
}

/*
 * distruttore classe TreeNode.
 */

TreeNode::~TreeNode() 
{
  if (!LeftC->IsDummy())
    delete LeftC;
  if (!RightC->IsDummy())
    delete RightC;
}

/*
 * Routine per la gestione di un singolo nodo.
 */

Descriptor& TreeNode::GetDescriptor()               { return descriptor;    }
void      TreeNode::SetDescriptor(Descriptor& _Des) { descriptor=_Des;      } 
TreeNode *TreeNode::GetLeftC()                      { return LeftC;         }
void      TreeNode::SetLeftC(TreeNode *_LeftC)      { LeftC=_LeftC;         }
TreeNode *TreeNode::GetRightC()                     { return RightC;        }
void      TreeNode::SetRightC(TreeNode *_RightC)    { RightC=_RightC;       }
int       TreeNode::IsDummy()          { return this==Dummy ? TRUE : FALSE; }

int       TreeNode::GetLine()                       { return line;          }
void      TreeNode::SetLine(int _line)              { line=_line;           } 
int       TreeNode::GetNodeKind()                   { return NodeKind;      }

/*
 * EXPNode::GetTrueList
 * EXPNode::SetTrueList
 * EXPNode::GetFalseList
 * EXPNode::SetFalseList
 *
 * A ogni nodo del parse-tree associamo due liste di backpatching
 * chiamate truelist e falselist. L'uso di queste liste e' riportato nel
 * libro "Compilers, principles techniques and tools" di Aho-Ullman a pag. 500,
 * dove viene mostrato come grazie ad esse si riesce a gestire in modo effi-
 * ciente la generazione del codice per le espressioni booleane.
 */

BPList *TreeNode::GetTrueList()           { return truelist;  }
BPList *TreeNode::GetFalseList()          { return falselist; }
void    TreeNode::SetTrueList(BPList *l)  { truelist=l;       }
void    TreeNode::SetFalseList(BPList *l) { falselist=l;      }

/*
 * Routine per il riconoscimento del tipo di nodo.
 */

int TreeNode::is_BOOLEANode() { return NodeKind==_BOOLEANode; }
int TreeNode::is_NULLNode()   { return NodeKind==_NULLNode;   }
int TreeNode::is_THISNode()   { return NodeKind==_THISNode;   }
int TreeNode::is_SUPERNode()  { return NodeKind==_SUPERNode;  }
int TreeNode::is_STRINGNode() { return NodeKind==_STRINGNode; }
int TreeNode::is_CHARNode()   { return NodeKind==_CHARNode;   }
int TreeNode::is_INUMNode()   { return NodeKind==_INUMNode;   }
int TreeNode::is_FNUMNode()   { return NodeKind==_FNUMNode;   }
int TreeNode::is_IDNode()     { return NodeKind==_IDNode;     }
int TreeNode::is_EXPNode()    { return NodeKind==_EXPNode;    }
int TreeNode::is_UNAMENode()  { return NodeKind==_UNAMENode;  }

/*
 * TreeNode::zerocrosses
 * TreeNode::indent
 *
 * metodi privati utilizzati per la stampa di un nodo.
 */

void TreeNode::zerocrosses() { for (register int i=0; i< 162; i++) crosses[i]=0; }

void TreeNode::indent(FILE *stream,int x)
{
  for (register int i=0; i < x; i++)
    fprintf(stream,"%s",crosses[i]? "| " : "  ");
  fprintf(stream,"%s", x ? "+-" : "R-");
  if (x)
    crosses[x]=(crosses[x]+1) % 2;
}

/*
 * TreeNode::PrintNode
 *
 * Stampa di un nodo.
 */

void TreeNode::PrintNode(FILE *stream, int depth) 
{
  if (IsDummy())
    {
      indent(stream,depth);
      fprintf(stream,"[*]\n");
    }
}

/*****************************************************************************
 * classe NULLNode                                                           *
 *****************************************************************************/

NULLNode::NULLNode() : TreeNode()
{
  NodeKind=_NULLNode;
  descriptor=DES_NULL;
} 

NULLNode::~NULLNode() { }

/*
 * NULLNode::PrintNode
 *
 * metodo che sovrascive quello della classe TreeNode e serve per stampare su
 * stream il nodo corrente.
 */

void NULLNode::PrintNode(FILE *stream, int depth)
{
  indent(stream,depth);
  if (descriptor==DES_NULL)
    fprintf(stream,"[NULLNode]\n");
  else
    fprintf(stream,"[NULLNode,\"%s\"]\n",descriptor.to_char());
      
}

/*****************************************************************************
 * implementazione classe THISNode.                                          *
 *****************************************************************************/

THISNode::THISNode() : TreeNode()
{
  NodeKind=_THISNode;
  descriptor=DES_NULL;
} 

THISNode::~THISNode() { }

void THISNode::PrintNode(FILE *stream, int depth)
{
  indent(stream,depth);
  fprintf(stream,"[THISNode,\"%s\"]\n",descriptor.to_char());
}

/*****************************************************************************
 * classe SUPERNode.                                                         *
 *****************************************************************************/

SUPERNode::SUPERNode() : TreeNode()
{
  NodeKind=_SUPERNode;
  descriptor=DES_NULL;
} 

SUPERNode::~SUPERNode() { }

void SUPERNode::PrintNode(FILE *stream, int depth)
{
  indent(stream,depth);
  fprintf(stream,"[SUPERNode,\"%s\"]\n",descriptor.to_char());
}

/*****************************************************************************
 * implementazione classe EXPNode.                                           *
 *****************************************************************************/

/*
 * costruttore/distruttore della classe.
 */
 
EXPNode::EXPNode(int _NodeOp, TreeNode *_LeftC, TreeNode *_RightC) : TreeNode()
{
  NodeKind=_EXPNode;
  NodeOp=_NodeOp;
  LeftC=_LeftC;
  RightC=_RightC;
} 

EXPNode::~EXPNode() 
{
  // Il compilatore si occupera' di eliminare le liste di backpatching.
}

/*
 * EXPNode::GetNodeOp
 * EXPNode::SetNodeOp
 *
 * restituisce/imposta l'operatore del nodo in questione.
 */

int  EXPNode::GetNodeOp()        { return NodeOp; }
void EXPNode::SetNodeOp(int op)  { NodeOp=op; }

/*
 * EXPNode::SetTreeOp
 *
 * Aggiunge l'operatore al nodo radice e a tutti i suoi figli sinistri.
 * Operatore molto utilizzato per la dichiarazione di campi, variabili locali,
 * etc.
 */

void EXPNode::SetTreeOp(int _NodeOp)
{
  for (EXPNode *p=this; !p->IsDummy(); p=(EXPNode *)p->GetLeftC())
    if (p->is_EXPNode())
      p->SetNodeOp(_NodeOp);
    else
      return;
}

/*
 * EXPNode::PrintNode
 *
 * Stampa del nodo su stream.
 */

void EXPNode::PrintNode(FILE *stream, int depth)
{ 
  if (!depth)
    zerocrosses();
  RightC->PrintNode(stream,depth+1);
  indent(stream,depth);
  if (descriptor==DES_NULL)
    fprintf(stream,"[%s]\n", NameNodeOp[NodeOp]);
  else
    fprintf(stream,"[%s,\"%s\"]\n", NameNodeOp[NodeOp],descriptor.to_char());
  LeftC->PrintNode(stream,depth+1);
}

/*****************************************************************************
 * classe STRINGNode.                                                        *
 *****************************************************************************/

STRINGNode::STRINGNode(char *_str) : TreeNode()
{
  NodeKind=_STRINGNode;
  descriptor=DES_STRING;
  str=_str;
} 

STRINGNode::STRINGNode(String &_str) : TreeNode()
{
  NodeKind=_STRINGNode;
  descriptor=DES_STRING;
  str=_str;
} 

STRINGNode::~STRINGNode() { }

/*
 * Altre routine per la classe STRINGNode di facile comprensione.
 */

String& STRINGNode::GetString()           { return str; }
void  STRINGNode::SetString(String& _str) { str=_str; }
void  STRINGNode::PrintNode(FILE *stream, int depth) 
{ 
  indent(stream,depth);
  fprintf(stream,"[STRINGNode,\"%s\"]\n",str.to_char()); 
}

/*****************************************************************************
 * classe CHARNode.                                                          *
 *****************************************************************************/

CHARNode::CHARNode(int val) : TreeNode()
{ 
  descriptor=DES_CHAR;
  character=val; 
  NodeKind=_CHARNode;
}
 
CHARNode::~CHARNode()                  { }
int CHARNode::GetCharacter()           { return character; }
void CHARNode::SetCharacter(int val)   { character=val; }
void CHARNode::PrintNode(FILE *stream, int depth)
{
  indent(stream,depth);
  fprintf(stream,"[CHARNode,\'%c\']\n",character);
}

/*****************************************************************************
 * classe INUMNode.                                                          *
 *****************************************************************************/

/*
 * Un nodo INUMNode nel parse-tree denota l'uso di una costante intera.
 */

INUMNode::INUMNode(long val) : TreeNode()
{
  NodeKind=_INUMNode;
  valint=val; 
  descriptor=DES_INT;
} 

INUMNode::~INUMNode()                   { }
long INUMNode::GetVal()                 { return valint; }
void INUMNode::SetVal(long val)         { valint=val; }

void INUMNode::PrintNode(FILE *stream, int depth)
{
  indent(stream,depth);
  fprintf(stream,"[INUMNode,%ld]\n",valint);
}

/*****************************************************************************
 * classe FNUMNode.                                                          *
 *****************************************************************************/

/*
 * Un nodo FNUMNode nel parse-tree denota l'uso di una costante reale.
 */

FNUMNode::FNUMNode(double val) : TreeNode()
{ 
  NodeKind=_FNUMNode;
  valfloat=val; 
  descriptor=DES_FLOAT;
} 

FNUMNode::~FNUMNode()                     { }
double FNUMNode::GetVal()                 { return valfloat; }
void FNUMNode::SetVal(double val)         { valfloat=val; }

void FNUMNode::PrintNode(FILE *stream, int depth)
{
  indent(stream,depth);
  fprintf(stream,"[FNUMNode,%f]\n",valfloat);
}

/*****************************************************************************
 * classe IDNode.                                                            *
 *****************************************************************************/

/*
 * Un nodo IDNode nel parse-tree denota l'uso di un identificatore "resolved".
 */

IDNode::IDNode(STNode *id) : TreeNode()
{ 
  NodeKind=_IDNode;
  iden=id;
  descriptor=id->getdescriptor();
} 

IDNode::~IDNode()                 { iden=NULL;   }
STNode *IDNode::GetIden()         { return iden; }
void IDNode::SetIden(STNode *id)  { iden=id;     }
void IDNode::PrintNode(FILE *stream, int depth) 
{
  indent(stream,depth);
  fprintf(stream,"[IDNode,\"%s\",\"%s\"]\n",
	  iden->getname().to_char(),iden->getdescriptor().to_char());
}

/*****************************************************************************
 * classe BOOLEANode.                                                        *
 *****************************************************************************/

/*
 * Un nodo BOOLEANode nel parse-tree denota l'uso di una costante booleana.
 */

BOOLEANode::BOOLEANode(int val) : TreeNode()
{ 
  NodeKind=_BOOLEANode;
  valbool=val;
  descriptor=DES_BOOLEAN;
} 

BOOLEANode::~BOOLEANode()                  { }
int BOOLEANode::GetBoolean()               { return valbool; }
void BOOLEANode::SetBoolean(int val)       { valbool=val; }

void BOOLEANode::PrintNode(FILE *stream, int depth)
{
  indent(stream,depth);
  if (valbool)
    fprintf(stream,"[BOOLEANode,true]\n");
  else
    fprintf(stream,"[BOOLEANode,false]\n");
}

/*****************************************************************************
 * Implementazione classe UNAMENode.                                         *
 *****************************************************************************/

/*
 * Un nodo UNAMENode nel parse-tree denota l'uso di un nome "unresolved".
 */

UNAMENode::UNAMENode(char *_name) : TreeNode()
{
  NodeKind=_UNAMENode;
  descriptor=DES_NULL;
  name=_name;
} 

UNAMENode::UNAMENode(String &_name) : TreeNode()
{
  NodeKind=_UNAMENode;
  descriptor=DES_NULL;
  name=_name;
} 

UNAMENode::~UNAMENode() { }

/*
 * Altre routine per la classe UNAMENode di facile comprensione.
 */

String& UNAMENode::GetName()              { return name; }
void    UNAMENode::SetName(String& _name) { name=_name;  }

void    UNAMENode::PrintNode(FILE *stream, int depth) 
{ 
  indent(stream,depth);
  fprintf(stream,"[UNAMENode,\"%s\"]\n",name.to_char()); 
}

/*****************************************************************************
 * classe Tree.                                                              *
 *****************************************************************************/

Tree::Tree(TreeNode *_Root=Dummy)    { Root=_Root;  }
Tree::~Tree()                        { delete Root; }
TreeNode *Tree::GetRoot()            { return Root; }
void      Tree::SetRoot(TreeNode *t) { Root=t;      }
int       Tree::IsDummy()            { return Root==Dummy ? TRUE:FALSE;}

void Tree::PrintTree(FILE *stream) { Root->PrintNode(stream,0); }
