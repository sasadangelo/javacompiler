/*
 * file genexpr.cc
 *
 * descrizione: questo file e' la continuazione di gen.cc, qui troviamo i meto-
 *              di della classe CompileUnit utilizzati per la generazione delle
 *              espressioni. Le espressioni, saranno gestite in base a due
 *              categorie
 *
 *                   a) lvalue;
 *                   b) rvalue;
 *
 *              per capire la differenza tra queste due categorie, si faccia
 *              riferimento al libro "Compilers principles, techniques and 
 *              tools" diAho-Ullmann. A loro volta le espressioni rvalue 
 *              saranno gestite in base a due grandi categorie: booleane e non
 *              booleane.
 *              Una proposta efficiente (quella da noi utilizzata) e' possibile
 *              trovarla su Aho-Ullmann pag. 500.
 *
 * Questo file e' parte del Compilatore Java.
 *
 * Marzo 1998, scritto da Salvatore D'Angelo, e-mail xc0261@xcom.it.
 */

/*****************************************************************************
 * Generazione Espressioni RValue                                            *
 *****************************************************************************/

#include <stdio.h>
#include <globals.h>
#include <opcodes.h>
#include <access.h>
#include <cstring.h>
#include <descriptor.h>
#include <hash.h>
#include <table.h>
#include <tree.h>
#include <jvm.h>
#include <compile.h>
#include <local.h>
#include <backpatch.h>

extern STNode *current_class;
extern TreeNode *Dummy;

extern Descriptor DesLong;
extern Descriptor DesFloat;
extern Descriptor DesVoid;
extern Descriptor DesStringBuffer;

extern String init_name;

/*
 * CompileUnit::GenExpr
 *
 * Genero il bytecode per un' espressione.
 */

void CompileUnit::GenExpr(TreeNode *root, Code_Attribute *ca)
{
  if (root->is_EXPNode())
    {
      int op=((EXPNode *)root)->GetNodeOp();

      if (op==AssignOp)
	GenAssignment(root,ca);
      else
	if (op==AssignMultOp || op==AssignDivOp   || op==AssignModOp    || 
	    op==AssignPlusOp || op==AssignMinusOp || op==AssignAndOp    ||
	    op==AssignOrOp   || op==AssignXorOp   || op==AssignLShiftOp ||
	    op==AssignRShiftOp || op==AssignURShiftOp)
	  GenCompAssignment(root,ca);
	else
	  GenCondExpr(root,ca);
    }
  else
    GenCondExpr(root,ca);
}

/*
 * CompileUnit::GenCondExpr
 *
 * Genero un'espressione del tipo: <expr> ? <expr> : <expr>
 */

void CompileUnit::GenCondExpr(TreeNode *root, Code_Attribute *ca)
{
  if (!root->is_EXPNode() || ((EXPNode *)root)->GetNodeOp()!=CondExprOp)
    {
      GenCondOrExpr(root,ca);
      return;
    }

  int label_1, label_2, label_3;

  TreeNode *etree=root->GetLeftC()->GetLeftC();
  TreeNode *stmt1=root->GetLeftC()->GetRightC();
  TreeNode *stmt2=root->GetRightC();

  root->GetLeftC()->SetLeftC(new EXPNode(LogicalNotOp,Dummy,etree));

  etree=root->GetLeftC()->GetLeftC();

  GenExpr(etree,ca);

  label_1=pc_count;

  GenCondOrExpr(stmt1,ca);

  if (stmt1->GetDescriptor()==DES_BOOLEAN && 
      (stmt1->GetTrueList() || stmt1->GetFalseList()))
    {
      ca->Gen(ICONST_0);
      ca->Gen(GOTO, (_u2)(pc_count+3));
      ca->Gen(ICONST_1);

      backpatch(ca,stmt1->GetTrueList(),pc_count-1);
      backpatch(ca,stmt1->GetFalseList(),pc_count-5);
      
      delete stmt1->GetTrueList();
      delete stmt1->GetFalseList();

      stmt1->SetTrueList(NULL);
      stmt1->SetFalseList(NULL);
    }
 
  label_3=pc_count;

  ca->Gen(GOTO,(_u2)0);

  label_2=pc_count;

  GenExpr(stmt2,ca);

  if (stmt2->GetDescriptor()==DES_BOOLEAN && 
      (stmt2->GetTrueList() || stmt2->GetFalseList()))
    {
      ca->Gen(ICONST_0);
      ca->Gen(GOTO, (_u2)(pc_count+3));
      ca->Gen(ICONST_1);

      backpatch(ca,stmt2->GetTrueList(),pc_count-1);
      backpatch(ca,stmt2->GetFalseList(),pc_count-5);

      delete stmt2->GetTrueList();
      delete stmt2->GetFalseList();

      stmt2->SetTrueList(NULL);
      stmt2->SetFalseList(NULL);
    }

  ca->Gen_to(label_3,GOTO,pc_count);

  if (etree->GetTrueList() || etree->GetFalseList())
    {
      backpatch(ca,etree->GetTrueList(),label_2);
      backpatch(ca,etree->GetFalseList(),label_1);

      delete etree->GetTrueList();
      delete etree->GetFalseList();

      etree->SetTrueList(NULL);
      etree->SetFalseList(NULL);
    }
}

/*
 * CompileUnit::GenCondOrExpr
 *
 * Compilo un'espressione booleana del tipo: <expr> || <expr>. Questa gestione
 * richiede l'impiego del backpatching sulle liste: truelist e falselist; 
 * tali liste sono utilizzate per gestire in modo efficiente le espressioni 
 * booleane, per cui la loro gestione avviene grazie a questo strumento. Per
 * ulteriori dettagli si faccia riferimento alla documentazione allegata.
 */

void CompileUnit::GenCondOrExpr(TreeNode *root, Code_Attribute *ca)
{
  if (!root->is_EXPNode() || ((EXPNode *)root)->GetNodeOp()!=OrOrOp)
    {
      GenCondAndExpr(root,ca);
      return;
    }

  TreeNode *Exp0=root->GetLeftC();
  TreeNode *Exp1=root->GetRightC();

  GenCondOrExpr(Exp0,ca);
  backpatch(ca,Exp0->GetFalseList(),pc_count);

  delete Exp0->GetFalseList();
  
  Exp0->SetFalseList(NULL);

  GenCondAndExpr(Exp1,ca);
  
  BPList *truelist=new BPList();

  truelist->Merge(Exp0->GetTrueList(), Exp1->GetTrueList());

  root->SetTrueList(truelist);		
  root->SetFalseList(Exp1->GetFalseList());
  
  Exp0->SetTrueList(NULL);

  Exp1->SetTrueList(NULL);
  Exp1->SetFalseList(NULL);
}

/*
 * CompileUnit::GenCondAndExpr
 *
 * Compilo un'espressione booleana del tipo: <expr> && <expr>. 
 * La gestione e' simile a quella di CompileUnit::GenCondOrExpr.
 */

void CompileUnit::GenCondAndExpr(TreeNode *root, Code_Attribute *ca)
{
  if (!root->is_EXPNode() || ((EXPNode *)root)->GetNodeOp()!=AndAndOp)
    {
      GenIorExpr(root,ca);
      return;
    }

  TreeNode *Exp0=new EXPNode(LogicalNotOp,Dummy,root->GetLeftC());
  TreeNode *Exp1=root->GetRightC();

  GenCondAndExpr(Exp0,ca);
  backpatch(ca,Exp0->GetFalseList(),pc_count);

  delete Exp0->GetFalseList();
  Exp0->SetFalseList(NULL);

  GenIorExpr(Exp1,ca);
  
  BPList *falselist=new BPList();

  falselist->Merge(Exp0->GetTrueList(),Exp1->GetFalseList());

  root->SetTrueList(Exp1->GetTrueList());
  root->SetFalseList(falselist);

  Exp0->SetTrueList(NULL);

  Exp1->SetTrueList(NULL);
  Exp1->SetFalseList(NULL);
}

/*
 * CompileUnit::GenIorExpr
 *
 * Genera espressioni di tipo: <expr> | <expr>.
 */

void CompileUnit::GenIorExpr(TreeNode *root, Code_Attribute *ca)
{
  if (!root->is_EXPNode() || ((EXPNode *)root)->GetNodeOp()!=OrOp)
    {
      GenXorExpr(root,ca);
      return;
    }

  TreeNode *Exp0=root->GetLeftC();
  TreeNode *Exp1=root->GetRightC();

  if (Exp0->GetDescriptor()==DES_BOOLEAN && Exp1->GetDescriptor()==DES_BOOLEAN)
    {
      if (Exp0->GetTrueList() || Exp0->GetFalseList())
	{
	  GenIorExpr(Exp0,ca);
	  ca->Gen(ICONST_0);
	  ca->Gen(GOTO,(_u2)(pc_count+3));
	  ca->Gen(ICONST_1);
	  backpatch(ca,Exp0->GetTrueList(),pc_count-1);
	  backpatch(ca,Exp0->GetFalseList(),pc_count-5);

	  delete Exp0->GetTrueList();
	  delete Exp0->GetFalseList();

	  Exp0->SetTrueList(NULL);
	  Exp0->SetFalseList(NULL);
	}
      else
	GenIorExpr(Exp0,ca);

      if (Exp1->GetTrueList() || Exp1->GetFalseList())
	{
	  GenXorExpr(Exp1,ca);
	  ca->Gen(ICONST_0);
	  ca->Gen(GOTO,(_u2)(pc_count+3));
	  ca->Gen(ICONST_1);
	  backpatch(ca,Exp1->GetTrueList(),pc_count-1);
	  backpatch(ca,Exp1->GetFalseList(),pc_count-5);

	  delete Exp1->GetTrueList();
	  delete Exp1->GetFalseList();

	  Exp1->SetTrueList(NULL);
	  Exp1->SetFalseList(NULL);
	}
      else
	GenXorExpr(Exp1,ca);

      ca->Gen(IOR);
      
    }
  else
    {
      /*
       * le due espressioni sono int o long.
       */

      GenIorExpr(Exp0,ca);
      GenXorExpr(Exp1,ca);

      if (Exp0->GetDescriptor()==DES_LONG && Exp1->GetDescriptor()==DES_LONG)
	ca->Gen(LOR);
      else
	ca->Gen(IOR);
    }
}

/*
 * CompileUnit::GenXorExpr
 *
 * Genera espressioni di tipo: <expr> ^ <expr>.
 */

void CompileUnit::GenXorExpr(TreeNode *root, Code_Attribute *ca)
{
  if (!root->is_EXPNode() || ((EXPNode *)root)->GetNodeOp()!=XorOp)
    {
      GenAndExpr(root,ca);
      return;
    }

  TreeNode *Exp0=root->GetLeftC();
  TreeNode *Exp1=root->GetRightC();

  if (Exp0->GetDescriptor()==DES_BOOLEAN && Exp1->GetDescriptor()==DES_BOOLEAN)
    {
      if (Exp0->GetTrueList() || Exp0->GetFalseList())
	{
	  GenXorExpr(Exp0,ca);
	  ca->Gen(ICONST_0);
	  ca->Gen(GOTO,(_u2)(pc_count+3));
	  ca->Gen(ICONST_1);
	  backpatch(ca,Exp0->GetTrueList(),pc_count-1);
	  backpatch(ca,Exp0->GetFalseList(),pc_count-5);

	  delete Exp0->GetTrueList();
	  delete Exp0->GetFalseList();

	  Exp0->SetTrueList(NULL);
	  Exp0->SetFalseList(NULL);
	}
      else
	GenXorExpr(Exp0,ca);

      if (Exp1->GetTrueList() || Exp1->GetFalseList())
	{
	  GenAndExpr(Exp1,ca);
	  ca->Gen(ICONST_0);
	  ca->Gen(GOTO,(_u2)(pc_count+3));
	  ca->Gen(ICONST_1);
	  backpatch(ca,Exp1->GetTrueList(),pc_count-1);
	  backpatch(ca,Exp1->GetFalseList(),pc_count-5);

	  delete Exp1->GetTrueList();
	  delete Exp1->GetFalseList();

	  Exp1->SetTrueList(NULL);
	  Exp1->SetFalseList(NULL);
	}
      else
	GenAndExpr(Exp1,ca);

      ca->Gen(IXOR);
      
    }
  else
    {
      /*
       * le due espressioni sono int o long.
       */

      GenXorExpr(Exp0,ca);
      GenAndExpr(Exp1,ca);

      if (Exp0->GetDescriptor()==DES_LONG && Exp1->GetDescriptor()==DES_LONG)
	ca->Gen(LXOR);
      else
	ca->Gen(IXOR);
    }
}

/*
 * CompileUnit::GenAndExpr
 *
 * Genera espressioni di tipo: <expr> & <expr>.
 */

void CompileUnit::GenAndExpr(TreeNode *root, Code_Attribute *ca)
{
  if (!root->is_EXPNode() || ((EXPNode *)root)->GetNodeOp()!=AndOp)
    {
      GenEqualityExpr(root,ca);
      return;
    }

  TreeNode *Exp0=root->GetLeftC();
  TreeNode *Exp1=root->GetRightC();

  if (Exp0->GetDescriptor()==DES_BOOLEAN && Exp1->GetDescriptor()==DES_BOOLEAN)
    {
      if (Exp0->GetTrueList() || Exp0->GetFalseList())
	{
	  GenAndExpr(Exp0,ca);
	  ca->Gen(ICONST_0);
	  ca->Gen(GOTO,(_u2)(pc_count+3));
	  ca->Gen(ICONST_1);
	  backpatch(ca,Exp0->GetTrueList(),pc_count-1);
	  backpatch(ca,Exp0->GetFalseList(),pc_count-5);

	  delete Exp0->GetTrueList();
	  delete Exp0->GetFalseList();

	  Exp0->SetTrueList(NULL);
	  Exp0->SetFalseList(NULL);
	}
      else
	GenAndExpr(Exp0,ca);

      if (Exp1->GetTrueList() || Exp1->GetFalseList())
	{
	  GenEqualityExpr(Exp1,ca);
	  ca->Gen(ICONST_0);
	  ca->Gen(GOTO,(_u2)(pc_count+3));
	  ca->Gen(ICONST_1);
	  backpatch(ca,Exp1->GetTrueList(),pc_count-1);
	  backpatch(ca,Exp1->GetFalseList(),pc_count-5);

	  delete Exp1->GetTrueList();
	  delete Exp1->GetFalseList();

	  Exp1->SetTrueList(NULL);
	  Exp1->SetFalseList(NULL);
	}
      else
	GenEqualityExpr(Exp1,ca);
      
      ca->Gen(IAND);

    }
  else
    {
      /*
       * le due espressioni sono int o long.
       */

      GenIorExpr(Exp0,ca);
      GenXorExpr(Exp1,ca);

      if (Exp0->GetDescriptor()==DES_LONG && Exp1->GetDescriptor()==DES_LONG)
	ca->Gen(LAND);
      else
	ca->Gen(IAND);
    }
}

/*
 * CompileUnit::GenEqualityExpr
 *
 * Generiamo il codice per un'espressione del tipo: <expr> == <expr>.
 * La gestione di questo tipo di espressioni e' leggermente piu' complesso 
 * rispetto a quelle viste finora.
 */

void CompileUnit::GenEqualityExpr(TreeNode *root, Code_Attribute *ca)
{
  int op=((EXPNode *)root)->GetNodeOp();

  if (!root->is_EXPNode() || (op!=EqOp && op!=NeOp))
    {
      GenRelopInstanceExpr(root,ca);
      return;
    }

  TreeNode *Exp0=root->GetLeftC();
  TreeNode *Exp1=root->GetRightC();

  if (Exp0->GetDescriptor().is_reference() && 
      Exp1->GetDescriptor().is_reference())
    {
      if (Exp0->is_NULLNode() || Exp1->is_NULLNode())
	{
	  if (Exp0->is_NULLNode())
	    GenRelopInstanceExpr(Exp1,ca);
	  else
	    GenEqualityExpr(Exp0,ca);
	  
	  if (op==EqOp)
	    ca->Gen(IFNULL,(_u2)0);
	  else
	    ca->Gen(IFNONNULL,(_u2)0);

	  BPList *truelist=new BPList;

	  truelist->Add(pc_count-3);

	  ((EXPNode *)root)->SetTrueList(truelist);
	  // false list rimane a null.
	}
      else
	{
	  /*
	   * Exp0 e Exp1 sono espressioni reference entrambi non null.
	   */

	  GenEqualityExpr(Exp0,ca);
	  GenRelopInstanceExpr(Exp1,ca);
	  
	  if (op==EqOp)
	    ca->Gen(IF_ACMPEQ,(_u2)0);
	  else
	    ca->Gen(IF_ACMPNE,(_u2)0);

	  BPList *truelist=new BPList;

	  truelist->Add(pc_count-3);

	  ((EXPNode *)root)->SetTrueList(truelist);
	  // false list rimane a null.
	}
    }
  else
    if (Exp0->GetDescriptor()==DES_BOOLEAN && 
	Exp1->GetDescriptor()==DES_BOOLEAN)
      {
	  GenEqualityExpr(Exp0,ca);	
	  
	  if (Exp0->GetTrueList() || Exp0->GetFalseList())
	    {
	      ca->Gen(ICONST_0);
	      ca->Gen(GOTO, (_u2)(pc_count+3));
	      ca->Gen(ICONST_1);
	      backpatch(ca,Exp0->GetTrueList(),pc_count-1);
	      backpatch(ca,Exp0->GetFalseList(),pc_count-5);

	      delete Exp0->GetTrueList();
	      delete Exp0->GetFalseList();
	      
	      Exp0->SetTrueList(NULL);
	      Exp0->SetFalseList(NULL);
	    }
	  
	  GenRelopInstanceExpr(Exp1,ca);

	  if (Exp1->GetTrueList() || Exp1->GetFalseList())
	    {
	      ca->Gen(ICONST_0);
	      ca->Gen(GOTO, (_u2)(pc_count+3));
	      ca->Gen(ICONST_1);
	      backpatch(ca,Exp1->GetTrueList(),pc_count-1);
	      backpatch(ca,Exp1->GetFalseList(),pc_count-5);

	      delete Exp1->GetTrueList();
	      delete Exp1->GetFalseList();
	      
	      Exp1->SetTrueList(NULL);
	      Exp1->SetFalseList(NULL);
	    }
	  
	  if (op==EqOp)
	    ca->Gen(IF_ICMPEQ,(_u2)0);
	  else
	    ca->Gen(IF_ICMPNE,(_u2)0);

	  BPList *truelist=new BPList();

	  truelist->Add(pc_count-3);

	  ((EXPNode *)root)->SetTrueList(truelist);
	  // falselist rimane a null.
      }
    else
      {
	/*
	 * sono di tipo numerical type (int, long, float, double).
	 */

	if (Exp0->GetDescriptor()==DES_INT && Exp1->GetDescriptor()==DES_INT)
	  {
	    if ((Exp0->is_INUMNode() && ((INUMNode *)Exp0)->GetVal()==0) ||
		(Exp1->is_INUMNode() && ((INUMNode *)Exp1)->GetVal()==0))
	      {
		/*
		 * Solo una delle due espressioni puo' essere un INUMNode.
		 */

		if (Exp0->is_INUMNode())
		  {
		    GenRelopInstanceExpr(Exp1,ca);
		    if (op==EqOp)
		      ca->Gen(IFEQ,(_u2)0);
		    else
		      ca->Gen(IFNE,(_u2)0);
		  }
		else
		  {
		    GenEqualityExpr(Exp0,ca);
		    if (op==EqOp)
		      ca->Gen(IFEQ,(_u2)0);
		    else
		      ca->Gen(IFNE,(_u2)0);
		  }
		
		BPList *truelist=new BPList();

		truelist->Add(pc_count-3);
		
		root->SetTrueList(truelist);
		// falselist rimane null.
	      }
	    else
	      {
		GenEqualityExpr(Exp0,ca);		
		GenRelopInstanceExpr(Exp1,ca);

		if (op==EqOp)
		  ca->Gen(IF_ICMPEQ,(_u2)0);
		else
		  ca->Gen(IF_ICMPNE,(_u2)0);
		
		BPList *truelist=new BPList();

		truelist->Add(pc_count-3);
		
		root->SetTrueList(truelist);
		// falselist rimane null.
	      }
	  }
	else
	  {
	    /*
	     * Le due espressioni sono di tipo long, float o double.
	     */

	    GenEqualityExpr(Exp0,ca);
	    GenRelopInstanceExpr(Exp1,ca);

	    if (Exp0->GetDescriptor()==DES_LONG && 
		Exp1->GetDescriptor()==DES_LONG)
	      ca->Gen(LCMP);
	    else
	      if (Exp0->GetDescriptor()==DES_FLOAT && 
		  Exp1->GetDescriptor()==DES_FLOAT)
		ca->Gen(FCMPL);
	      else
		ca->Gen(DCMPL);

	    if (op==EqOp)
	      ca->Gen(IFEQ,(_u2)0);
	    else
	      ca->Gen(IFNE,(_u2)0);

	    BPList *truelist=new BPList();

	    truelist->Add(pc_count-3);

	    root->SetTrueList(truelist);
	    // falselist rimane a null.
	  }
      }
}

/*
 * CompileUnit::GenRelopInstanceExpr
 *
 * genero espressioni di tipo: <expr> relop <expr> con relop che puo' essere
 * >, >=, <, <=; oppure <expr> instanceof <classtype>.
 */

void CompileUnit::GenRelopInstanceExpr(TreeNode *root, Code_Attribute *ca)
{
  if (root->is_EXPNode() && ((EXPNode *)root)->GetNodeOp()==InstanceOfOp)
    GenInstanceExpr(root,ca);
  else
    GenRelopExpr(root,ca);
}

/*
 * CompileUnit::GenRelopExpr
 *
 * Genero espressioni relazionali del tipo: <expr> relop <expr> con relop pari
 * a: <, <=, >, >=.
 */

void CompileUnit::GenRelopExpr(TreeNode *root, Code_Attribute *ca)
{
  int  op=((EXPNode *)root)->GetNodeOp();

  if (!root->is_EXPNode() || (op!=LtOp && op!= GtOp && op!=LeOp && op!=GeOp))
    {
      GenShiftExpr(root,ca);
      return;
    }

  TreeNode *Exp0=root->GetLeftC();
  TreeNode *Exp1=root->GetRightC();

  /*
   * Exp0 e Exp1 possono essere solo di tipo numerico (int, long, float, 
   * double).
   */

  if (Exp0->GetDescriptor()==DES_INT && Exp1->GetDescriptor()==DES_INT)
    {
      if ((Exp0->is_INUMNode() && ((INUMNode *)Exp0)->GetVal()==0) ||
	  (Exp1->is_INUMNode() && ((INUMNode *)Exp1)->GetVal()==0))
	{
	  if (Exp0->is_INUMNode())
	    GenShiftExpr(Exp1,ca);
	  else
	    GenRelopInstanceExpr(Exp0,ca);

	  switch (op)
	    {
	    case LtOp: ca->Gen(IFLT,(_u2)0); break;
	    case GtOp: ca->Gen(IFGT,(_u2)0); break;
	    case LeOp: ca->Gen(IFLE,(_u2)0); break;
	    case GeOp: ca->Gen(IFGE,(_u2)0); break;
	    }
	}
      else
	{
	  GenRelopInstanceExpr(Exp0,ca);
	  GenShiftExpr(Exp1,ca);
	  
	  switch (op)
	    {
	    case LtOp: ca->Gen(IF_ICMPLT,(_u2)0); break;
	    case GtOp: ca->Gen(IF_ICMPGT,(_u2)0); break;
	    case LeOp: ca->Gen(IF_ICMPLE,(_u2)0); break;
	    case GeOp: ca->Gen(IF_ICMPGE,(_u2)0); break;
	    }
	}

      BPList *truelist=new BPList();

      truelist->Add(pc_count-3);

      root->SetTrueList(truelist);
      // falselist rimane a null.
    }
  else
    {
      /*
       * Exp0 e Exp1 sono di tipo: long, float o double.
       */

      GenRelopInstanceExpr(Exp0,ca);
      GenShiftExpr(Exp1,ca);

      if (Exp0->GetDescriptor()==DES_LONG && Exp1->GetDescriptor()==DES_LONG)
	ca->Gen(LCMP);
      else
	if (Exp0->GetDescriptor()==DES_FLOAT && 
	    Exp1->GetDescriptor()==DES_FLOAT)
	  ca->Gen(FCMPL);
	else
	  ca->Gen(DCMPL);

      switch (op)
	{
	case LtOp: ca->Gen(IFLT,(_u2)0); break;
	case GtOp: ca->Gen(IFGT,(_u2)0); break;
	case LeOp: ca->Gen(IFLE,(_u2)0); break;
	case GeOp: ca->Gen(IFGE,(_u2)0); break;
	}

      BPList *truelist=new BPList();

      truelist->Add(pc_count-3);

      root->SetTrueList(truelist);
      // falselist rimane a null.
    }
}

/*
 * CompileUnit::GenInstanceExpr
 *
 * Genera un'espressione del tipo: <expr> instanceof <classtype>.
 */

void CompileUnit::GenInstanceExpr(TreeNode *root, Code_Attribute *ca)
{
  String Name;

  Name=root->GetDescriptor().to_fullname();

  _u2 i=Class->Load_Constant_Class(Name);

  GenRelopInstanceExpr(root->GetLeftC(),ca);

  ca->Gen(INSTANCEOF,i);
}

/*
 * CompileUnit::GenShiftExpr
 *
 * Genero espressioni di tipo: <expr> shift <expr>, dove shift puo' essere
 * >>, <<, >>>.
 */

void CompileUnit::GenShiftExpr(TreeNode *root, Code_Attribute *ca)
{
  int op=((EXPNode *) root)->GetNodeOp();

  if (!root->is_EXPNode() || (op!=LShiftOp && op!=RShiftOp && op!=UrShiftOp))
    {
      GenAddSubExpr(root,ca);
      return;
    }

  TreeNode *Exp0=root->GetLeftC();
  TreeNode *Exp1=root->GetRightC();

  GenShiftExpr(Exp0,ca);
  GenAddSubExpr(Exp1,ca);

  if (Exp0->GetDescriptor()==DES_INT && Exp1->GetDescriptor()==DES_INT)
    switch (op)
      {
      case LShiftOp : ca->Gen(ISHL);  break; 
      case RShiftOp : ca->Gen(ISHR);  break;
      case UrShiftOp: ca->Gen(IUSHR); break;
      }
  else
    switch (op)
      {
      case LShiftOp : ca->Gen(LSHL);  break; 
      case RShiftOp : ca->Gen(LSHR);  break;
      case UrShiftOp: ca->Gen(LUSHR); break;
      }
}

/*
 * CompileUnit::GenAddSubExpr
 *
 * Genero operazioni di somma o sottrazione.
 */

void CompileUnit::GenAddSubExpr(TreeNode *root, Code_Attribute *ca)
{
  if (root->is_EXPNode() && ((EXPNode *) root)->GetNodeOp()==AddOp)
    GenAddExpr(root,ca);
  else
    GenSubExpr(root,ca);
}

/*
 * CompileUnit::GenAddExpr
 *
 * Genera espressioni di somma. Momentaneamente trascureremo somme tra Stringhe
 * con qualsiasi altro tipo di dato, dato che richiede una forte iterazione
 * con alcune funzioni API di String.
 */

void CompileUnit::GenAddExpr(TreeNode *root, Code_Attribute *ca)
{
  /*
   * Nella versione del 07/11/97 non teniamo conto della possibilita' che una
   * delle due espressioni possa essere una stringa. Questa eventualita' non 
   * e' molto complessa da gestire, perche' richiede solo l'invocazione di due
   * metodi:
   *
   * - append()   in java.lang.StringBuffer;
   * - toString() in java.lang.StringBuffer.
   */

  TreeNode *Exp0=root->GetLeftC();
  TreeNode *Exp1=root->GetRightC();

  Descriptor DesExp0, DesExp1;

  DesExp0=Exp0->GetDescriptor();
  DesExp1=Exp1->GetDescriptor();

  if (DesExp0.is_primitive() && DesExp1.is_primitive())
    {
      GenAddSubExpr(Exp0,ca);
      GenMultDivExpr(Exp1,ca);

      if (DesExp0==DES_INT && DesExp1==DES_INT)
	ca->Gen(IADD);
      else
	if (DesExp0==DES_LONG && DesExp1==DES_LONG)
	  ca->Gen(LADD);
	else
	  if (DesExp0==DES_FLOAT && DesExp1==DES_FLOAT)
	    ca->Gen(FADD);
	  else
	    if (DesExp0==DES_DOUBLE && DesExp1==DES_DOUBLE)
	      ca->Gen(DADD);
    }

  /*
   * Somma tra stringhe.
   */

  String     StringBufferName, toString, key_1, key_2;

  StringBufferName="java/lang/StringBuffer";

  key_1=StringBufferName+","+init_name+","+"()V";
  key_2=StringBufferName+","+"toString"+","+"()Ljava/lang/String;";

  if (DesExp0==DES_STRING || DesExp1==DES_STRING)
    {
      _u2 index_0=Class->Load_Constant_Class(StringBufferName);

      _u2 index_1=Class->Load_Methodref(key_1);
      _u2 index_2=Class->Load_Methodref(key_2);

      ca->Gen(NEW,index_0);
      ca->Gen(DUP);
      ca->Gen(INVOKESPECIAL,index_1);

      GenAddString(root, ca);

      ca->Gen(INVOKEVIRTUAL,index_2);
    }
}

/*
 * CompileUnit::GenAddString
 *
 * Genera il codice per la somma tra stringhe.
 */

void CompileUnit::GenAddString(TreeNode *root, Code_Attribute *ca)
{
  TreeNode *Exp0=root->GetLeftC();
  TreeNode *Exp1=root->GetRightC();

  Descriptor DesExp0, DesExp1, descriptor;

  DesExp0=Exp0->GetDescriptor();
  DesExp1=Exp1->GetDescriptor();

  String key, StringBufferName;

  if (Exp0->is_EXPNode() && ((EXPNode *)Exp0)->GetNodeOp()==AddOp)
    GenAddString(Exp0, ca);
  else
    {
      GenMultDivExpr(Exp0, ca);

      descriptor.build_method(DesExp0,DesStringBuffer);
      key="java/lang/StringBuffer";
	  
      key=key+","+"append"+","+descriptor.to_char();
      _u2 index=Class->Load_Methodref(key);
      ca->Gen(INVOKEVIRTUAL,index);
    }

  if (Exp1->is_EXPNode() && ((EXPNode *)Exp1)->GetNodeOp()==AddOp)
    GenAddString(Exp1, ca);
  else
    {
      GenMultDivExpr(Exp1, ca);

      descriptor.build_method(DesExp1,DesStringBuffer);
      key="java/lang/StringBuffer";
      
      key=key+","+"append"+","+descriptor.to_char();
      _u2 index=Class->Load_Methodref(key);
      ca->Gen(INVOKEVIRTUAL,index);
    }
}

/*
 * CompileUnit::GenSubExpr
 *
 * Genera un'espressione di sottrazione.
 */

void CompileUnit::GenSubExpr(TreeNode *root, Code_Attribute *ca)
{
  if (!root->is_EXPNode() || ((EXPNode *)root)->GetNodeOp()!=SubOp)
    {
      GenMultDivExpr(root,ca);
      return;
    }

  TreeNode *Exp0=root->GetLeftC();
  TreeNode *Exp1=root->GetRightC();

  Descriptor DesExp0, DesExp1;

  DesExp0=Exp0->GetDescriptor();
  DesExp1=Exp1->GetDescriptor();

  GenAddSubExpr(Exp0,ca);
  GenMultDivExpr(Exp1,ca);
  
  if (DesExp0==DES_INT && DesExp1==DES_INT)
    ca->Gen(ISUB);
  else
    if (DesExp0==DES_LONG && DesExp1==DES_LONG)
      ca->Gen(LSUB);
    else
      if (DesExp0==DES_FLOAT && DesExp1==DES_FLOAT)
	ca->Gen(FSUB);
      else
	if (DesExp0==DES_DOUBLE && DesExp1==DES_DOUBLE)
	  ca->Gen(DSUB);
}

/*
 * CompileUnit::GenMultDivExpr
 *
 * Genero operazioni di moltiplicazione, divisione o mod.
 */

void CompileUnit::GenMultDivExpr(TreeNode *root, Code_Attribute *ca)
{
  if (root->is_EXPNode() && ((EXPNode *) root)->GetNodeOp()==MultOp)
    GenMultExpr(root,ca);
  else
    GenDivExpr(root,ca);
}

/*
 * CompileUnit::GenMultExpr
 *
 * Genera un'espressione di moltiplicazione.
 */

void CompileUnit::GenMultExpr(TreeNode *root, Code_Attribute *ca)
{
  TreeNode *Exp0=root->GetLeftC();
  TreeNode *Exp1=root->GetRightC();

  Descriptor DesExp0, DesExp1;

  DesExp0=Exp0->GetDescriptor();
  DesExp1=Exp1->GetDescriptor();

  GenMultDivExpr(Exp0,ca);
  GenUnaryExpr(Exp1,ca);
  
  if (DesExp0==DES_INT && DesExp1==DES_INT)
    ca->Gen(IMUL);
  else
    if (DesExp0==DES_LONG && DesExp1==DES_LONG)
      ca->Gen(LMUL);
    else
      if (DesExp0==DES_FLOAT && DesExp1==DES_FLOAT)
	ca->Gen(FMUL);
      else
	if (DesExp0==DES_DOUBLE && DesExp1==DES_DOUBLE)
	  ca->Gen(DMUL);
}

/*
 * CompileUnit::GenDivExpr
 *
 * Genero espressioni di divisione o modulo.
 */

void CompileUnit::GenDivExpr(TreeNode *root, Code_Attribute *ca)
{
  int op=((EXPNode *) root)->GetNodeOp();

  if (!root->is_EXPNode() || (op!=DivOp && op!=ModOp))
    {
      GenUnaryExpr(root,ca);
      return;
    }

  TreeNode *Exp0=root->GetLeftC();
  TreeNode *Exp1=root->GetRightC();

  Descriptor DesExp0, DesExp1;

  DesExp0=Exp0->GetDescriptor();
  DesExp1=Exp1->GetDescriptor();

  GenMultDivExpr(Exp0,ca);
  GenUnaryExpr(Exp1,ca);
  
  if (DesExp0==DES_INT && DesExp1==DES_INT)
    if (op==DivOp)
      ca->Gen(IDIV);
    else
      ca->Gen(IREM);
  else
    if (DesExp0==DES_LONG && DesExp1==DES_LONG)
      if (op==DivOp)
	ca->Gen(LDIV);
      else
	ca->Gen(LREM);
    else
      if (DesExp0==DES_FLOAT && DesExp1==DES_FLOAT)
	if (op==DivOp)
	  ca->Gen(FDIV);
	else
	  ca->Gen(FREM);
      else
	if (DesExp0==DES_DOUBLE && DesExp1==DES_DOUBLE)
	  if (op==DivOp)
	    ca->Gen(DDIV);
	  else
	    ca->Gen(DREM);
}

/*
 * CompileUnit::GenUnaryExpr
 *
 * CompileUnit::GenPreIncDecExpr
 * CompileUnit::GenUnaryAddExpr
 * CompileUnit::GenUnarySubExpr
 * CompileUnit::GenUnaryNotPlusMinusExpr
 *
 * Effettua la generazione di espressioni unarie, tipo:
 *
 * + <expr>; - <expr>; ++ <expr>; -- <expr>.
 *
 * In particolare gestiremo le operazioni di somma e sottrazione unaria,
 * preincremento e predecremento.
 * Per svolgere in pieno le sue funzioni, il metodo seguente utilizza altri 
 * 4 metodi che lo completano e che sono elencati sopra.
 */

void CompileUnit::GenUnaryExpr(TreeNode *root, Code_Attribute *ca)
{
  if (root->is_EXPNode())
    switch (((EXPNode *)root)->GetNodeOp())
      {
      case PlusPlusOp:
      case MinusMinusOp:
	if (!root->GetRightC()->IsDummy())
	  GenPreIncDecExpr(root,ca);
	else
	  if (!root->GetLeftC()->IsDummy())
	    GenPostIncDecExpr(root,ca);
	break;
      case UniPlusOp : GenUnaryAddExpr(root,ca);          break;
      case UniMinusOp: GenUnarySubExpr(root,ca);          break;
      default        : GenUnaryExprNotPlusMinus(root,ca); break;
      }
  else
    GenUnaryExprNotPlusMinus(root,ca);
}

void CompileUnit::GenPreIncDecExpr(TreeNode *root, Code_Attribute *ca)
{
  STNode *variable;
  int op=((EXPNode *)root)->GetNodeOp();
  Descriptor DesExp0;

  TreeNode   *Exp0=root->GetRightC();

  DesExp0=Exp0->GetDescriptor();

  if (Exp0->is_IDNode())
    variable=((IDNode *) Exp0)->GetIden();

  if (Exp0->is_IDNode && variable->is_local() && DesExp0==DES_INT)
    {
      if (op==PlusPlusOp)
	ca->Gen(IINC,variable->getlocalindex(),(char)1);
      else
	ca->Gen(IINC,variable->getlocalindex(),(char)-1);

      int index;

      if ((index=variable->getlocalindex())>=0 && index<=3)
	ca->Gen(INIT_LOAD+variable->getlocalindex());
      else
	ca->Gen(ILOAD,(_u1)variable->getlocalindex());
    }
  else
    {
      /*
       * - carico il valore della variabile da incrementare sul top dell'ope-
       *   rand stack.
       * - incremento il valore, conservandone una copia sul top dell'operand 
       *   stack.
       * - scarico il nuovo valore di nuovo nella variabile e il valore sullo
       *   operand stack e' pronto per essere utilizzato.
       *
       * Attenzione!!!
       *
       * La variabile da incrementare, puo' essere un campo, una variabile 
       * locale o un accesso a un componente di un array.       
       */
      

      if (Exp0->is_EXPNode() && ((EXPNode *)Exp0)->GetNodeOp()==FieldAccessOp)
	GenFieldAccess(Exp0,ca,TRUE);
      else
	GenUnaryExpr(Exp0,ca);

      // int offset;

      /*
       * ATTENZIONE!!!
       *
       * In base ai casi (quando si presenteranno), io dovro' utilizzare
       * tutte le istruzioni di DUP. 
       */

      if (DesExp0==DES_INT)
	if (op==PlusPlusOp)
	  {
	    ca->Gen(ICONST_1);
	    ca->Gen(IADD);
	    if (Exp0->is_EXPNode() && 
		((EXPNode *)Exp0)->GetNodeOp()==FieldAccessOp &&
	     !IS_STATIC(((IDNode *)Exp0->GetRightC())->GetIden()->getaccess()))
	      ca->Gen(DUP_X1);
	    else
	      ca->Gen(DUP);
	  }
	else
	  {
	    ca->Gen(ICONST_1);
	    ca->Gen(ISUB);
	    if (Exp0->is_EXPNode() && 
		((EXPNode *)Exp0)->GetNodeOp()==FieldAccessOp &&
	     !IS_STATIC(((IDNode *)Exp0->GetRightC())->GetIden()->getaccess()))
	      ca->Gen(DUP_X1);
	    else
	      ca->Gen(DUP);
	  }
      else
	if (DesExp0==DES_LONG)
	  if (op==PlusPlusOp)
	    {
	      ca->Gen(LCONST_1);
	      ca->Gen(LADD);
	      ca->Gen(DUP2);
	    }
	  else
	    {
	      ca->Gen(LCONST_1);
	      ca->Gen(LSUB);
	      ca->Gen(DUP2);
	    }
	else
	  if (DesExp0==DES_FLOAT)
	    if (op==PlusPlusOp)
	      {
		ca->Gen(FCONST_1);
		ca->Gen(FADD);
		ca->Gen(DUP);
	      }
	    else
	      {
		ca->Gen(FCONST_1);
		ca->Gen(FSUB);
		ca->Gen(DUP);
	      }      
	  else
	    if (DesExp0==DES_DOUBLE)
	      if (op==PlusPlusOp)
		{
		  ca->Gen(DCONST_1);
		  ca->Gen(DADD);
		  ca->Gen(DUP2);
		}
	      else
		{
		  ca->Gen(DCONST_1);
		  ca->Gen(DSUB);
		  ca->Gen(DUP2);
		}
      
      GenLValue(Exp0,ca);

    }
}

/*
 * CompileUnit::GenUnaryAddExpr
 *
 * Generare un'espressione del tipo: + <expr>; significa generare <expr> 
 * trascurando il segno piu' che non altera il significato dell'espressione.
 */

void CompileUnit::GenUnaryAddExpr(TreeNode *root, Code_Attribute *ca)
{
  GenUnaryExpr(root->GetRightC(),ca);
}

/*
 * CompileUnit::GenUnarySubExpr
 *
 * Genero espressioni di tipo: - <expr>; dove il tipo di <expr> deve essere
 * per forza (lo impone il parser) di tipo: double, float, long o int.
 */

void CompileUnit::GenUnarySubExpr(TreeNode *root, Code_Attribute *ca)
{
  Descriptor DesExp0;

  TreeNode *Exp0=root->GetRightC();

  DesExp0=Exp0->GetDescriptor();  

  GenUnaryExpr(Exp0,ca);

  if (DesExp0==DES_INT)
    ca->Gen(INEG);
  else
    if (DesExp0==DES_LONG)
      ca->Gen(LNEG);
    else
      if (DesExp0==DES_FLOAT)
	ca->Gen(FNEG);
      else
	if (DesExp0==DES_DOUBLE)
	  ca->Gen(DNEG);
}

/*
 * CompileUnit::GenUnaryExprNotPlusMinus
 *
 * Da qui inizia la generazione del codice per espressioni del tipo:
 * !<expr>, ~<expr>, espressioni di casting e espressioni postfisse.
 */

void CompileUnit::GenUnaryExprNotPlusMinus(TreeNode *root, Code_Attribute *ca)
{
  switch (((EXPNode *)root)->GetNodeOp())
    {
    case LogicalNotOp: GenLogicalNotExpr(root,ca); break;
    case BitwiseNotOp: GenBitwiseNotExpr(root,ca); break;
    case CastOp      : GenCastExpr(root,ca);       break;
    default          : GenPostFixExpr(root,ca);    break;
    }
}

/*
 * CompileUnit::GenBitwiseNotExpr
 *
 * Per eseguire il complemento a 1 di un'espressione, viene utilizzato il
 * seguente stratagemma. Viene posto -1 sul top dell'operand stack. -1 in
 * notazione complemento a 2 diventa 1111111..1, e poi si esegue lo xor con
 * il valore dell'espressione. Esempio:
 *
 * <expr>==1010  --->   1010 ^ 1111 == 0101
 */

void CompileUnit::GenBitwiseNotExpr(TreeNode *root, Code_Attribute *ca)
{
  Descriptor DesExp0;
  _u2 index;

  TreeNode *Exp0=root->GetRightC();

  DesExp0=Exp0->GetDescriptor();
  
  GenUnaryExpr(Exp0,ca);

  if (DesExp0==DES_INT)
    {
      ca->Gen(ICONST_M1);
      ca->Gen(IXOR);
    }      
  else
    {
      index=Class->Load_Constant_Long((long)-1);
      ca->Gen(LDC2_W, index);
      ca->Gen(LXOR);      
    }
}

void CompileUnit::GenCastExpr(TreeNode *root, Code_Attribute *ca)
{
  /*
   * Il cast per i tipi reference e' implicito, la cosa che pero' bisogna fare
   * e' generare un'istruzione di CHECKCAST, cosi' l'interprete potra' testare
   * a run-time la compatibilita' della conversione.
   */

  Descriptor DesExp0, DesCast;

  TreeNode *Exp0=root->GetRightC();

  DesExp0=Exp0->GetDescriptor();
  DesCast=root->GetDescriptor();

  /*
   * Quando compare in un file java un numero intero o reale, il tipo di 
   * default associato e', rispettivamente, int e double. Tuttavia, puo'
   * accadere che in realta', i due valori, nel contesto del programma
   * devono assumere un formato long o float. Per questo motivo aggiungiamo
   * alle normali conversioni due istruzioni if..then per controllare tali
   * casi.
   */

  if (Exp0->is_INUMNode() && DesCast.is_integral())
    {
      Exp0->SetDescriptor(DesCast);
      GenUnaryExpr(Exp0,ca);
      return;
    }
  else
    if (Exp0->is_FNUMNode() && !DesCast.is_integral())
      {
	Exp0->SetDescriptor(DesCast);
	GenUnaryExpr(Exp0,ca);
	return;
      }
    else
      if (Exp0->is_INUMNode() && !DesCast.is_integral())
	{
	  FNUMNode *node=new FNUMNode(((INUMNode *) Exp0)->GetVal());
	  node->SetDescriptor(DesCast);
	  delete Exp0;
	  Exp0=node;
	  root->SetRightC(node);
	  GenUnaryExpr(Exp0,ca);
	  return; 
	}

  GenUnaryExpr(Exp0,ca);

  if (DesExp0.is_reference() && DesCast.is_reference())
    {
      /*
       * NON GESTITO!!!
       *
       * Bisogna generare il CHECKCAST. Tralasciamo momentaneamente.
       */
    }
  else
    {
      if (DesExp0==DES_INT)
	if (DesCast==DES_BYTE)
	  ca->Gen(I2B);
	else
	  if (DesCast==DES_CHAR)
	    ca->Gen(I2C);
	  else
	    if (DesCast==DES_LONG)
	      ca->Gen(I2L);
	    else
	      if (DesCast==DES_FLOAT)
		ca->Gen(I2F);
	      else
		{
		  if (DesCast==DES_DOUBLE)
		    ca->Gen(I2D);
		}
      else
	if (DesExp0==DES_LONG)
	  if (DesCast==DES_INT)
	    ca->Gen(L2I);
	  else
	    if (DesCast==DES_FLOAT)
	      ca->Gen(L2F);
	    else
	      {
		if (DesCast==DES_DOUBLE)
		  ca->Gen(L2D);
	      }
	else
	  if (DesExp0==DES_FLOAT)
	    if (DesCast==DES_INT)
	      ca->Gen(F2I);
	    else
	      if (DesCast==DES_LONG)
		ca->Gen(F2L);
	      else
		{
		  if (DesCast==DES_DOUBLE)
		    ca->Gen(F2D);
		}
	  else
	    if (DesExp0==DES_DOUBLE)
	      if (DesCast==DES_INT)
		ca->Gen(D2I);
	      else
		if (DesCast==DES_LONG)
		  ca->Gen(D2L);
		else
		  if (DesCast==DES_FLOAT)
		    ca->Gen(D2F);
    }
}

/*
 * CompileUnit::GenLogicalNotExpr
 *
 * Questo metodo, piu' che generare codice, viene utilizzato per modificare
 * il parse-tree affinche' vengano calcolate espressioni del tipo !<expr>.
 * Il motivo per cui cio' viene fatto, e' che la JVM non prevede per tale ope-
 * razione un'istruzione specifica, quindi l'alternativa da noi pensata e'
 * quella di applicare semplici modifiche al parse-tree, tenendo conto di
 * semplici proprieta' algebriche come ad esempio quelle di De Morgan.
 *
 * Attenzione!!!
 *
 * Resta da gestire lo xor e l'assegnazione booleana.
 */

void CompileUnit::GenLogicalNotExpr(TreeNode *root, Code_Attribute *ca)
{
  TreeNode *etree, *s1, *s2;
  TreeNode *tree;

  if (root->GetRightC()->is_BOOLEANode())
    {
      if (((BOOLEANode *)root->GetRightC())->GetBoolean())
	ca->Gen(ICONST_0);
      else
	ca->Gen(ICONST_1);
    }
  else
    if (root->GetRightC()->is_EXPNode())
      {
	tree=root->GetRightC();

	switch (((EXPNode *)tree)->GetNodeOp())
	  {
	  case LtOp: ((EXPNode *)tree)->SetNodeOp(GeOp); break;
	  case GtOp: ((EXPNode *)tree)->SetNodeOp(LeOp); break;
	  case LeOp: ((EXPNode *)tree)->SetNodeOp(GtOp); break;
	  case GeOp: ((EXPNode *)tree)->SetNodeOp(LtOp); break;
	  case EqOp: ((EXPNode *)tree)->SetNodeOp(NeOp); break;
	  case NeOp: ((EXPNode *)tree)->SetNodeOp(EqOp); break;
	  case AndAndOp: 
	    ((EXPNode *)tree)->SetNodeOp(OrOrOp);
	    tree->SetLeftC(new EXPNode(LogicalNotOp,Dummy,tree->GetLeftC()));
	    tree->SetRightC(new EXPNode(LogicalNotOp,Dummy,tree->GetRightC()));
	    break;
	  case OrOrOp: 
	    ((EXPNode *)tree)->SetNodeOp(AndAndOp);
	    tree->SetLeftC(new EXPNode(LogicalNotOp,Dummy,tree->GetLeftC()));
	    tree->SetRightC(new EXPNode(LogicalNotOp,Dummy,tree->GetRightC()));
	    break;
	  case AndOp: 
	    ((EXPNode *)tree)->SetNodeOp(OrOp);
	    tree->SetLeftC(new EXPNode(LogicalNotOp,Dummy,tree->GetLeftC()));
	    tree->SetRightC(new EXPNode(LogicalNotOp,Dummy,tree->GetRightC()));
	    break;
	  case OrOp: 
	    ((EXPNode *)tree)->SetNodeOp(AndOp);
	    tree->SetLeftC(new EXPNode(LogicalNotOp,Dummy,tree->GetLeftC()));
	    tree->SetRightC(new EXPNode(LogicalNotOp,Dummy,tree->GetRightC()));
	    break;
	  case LogicalNotOp:
	    tree=tree->GetRightC();
	    break;
	  case CondExprOp:
	    etree=tree->GetLeftC()->GetLeftC();
	    s1=tree->GetLeftC()->GetRightC();
	    s2=tree->GetRightC();

	    tree->GetLeftC()->SetLeftC(new EXPNode(LogicalNotOp,Dummy,etree));
	    tree->GetLeftC()->SetRightC(s2);
	    tree->SetRightC(s1);
	    break;
	  case FieldAccessOp:
	    tree=root->GetRightC();
	    break;
	  case ArrayAccessOp:
	    tree=root->GetRightC();
	    break;
	  case MethodCallOp:
	    tree=root->GetRightC();
	    break;
	  case AssignOp: break;
	  case XorOp:    break;
	  }
	
	GenExpr(tree,ca);
	
	root->SetTrueList(tree->GetTrueList());
	root->SetFalseList(tree->GetFalseList());
	
	tree->SetTrueList(NULL);
	tree->SetFalseList(NULL);
      }
    else
      if (root->GetRightC()->is_IDNode())
	{
	  tree=root->GetRightC();
	  GenExpr(tree,ca);
	  ca->Gen(IFEQ,(_u2) 0);

	  BPList *truelist=new BPList;

	  truelist->Add(pc_count-3);

	  root->SetTrueList(truelist);
	}
}

/*
 * CompileUnit::GenPostFixExpr
 *
 * Da questo metodo inizia la generazione del codice per espressioni postfisse.
 * Ricordiamo che le espressioni postfisse, sono del tipo: 
 *
 * - post-incremento/decremento;
 * - accesso a variabili locali;
 * - accesso a campi;
 * - creazione di array;
 * - creazione classi;
 * - invocazione di metodi;
 * - accesso ad array;
 * - this;
 * - literal: costante intera, reale, booleana, carattere, stringa, null;
 * - espressione generica.
 *
 */

void CompileUnit::GenPostFixExpr(TreeNode *root, Code_Attribute *ca)
{
  int op;

  switch (root->GetNodeKind())
    {
    case _EXPNode:
      op=((EXPNode *)root)->GetNodeOp();
      
      switch (op)
	{
	case PlusPlusOp   : 
	case MinusMinusOp : GenPostIncDecExpr(root,ca);    break;
	case ArrayAccessOp: GenArrayAccess(root,ca);       break;
	case FieldAccessOp: GenFieldAccess(root,ca,FALSE); break;
	case MethodCallOp : GenMethodCall(root,ca,TRUE);   break;
	case NewOp        : GenNewClassInstance(root,ca,TRUE);  break;
	case NewArrayOp   : GenNewArray(root,ca);          break;
	default           : GenExpr(root,ca);              break;
	}
      break;

    case _IDNode    : GenLoadLocVar(root,ca);       break;
    case _INUMNode  : GenConstantInteger(root,ca);  break;
    case _FNUMNode  : GenConstantFloat(root,ca);    break;
    case _BOOLEANode: GenBoolean(root,ca);          break;
    case _CHARNode  : GenChar(root,ca);             break;
    case _STRINGNode: GenString(root,ca);           break;
    case _NULLNode  : ca->Gen(ACONST_NULL);         break;
    case _THISNode  : ca->Gen(ALOAD_0);             break; 
    case _SUPERNode : ca->Gen(ALOAD_0);             break;
    }
}

/*
 * CompileUnit::GenPostIncDecExpr
 *
 * Genera espressioni di post-incremento/decremento.
 */

void CompileUnit::GenPostIncDecExpr(TreeNode *root, Code_Attribute *ca)
{
  STNode *variable;
  int op=((EXPNode *)root)->GetNodeOp();
  Descriptor DesExp0;

  TreeNode   *Exp0=root->GetLeftC();

  DesExp0=Exp0->GetDescriptor();

  if (Exp0->is_IDNode())
    variable=((IDNode *) Exp0)->GetIden();

  if (Exp0->is_IDNode && variable->is_local() && DesExp0==DES_INT)
    {
      int index;

      if ((index=variable->getlocalindex())>=0 && index<=3)
	ca->Gen(INIT_LOAD+variable->getlocalindex());
      else
	ca->Gen(ILOAD,(_u1)variable->getlocalindex());

      if (op==PlusPlusOp)
	ca->Gen(IINC,variable->getlocalindex(),(char)1);
      else
	ca->Gen(IINC,variable->getlocalindex(),(char)-1);
    }
  else
    {
      /*
       * - carico il valore della variabile da incrementare sul top dell'ope-
       *   rand stack.
       * - conservo una copia
       * - incremento il valore, conservandone una copia sul top dell'operand 
       *   stack.
       * - scarico il nuovo valore di nuovo nella variabile e il valore sullo
       *   operand stack e' pronto per essere utilizzato.
       */

      GenUnaryExpr(Exp0,ca);

      /*
       * ATTENZIONE!!!
       *
       * Aggiungere ad ogni caso l'istruzione adeguata di DUP.
       */

      if (DesExp0==DES_INT)
	if (op==PlusPlusOp)
	  {
	    ca->Gen(DUP);
	    ca->Gen(ICONST_1);
	    ca->Gen(IADD);
	  }
	else
	  {
	    ca->Gen(DUP);
	    ca->Gen(ICONST_1);
	    ca->Gen(ISUB);
	  }
      else
	if (DesExp0==DES_LONG)
	  if (op==PlusPlusOp)
	    {
	      ca->Gen(DUP2);
	      ca->Gen(LCONST_1);
	      ca->Gen(LADD);
	    }
	  else
	    {
	      ca->Gen(DUP2);
	      ca->Gen(LCONST_1);
	      ca->Gen(LSUB);
	    }
	else
	  if (DesExp0==DES_FLOAT)
	    if (op==PlusPlusOp)
	      {
		ca->Gen(DUP);
		ca->Gen(FCONST_1);
		ca->Gen(FADD);
	      }
	    else
	      {
		ca->Gen(DUP);
		ca->Gen(FCONST_1);
		ca->Gen(FSUB);
	      }      
	  else
	    if (DesExp0==DES_DOUBLE)
	      if (op==PlusPlusOp)
		{
		  ca->Gen(DUP2);
		  ca->Gen(DCONST_1);
		  ca->Gen(DADD);
		}
	      else
		{
		  ca->Gen(DUP2);
		  ca->Gen(DCONST_1);
		  ca->Gen(DSUB);
		}
      GenLValue(Exp0,ca);
    }
}

/*
 * CompileUnit::GenArrayAccess
 *
 * Qui noi gestiremo l'accesso a componenti di un array, affinche' possa
 * essere caricato sull'operand stack il valore di un suo componente.
 */

void CompileUnit::GenArrayAccess(TreeNode *root, Code_Attribute *ca)
{
  Descriptor descriptor;

  GenExpr(root->GetLeftC(),ca);

  GenExpr(root->GetRightC(),ca);

  descriptor=root->GetDescriptor();

  if (descriptor.is_reference()) { ca->Gen(AALOAD); return; }
  if (descriptor==DES_BYTE)      { ca->Gen(BALOAD); return; }
  if (descriptor==DES_CHAR)      { ca->Gen(CALOAD); return; }
  if (descriptor==DES_SHORT)     { ca->Gen(SALOAD); return; }
  if (descriptor==DES_INT)       { ca->Gen(IALOAD); return; }
  if (descriptor==DES_LONG)      { ca->Gen(LALOAD); return; }
  if (descriptor==DES_FLOAT)     { ca->Gen(FALOAD); return; }
  if (descriptor==DES_DOUBLE)    { ca->Gen(DALOAD); return; }
}

/*
 * CompileUnit::GenFieldAccess
 *
 * Gestisce il caricamento di un valore da un campo al top dell'operand stack.
 */

void CompileUnit::GenFieldAccess(TreeNode *root, Code_Attribute *ca, 
				 int is_dup)
{
  String NameClass, NameField;
  Descriptor descriptor;

  STNode *Id=((IDNode *)root->GetRightC())->GetIden();

  _u2 index;

  if (root->GetLeftC()->GetDescriptor().is_array() && Id->getname()=="length")
    {
      GenPostFixExpr(root->GetLeftC(),ca);
      if (is_dup) ca->Gen(DUP);
      ca->Gen(ARRAYLENGTH);
      return;
    }

  NameClass=root->GetLeftC()->GetDescriptor().to_fullname();

  String key;

  key=NameClass+","+Id->getname()+","+Id->getdescriptor();

  index=Class->Load_Fieldref(key);

  if (IS_STATIC(Id->getaccess()))
    ca->Gen(GETSTATIC,index);
  else
    {
      GenPostFixExpr(root->GetLeftC(),ca);
      if (is_dup) ca->Gen(DUP);
      ca->Gen(GETFIELD,index);
    }
}

/*
 * CompileUnit::GenMethodCall
 *
 * Genero il codice necessario per l'invocazione di un metodo. Per codificare
 * una chiamata a metodo, utilizzeremo le seguenti istruzioni:
 *
 * - INVOKEINTERFACE, per invocazioni di metodi di interfacce;
 * - INVOKESPECIAL, per invocazioni di metodi privati, di metodi della super-
 *                  classe o per costruttori;
 * - INVOKESTATIC, per invocazioni di metodi statici;
 * - INVOKEVIRTUAL, per l'invocazione di metodi virtual.
 *
 * Per prima cosa noi genereremo il codice per i vari argomenti, facendone 
 * il push sull'operand-stack, dopo di che', a seconda dei casi, invokeremo
 * il nome del metodo.
 */

void CompileUnit::GenMethodCall(TreeNode *root, Code_Attribute *ca, int rvalue)
{
  STNode *meth=((IDNode *)root->GetLeftC()->GetRightC())->GetIden();
  STNode *class_meth=meth->getmyclass();

  _u1 nargs=1;

  if (!IS_STATIC(meth->getaccess()))
    GenPostFixExpr(root->GetLeftC()->GetLeftC(),ca);

  /*
   * Faccio il push sull'operand-stack dei valori degli argomenti.
   */

  for (TreeNode *arg=root->GetRightC(); !arg->IsDummy(); arg=arg->GetRightC())
    {
      TreeNode *expr=arg->GetLeftC();

      GenExpr(expr,ca);

      if (expr->GetDescriptor()==DES_BOOLEAN && 
	  (expr->GetTrueList() || expr->GetFalseList()))
	{
	  ca->Gen(ICONST_0);
	  ca->Gen(GOTO,(_u2)(pc_count+3));
	  ca->Gen(ICONST_1);
	  backpatch(ca,expr->GetTrueList(),pc_count-1);
	  backpatch(ca,expr->GetFalseList(),pc_count-5);

	  delete expr->GetTrueList();
	  delete expr->GetFalseList();

	  expr->SetTrueList(NULL);
	  expr->SetFalseList(NULL);
	}

      nargs++;
    }

  _u2 k;
  String key;

  key=class_meth->getfullname()+","+meth->getname()+","+meth->getdescriptor();

  if (class_meth->is_class())
    k=Class->Load_Methodref(key);
  else
    k=Class->Load_IntfMethodref(key);

  /*
   * INVOKESPECIAL per costruttori viene gestito in GenNewClassInstance
   */

  if (IS_STATIC(meth->getaccess()))
    ca->Gen(INVOKESTATIC,k);
  else
    if (class_meth->is_interface())
      {
	ca->Gen(INVOKEINTERFACE,k);
	ca->Gen(nargs);
	ca->Gen(0);
      }
    else
      if (meth->getname()=="<init>")
      /*      if (IS_PRIVATE(meth->getaccess())          || 
	  current_class->is_subclass(class_meth))*/
	ca->Gen(INVOKESPECIAL,k);
      else
      	ca->Gen(INVOKEVIRTUAL,k);

  /*
   * Se il metodo non e' usato in una espressione e restituisce un valore, 
   * allora questi va tolto dall'operand stack, perche' non sara' mai utiliz-
   * zato.
   */

  Descriptor DesReturn;
  
  DesReturn=meth->getdescriptor().return_type();

  if (!rvalue && DesReturn!=DES_VOID)
    {

      if (DesReturn==DES_LONG || DesReturn==DES_DOUBLE)
	ca->Gen(POP2);
      else
	ca->Gen(POP);
    }
}

/*
 * CompileUnit::GenNewClassInstance
 *
 * Genero il codice per la creazione di un' istanza per una classe.
 */

void CompileUnit::GenNewClassInstance(TreeNode *root, Code_Attribute *ca,
				      int is_expr)
{
  STNode *meth=((IDNode *)root->GetLeftC()->GetRightC())->GetIden();
  STNode *class_meth=meth->getmyclass();

  String key;

  _u2 i=Class->Load_Constant_Class(class_meth->getfullname());

  ca->Gen(NEW,i);

  if (is_expr)
    ca->Gen(DUP);

  key=class_meth->getfullname()+","+meth->getname()+","+meth->getdescriptor();

  _u2 k=Class->Load_Methodref(key);

  /*
   * Faccio il push sull'operand-stack dei valori degli argomenti.
   */

  for (TreeNode *arg=root->GetRightC(); !arg->IsDummy(); arg=arg->GetRightC())
    {
      TreeNode *expr=arg->GetLeftC();
      
      GenExpr(expr,ca);
      
      if (expr->GetDescriptor()==DES_BOOLEAN && 
	  (expr->GetTrueList()|| expr->GetFalseList()))
	{
	  ca->Gen(ICONST_0);
	  ca->Gen(GOTO,(_u2)(pc_count+3));
	  ca->Gen(ICONST_1);
	  backpatch(ca,expr->GetTrueList(),pc_count-1);
	  backpatch(ca,expr->GetFalseList(),pc_count-5);
	  
	  delete expr->GetTrueList();
	  delete expr->GetFalseList();

	  expr->SetTrueList(NULL);
	  expr->SetFalseList(NULL);
	}
    }

  ca->Gen(INVOKESPECIAL,k);
}

/*
 * CompileUnit::GenNewArray
 * CompileUnit::GenArrayDimension
 *
 * Genera il codice necessario per la creazione di array.
 */

void CompileUnit::GenNewArray(TreeNode *root, Code_Attribute *ca)
{
  int dimension=0;
  _u2 index;

  // calcoliamo la dimensionalita' dell'array.

  for (TreeNode *t=root->GetRightC(); !t->IsDummy(); t=t->GetLeftC())
    dimension++;

  if (root->GetLeftC()->IsDummy() && dimension==1)
    {
      /*
       * Si deve creare un array di dimensione unitaria di tipo primitive-type.
       */

      Descriptor descriptor;

      descriptor=root->GetRightC()->GetRightC()->GetDescriptor();

      if (descriptor==DES_DOUBLE) { index=7; goto label;  }
      if (descriptor==DES_FLOAT)  { index=6; goto label;  }
      if (descriptor==DES_LONG)   { index=11; goto label; }
      if (descriptor==DES_INT)    { index=10; goto label; }
      if (descriptor==DES_SHORT)  { index=9; goto label;  }
      if (descriptor==DES_CHAR)   { index=5; goto label;  }
      if (descriptor==DES_BYTE)   { index=8; goto label;  }
      if (descriptor==DES_BOOLEAN){ index=4; goto label;  }

    label:

      GenExpr(root->GetRightC()->GetRightC(),ca);

      ca->Gen(NEWARRAY,(_u1)index);
    }
  else
    if (!root->GetLeftC()->IsDummy() && dimension==1)
      {
	/*
	 * Si deve creare un array di dimensione unitaria di tipo reference.
	 */

	String Name;

	Name=((IDNode *)root->GetLeftC())->GetIden()->getfullname();
	index=Class->Load_Constant_Class(Name);
	GenExpr(root->GetRightC()->GetRightC(),ca);
	ca->Gen(ANEWARRAY,index);
      }
    else
      {
	/*
	 * Si deve creare un array multidimensionale.
	 */

	GenArrayDimension(root->GetRightC(),ca);
	index=Class->Load_Constant_Class(root->GetDescriptor());
	ca->Gen(MULTIANEWARRAY,index,dimension);
      }
}

void CompileUnit::GenArrayDimension(TreeNode *root, Code_Attribute *ca)
{
  if (!root->IsDummy())
    {
      GenArrayDimension(root->GetLeftC(),ca);
      
      GenExpr(root->GetRightC(),ca);
    }
}

/*
 * CompileUnit::GenLoadLocVar
 *
 * Carica il valore di una variabile locale sul top dell'operand stack.
 */

void CompileUnit::GenLoadLocVar(TreeNode *root, Code_Attribute *ca)
{
  _u1 offset, localindex;
  Descriptor descriptor;
  STNode *Id=((IDNode *)root)->GetIden();

  descriptor=Id->getdescriptor();
  
  localindex=Id->getlocalindex();

  if (descriptor==DES_INT  || descriptor==DES_CHAR  || 
      descriptor==DES_BYTE || descriptor==DES_SHORT || 
      descriptor==DES_BOOLEAN)       
    { offset=0; goto label; }
  if (descriptor==DES_LONG)      { offset=1; goto label; }
  if (descriptor==DES_FLOAT)     { offset=2; goto label; }
  if (descriptor==DES_DOUBLE)    { offset=3; goto label; }
  if (descriptor.is_reference()) { offset=4; goto label; }

label:

  switch (localindex)
    {
    case 0:
    case 1:
    case 2:
    case 3:
      ca->Gen(INIT_LOAD+offset*4+localindex);
      break;
    default:
      ca->Gen(INIT_LOAD_INDEX+offset,localindex);
      break;
    }
}

/*
 * CompileUnit::GenConstantInteger
 *
 * Genera il codice per far caricare una costante intera sul top delloperand 
 * stack.
 */

void CompileUnit::GenConstantInteger(TreeNode *root, Code_Attribute *ca)
{
  _u2 index;
  long valore=((INUMNode *)root)->GetVal();

  if (root->GetDescriptor()==DES_INT || root->GetDescriptor()==DES_SHORT ||
      root->GetDescriptor()==DES_BYTE)
    {
      /*
       * La costante e' di tipo intera.
       */

      if (valore>=-1 && valore <=5) 
	{ 
	  ca->Gen((_u1)(ICONST_0+valore));
	  return; 
	}
      if (valore>=-127 && valore <=127) 
	{ 
	  ca->Gen(BIPUSH,(_u1)valore); 
	  return;    
	}
      if (valore>=-32767 && valore <=32767) 
	{ 
	  ca->Gen(SIPUSH,(_u2)valore); 
	  return;    
	}
      
      index=Class->Load_Constant_Integer(valore);

      /*
       * Se index non e' rappresentabile con 8 bit, utilizzare LDC_W.
       */

      ca->Gen(LDC, (_u1) index);
    }
  else
    {
      /*
       * La costante e' di tipo long.
       */

      if (valore==0 || valore==1) 
	{ 
	  ca->Gen((_u1)(LCONST_0+valore)); 
	  return; 
	}
      index=Class->Load_Constant_Long(valore);
      ca->Gen(LDC2_W, index);
    }
}

/*
 * CompileUnit::GenConstantFloat
 *
 * Genera il codice per far caricare una costante intera sul top delloperand 
 * stack.
 */

void CompileUnit::GenConstantFloat(TreeNode *root, Code_Attribute *ca)
{
  _u2 index;
  double valore=((FNUMNode *)root)->GetVal();

  if (root->GetDescriptor()==DES_FLOAT)
    {
      /*
       * La costante e' di tipo float.
       */

      if (valore==0 || valore==1 || valore==2) 
	{ 
	  ca->Gen((_u1)(FCONST_0+valore)); 
	  return; 
	}

      index=Class->Load_Constant_Float(valore);

      /*
       * Se index non e' rappresentabile con 8 bit, utilizzare LDC_W.
       */

      ca->Gen(LDC, (_u1) index);
    }
  else
    {
      /*
       * La costante e' di tipo double.
       */

      if (valore==0 || valore==1) 
	{ 
	  ca->Gen((_u1)(DCONST_0+valore)); 
	  return; 
	}
      index=Class->Load_Constant_Double(valore);
      ca->Gen(LDC2_W, index);
    }
}

/*
 * CompileUnit::GenBoolean
 *
 * Genera il codice per far caricare una costante intera sul top delloperand 
 * stack.
 */

void CompileUnit::GenBoolean(TreeNode *root, Code_Attribute *ca)
{
  if (((BOOLEANode *)root)->GetBoolean())
    ca->Gen(ICONST_1);
  else
    ca->Gen(ICONST_0);
}

/*
 * CompileUnit::GenChar
 *
 * Genera il codice affinche' un carattere venga caricato sul top delloperand 
 * stack.
 */

void CompileUnit::GenChar(TreeNode *root, Code_Attribute *ca)
{
  _u2 index;
  long valore=((CHARNode *)root)->GetCharacter();


  if (valore>=0 && valore <=5) 
    ca->Gen((_u1)(ICONST_0+valore));
  else
    if (valore>=6 && valore <=255) 
      ca->Gen(BIPUSH,(_u1)valore);
    else
      {
	index=Class->Load_Constant_Integer(valore);
  
	/*
	 * Se index non e' rappresentabile con 8 bit, utilizzare LDC_W.
	 */
	
	ca->Gen(LDC, (_u1) index);
      }
}

/*
 * CompileUnit::GenString
 *
 * Genera il codice affinche' una stringa venga caricata sul top dell'operand 
 * stack.
 */

void CompileUnit::GenString(TreeNode *root, Code_Attribute *ca)
{
  String str;
  _u2 index;

  str=((STRINGNode *)root)->GetString();

  index=Class->Load_Constant_String(str);
  
  /*
   * Se index non e' rappresentabile con 8 bit, utilizzare LDC_W.
   */
  
  ca->Gen(LDC, (_u1) index);
}

/*
 * CompileUnit::GenAssignment
 *
 * Genera un'istruzione di assegnamento semplice.
 */

void CompileUnit::GenAssignment(TreeNode *root, Code_Attribute *ca)
{
  /*
   * Se l'espressione di sinistra (accesso a campo, array o variabile locale)
   * non e' un riferimento statico, prima si genera l'oggetto in cui e'
   * presente l'entita' lvalue. Poi bisogna generare l'spressione di destra, 
   * cosi' che il risultato venga posto sul top dell'operand stack. Poi si 
   * valuta l'espressione di sinistra.
   * Se l'espressione di destra e' di tipo boolean ed e' almeno attiva una
   * delle liste di backpatching, allora bisogna caricarle e gestirle in modo
   * adeguato.
   */

  TreeNode *Exp0=root->GetLeftC();
  TreeNode *Exp1=root->GetRightC();

  if (Exp0->is_EXPNode() && ((EXPNode *)Exp0)->GetNodeOp()==FieldAccessOp &&
      !IS_STATIC(((IDNode *)Exp0->GetRightC())->GetIden()->getaccess()))
    GenPostFixExpr(Exp0->GetLeftC(),ca);

  /*
   * NON GESTITO!!!
   *
   * GenPostFixExpr non va eseguito se l'array e' statico. Per ora non
   * riesco a fare un controllo semplice come visto sopra per i campi.
   */

  if (Exp0->is_EXPNode() && ((EXPNode *)Exp0)->GetNodeOp()==ArrayAccessOp)
    {
      GenPostFixExpr(Exp0->GetLeftC(),ca);
      GenExpr(Exp0->GetRightC(),ca);
    }

  GenExpr(Exp1,ca);

  if (Exp1->GetDescriptor()==DES_BOOLEAN && 
      (Exp1->GetTrueList() || Exp1->GetFalseList()))
    {
      ca->Gen(ICONST_0);
      ca->Gen(GOTO,(_u2) 4);
      ca->Gen(ICONST_1);
      backpatch(ca,Exp1->GetTrueList(),pc_count-1);
      backpatch(ca,Exp1->GetFalseList(),pc_count-5);

      delete Exp1->GetTrueList();
      delete Exp1->GetFalseList();

      Exp1->SetTrueList(NULL);
      Exp1->SetFalseList(NULL);
    }
  
  GenLValue(Exp0,ca);
}

/*
 * CompileUnit::GenCompAssignment
 *
 * Genera istruzioni di assegnazioni con operandi: +=, -= *=, /=, %=, &=, |=
 * ^=.
 * Nel gestire questo tipo di espressione, bisogna ricordarsi che
 *
 *                      <expr1> op= <expr1>
 * 
 * e' equivalente a <expr1>=<expr1> op <expr2>.
 */

void CompileUnit::GenCompAssignment(TreeNode *root, Code_Attribute *ca)
{
  TreeNode *Exp0=root->GetLeftC();
  TreeNode *Exp1=root->GetRightC();

  if (Exp0->is_EXPNode() && ((EXPNode *)Exp0)->GetNodeOp()==FieldAccessOp &&
      !IS_STATIC(((IDNode *)Exp0->GetRightC())->GetIden()->getaccess()))
    {
      GenPostFixExpr(Exp0->GetLeftC(),ca);
      ca->Gen(DUP);
    }

  /*
   * NON GESTITO!!!
   *
   * GenPostFixExpr non va eseguito se l'array e' statico. Per ora non
   * riesco a fare un controllo semplice come visto sopra per i campi.
   */

  if (Exp0->is_EXPNode() && ((EXPNode *)Exp0)->GetNodeOp()==ArrayAccessOp)
    {
      GenPostFixExpr(Exp0->GetLeftC(),ca);
      ca->Gen(DUP);
      GenExpr(Exp0->GetRightC(),ca);
    }

  GenExpr(Exp1,ca);

  if (Exp1->GetDescriptor()==DES_BOOLEAN && 
      (Exp1->GetTrueList() || Exp1->GetFalseList()))
    {
      ca->Gen(ICONST_0);
      ca->Gen(GOTO,(_u2) 4);
      ca->Gen(ICONST_1);
      backpatch(ca,Exp1->GetTrueList(),pc_count-1);
      backpatch(ca,Exp1->GetFalseList(),pc_count-5);

      delete Exp1->GetTrueList();
      delete Exp1->GetFalseList();

      Exp1->SetTrueList(NULL);
      Exp1->SetFalseList(NULL);
    }
  
  GenLValue(Exp0,ca);
}

/*****************************************************************************
 * Generazione Espressioni LValue                                            *
 *****************************************************************************/

/*
 * CompileUnit::GenLValue
 *
 * Genero codice per un'espressione lvalue. Ricordiamo che in teoria dei 
 * compilatori un'espressione e' rvalue quando il risultato dell'espressione e'
 * un valore (che nel nostro caso si trova sull'operand-stack), mentre una
 * espressione lvalue e' un'espressione che ha come risultato una variabile
 * (nel nostro caso un valore da memorizzare in un array, in un campo o in una
 * variabile locale).
 */

void CompileUnit::GenLValue(TreeNode *root, Code_Attribute *ca)
{
  if (root->is_IDNode())
    GenLValueLocal(root,ca);
  else
    if (((EXPNode *)root)->GetNodeOp()==FieldAccessOp)
      GenLValueField(root,ca);
    else
      GenLValueArray(root,ca);
}

/*
 * CompileUnit::GenLValueLocal
 *
 * Genero codice affinche' il valore sul top dell'operand-stack venga memoriz-
 * zato in una variabile locale.
 */

void CompileUnit::GenLValueLocal(TreeNode *root, Code_Attribute *ca)
{
  _u1 offset, localindex;
  Descriptor descriptor;
  STNode *Id=((IDNode *)root)->GetIden();

  /*
   * Sicuramente la variabile locale si trova su stack frame, quindi non
   * e' necessario controllare se e' gia' stata allocata.
   */

  descriptor=Id->getdescriptor();

  localindex=Id->getlocalindex();

  if (descriptor==DES_INT  || descriptor==DES_BOOLEAN || 
      descriptor==DES_CHAR || descriptor==DES_SHORT   || 
      descriptor==DES_BYTE)
    { offset=0; goto label; }
  if (descriptor==DES_LONG)      { offset=1; goto label; }
  if (descriptor==DES_FLOAT)     { offset=2; goto label; }
  if (descriptor==DES_DOUBLE)    { offset=3; goto label; }
  if (descriptor.is_reference()) { offset=4; goto label; }

label:

  switch (localindex)
    {
    case 0:
    case 1:
    case 2:
    case 3:
      ca->Gen(INIT_STORE+offset*4+localindex);
      break;
    default:
      ca->Gen(INIT_STORE_INDEX+offset,localindex);
      break;
    }
}

/*
 * CompileUnit::GenLValueField
 *
 * Genero codice affinche' il valore sul top dell'operand-stack venga memoriz-
 * zato in un campo.
 */ 

void CompileUnit::GenLValueField(TreeNode *root, Code_Attribute *ca)
{
  String NameClass, key;
  Descriptor descriptor;

  STNode *Id=((IDNode *)root->GetRightC())->GetIden();

  _u2 index;

  NameClass=root->GetLeftC()->GetDescriptor().to_fullname();

  key=NameClass+","+Id->getname()+","+Id->getdescriptor();

  index=Class->Load_Fieldref(key);

  if (IS_STATIC(Id->getaccess()))
    ca->Gen(PUTSTATIC,index);
  else
    ca->Gen(PUTFIELD,index);  
}

/*
 * CompileUnit::GenArrayAccess
 * CompileUnit::GenArrayIndex
 *
 * Qui noi gestiremo l'accesso a componenti di un array, affinche' possa
 * essere caricato sull'operand stack il valore di un suo componente.
 */

void CompileUnit::GenLValueArray(TreeNode *root, Code_Attribute *ca)
{
  Descriptor descriptor;

  descriptor=root->GetDescriptor();

  if (descriptor.is_reference()) { ca->Gen(AASTORE); return; }
  if (descriptor==DES_BYTE)      { ca->Gen(BASTORE); return; }
  if (descriptor==DES_CHAR)      { ca->Gen(CASTORE); return; }
  if (descriptor==DES_SHORT)     { ca->Gen(SASTORE); return; }
  if (descriptor==DES_INT)       { ca->Gen(IASTORE); return; }
  if (descriptor==DES_LONG)      { ca->Gen(LASTORE); return; }
  if (descriptor==DES_FLOAT)     { ca->Gen(FASTORE); return; }
  if (descriptor==DES_DOUBLE)    { ca->Gen(DASTORE); return; }
}
