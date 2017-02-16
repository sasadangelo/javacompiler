/*
 * file expr.cc
 * 
 * descrizione: questo file, insieme a compile.cc e parse.cc, implementa la
 *              classe CompileUnit. In particolare, in questo file troviamo i 
 *              metodi che si occuperanno del parsing di espressioni.
 *
 * Questo file e' parte del Compilatore Java.
 *
 * Maggio 1997, scritto da Salvatore D'Angelo e-mail xc0261@xcom.it.
 */

#include <math.h>
#include <stdio.h>
#include <globals.h>
#include <cstring.h>
#include <descriptor.h>
#include <hash.h>
#include <table.h>
#include <local.h>
#include <compile.h>
#include <tree.h>
#include <errors.h>
#include <environment.h>
#include <lex.h>
#include <access.h>

extern int      Nest_lev;
extern int      LastIndex;
extern int      yylineno;
extern TreeNode *Dummy;
extern char     *msg_errors[];
extern String   init_name;
extern STNode   *current_class;

extern Descriptor DesBoolean;
extern Descriptor DesString;
extern Descriptor DesInt;
extern Descriptor DesNull;
extern Descriptor DesVoid;

String ArrayLengthName;
Descriptor ArrayLengthDescriptor;
STNode *ArrayLengthNode;

/*
 * CompileUnit::ParseExpr
 *
 * Metodo base da cui parte il parsing di qualsiasi espressione.
 */

TreeNode *CompileUnit::ParseExpr(TreeNode *root)
{
  /*
   * L'espressione puo' essere o un'assegnazione o un'istruzione "(expr) ? :".
   */

  if (root->is_EXPNode())
    {
      int op=((EXPNode *)root)->GetNodeOp();

      if (op==AssignOp)
	return ParseAssignment(root);
      if (op==AssignMultOp || op==AssignDivOp   || op==AssignModOp    || 
	  op==AssignPlusOp || op==AssignMinusOp || op==AssignAndOp    ||
	  op==AssignOrOp   || op==AssignXorOp   || op==AssignLShiftOp ||
	  op==AssignRShiftOp || op==AssignURShiftOp)
	return ParseCompAssignment(root);
      else
	return ParseCondExpr(root);
    }
  
  return ParseCondExpr(root);

}

/*
 * CompileUnit::ParseCondExpr
 *
 * Esegue il parsing di un'espressione del tipo "<expr> ? <expr> : <condexpr>".
 */

TreeNode *CompileUnit::ParseCondExpr(TreeNode *root)
{
  if (!root->is_EXPNode() || ((EXPNode *)root)->GetNodeOp()!=CondExprOp)
    return ParseCondOrExpr(root);

  TreeNode   *Exp0=root->GetLeftC()->GetLeftC();
  TreeNode   *Exp1=root->GetLeftC()->GetRightC();
  TreeNode   *Exp2=root->GetRightC();
  Descriptor DesReturn;

  root->GetLeftC()->SetLeftC(Dummy);
  root->GetLeftC()->SetRightC(Dummy);
  root->SetRightC(Dummy);

  Exp0=ParseCondOrExpr(Exp0);
  Exp1=ParseExpr(Exp1);
  Exp2=ParseCondExpr(Exp2);

  Descriptor DesExp0;
  Descriptor DesExp1;
  Descriptor DesExp2;
  
  DesExp0=Exp0->GetDescriptor();
  DesExp1=Exp1->GetDescriptor();
  DesExp2=Exp2->GetDescriptor();

  if (DesExp0!=DES_BOOLEAN)
    MsgErrors(Exp0->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"?:",
	      DesExp0.to_typename(),"boolean");
  
  if (DesExp1.is_primitive() && DesExp2.is_primitive())
    if (DesExp1==DesExp2)
      DesReturn=DesExp2;
    else
      if ((DesExp1==DES_BYTE && DesExp2==DES_SHORT))
	{
	  DesReturn=DES_SHORT;
	  Exp1=new EXPNode(CastOp,Dummy,Exp1);
	  Exp1->SetDescriptor(DesReturn);
	}
      else
	if (DesExp1==DES_SHORT && DesExp2==DES_BYTE)
	  {
	    DesReturn=DES_SHORT;
	    Exp2=new EXPNode(CastOp,Dummy,Exp2);
	    Exp2->SetDescriptor(DesReturn);
	  }
	else
	  {
	    /*
	     * NON GESTITO!!!
	     *
	     * Se il tipo di Exp2 e' un intero costante, con valore
	     * rappresentabile in short, char, byte, allora avverra'
	     * una restrizione di tipo.
	     */
	    
	    DesReturn=BArithmetic_Promotion(DesExp1,DesExp2);
	    if (DesReturn!=DesExp1)
	      {
		Exp1=new EXPNode(CastOp,Dummy,Exp1);
		Exp1->SetDescriptor(DesReturn);
	      }
	    else
	      if (DesReturn!=DesExp2)
		{
		  Exp2=new EXPNode(CastOp,Dummy,Exp2); 
		  Exp2->SetDescriptor(DesReturn);
		}
	  }
  else
    if (DesExp1==DES_BOOLEAN && DesExp2==DES_BOOLEAN)
      DesReturn=DES_BOOLEAN;
    else
      {
	if (Exp1->is_NULLNode() && Exp2->is_NULLNode())
	  {
	    delete root;
	    delete Exp0;
	    delete Exp1;
	    return Exp2;
	  }
	if (Exp1->is_NULLNode() && DesExp2.is_reference())
	  {
	    Exp1->SetDescriptor(DesExp2);
	    DesReturn=DesExp2;
	  }
	else
	  if (Exp2->is_NULLNode() && DesExp1.is_reference())
	    { 
	      Exp2->SetDescriptor(DesExp1);
	      DesReturn=DesExp1;
	    }
	  else
	    if (DesExp1.is_reference() && DesExp2.is_reference())
	      if (is_Assign_Conversion(DesExp1,DesExp2))
		{
		  DesReturn=DesExp1;
		  if (DesExp1!=DesExp2)
		    {
		      Exp2=new EXPNode(CastOp,Dummy,Exp2);
		      Exp2->SetDescriptor(DesReturn);
		    }
		}
	      else
		if (is_Assign_Conversion(DesExp2,DesExp1))
		  {
		    DesReturn=DesExp2;
		    if (DesExp1!=DesExp2)
		      {
			Exp1=new EXPNode(CastOp,Dummy,Exp1);
			Exp1->SetDescriptor(DesReturn);
		      }
		  }
		else
		  {
		    MsgErrors(root->GetLine(),msg_errors[ERR_INVALID_CAST],
			      DesExp1.to_typename(),DesExp2.to_typename());
		    DesReturn=DES_NULL;
		  }
      }
  if (Exp0->is_BOOLEANode())
    if (((BOOLEANode *)Exp0)->GetBoolean())
      {
	delete root;
	delete Exp0;
	delete Exp2;
	return Exp1;
      }
    else
      {
	delete root;
	delete Exp0;
	delete Exp1;
	return Exp2;
      }
  else
    {
      /*
       * ATTENZIONE!!!
       *
       * non dimenticare di eliminare il nodo root e CommaOp.
       */

      root->GetLeftC()->SetLeftC(Exp0);
      root->GetLeftC()->SetRightC(Exp1);
      root->SetRightC(Exp2);
      root->SetDescriptor(DesReturn);
      return root;
    }
}

/*
 * CompileUnit::ParseCondOrExpr
 * 
 * Parsing di un'espressione del tipo <expr> || <expr>
 */

TreeNode *CompileUnit::ParseCondOrExpr(TreeNode *root)
{
  if (!root->is_EXPNode() || ((EXPNode *)root)->GetNodeOp()!=OrOrOp)
    return ParseCondAndExpr(root);

  TreeNode   *Exp0=root->GetLeftC();
  TreeNode   *Exp1=root->GetRightC();

  root->SetLeftC(Dummy);
  root->SetRightC(Dummy);

  Exp0=ParseCondOrExpr(Exp0);
  Exp1=ParseCondAndExpr(Exp1);

  Descriptor DesExp0;
  Descriptor DesExp1;
  
  DesExp0=Exp0->GetDescriptor();
  DesExp1=Exp1->GetDescriptor();

  if (DesExp0!=DES_BOOLEAN)
    MsgErrors(Exp0->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"||",
	      DesExp0.to_typename(),"boolean");

  if (DesExp1!=DES_BOOLEAN)
    MsgErrors(Exp1->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"||",
	      DesExp1.to_typename(),"boolean");

  /*
   * Applichiamo il "short circuits" a tempo di compilazione.
   */
  
  if (Exp0->is_BOOLEANode() && ((BOOLEANode *)Exp0)->GetBoolean())
    {
      delete root;
      delete Exp1;
      return Exp0;
    }

  if (Exp0->is_BOOLEANode() && !((BOOLEANode *)Exp0)->GetBoolean())
    {
      delete root;
      delete Exp0;
      return Exp1;
    }

  /*
   * Applico una semplice ottimizzazione di "costant folding".
   */

  if (Exp1->is_BOOLEANode() && !((BOOLEANode *)Exp1)->GetBoolean())
    {
      delete root;
      delete Exp1;
      return Exp0;
    }

  root->SetLeftC(Exp0);
  root->SetRightC(Exp1);
  root->SetDescriptor(DesBoolean);
  return root;
}

/*
 * CompileUnit::ParseCondAndExpr
 *
 * Parsing di espressioni del tipo <expr> && <expr>
 */

TreeNode *CompileUnit::ParseCondAndExpr(TreeNode *root)
{
  if (!root->is_EXPNode() || ((EXPNode *)root)->GetNodeOp()!=AndAndOp)
    return ParseIorExpr(root);

  TreeNode   *Exp0=root->GetLeftC();
  TreeNode   *Exp1=root->GetRightC();

  root->SetLeftC(Dummy);
  root->SetRightC(Dummy);

  Exp0=ParseCondAndExpr(Exp0);
  Exp1=ParseIorExpr(Exp1);

  Descriptor DesExp0;
  Descriptor DesExp1;

  DesExp0=Exp0->GetDescriptor();
  DesExp1=Exp1->GetDescriptor();

  if (DesExp0!=DES_BOOLEAN)
    MsgErrors(Exp0->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"&&",
	      DesExp0.to_typename(),"boolean");
  if (DesExp1!=DES_BOOLEAN)
    MsgErrors(Exp1->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"&&",
	      DesExp1.to_typename(),"boolean");
  
  /*
   * Applico lo "short circuits" a tempo di compilazione.
   */
  
  if (Exp0->is_BOOLEANode() && !((BOOLEANode *)Exp0)->GetBoolean())
    {
      delete root;
      delete Exp1;
      return Exp0;
    }

  /*
   * Applico due semplici ottimizzazioni di "costant folding".
   */

  if (Exp0->is_BOOLEANode() && ((BOOLEANode *)Exp0)->GetBoolean())
    {
      delete root;
      delete Exp0;
      return Exp1;
    }

  if (Exp1->is_BOOLEANode() && ((BOOLEANode *)Exp1)->GetBoolean())
    {
      delete root;
      delete Exp1;
      return Exp0;
    }

  /*
   * NON GESTITO!!!
   * se abbiamo (expr && false) il risultato e' "false", pero' la prima es-
   * pressione va analizzata a tempo di compilazione e eseguita a tempo di 
   * esecuzione.
   */

  root->SetLeftC(Exp0);
  root->SetRightC(Exp1);
  root->SetDescriptor(DesBoolean);
  return root;
}

/*
 * CompileUnit::ParseIorExpr
 *
 * Parsing di espressioni del tipo <expr> | <expr>
 */

TreeNode *CompileUnit::ParseIorExpr(TreeNode *root)
{
  if (!root->is_EXPNode() || ((EXPNode *)root)->GetNodeOp()!=OrOp)
    return ParseXorExpr(root);

  TreeNode   *Exp0=root->GetLeftC();
  TreeNode   *Exp1=root->GetRightC();

  root->SetLeftC(Dummy);
  root->SetRightC(Dummy);

  Exp0=ParseIorExpr(Exp0);
  Exp1=ParseXorExpr(Exp1);

  Descriptor DesExp0;
  Descriptor DesExp1;
  Descriptor DesReturn;

  DesExp0=Exp0->GetDescriptor();
  DesExp1=Exp1->GetDescriptor();

  if (DesExp0.is_integral() && DesExp1.is_integral())
    {
      DesReturn=BArithmetic_Promotion(DesExp0,DesExp1);

      /*
       * Semplici ottimizzazioni.
       */

      if (Exp0->is_INUMNode() && Exp1->is_INUMNode())
	if (DesReturn==DesExp0)
	  {
	    ((INUMNode *)Exp0)->SetVal(((INUMNode *)Exp0)->GetVal()|
				       ((INUMNode *)Exp1)->GetVal());
	    delete root;
	    delete Exp1;
	    return Exp0;
	  }
	else
	  {
	    ((INUMNode *)Exp1)->SetVal(((INUMNode *)Exp0)->GetVal()|
				       ((INUMNode *)Exp1)->GetVal());
	    delete root;
	    delete Exp0;
	    return Exp1;
	  }
      
      if (DesExp0!=DesReturn)
	{
	  Exp0=new EXPNode(CastOp,Dummy,Exp0);
	  Exp0->SetDescriptor(DesReturn);
	}
      if (DesExp1!=DesReturn)
	{
	  Exp1=new EXPNode(CastOp,Dummy,Exp1);
	  Exp1->SetDescriptor(DesReturn);
	}
    }    
  else
    if (DesExp0==DES_BOOLEAN && DesExp1==DES_BOOLEAN)
      {
	/*
	 * Semplici ottimizzazioni.
	 */

	if (Exp0->is_BOOLEANode() && Exp1->is_BOOLEANode())
	  {
           ((BOOLEANode *)Exp0)->SetBoolean(((BOOLEANode *)Exp0)->GetBoolean()|
					   ((BOOLEANode *)Exp1)->GetBoolean());
	   delete root;
	   delete Exp1;
	   return Exp0;
	  }
	DesReturn=DES_BOOLEAN;
      }
    else
      {
	/*
	 * Non so se cio' corrisponde a verita', pero' e' inutile inviare
	 * questi messaggi di errore se uno dei due descrittori e'
	 * uguale a DES_NULL.
	 */

	if (!DesExp0.is_primitive() || !DesExp1.is_primitive())
	  {
	    MsgErrors(root->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"|",
		      DesExp0.to_typename(),DesExp1.to_typename());

	    MsgErrors(root->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"|",
		      DesExp1.to_typename(),DesExp0.to_typename());
	  }
	else
	  {
	    if (!DesExp0.is_integral())
	      MsgErrors(root->GetLine(),msg_errors[ERR_EXPLICIT_CAST_NEEDED],
			"|",DesExp0.to_typename(),"int");

	    if (!DesExp1.is_integral())
	      MsgErrors(root->GetLine(),msg_errors[ERR_EXPLICIT_CAST_NEEDED],
			"|",DesExp1.to_typename(),"int");
	  }

	DesReturn=DES_NULL;
      }    

  root->SetLeftC(Exp0);
  root->SetRightC(Exp1);
  root->SetDescriptor(DesReturn);
  return root;
}

/*
 * CompileUnit::ParseXorExpr
 *
 * Parsing di un'operazione "^" (XOR).
 */

TreeNode *CompileUnit::ParseXorExpr(TreeNode *root)
{
  if (!root->is_EXPNode() || ((EXPNode *)root)->GetNodeOp()!=XorOp)
    return ParseAndExpr(root);

  TreeNode   *Exp0=root->GetLeftC();
  TreeNode   *Exp1=root->GetRightC();

  Exp0=ParseXorExpr(Exp0);
  Exp1=ParseAndExpr(Exp1);

  root->SetLeftC(Dummy);
  root->SetRightC(Dummy);

  Descriptor DesExp0;
  Descriptor DesExp1;
  Descriptor DesReturn;

  DesExp0=Exp0->GetDescriptor();
  DesExp1=Exp1->GetDescriptor();

  if (DesExp0.is_integral() && DesExp1.is_integral())
    {
      DesReturn=BArithmetic_Promotion(DesExp0,DesExp1);

      /*
       * Semplici ottimizzazioni di calcolo a tempo di compilazione.
       */

      if (Exp0->is_INUMNode() && Exp1->is_INUMNode())
	if (DesReturn==DesExp0)
	  {
	    ((INUMNode *)Exp0)->SetVal(((INUMNode *)Exp0)->GetVal()^
				       ((INUMNode *)Exp1)->GetVal());
	    delete root;
	    delete Exp1;
	    return Exp0;
	  }
	else
	  {
	    ((INUMNode *)Exp1)->SetVal(((INUMNode *)Exp0)->GetVal()^
				       ((INUMNode *)Exp1)->GetVal());
	    delete root;
	    delete Exp0;
	    return Exp1;
	  }
      
      if (DesExp0!=DesReturn)
	{
	  Exp0=new EXPNode(CastOp,Dummy,Exp0);
	  Exp0->SetDescriptor(DesReturn);
	}
      if (DesExp1!=DesReturn)
	{
	  Exp1=new EXPNode(CastOp,Dummy,Exp1);
	  Exp1->SetDescriptor(DesReturn);
	}
    }    
  else
    if (DesExp0==DES_BOOLEAN && DesExp1==DES_BOOLEAN)
      {

	/*
	 * Alcune semplici ottimizzazioni di calcolo.
	 */

	if (Exp0->is_BOOLEANode() && Exp1->is_BOOLEANode())
	  {
	   ((BOOLEANode *)Exp0)->SetBoolean(((BOOLEANode *)Exp0)->GetBoolean()^
					   ((BOOLEANode *)Exp1)->GetBoolean());
	   delete root;
	   delete Exp1;
	   return Exp0;
	  }

	DesReturn=DES_BOOLEAN;
      }
    else
      {
	/*
	 * Non so se cio' corrisponde a verita', pero' e' inutile inviare
	 * questi messaggi di errore se uno dei due descrittori e'
	 * uguale a DES_NULL.
	 */

	if (!DesExp0.is_primitive() || !DesExp1.is_primitive())
	  {
	    MsgErrors(root->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"^",
		      DesExp0.to_typename(),DesExp1.to_typename());

	    MsgErrors(root->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"^",
		      DesExp1.to_typename(),DesExp0.to_typename());
	  }
	else
	  {
	    if (!DesExp0.is_integral())
	      MsgErrors(root->GetLine(),msg_errors[ERR_EXPLICIT_CAST_NEEDED],
			"^",DesExp0.to_typename(),"int");

	    if (!DesExp1.is_integral())
	      MsgErrors(root->GetLine(),msg_errors[ERR_EXPLICIT_CAST_NEEDED],
			"^",DesExp1.to_typename(),"int");
	  }

	DesReturn=DES_NULL;
      }

  root->SetLeftC(Exp0);
  root->SetRightC(Exp1);
  root->SetDescriptor(DesReturn);
  return root;
}

/*
 * CompileUnit::ParseAndExpr
 *
 * Viene fatto il parsing di un "and" tra due espressioni.
 */

TreeNode *CompileUnit::ParseAndExpr(TreeNode *root)
{
  if (!root->is_EXPNode() || ((EXPNode *)root)->GetNodeOp()!=AndOp)
    return ParseEqualityExpr(root);

  TreeNode   *Exp0=root->GetLeftC();
  TreeNode   *Exp1=root->GetRightC();

  root->SetLeftC(Dummy);
  root->SetRightC(Dummy);

  Exp0=ParseAndExpr(Exp0);
  Exp1=ParseEqualityExpr(Exp1);

  Descriptor DesExp0;
  Descriptor DesExp1;
  Descriptor DesReturn;

  DesExp0=Exp0->GetDescriptor();
  DesExp1=Exp1->GetDescriptor();

  if (DesExp0.is_integral() && DesExp1.is_integral())
    {
      DesReturn=BArithmetic_Promotion(DesExp0,DesExp1);

      /*
       * Alcune semplici ottimizzazioni.
       */

      if (Exp0->is_INUMNode() && Exp1->is_INUMNode())
	if (DesReturn==DesExp0)
	  {
	    ((INUMNode *)Exp0)->SetVal(((INUMNode *)Exp0)->GetVal() &
				       ((INUMNode *)Exp1)->GetVal());
	    delete root;
	    delete Exp1;
	    return Exp0;
	  }
	else
	  {
	    ((INUMNode *)Exp1)->SetVal(((INUMNode *)Exp0)->GetVal() &
				       ((INUMNode *)Exp1)->GetVal());
	    delete root;
	    delete Exp0;
	    return Exp1;
	  }
      
      if (DesExp0!=DesReturn)
	{
	  Exp0=new EXPNode(CastOp,Dummy,Exp0);
	  Exp0->SetDescriptor(DesReturn);
	}
      if (DesExp1!=DesReturn)
	{
	  Exp1=new EXPNode(CastOp,Dummy,Exp1);
	  Exp1->SetDescriptor(DesReturn);
	}
    }    
  else
    if (DesExp0==DES_BOOLEAN && DesExp1==DES_BOOLEAN)
      {
	/*
	 * Semplici ottimizzazioni.
	 */

	if (Exp0->is_BOOLEANode() && Exp1->is_BOOLEANode())
	  {
	   ((BOOLEANode *)Exp0)->SetBoolean(((BOOLEANode *)Exp0)->GetBoolean()&
					   ((BOOLEANode *)Exp1)->GetBoolean());
	   delete root;
	   delete Exp1;
	   return Exp0;
	  }

	DesReturn=DES_BOOLEAN;
      }
    else
      {
	/*
	 * Non so se cio' corrisponde a verita', pero' e' inutile inviare
	 * questi messaggi di errore se uno dei due descrittori e'
	 * uguale a DES_NULL.
	 */

	if (!DesExp0.is_primitive() || !DesExp1.is_primitive())
	  {
	    MsgErrors(root->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"&",
		      DesExp0.to_typename(),DesExp1.to_typename());
	    
	    MsgErrors(root->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"&",
		      DesExp1.to_typename(),DesExp0.to_typename());
	  }
	else
	  {
	    if (!DesExp0.is_integral())
	      MsgErrors(root->GetLine(),msg_errors[ERR_EXPLICIT_CAST_NEEDED],
			"&",DesExp0.to_typename(),"int");

	    if (!DesExp1.is_integral())
	      MsgErrors(root->GetLine(),msg_errors[ERR_EXPLICIT_CAST_NEEDED],
			"&",DesExp1.to_typename(),"int");
	  }
	
	DesReturn=DES_NULL;
      }    

  root->SetLeftC(Exp0);
  root->SetRightC(Exp1);
  root->SetDescriptor(DesReturn);
  return root;
}

/*
 * CompileUnit::ParseEqualityExpr
 *
 * Parsing di espressioni di uguaglianza.
 */

TreeNode *CompileUnit::ParseEqualityExpr(TreeNode *root)
{
  if (!root->is_EXPNode() || 
      (((EXPNode *)root)->GetNodeOp()!=EqOp && 
       ((EXPNode *)root)->GetNodeOp()!=NeOp))
    return ParseRelopInstanceExpr(root);

  int is_equal=((EXPNode *)root)->GetNodeOp()==EqOp ? TRUE : FALSE;

  TreeNode   *Exp0=root->GetLeftC();
  TreeNode   *Exp1=root->GetRightC();
  TreeNode   *TreeReturn;

  root->SetLeftC(Dummy);
  root->SetRightC(Dummy);

  Exp0=ParseEqualityExpr(Exp0);
  Exp1=ParseRelopInstanceExpr(Exp1);

  Descriptor DesExp0;
  Descriptor DesExp1;
  Descriptor DesReturn;

  DesExp0=Exp0->GetDescriptor();
  DesExp1=Exp1->GetDescriptor();

  if (DesExp0.is_primitive() && DesExp1.is_primitive())
    {
      DesReturn=BArithmetic_Promotion(DesExp0,DesExp1);

      /*
       * Alcune semplici ottimizzazioni.
       */

      if ((Exp0->is_INUMNode() || Exp0->is_FNUMNode()) &&
	  (Exp1->is_INUMNode() || Exp1->is_FNUMNode()))
	{
	  if (is_equal)
	    TreeReturn=new BOOLEANode(((Exp0->is_INUMNode())         ? 
				       ((INUMNode *)Exp0)->GetVal()  :
				       ((FNUMNode *)Exp0)->GetVal()) ==
				      ((Exp1->is_INUMNode())         ? 
				       ((INUMNode *)Exp1)->GetVal()  :
				       ((FNUMNode *)Exp1)->GetVal())
				      );
	  else
	    TreeReturn=new BOOLEANode(((Exp0->is_INUMNode())          ? 
				       ((INUMNode *)Exp0)->GetVal()   :
				       ((FNUMNode *)Exp0)->GetVal())  !=
				      ((Exp1->is_INUMNode())          ? 
				       ((INUMNode *)Exp1)->GetVal()   :
				       ((FNUMNode *)Exp1)->GetVal())
				      );
	  delete root;
	  delete Exp0; delete Exp1;
	  return TreeReturn;
	}

      if (DesExp0!=DesReturn)
	{
	  Exp0=new EXPNode(CastOp,Dummy,Exp0);
	  Exp0->SetDescriptor(DesReturn);
	}
      if (DesExp1!=DesReturn)
	{
	  Exp1=new EXPNode(CastOp,Dummy,Exp1);
	  Exp1->SetDescriptor(DesReturn);
	}
    }
  else
    if (DesExp0==DES_BOOLEAN && DesExp1==DES_BOOLEAN)
      {
	/*
	 * Alcune semplici ottimizzazioni.
	 */
	
	if (Exp0->is_BOOLEANode() && Exp1->is_BOOLEANode())
	  {
	    if (is_equal)
	      TreeReturn=new BOOLEANode(((BOOLEANode *)Exp0)->GetBoolean() ==
					((BOOLEANode *)Exp1)->GetBoolean());
	    else
	      TreeReturn=new BOOLEANode(((BOOLEANode *)Exp0)->GetBoolean() !=
					((BOOLEANode *)Exp1)->GetBoolean());
	   
	    delete root;
	    delete Exp0; delete Exp1;
	    return TreeReturn;
	  }
      }
    else
      if (DesExp0.is_reference() && DesExp1.is_reference())
	if (is_Cast_Conversion(DesExp0,DesExp1))
	  {	    	  
	    if (DesExp0!=DesExp1) 
	      {
		Exp1=new EXPNode(CastOp,Dummy,Exp1);
		Exp1->SetDescriptor(DesExp0);
	      }
	  }
	else
	  if (is_Cast_Conversion(DesExp1,DesExp0))
	    {
	      if (DesExp0!=DesExp1) 
		{
		  Exp0=new EXPNode(CastOp,Dummy,Exp0);
		  Exp0->SetDescriptor(DesExp1);
		}
	    }
	  else
	    if (is_equal)
	      MsgErrors(root->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"==",
			DesExp0.to_typename(),DesExp1.to_typename());
	    else
	      MsgErrors(root->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"!=",
			DesExp0.to_typename(),DesExp1.to_typename());
      else
	if (DesExp0.is_reference() && Exp1->is_NULLNode())
	  Exp1->SetDescriptor(DesExp0);
	else
	  if (DesExp1.is_reference() && Exp0->is_NULLNode())
	    Exp0->SetDescriptor(DesExp1);
	  else
	    if (is_equal)
	      MsgErrors(root->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"==",
			DesExp0.to_typename(),DesExp1.to_typename());
	    else
	      MsgErrors(root->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"!=",
			DesExp0.to_typename(),DesExp1.to_typename());

  root->SetLeftC(Exp0);
  root->SetRightC(Exp1);
  root->SetDescriptor(DesBoolean);
  return root;
}

/*
 * CompileUnit::ParseRelopInstanceExpr
 *
 * Parsing o di espressioni relazionale o di INSTANCEOF.
 */

TreeNode *CompileUnit::ParseRelopInstanceExpr(TreeNode *root)
{
  if (root->is_EXPNode() && ((EXPNode *)root)->GetNodeOp()==InstanceOfOp)
    return ParseInstanceOfExpr(root);
  else
    return ParseRelopExpr(root);
}

/*
 * CompileUnit::ParseRelopExpr
 *
 * Parsing di espressioni relazionali, ossia espressioni del tipo:
 *
 *            - A >  B;
 *            - A >= B;
 *            - A <  B;
 *            - A <= B;
 *
 * e espressioni di tipo <expr> instanceof <reference type>.
 */

TreeNode *CompileUnit::ParseRelopExpr(TreeNode *root)
{
  const char *opstr;

  Descriptor DesExp0;
  Descriptor DesExp1;
  Descriptor DesHelp;

  int  op=((EXPNode *)root)->GetNodeOp();

  if (!root->is_EXPNode() || (op!=LtOp && op!=LeOp && op!=GtOp && op!=GeOp))
    return ParseShiftExpr(root);

  TreeNode   *Exp0=root->GetLeftC();
  TreeNode   *Exp1=root->GetRightC();
  TreeNode   *TreeReturn;

  root->SetLeftC(Dummy);
  root->SetRightC(Dummy);

  Exp0=ParseRelopInstanceExpr(Exp0);
  Exp1=ParseShiftExpr(Exp1);

  DesExp0=Exp0->GetDescriptor();
  DesExp1=Exp1->GetDescriptor();

  switch (op)
    {
    case LtOp: opstr="<";  break;
    case GtOp: opstr=">";  break;
    case LeOp: opstr="<="; break;
    case GeOp: opstr=">="; break;
    }

  if (DesExp0.is_primitive() && DesExp1.is_primitive())
    {
      /*
       * Alcune semplici ottimizzazioni.
       */
      
      if ((Exp0->is_INUMNode() || Exp0->is_FNUMNode()) &&
	  (Exp1->is_INUMNode() || Exp1->is_FNUMNode()))
	{

	  switch (op)
	    {
	    case LtOp: 
	      TreeReturn=new BOOLEANode(((Exp0->is_INUMNode())         ? 
					 ((INUMNode *)Exp0)->GetVal()  :
					 ((FNUMNode *)Exp0)->GetVal()) <
					((Exp1->is_INUMNode())         ? 
					 ((INUMNode *)Exp1)->GetVal()  :
					 ((FNUMNode *)Exp1)->GetVal())
					);
	      break;
	    case LeOp: 
	      TreeReturn=new BOOLEANode(((Exp0->is_INUMNode())         ? 
					 ((INUMNode *)Exp0)->GetVal()  :
					 ((FNUMNode *)Exp0)->GetVal()) <=
					((Exp1->is_INUMNode())         ? 
					 ((INUMNode *)Exp1)->GetVal()  :
					 ((FNUMNode *)Exp1)->GetVal())
					); 
	      break;
	    case GtOp:
	      TreeReturn=new BOOLEANode(((Exp0->is_INUMNode())         ? 
					 ((INUMNode *)Exp0)->GetVal()  :
					 ((FNUMNode *)Exp0)->GetVal()) >
					((Exp1->is_INUMNode())         ? 
					 ((INUMNode *)Exp1)->GetVal()  :
					 ((FNUMNode *)Exp1)->GetVal())
					);  
	      break; 
	    case GeOp:
	      TreeReturn=new BOOLEANode(((Exp0->is_INUMNode())         ? 
					 ((INUMNode *)Exp0)->GetVal()  :
					 ((FNUMNode *)Exp0)->GetVal()) >=
					((Exp1->is_INUMNode())         ? 
					 ((INUMNode *)Exp1)->GetVal()  :
					 ((FNUMNode *)Exp1)->GetVal())
					); 
	      break; 
	    }

	  delete root;
	  delete Exp0; delete Exp1;
	  return TreeReturn;
	}

      DesHelp=BArithmetic_Promotion(DesExp0,DesExp1);
      
      if (DesExp0!=DesHelp)
	{
	  Exp0=new EXPNode(CastOp,Dummy,Exp0);
	  Exp0->SetDescriptor(DesHelp);
	}
      if (DesExp1!=DesHelp)
	{
	  Exp1=new EXPNode(CastOp,Dummy,Exp1);
	  Exp1->SetDescriptor(DesHelp);
	}
    }
  else
    {
      /*
       * Ricordarsi sempre che se un'errore e' gia' stato segnalato per Exp0 o
       * Exp1, e' inutile risegnalarlo. Cio' avviene quando DesExp0==DES_NULL
       * o DesExp1==DES_NULL.
       */

      if (!DesExp0.is_primitive() && DesExp0!=DES_NULL)
	if (DesExp1.is_primitive())
	  MsgErrors(Exp0->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],opstr,
		    DesExp0.to_typename(),DesExp1.to_typename());
	else
	  MsgErrors(Exp0->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],opstr,
		    DesExp0.to_typename(),"int");

      if (!DesExp1.is_primitive() && DesExp1!=DES_NULL)
	if (DesExp0.is_primitive())
	  MsgErrors(Exp1->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],opstr,
		    DesExp1.to_typename(),DesExp0.to_typename());
	else
	  MsgErrors(Exp1->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],opstr,
		    DesExp1.to_typename(),"int");
    }

  root->SetLeftC(Exp0);
  root->SetRightC(Exp1);
  root->SetDescriptor(DesBoolean);
  return root;
}

/*
 * CompileUnit::ParseInstanceOfExpr
 *
 * Effettua il parsing di un'espressione del tipo:
 *
 *             <expr> instanceof <reference type>
 *
 * Si tenga presente che il reference type oltre a essere un riferimento a
 * classe o interfaccia, puo' essere anche un riferimento ad array.
 */

TreeNode *CompileUnit::ParseInstanceOfExpr(TreeNode *root)
{
  if (!root->is_EXPNode() || ((EXPNode *)root)->GetNodeOp()!=InstanceOfOp)
    return ParseShiftExpr(root);

  TreeNode  *Exp0=ParseRelopInstanceExpr(root->GetLeftC());
  Descriptor DesExp0;

  DesExp0=Exp0->GetDescriptor();

  if (!DesExp0.is_reference())
    MsgErrors(root->GetLine(),msg_errors[ERR_INVALID_INSTANCEOF],
	      DesExp0.to_typename(),root->GetDescriptor().to_typename());

  if (!is_Cast_Conversion(root->GetDescriptor(),DesExp0))
        MsgErrors(root->GetLine(),msg_errors[ERR_INVALID_INSTANCEOF],
	      DesExp0.to_typename(),root->GetDescriptor().to_typename());
  else
    if (DesExp0!=root->GetDescriptor())
      {
	Exp0=new EXPNode(CastOp,Dummy,Exp0);
	Exp0->SetDescriptor(root->GetDescriptor());
      }
  
  root->SetLeftC(Exp0);
  return root;

}

/*
 * CompileUnit::ParseShiftExpr
 *
 * Parsing di espressioni del tipo:
 *
 * - <expr>  >>  <expr>
 * - <expr>  <<  <expr>
 * - <expr>  >>> <expr>
 */

TreeNode *CompileUnit::ParseShiftExpr(TreeNode *root)
{
  const char *opstr;

  int  op=((EXPNode *)root)->GetNodeOp();

  if (!root->is_EXPNode() || (op!=LShiftOp && op!=RShiftOp && op!=UrShiftOp))
    return ParseAddSubExpr(root);

  TreeNode   *Exp0=root->GetLeftC();
  TreeNode   *Exp1=root->GetRightC();

  root->SetLeftC(Dummy);
  root->SetRightC(Dummy);

  Exp0=ParseShiftExpr(Exp0);
  Exp1=ParseAddSubExpr(Exp1);

  Descriptor DesExp0;
  Descriptor DesExp1;
  Descriptor DesReturn;  

  DesExp0=Exp0->GetDescriptor();
  DesExp1=Exp1->GetDescriptor();
  DesReturn=DES_INT;

  switch (op)
    {
    case LShiftOp : opstr="<<"; break;
    case RShiftOp : opstr=">>"; break;
    case UrShiftOp: opstr=">>>"; break;
    }

  if (DesExp0.is_integral() && DesExp1.is_integral())
    {
      /*
       * Alcune semplici ottimizzazioni numeriche.
       */
      
      if (Exp0->is_INUMNode() && Exp1->is_INUMNode())
	{
	  long n,s,k;
	  
	  n=((INUMNode *)Exp0)->GetVal();
  	  s=((INUMNode *)Exp1)->GetVal();

	  switch (op)
	    {
	    case LShiftOp:
	      ((INUMNode *)Exp0)->SetVal(n << s);
	      break;
	    case RShiftOp:
	      ((INUMNode *)Exp0)->SetVal(n >> s);
	      break;
	    case UrShiftOp:
	      if (DesExp0==DES_LONG) 
		k=64;
	      else
		k=32;
	      if (n>=0)
	        ((INUMNode *)Exp0)->SetVal(n >> s);
	      else
		((INUMNode *)Exp0)->SetVal((n >> s)+(2<<k-s-1));
	      break;
	    }
	  
	  delete root;
	  delete Exp1; return Exp0;
	}
      
      if (DesReturn!=DesExp1)
	{
	  Exp1=new EXPNode(CastOp,Dummy,Exp1);
	  Exp1->SetDescriptor(DesReturn);
	}

      if (DesReturn!=DesExp0)
	{
	  Exp0=new EXPNode(CastOp,Dummy,Exp0);
	  Exp0->SetDescriptor(DesReturn);
	}
    }
  else
    {
      if (DesExp0.is_primitive() && !DesExp0.is_integral())
	MsgErrors(Exp0->GetLine(),msg_errors[ERR_EXPLICIT_CAST_NEEDED],opstr,
		  DesExp0.to_typename(),"int");
      else
	if (!DesExp0.is_primitive())
	  MsgErrors(Exp0->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],opstr,
		    DesExp0.to_typename(),"int");

      if (DesExp1.is_primitive() && !DesExp1.is_integral())
	MsgErrors(Exp0->GetLine(),msg_errors[ERR_EXPLICIT_CAST_NEEDED],opstr,
		  DesExp1.to_typename(),"int");
      else
	if (!DesExp1.is_primitive())
	  MsgErrors(Exp1->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],opstr,
		    DesExp1.to_typename(),"int");
    }
  
  root->SetLeftC(Exp0);
  root->SetRightC(Exp1);
  root->SetDescriptor(DesReturn);
  return root;
}

/*
 * CompileUnit::ParseAddSubExpr
 *
 * Parsing di espressioni di somma e sottrazione.
 */

TreeNode *CompileUnit::ParseAddSubExpr(TreeNode *root)
{
  int  op=((EXPNode *)root)->GetNodeOp();

  if (!root->is_EXPNode() || (op!=AddOp && op!=SubOp))
    return ParseMultDivExpr(root);

  if (op==AddOp)
    return ParseAddExpr(root);
  else
    return ParseSubExpr(root);
}

/*
 * CompileUnit::ParseAddExpr
 *
 * Parsing di un'espressione di somma tra due espressioni.
 */

TreeNode *CompileUnit::ParseAddExpr(TreeNode *root)
{
  TreeNode   *Exp0=root->GetLeftC();
  TreeNode   *Exp1=root->GetRightC();

  root->SetLeftC(Dummy);
  root->SetRightC(Dummy);

  Exp0=ParseAddSubExpr(Exp0);
  Exp1=ParseMultDivExpr(Exp1);

  Descriptor DesExp0;
  Descriptor DesExp1;
  Descriptor DesReturn;  

  DesExp0=Exp0->GetDescriptor();
  DesExp1=Exp1->GetDescriptor();

  if (DesExp0.is_primitive() && DesExp1.is_primitive())
    {
      /*
       * Constant folding numerico.
       */
      
      if (Exp0->is_INUMNode() && Exp1->is_INUMNode())
	{
	  ((INUMNode *)Exp0)->SetVal(((INUMNode *)Exp0)->GetVal()+
				     ((INUMNode *)Exp1)->GetVal());
	  delete root;
	  delete Exp1;
	  return Exp0;
	}

      if (Exp0->is_FNUMNode() && Exp1->is_INUMNode())
	{
	  ((FNUMNode *)Exp0)->SetVal(((FNUMNode *)Exp0)->GetVal()+
				     ((INUMNode *)Exp1)->GetVal());
	  delete root;
	  delete Exp1;
	  return Exp0;
	}

      if (Exp0->is_INUMNode() && Exp1->is_FNUMNode())
	{
	  ((FNUMNode *)Exp1)->SetVal(((INUMNode *)Exp0)->GetVal()+
				     ((FNUMNode *)Exp1)->GetVal());
	  delete root;
	  delete Exp0;
	  return Exp1;
	}

      if (Exp0->is_FNUMNode() && Exp1->is_FNUMNode())
	{
	  ((FNUMNode *)Exp0)->SetVal(((FNUMNode *)Exp0)->GetVal()+
				     ((FNUMNode *)Exp1)->GetVal());
	  delete root;
	  delete Exp1;
	  return Exp0;
	}

      DesReturn=BArithmetic_Promotion(DesExp0,DesExp1);

      if (DesExp0!=DesReturn)
	{
	  Exp0=new EXPNode(CastOp,Dummy,Exp0);
	  Exp0->SetDescriptor(DesReturn);
	}
      else
	if (DesExp1!=DesReturn)
	  {
	    Exp1=new EXPNode(CastOp,Dummy,Exp1);
	    Exp1->SetDescriptor(DesReturn);
	  }
    }
  else
    if (DesExp0==DES_STRING || DesExp1==DES_STRING )
      {
	char *num2string=new char[20];

	DesReturn=DES_STRING;

	/*
	 * Costant folding per stringhe.
	 */
	
	if (Exp0->is_STRINGNode() && Exp1->is_STRINGNode())
	  {
	    ((STRINGNode *)Exp0)->SetString(((STRINGNode*)Exp0)->GetString()+
					    ((STRINGNode*)Exp1)->GetString());
	    delete root;
	    delete Exp1;
	    return Exp0;
	  }

	if (Exp0->is_STRINGNode() && Exp1->is_INUMNode())
	  {
	    sprintf(num2string,"%ld",((INUMNode*)Exp1)->GetVal());

	    ((STRINGNode *)Exp0)->SetString(((STRINGNode*)Exp0)->GetString()+
					    num2string);
	    delete root;
	    delete Exp1;
	    return Exp0;
	  }
	
	if (Exp1->is_STRINGNode() && Exp0->is_INUMNode())
	  {
	    sprintf(num2string,"%ld",((INUMNode*)Exp0)->GetVal());
	    String *s=new String(num2string);

          ((STRINGNode *)Exp1)->SetString(*s+((STRINGNode*)Exp1)->GetString());
	    delete s;
	    delete root;
	    delete Exp0;
	    return Exp1;
	  }
	
	if (Exp0->is_STRINGNode() && Exp1->is_FNUMNode())
	  {
	    sprintf(num2string,"%f",((FNUMNode*)Exp1)->GetVal());

	    ((STRINGNode *)Exp0)->SetString(((STRINGNode*)Exp0)->GetString()+
					    num2string);
	    delete root;
	    delete Exp1;
	    return Exp0;
	  }

	if (Exp1->is_STRINGNode() && Exp0->is_FNUMNode())
	  {
	    sprintf(num2string,"%f",((FNUMNode*)Exp0)->GetVal());
	    String *s=new String(num2string);

          ((STRINGNode *)Exp1)->SetString(*s+((STRINGNode*)Exp1)->GetString());
	    delete s;
	    delete root;
	    delete Exp0;
	    return Exp1;
	  }
	
	if (Exp0->is_STRINGNode() && Exp1->is_CHARNode())
	  {
	    sprintf(num2string,"%c",((CHARNode*)Exp1)->GetCharacter());

	    ((STRINGNode *)Exp0)->SetString(((STRINGNode*)Exp0)->GetString()+
					    num2string);
	    delete root;
	    delete Exp1;
	    return Exp0;
	  }
	
	if (Exp1->is_STRINGNode() && Exp0->is_CHARNode())
	  {
	    sprintf(num2string,"%c",((CHARNode*)Exp0)->GetCharacter());
	    String *s=new String(num2string);

          ((STRINGNode *)Exp1)->SetString(*s+((STRINGNode*)Exp1)->GetString());
	    delete s;
	    delete root;
	    delete Exp0;
	    return Exp1;
	  }

	if (Exp0->is_STRINGNode() && Exp1->is_NULLNode())
	  {
	    ((STRINGNode *)Exp0)->SetString(((STRINGNode*)Exp0)->GetString()+
					    "null");
	    delete root;
	    delete Exp1;
	    return Exp0;
	  }

	if (Exp1->is_STRINGNode() && Exp0->is_NULLNode())
	  {
	    String *s=new String("null");

          ((STRINGNode *)Exp1)->SetString(*s+((STRINGNode*)Exp1)->GetString());
	    delete s;
	    delete root;
	    delete Exp0;
	    return Exp1;
	  }

	if (Exp0->is_STRINGNode() && Exp1->is_BOOLEANode())
	  {
	    if (((BOOLEANode *)Exp1)->GetBoolean())
	      ((STRINGNode *)Exp0)->SetString(((STRINGNode*)Exp0)->GetString()+
					      "true");
	    else
	      ((STRINGNode *)Exp0)->SetString(((STRINGNode*)Exp0)->GetString()+
					      "false");
	    delete root;
	    delete Exp1;
	    return Exp0;
	  }

	if (Exp1->is_STRINGNode() && Exp0->is_BOOLEANode())
	  {
	    String *s;

	    if (((BOOLEANode *)Exp0)->GetBoolean()) 
	      s=new String("true");
	    else
	      s=new String("false");
          ((STRINGNode *)Exp1)->SetString(*s+((STRINGNode*)Exp1)->GetString());
	    delete root;
	    delete Exp0;
	    return Exp1;
	  }

	/*
	 * NON GESTITO!!!
	 *
	 * Se un nodo e' una stringa e un' altro e' un reference type, va invo-
	 * cato il metodo toString del suddetto passando 0 argomenti.
	 */

	/*

	if (DesExp0==DES_STRING && 
	    (DesExp1.is_primitive() || DesExp1==DES_BOOLEAN))
	  {
	    Exp1=new EXPNode(CastOp,Dummy,Exp1);
	    Exp1->SetDescriptor(DesString);
	  }

	if (DesExp1==DES_STRING && 
	    (DesExp0.is_primitive() || DesExp0==DES_BOOLEAN))
	  {
	    Exp0=new EXPNode(CastOp,Dummy,Exp0);
	    Exp0->SetDescriptor(DesString);
	  }

	  */

	if (DesExp0==DES_STRING && Exp1->is_NULLNode())
	  {
	    delete Exp1;
	    Exp1=new STRINGNode("null");
	  }

	if (DesExp1==DES_STRING && Exp0->is_NULLNode())
	  {
	    delete Exp0;
	    Exp0=new STRINGNode("null");
	  }

	/*
	 * NON GESTITO!!!
	 *
	 * Se ho un operando stringa e un altro di tipo reference type, dovrei
	 * invocare il suo metodo toString.
	 */

	delete num2string;
      }
    else
      {
	if (!DesExp0.is_primitive())
	  if (DesExp1.is_primitive() || DesExp1==DES_STRING)
	    MsgErrors(Exp0->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"+",
		      DesExp0.to_typename(),DesExp1.to_typename());
	  else
	    MsgErrors(Exp0->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"+",
		      DesExp0.to_typename(),"int");

	if (!DesExp1.is_primitive())
	  if (DesExp0.is_primitive() || DesExp0==DES_STRING)
	    MsgErrors(Exp1->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"+",
		      DesExp1.to_typename(),DesExp0.to_typename());
	  else
	    MsgErrors(Exp1->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"+",
		      DesExp1.to_typename(),"int");
	
	DesReturn=DES_INT;
      }

  root->SetLeftC(Exp0);
  root->SetRightC(Exp1);
  root->SetDescriptor(DesReturn);
  return root;
}

/*
 * CompileUnit::ParseSubExpr
 *
 * Parsing di un'espressione di sottrazione tra due espressioni.
 */

TreeNode *CompileUnit::ParseSubExpr(TreeNode *root)
{
  TreeNode   *Exp0=root->GetLeftC();
  TreeNode   *Exp1=root->GetRightC();

  root->SetLeftC(Dummy);
  root->SetRightC(Dummy);

  Exp0=ParseAddSubExpr(Exp0);
  Exp1=ParseMultDivExpr(Exp1);

  Descriptor DesExp0;
  Descriptor DesExp1;
  Descriptor DesReturn;  

  DesExp0=Exp0->GetDescriptor();
  DesExp1=Exp1->GetDescriptor();

  if (DesExp0.is_primitive() && DesExp1.is_primitive())
    {
      /*
       * Alcune semplici ottimizzazioni numeriche.
       */
      
      if (Exp0->is_INUMNode() && Exp1->is_INUMNode())
	{
	  ((INUMNode *)Exp0)->SetVal(((INUMNode *)Exp0)->GetVal()-
				     ((INUMNode *)Exp1)->GetVal());
	  delete root;
	  delete Exp1;
	  return Exp0;
	}

      if (Exp0->is_FNUMNode() && Exp1->is_INUMNode())
	{
	  ((FNUMNode *)Exp0)->SetVal(((FNUMNode *)Exp0)->GetVal()-
				     ((INUMNode *)Exp1)->GetVal());
	  delete root;
	  delete Exp1;
	  return Exp0;
	}

      if (Exp0->is_INUMNode() && Exp1->is_FNUMNode())
	{
	  ((FNUMNode *)Exp1)->SetVal(((INUMNode *)Exp0)->GetVal()-
				     ((FNUMNode *)Exp1)->GetVal());
	  delete root;
	  delete Exp0;
	  return Exp1;
	}

      if (Exp0->is_FNUMNode() && Exp1->is_FNUMNode())
	{
	  ((FNUMNode *)Exp1)->SetVal(((FNUMNode *)Exp0)->GetVal()-
				     ((FNUMNode *)Exp1)->GetVal());
	  delete root;
	  delete Exp0;
	  return Exp1;
	}

      DesReturn=BArithmetic_Promotion(DesExp0,DesExp1);

      if (DesExp0!=DesReturn)
	{
	  Exp0=new EXPNode(CastOp,Dummy,Exp0);
	  Exp0->SetDescriptor(DesReturn);
	}
      else
	if (DesExp1!=DesReturn)
	  {
	    Exp1=new EXPNode(CastOp,Dummy,Exp1);
	    Exp1->SetDescriptor(DesReturn);
	  }
    }
  else
    {
      DesReturn=DES_INT;

      if (!DesExp0.is_primitive()) 
	if (DesExp1.is_primitive())
	  MsgErrors(Exp0->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"-",
		    DesExp0.to_typename(),DesExp1.to_typename());
	else
	  MsgErrors(Exp0->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"-",
		    DesExp0.to_typename(),"int");

      if (!DesExp1.is_primitive()) 
	if (DesExp0.is_primitive())
	  MsgErrors(Exp1->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"-",
		    DesExp1.to_typename(),DesExp0.to_typename());
	else
	  MsgErrors(Exp1->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"-",
		    DesExp1.to_typename(),"int");
    }

  root->SetLeftC(Exp0);
  root->SetRightC(Exp1);
  root->SetDescriptor(DesReturn);
  return root;
}

/*
 * CompileUnit::ParseMultDivExpr
 *
 * Parsing di espressioni di moltiplicazione, divisione e modulo.
 */

TreeNode *CompileUnit::ParseMultDivExpr(TreeNode *root)
{
  int  op=((EXPNode *)root)->GetNodeOp();

  if (!root->is_EXPNode() || (op!=MultOp && op!=DivOp && op!=ModOp))
    return ParseUnaryExpr(root);

  /*
   * Si tenga presente che ParseDivOp gestisce sia la divisione che il modulo.
   */

  if (op==MultOp)
    return ParseMultExpr(root);
  else
    return ParseDivExpr(root);
}

/*
 * CompileUnit::ParseMultExpr
 *
 * Parsing di espressioni di moltiplicazione.
 */

TreeNode *CompileUnit::ParseMultExpr(TreeNode *root)
{
  TreeNode   *Exp0=root->GetLeftC();
  TreeNode   *Exp1=root->GetRightC();

  root->SetLeftC(Dummy);
  root->SetRightC(Dummy);

  Exp0=ParseMultDivExpr(Exp0);
  Exp1=ParseUnaryExpr(Exp1);

  Descriptor DesExp0;
  Descriptor DesExp1;
  Descriptor DesReturn;  

  DesExp0=Exp0->GetDescriptor();
  DesExp1=Exp1->GetDescriptor();

  if (DesExp0.is_primitive() && DesExp1.is_primitive())
    {
      /* 
       * Semplici ottimizzazioni numeriche.
       */

      if (Exp0->is_INUMNode() && Exp1->is_INUMNode())
	{
	  ((INUMNode *)Exp0)->SetVal(((INUMNode *)Exp0)->GetVal()*
				       ((INUMNode *)Exp1)->GetVal());
	  delete root;
	  delete Exp1;
	  return Exp0;
	}

      if (Exp0->is_FNUMNode() && Exp1->is_INUMNode())
	{
	  ((FNUMNode *)Exp0)->SetVal(((FNUMNode *)Exp0)->GetVal()*
				     ((INUMNode *)Exp1)->GetVal());
	  delete root;
	  delete Exp1;
	  return Exp0;
	}

      if (Exp0->is_INUMNode() && Exp1->is_FNUMNode())
	{
	  ((FNUMNode *)Exp1)->SetVal(((INUMNode *)Exp0)->GetVal()*
				     ((FNUMNode *)Exp1)->GetVal());
	  delete root;
	  delete Exp0;
	  return Exp1;
	}

      if (Exp0->is_FNUMNode() && Exp1->is_FNUMNode())
	{
	  ((FNUMNode *)Exp0)->SetVal(((FNUMNode *)Exp0)->GetVal()*
				     ((FNUMNode *)Exp1)->GetVal());
	  delete root;
	  delete Exp1;
	  return Exp0;
	}


      /*
      if ((Exp0->is_INUMNode() || Exp0->is_FNUMNode()) &&
	  (Exp1->is_INUMNode() || Exp1->is_FNUMNode()))
	if (DesExp0==DesReturn)
	  {
	    if (Exp0->is_INUMNode())
	      ((INUMNode *)Exp0)->SetVal(((INUMNode *)Exp0)->GetVal()*
					 ((INUMNode *)Exp1)->GetVal());
	    else
	      if (Exp1->is_INUMNode())
		((FNUMNode *)Exp0)->SetVal(((FNUMNode *)Exp0)->GetVal()*
					   ((INUMNode *)Exp1)->GetVal());
	      else
		((FNUMNode *)Exp0)->SetVal(((FNUMNode *)Exp0)->GetVal()*
					   ((FNUMNode *)Exp1)->GetVal());
	    delete root;
	    delete Exp1;
	    return Exp0;
	  }
	else
	  {
	    if (Exp1->is_INUMNode())
	      ((INUMNode *)Exp1)->SetVal(((INUMNode *)Exp1)->GetVal()*
					 ((INUMNode *)Exp0)->GetVal());
	    else
	      if (Exp0->is_INUMNode())
		((FNUMNode *)Exp1)->SetVal(((FNUMNode *)Exp1)->GetVal()*
					   ((INUMNode *)Exp0)->GetVal());
	      else
		((FNUMNode *)Exp1)->SetVal(((FNUMNode *)Exp1)->GetVal()*
					   ((FNUMNode *)Exp0)->GetVal());
	    delete root;
	    delete Exp0;
	    return Exp1;
	  }
	  */      
      DesReturn=BArithmetic_Promotion(DesExp0,DesExp1);

      if (DesExp0!=DesReturn)
	{
	  Exp0=new EXPNode(CastOp,Dummy,Exp0);
	  Exp0->SetDescriptor(DesReturn);
	}
      else
	if (DesExp1!=DesReturn)
	  {
	    Exp1=new EXPNode(CastOp,Dummy,Exp1);
	    Exp1->SetDescriptor(DesReturn);
	  }
    }
  else
    {
      if (!DesExp0.is_primitive())
	if (DesExp1.is_primitive())
	  MsgErrors(Exp0->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"*",
		    DesExp0.to_typename(),DesExp1.to_typename());
	else
	  MsgErrors(Exp0->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"*",
		    DesExp0.to_typename(),"int");

      if (!DesExp1.is_primitive())
	if (DesExp0.is_primitive())
	  MsgErrors(Exp1->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"*",
		    DesExp1.to_typename(),DesExp0.to_typename());
	else
	  MsgErrors(Exp1->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"*",
		    DesExp1.to_typename(),"int");

      DesReturn=DES_INT;
    }

  root->SetLeftC(Exp0);
  root->SetRightC(Exp1);
  root->SetDescriptor(DesReturn);
  return root;
}

/*
 * CompileUnit::ParseDivExpr
 *
 * Parsing di divisioni e operazioni di modulo tra espressioni.
 */

TreeNode *CompileUnit::ParseDivExpr(TreeNode *root)
{
  const char       *opstr;
  int        op=((EXPNode*)root)->GetNodeOp();

  TreeNode   *Exp0=root->GetLeftC();
  TreeNode   *Exp1=root->GetRightC();

  root->SetLeftC(Dummy);
  root->SetRightC(Dummy);

  Exp0=ParseMultDivExpr(Exp0);
  Exp1=ParseUnaryExpr(Exp1);

  Descriptor DesExp0;
  Descriptor DesExp1;
  Descriptor DesReturn;  

  DesExp0=Exp0->GetDescriptor();
  DesExp1=Exp1->GetDescriptor();

  if (op==DivOp) 
    opstr="/";
  else
    opstr="%";

  if (DesExp0.is_primitive() && DesExp1.is_primitive())
    {
      /* 
       * Ottimizzazioni di costant folding (numeriche).
       */

      /*
       * NON GESTITO!!!
       *
       * La divisione per 0 genera un errore irreversibile.
       */
      
      if (Exp0->is_INUMNode() && Exp1->is_INUMNode())
	{
	  if (op==DivOp)
	    ((INUMNode *)Exp0)->SetVal(((INUMNode *)Exp0)->GetVal() / 
				       ((INUMNode *)Exp1)->GetVal());
	  else
	    ((INUMNode *)Exp0)->SetVal(((INUMNode *)Exp0)->GetVal() % 
				       ((INUMNode *)Exp1)->GetVal());
	  delete root;
	  delete Exp1;
	  return Exp0;
	}

      if (Exp0->is_FNUMNode() && Exp1->is_INUMNode())
	{
	  if (op==DivOp)
	    ((FNUMNode *)Exp0)->SetVal(((FNUMNode *)Exp0)->GetVal() / 
				       ((INUMNode *)Exp1)->GetVal());
	  else
	    ((FNUMNode *)Exp0)->SetVal(fmod(((FNUMNode *)Exp0)->GetVal(), 
					    ((INUMNode *)Exp1)->GetVal()));
	  delete root;
	  delete Exp1;
	  return Exp0;
	}

      if (Exp0->is_FNUMNode() && Exp1->is_FNUMNode())
	{
	  if (op==DivOp)
	    ((FNUMNode *)Exp0)->SetVal(((FNUMNode *)Exp0)->GetVal() / 
				       ((FNUMNode *)Exp1)->GetVal());
	  else
	    ((FNUMNode *)Exp0)->SetVal(fmod(((FNUMNode *)Exp0)->GetVal(), 
					    ((FNUMNode *)Exp1)->GetVal()));
	  delete root;
	  delete Exp1;
	  return Exp0;
	}

      DesReturn=BArithmetic_Promotion(DesExp0,DesExp1);
      
      if (DesExp0!=DesReturn)
	{
	  Exp0=new EXPNode(CastOp,Dummy,Exp0);
	  Exp0->SetDescriptor(DesReturn);
	}
      else
	if (DesExp1!=DesReturn)
	  {
	    Exp1=new EXPNode(CastOp,Dummy,Exp1);
	    Exp1->SetDescriptor(DesReturn);
	  }
    }
  else
    {
      if (!DesExp0.is_primitive())
	if (DesExp1.is_primitive())
	  MsgErrors(Exp0->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],opstr,
		    DesExp0.to_typename(),DesExp1.to_typename());
	else
	  MsgErrors(Exp0->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],opstr,
		    DesExp0.to_typename(),"int");

      if (!DesExp1.is_primitive())
	if (DesExp0.is_primitive())
	  MsgErrors(Exp1->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],opstr,
		    DesExp1.to_typename(),DesExp0.to_typename());
	else
	  MsgErrors(Exp1->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],opstr,
		    DesExp1.to_typename(),"int");

      DesReturn=DES_INT;
    }

  root->SetLeftC(Exp0);
  root->SetRightC(Exp1);
  root->SetDescriptor(DesReturn);
  return root;
}

/*
 * CompileUnit::ParseUnaryExpr
 *
 * CompileUnit::ParsePreIncDecExpr
 * CompileUnit::ParseUnaryAddExpr
 * CompileUnit::ParseUnarySubExpr
 * CompileUnit::ParseUnaryNotPlusMinusExpr
 *
 * Effettua il parsing di espressioni unarie, tipo:
 *
 * + <expr>; - <expr>; ++ <expr>; -- <expr>.
 *
 * In particolare gestiremo le operazioni di somma e sottrazione unaria,
 * preincremento e predecremento.
 * Per svolgere in pieno le sue funzioni, il metodo seguente utilizza altri 
 * 4 metodi che lo completano e che sono elencati sopra.
 */

TreeNode *CompileUnit::ParseUnaryExpr(TreeNode *root)
{
  if (root->is_EXPNode())
    switch (((EXPNode *)root)->GetNodeOp())
      {
      case PlusPlusOp:
	if (!root->GetRightC()->IsDummy())
	  return ParsePreIncDecExpr(root);
	else
	  if (!root->GetLeftC()->IsDummy())
	    return ParsePostIncDecExpr(root);
	break;
      case MinusMinusOp:
	if (!root->GetRightC()->IsDummy())
	  return ParsePreIncDecExpr(root);
	else
	  if (!root->GetLeftC()->IsDummy())
	    return ParsePostIncDecExpr(root);
	break;
      case UniPlusOp : return ParseUnaryAddExpr(root);
      case UniMinusOp: return ParseUnarySubExpr(root);
      default        : return ParseUnaryNotPlusMinusExpr(root);
      }
  else
    return ParseUnaryNotPlusMinusExpr(root);
}

TreeNode *CompileUnit::ParsePreIncDecExpr(TreeNode *root)
{
  const char *     opstr;
  int        op=((EXPNode *)root)->GetNodeOp();

  TreeNode   *Exp0=root->GetRightC();

  root->SetRightC(Dummy);

  Exp0=ParseUnaryExpr(Exp0);

  Descriptor DesExp0;
  Descriptor DesReturn;

  DesExp0=Exp0->GetDescriptor();
  DesReturn=Exp0->GetDescriptor();

  /*
   * Attenzione!!!
   *
   * L'argomento di un operatore di preincremento/postdecremento, deve essere
   * sempre una variabile e mai un valore, altrimenti verra' lanciato
   * l'errore ERR_INVALID_ARG. Il risultato del pre-incremento/decremento
   * nella generazione del codice, dovra' essere un valore e non una variabile.
   * Si ricordi che gli operatori di pre-incremento/decremento, non possono
   * essere applicati a vaiabili (campi) final.
   */

  if (op==PlusPlusOp) 
    opstr="++";
  else 
    opstr="--";

  if (!IsVariable(Exp0))
    MsgErrors(Exp0->GetLine(),msg_errors[ERR_INVALID_ARG],opstr);

  if (!DesReturn.is_primitive())
    {
      MsgErrors(Exp0->GetLine(),msg_errors[ERR_INVALID_ARG_TYPE],
		DesExp0.to_typename(),opstr);
      DesReturn=DES_INT;
    }
  
  root->SetRightC(Exp0);
  root->SetDescriptor(DesReturn);
  return root;
}

TreeNode *CompileUnit::ParseUnaryAddExpr(TreeNode *root)
{
  TreeNode   *Exp0=root->GetRightC();

  root->SetRightC(Dummy);

  Exp0=ParseUnaryExpr(Exp0);

  Descriptor DesExp0;
  Descriptor DesReturn;

  DesExp0=Exp0->GetDescriptor();  

  if (DesExp0.is_primitive())
    {
      DesReturn=UArithmetic_Promotion(DesExp0);

      /*
       * Alcune semplici ottimizzazioni di costant folding.
       */
      
      if (Exp0->is_INUMNode() || Exp0->is_FNUMNode())
	{
	  Exp0->SetDescriptor(DesReturn);
	  delete root;
	  return Exp0;
	}
      
      if (DesReturn!=DesExp0)
	{
	  Exp0=new EXPNode(CastOp,Dummy,Exp0);
	  Exp0->SetDescriptor(DesReturn);
	}
    }
  else
    {
      MsgErrors(Exp0->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"+",
		DesExp0.to_typename(),"int");
      DesReturn=DES_INT;
    }

  root->SetRightC(Exp0);
  root->SetDescriptor(DesReturn);
  return root;
}

TreeNode *CompileUnit::ParseUnarySubExpr(TreeNode *root)
{
  TreeNode   *Exp0=root->GetRightC();

  root->SetRightC(Dummy);

  Exp0=ParseUnaryExpr(Exp0);

  Descriptor DesExp0;
  Descriptor DesReturn;

  DesExp0=Exp0->GetDescriptor();  

  if (DesExp0.is_primitive())
    {
      DesReturn=UArithmetic_Promotion(DesExp0);
      
      /*
       * Alcune semplici ottimizzazioni di costant folding.
       */
      
      if (Exp0->is_INUMNode() || Exp0->is_FNUMNode())
	{
	  if (Exp0->is_INUMNode())
	    ((INUMNode *)Exp0)->SetVal(-((INUMNode *)Exp0)->GetVal());
	  else
	    ((FNUMNode *)Exp0)->SetVal(-((FNUMNode *)Exp0)->GetVal());
	  Exp0->SetDescriptor(DesReturn);
	  delete root;
	  return Exp0;
	}

      if (DesReturn!=DesExp0)
	{
	  Exp0=new EXPNode(CastOp,Dummy,Exp0);
	  Exp0->SetDescriptor(DesReturn);
	}
    }
  else
    {
      MsgErrors(Exp0->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"-",
		DesExp0.to_typename(),"int");
      DesReturn=DES_INT;
    }

  root->SetRightC(Exp0);
  root->SetDescriptor(DesReturn);
  return root;
}

/*
 * CompileUnit::ParseUnaryNotPlusMinusExpr
 *
 * Questo metodo implementa il parsing di una seconda categoria di espressioni
 * unarie:
 *
 * - casting;
 * - not logico;
 * - not dei bit;
 * - espressioni postfisse.
 */

TreeNode *CompileUnit::ParseUnaryNotPlusMinusExpr(TreeNode *root)
{
  switch (((EXPNode *)root)->GetNodeOp())
    {
    case LogicalNotOp: return ParseLogicalNotExpr(root);
    case BitwiseNotOp: return ParseBitwiseNotExpr(root);
    case CastOp      : return ParseCastExpr(root);
    default          : return ParsePostFixExpr(root);
    }  
}

/*
 * CompileUnit::ParseLogicalNotExpr
 *
 * Effettua il parsing di un'espressione di not logico, ad esempio:
 *
 *                              ! <expr>
 */

TreeNode *CompileUnit::ParseLogicalNotExpr(TreeNode *root)
{
  TreeNode   *Exp0=root->GetRightC();

  root->SetRightC(Dummy);

  Exp0=ParseUnaryExpr(Exp0);

  Descriptor DesExp0;

  DesExp0=Exp0->GetDescriptor();  

  if (DesExp0==DES_BOOLEAN)
    {
      if (Exp0->is_BOOLEANode())
	{
	  ((BOOLEANode *)
	   Exp0)->SetBoolean(!((BOOLEANode *)Exp0)->GetBoolean());
	  delete root;
	  return Exp0;
	}
    }
  else
    MsgErrors(Exp0->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"!",
	      DesExp0.to_typename(),"boolean");

  root->SetRightC(Exp0);
  root->SetDescriptor(DesBoolean);

  return root;
}

/*
 * CompileUnit::ParseBitwiseNotExpr
 *
 * Effettua il parsing di un'espressione di not bit a bit, ad esempio:
 *
 *                              ~ <expr>
 */

TreeNode *CompileUnit::ParseBitwiseNotExpr(TreeNode *root)
{
  TreeNode   *Exp0=root->GetRightC();

  root->SetRightC(Dummy);

  Exp0=ParseUnaryExpr(Exp0);

  Descriptor DesExp0;
  Descriptor DesReturn;

  DesExp0=Exp0->GetDescriptor();  

  if (DesExp0.is_integral())
    {
      DesReturn=UArithmetic_Promotion(DesExp0);

      if (Exp0->is_INUMNode())
	{
	  ((INUMNode *)Exp0)->SetVal(~((INUMNode *)Exp0)->GetVal());
	  delete root;
	  return Exp0;
	}
      if (DesExp0!=DesReturn)
	{
	  Exp0=new EXPNode(CastOp,Dummy,Exp0);
	  Exp0->SetDescriptor(DesReturn);
	}
    }
  else
    {
      DesReturn=DES_NULL;
      if (DesExp0.is_primitive())
	MsgErrors(Exp0->GetLine(),msg_errors[ERR_EXPLICIT_CAST_NEEDED],"~",
		  DesExp0.to_typename(),"int");
      else
	MsgErrors(Exp0->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"~",
		  DesExp0.to_typename(),"int");	
    }

  root->SetRightC(Exp0);
  root->SetDescriptor(DesReturn);
  return root;
}

/*
 * CompileUnit::ParseCastExpr
 *
 * Parsing di istruzioni di cast, come ad esempio:
 *
 *                      (<tipo>) <expr>
 */

TreeNode *CompileUnit::ParseCastExpr(TreeNode *root)
{
  TreeNode   *Exp0=root->GetRightC();
  TreeNode   *ETree;

  Exp0=ParseUnaryExpr(Exp0);
  
  Descriptor DesExp0;
  Descriptor DesReturn;

  DesExp0=Exp0->GetDescriptor();

  if (!root->GetLeftC()->IsDummy())
    {
      ETree=ParseExpr(root->GetLeftC());
      DesReturn=ETree->GetDescriptor();
      delete ETree;
      root->SetLeftC(Dummy);
    }
  else
    DesReturn=root->GetDescriptor();

  if (!is_Cast_Conversion(DesReturn,DesExp0))
    MsgErrors(Exp0->GetLine(),msg_errors[ERR_INVALID_CAST],
	      DesExp0.to_typename(),DesReturn.to_typename());

  root->SetRightC(Exp0);
  root->SetDescriptor(DesReturn);

  return root;  
}

/*
 * CompileUnit::ParsePostFixExpr
 *
 * Gestisce il parsing di espressioni postfisse come:
 *
 * - post incremento/decremento;
 * - riferimenti a nomi (singoli o composti);
 * - espressioni primary.
 *
 * Con il termine espressioni primary, intendiamo:
 *
 * - creazione array;
 * - creazione classi
 * - accessi a campi;
 * - invocazione metodi;
 * - accesso ad array;
 * - espressioni generiche il cui parsing verra' fatto con ParseExpr;
 * - this;
 * - literal: costante intera, reale, booleana, carattere, stringa, null.
 */  

TreeNode *CompileUnit::ParsePostFixExpr(TreeNode *root)
{
  if (root->is_UNAMENode())
    return ParseNameExpr(root);

  if (root->is_EXPNode())
    {
      int op=((EXPNode *)root)->GetNodeOp();
      
      if (op==PlusPlusOp || op==MinusMinusOp)
	return ParsePostIncDecExpr(root);
    }
  return ParsePrimaryExpr(root);
}

/*
 * CompileUnit::ParseNameExpr
 *
 * Si preoccupa della risoluzione di nomi, costrundo il parse-tree che descri-
 * vera' tale nodo.
 */

TreeNode *CompileUnit::ParseNameExpr(TreeNode *root)
{
  String name;
  STNode *node, *startnode;
  TreeNode *t1, *t2, *t3;
  int ambigous=FALSE;

  name=((UNAMENode *)root)->GetName();

  if (name.is_singlename())
    {
      /*
       * Possono verificarsi due casi:
       * 
       * 1) se l'identificatore compare nello scoping di variabili locali o 
       *    parametri, allora questo nome fa riferimento a uno di questi.
       *    Il tipo dell'espressione sara' il tipo del parametro o variabile 
       *    globale.
       * 2) Altrimenti, il nome e' trattato come un'espressione del tipo
       *                 this.identificatore
       */

      if (((node=table->LookUpHere(name, FOURTH_LVL))!=NULL))
	{
	  t1=new IDNode(node);
	  t1->SetLine(root->GetLine());
	  delete root;
	  return t1;
	}
      else

	/*
	 * Attenzione!!!
	 * node potrebbe essere NULL anche perche' il riferimento al campo e' 
	 * ambiguo.
	 */

	if ((node=SearchField(current_class, name, &ambigous,
			      root->GetLine()))!=NULL)
	  {
	    if (IS_FINAL(node->getaccess()))
	      {
		/*
		 * NON GESTITO!!!
		 *
		 * Il campo e' una costante, per cui volendo eseguire a
		 * dovere le ottimizzazioni di costant folding, dovremo 
		 * restituire il rispettivo valore.
		 */		  
		}

	    if (node->getmyclass()==current_class)
	      {
		t1=new THISNode();
		t1->SetDescriptor(current_class->getdescriptor());
	      }
	    else
	      t1=new IDNode(node->getmyclass());
	    
	    t1->SetLine(root->GetLine());
	    
	    t2=new EXPNode(FieldAccessOp,t1,new IDNode(node) /*  root */ );
	    t2->SetDescriptor(node->getdescriptor());
	    t2->SetLine(root->GetLine());
	    
	    return t2  /* ParseFieldAccess(t2) */ ;
	  }
	else
	  if (ambigous)
	    return root;
	  else
	    if (((node=table->LookUpHere(name, SECOND_LVL))!=NULL))
	      {
		t1=new IDNode(node);
		t1->SetLine(root->GetLine());
		delete root;
		return t1;
	      }
	    else
	      if ((node=LoadClass(name,++LastIndex))!=NULL)
		{
		  /*
		   * NON GESTITO!!!
		   *
		   * Quando ho testato l'accesso ai campi, non ho tenuto conto 
		   * del fatto che un singlename puo' far riferimento a una 
		   * classe. I test gia' fatti continuerranno a funzionare, il 
		   * problema forse riguardera' nel farne nuovi che tengano 
		   * conto di questo fatto.
		   */
		  
		  t1=new IDNode(node);
		  t1->SetLine(root->GetLine());
		  delete root;
		  return t1;
		}
    }
  else
    {
      /*
       * Ci sono 3 casi:
       *
       * 1) se e' un nome composto del tipo PackageName.Iden, allora ci sara'
       *    un errore di compilazione.
       * 2) Altrimenti, se e' un nome composto del tipo TypeName.Iden, allora
       *    esso si riferisce a un campo static della classe o interfaccia
       *    TypeName.
       *    Ci sara' un errore di compilazione se TypeName non denota una clas-
       *    se o interfaccia. Inoltre, un errore di compilazione si verifica
       *    se TypeName non contiene un campo accessibile con quel nome.
       *    Il tipo dell'espressione e' in effetti il tipo del campo static, 
       *    mentre il valore e' il campo stesso.
       * 3) Altrimenti, se e' un nome del tipo Ename.Identifier, con Ename
       *    espressione qualsiasi, allora essa viene trattata come un accesso
       *    a campo del tipo
       *                     (Ename).Identifier
       */

      if (name.to_package()==package_name)

	/*
	 * Caso 1.
	 */

	MsgErrors(root->GetLine(),msg_errors[ERR_UNDEF_VAR],name.to_char());
      else
	if ((startnode=LoadClass(name.to_package(),++LastIndex))!=NULL)
	  {
	    /*
	     * Caso 2.
	     */

	    t1=new IDNode(startnode);
	    t1->SetLine(root->GetLine());
	    t2=new UNAMENode(name.to_name());
	    t2->SetLine(root->GetLine());
	    t3=new EXPNode(FieldAccessOp,t1,t2);
	    t3->SetLine(root->GetLine());
	    return ParseFieldAccess(t3);
	  }
	else
	  {
	    /*
	     * Caso 3.
	     */
	    
	    t1=new UNAMENode(name.to_package());
	    t1->SetLine(root->GetLine());
	    t1=ParseNameExpr(t1);
	    t1->SetLine(root->GetLine());
	    t2=new UNAMENode(name.to_name());
	    t2->SetLine(root->GetLine());
	    t3=new EXPNode(FieldAccessOp,t1,t2);
	    t3->SetLine(root->GetLine());
	    return ParseFieldAccess(t3);
	  }
    }  
}

/*
 * CompileUnit::ParsePostIncDecExpr
 *
 * Compilazione di espressioni unarie del tipo
 *
 *                   <expr> ++
 *                   <expr> --
 */

TreeNode *CompileUnit::ParsePostIncDecExpr(TreeNode *root)
{
  const char       *opstr;
  int        op=((EXPNode *)root)->GetNodeOp();

  TreeNode   *Exp0=root->GetLeftC();

  Exp0=ParsePostFixExpr(Exp0);

  Descriptor DesExp0;
  Descriptor DesReturn;

  DesExp0=Exp0->GetDescriptor();
  DesReturn=Exp0->GetDescriptor();

  /*
   * Attenzione!!!
   *
   * L'argomento di un operatore di post-incremento/decremento, deve essere
   * sempre una variabile e mai un valore, altrimenti verra' lanciato
   * l'errore ERR_INVALID_ARG. Il risultato del post-incremento/decremento
   * nella generazione del codice, dovra' essere un valore e non una variabile.
   * Si ricordi che gli operatori di post-incremento/decremento, non possono
   * essere applicati a vaiabili (campi) final.
   */

  if (op==PlusPlusOp)
    opstr="++";
  else
    opstr="--";
  
  if (!IsVariable(Exp0))
    MsgErrors(Exp0->GetLine(),msg_errors[ERR_INVALID_ARG],opstr);

  if (!DesReturn.is_primitive())
    {
      MsgErrors(Exp0->GetLine(),msg_errors[ERR_INVALID_ARG_TYPE],
		DesExp0.to_typename(),opstr);

      DesReturn=DES_INT;
    }

  root->SetLeftC(Exp0);
  root->SetDescriptor(DesReturn);
  return root;  
}

/*
 * CompileUnit::ParsePrimaryExpr
 *
 * Da questo metodo parte il parsing per istruzioni come:
 *
 * - creazione istanza di array (non gestito nella versione del 03/09/97);
 * - riferimenti a: num. interi, num. reali, valori booleani, caratteri,
 *   stringhe, null;
 * - riferimenti a this.
 * - nuove espressioni tra parentesi (<expr>);
 * - creazione istanza di una classe;
 * - accesso a campi;
 * - invocazione di metodi;
 * - accesso ad array.
 *
 * Per capire questa breve sintesi basta guardare la grammatica (java.y) a
 * partire da simbolo non terminale Primary.
 */

TreeNode *CompileUnit::ParsePrimaryExpr(TreeNode *root)
{
  if (root->is_EXPNode())
    switch (((EXPNode *)root)->GetNodeOp())
      {
      case ArrayAccessOp: return ParseArrayAccess(root);
      case FieldAccessOp: return ParseFieldAccess(root);
      case MethodCallOp : return ParseMethodCall(root);
      case NewOp        : return ParseCreateClassInstance(root);
      case NewArrayOp   : return ParseArrayCreation(root);
      }
 
  if (root->is_INUMNode()  || root->is_FNUMNode()   || root->is_BOOLEANode() ||
      root->is_CHARNode()  || root->is_STRINGNode() || root->is_NULLNode()   ||
      root->is_THISNode())
    return root;

  if (root->is_SUPERNode())
    {
      STNode *superclass=current_class->get_superclass();
      root->SetDescriptor(superclass->getdescriptor());
      return root;
    }

  /*
   * Poiche' non si sono verificati i test precedenti, allora l'espressione
   * e' di tipo (<expr>) per cui attivo ParseExpr su root.
   */

  return ParseExpr(root);
}

/*
 * CompileUnit::ParseArrayAccess
 *
 * Gestisce il parsing dell'accesso ad array.
 */

TreeNode *CompileUnit::ParseArrayAccess(TreeNode *root)
{
  TreeNode   *Exp0=root->GetLeftC();
  TreeNode   *Exp1=root->GetRightC();

  if (Exp0->is_UNAMENode())
    Exp0=ParseNameExpr(Exp0);
  else
    Exp0=ParsePrimaryExpr(Exp0);

  Exp1=ParseExpr(Exp1);

  Descriptor DesExp0;
  Descriptor DesExp1;
  Descriptor DesReturn;  

  DesExp0=Exp0->GetDescriptor();
  DesExp1=Exp1->GetDescriptor();

  if (!DesExp0.is_array())
    {
      MsgErrors(Exp0->GetLine(),msg_errors[ERR_NOT_ARRAY],
		DesExp0.to_typename());
      DesReturn=DES_NULL;
    }
  else
    {
      DesReturn=UArithmetic_Promotion(DesExp1);
      if (DesExp1!=DesReturn)
	Exp1=new EXPNode(CastOp,Dummy,Exp1);
      DesReturn=DesExp0.array_comp_type();
    }

  root->SetLeftC(Exp0);
  root->SetRightC(Exp1);
  root->SetDescriptor(DesReturn);
  return root;
}

/*
 * CompileUnit::ParseFieldAccess
 * CompileUnit::SearchField
 * CompileUnit::SearchFieldInSuperInterfaces
 * CompileUnit::IsAccessibleField
 *
 * Effettua il parsing di un'espressione di accesso a campi. ParseFieldAccess
 * utilizza alcuni metodi di supporto come SearchField che ricerca il metodo
 * nella classe specificata nel file sorgente. Questo metodo utilizza altri
 * tre metodi: SerachFieldInSuperClass, SearchFieldInSuperInterfaces e 
 * IsAccessibleField; il primo ricerca il campo tra le superclassi della classe
 * specificata, il secondo ricerca il campo tra le superinterfacce, mentre il
 * terzo controlla l'accessibilita' di un campo.
 */

TreeNode *CompileUnit::ParseFieldAccess(TreeNode *root)
{
  STNode *FNode, *StartNode;

  String Name;

  TreeNode   *Exp0=root->GetLeftC();
  TreeNode   *Exp1=root->GetRightC();

  root->SetLeftC(Dummy);
  root->SetRightC(Dummy);

  /*
   * La chiamata a ParseFieldAccess, puo' avvenire anche da ParseNameExpr,
   * dove la parte sinistra di root e' gia' risolta e quindi qua Exp1 puo'
   * anche essere un IDNode.
   */

  if (Exp1->is_IDNode())
    Name=((IDNode *)Exp1)->GetIden()->getname();
  else
    if (Exp1->is_UNAMENode())
      Name=((UNAMENode *)Exp1)->GetName();

  /*
   * Exp0 puo' essere una Primary Expression o un SUPERNode, mentre Exp1 e'
   * sicuramente un UNAMENode.
   * Analizzando il test47, mi sono reso conto che una chiamata a 
   * ParseFieldAccess, puo' avvenire da ParseNameExpr dove Exp0 puo' essere
   * un IDNode.
   */

  if (!Exp0->is_IDNode())
    if (!Exp0->is_SUPERNode())
      Exp0=ParsePrimaryExpr(Exp0);
    else
      Exp0->SetDescriptor(current_class->get_superclass()->getdescriptor());

  Descriptor DesExp0;

  DesExp0=Exp0->GetDescriptor();

  if (DesExp0.is_reference())
    {
      /*
       * Possiamo accedere a campi di classe, interfaccia o array. Gli array 
       * dispongono di un solo campo, chiamato
       * "length" che fornisce la lunghezza dell'array.
       */

      if (DesExp0.is_array())
	if (Name!="length")
	  {
	    MsgErrors(Exp0->GetLine(),msg_errors[ERR_INVALID_FIELD_REFERENCE],
		      Name.to_char(),"array");
	    FNode=table->Install_Id(Name,0,0,DesNull);
	    table->Discard(FNode);
	  }
	else
	  FNode=ArrayLengthNode;
      else
	{
	  StartNode=LoadClass(DesExp0.to_fullname(),++LastIndex);

	  /*
	   * Se FNode==NULL, nel seguente if, allora significa che il 
	   * campo o non e' accessibile o non esiste. In ogni caso e'
	   * bene che il messaggio di errore venga inviato nel metodo
	   * CompileUnit::AccessibleField.
	   */

	  int ambigous=FALSE;
	  
	  if ((FNode=SearchField(StartNode,Name,&ambigous,
				 root->GetLine()))!=NULL)
	    {
	      STNode *node=Exp0->is_IDNode() ? ((IDNode*)Exp0)->GetIden() :
		(STNode *)NULL;

	      /*
	       * Se node denota una classe o interfaccia, allora stiamo
	       * in presenza di un riferimento a un campo statico. In tal
	       * caso e' necessario che il campo sia STATIC, altrimenti si 
	       * verifica un errore di compilazione.
	       */
	      
	      if (node!=NULL && (node->is_class() || node->is_interface()) &&
		  !IS_STATIC(FNode->getaccess()))
		MsgErrors(root->GetLine(),
			  msg_errors[ERR_NO_STATIC_FIELD_ACCESS],
			  Name.to_char(),current_class->getname().to_char());

	      /*
	       * NON GESTITO!!!
	       *
	       * se il campo e' final ed e' un rvalue, allora il risultato 
	       * dovra' essere un valore.
	       * Questo passo dovra' essere svolto se si vorra' implementare
	       * una corretta gestione di ottimizzazioni di costant folding.
	       */

	    }
	  else
	    {
	      FNode=table->Install_Id(Name,0,0,DesNull);
	      table->Discard(FNode);
	    }
	}
    }
  else
    {
      MsgErrors(root->GetLine(),msg_errors[ERR_INVALID_FIELD_REFERENCE],
		Name.to_char(),current_class->getname().to_char());
      FNode=table->Install_Id(Name,0,0,DesNull);
      table->Discard(FNode);
    }

  /*
   * Se stiamo in un inizializzatore statico e il campo non e' anch' esso
   * statico, allora ci sara' un errore di compilazione.
   */
  
  if (env->is_inContext(IN_STATIC) && !IS_STATIC(FNode->getaccess()))
    MsgErrors(FNode->getline(),msg_errors[ERR_NO_STATIC_METH_ACCESS],
	      FNode->getname().to_char(),current_class->getname().to_char());

  root->SetLeftC(Exp0);
  delete Exp1;
  root->SetRightC(new IDNode(FNode));
  root->SetDescriptor(FNode->getdescriptor());
  return root;
}

STNode *CompileUnit::SearchField(STNode *SNode, String &FName, int *ambigous,
				 int line)
{
  STNode *n=NULL, *m=NULL, *fnode=NULL;

  for (fnode=table->LookUpHere(FName,THIRD_LVL,SNode->getindex()); fnode;
       fnode=table->NextSym(fnode,THIRD_LVL,SNode->getindex()))
    if (fnode->is_field()) break;
  
  if (fnode)
    if (!IsAccessibleField(fnode,SNode))
      {
	MsgErrors(line,msg_errors[ERR_NO_FIELD_ACCESS],FName.to_char(),
		  SNode->getname().to_char(),
		  current_class->getname().to_char());
	return NULL;
      }
    else
      return fnode;
  else
    {
      /*
       * Il campo non e' presente nella classe specificata in SearchNode, per 
       * cui bisognera' ricercare il campo nelle superclassi e superinterfacce
       * controllando eventuali ambiguita'.
       */
      
      m=SearchFieldInSuperInterfaces(SNode, SNode, FName, line, ambigous);

      if (*ambigous) return NULL;

      /*
       * Ricerco il campo tra le superclassi della classe definita in SNode.
       */

      if (SNode->get_superclass())
	n=SearchFieldInSuperClass(SNode->get_superclass(), SNode, FName, 
				  ambigous, line);

      if (*ambigous) return NULL;

      if (!m && !n) 
	{
	  MsgErrors(line,msg_errors[ERR_NO_SUCH_FIELD],FName.to_char(),
		    SNode->getname().to_char());
	  return NULL;
	}

      if (m && !n) return m;

      if (!m && n) return n;

      if (m && n)
	{
	  MsgErrors(line,msg_errors[ERR_AMBIG_FIELD],FName.to_char(),
		    n->getname().to_char(),m->getname().to_char());
	  *ambigous=TRUE;
	  return NULL;
	}
    }
}

STNode *CompileUnit::SearchFieldInSuperClass(STNode* SearchNode, STNode *SNode,
					     String &FName, int *ambigous,
					     int line)
{
  STNode *n=NULL, *m=NULL, *fnode=NULL;

  for (fnode=table->LookUpHere(FName,THIRD_LVL,SearchNode->getindex()); fnode;
       fnode=table->NextSym(fnode,THIRD_LVL,SearchNode->getindex()))
    if (fnode->is_field()) break;
  
  if (fnode)
    if (!IsAccessibleField(fnode,SNode))
      {
	MsgErrors(line,msg_errors[ERR_NO_FIELD_ACCESS],FName.to_char(),
		  SNode->getname().to_char(),
		  current_class->getname().to_char());
	return NULL;
      }
    else
      return fnode;
  else
    {
      /*
       * Il campo non e' presente nella classe specificata in SearchNode, per 
       * cui bisognera' ricercare il campo nelle superclassi e superinterfacce
       * controllando eventuali ambiguita'.
       */
      
      m=SearchFieldInSuperInterfaces(SearchNode, SNode, FName, line, ambigous);

      if (*ambigous) return NULL;

      /*
       * Ricerco il campo tra le superclassi della classe definita in SNode.
       */

      if (SearchNode->get_superclass())
	n=SearchFieldInSuperClass(SearchNode->get_superclass(), SNode, FName, 
				  ambigous, line);

      if (*ambigous) return NULL;

      if (m && n)
	{
	  MsgErrors(line,msg_errors[ERR_AMBIG_FIELD],FName.to_char(),
		    n->getname().to_char(),m->getname().to_char());
	  *ambigous=TRUE;
	  return NULL;
	}

      if (m) 
	return m;
      else
	return n;
    }
}

STNode *CompileUnit::SearchFieldInSuperInterfaces(STNode *SearchNode, 
						  STNode *SNode, String &FName,
						  int line, int *ambigous)
{
  TreeNode *tree;
  STNode   *fnode=NULL;

  if (SearchNode->is_class())
    tree=SearchNode->getmyheader()->GetLeftC()->GetRightC()->GetRightC();
  else
    tree=SearchNode->getmyheader()->GetLeftC()->GetRightC();

  for (; !tree->IsDummy(); tree=tree->GetLeftC())
    {
      STNode *m, *intfnode;

      intfnode=((IDNode *)tree->GetRightC())->GetIden();

      int index=intfnode->getindex();

      for (m=table->LookUpHere(FName,THIRD_LVL,index); m; 
	   m=table->NextSym(m,THIRD_LVL,index))
	if (m->is_field() && IsAccessibleField(m,SNode))
	  return m;

      m=SearchFieldInSuperInterfaces(intfnode, SNode, FName, line, ambigous);

      if (*ambigous) return NULL;

      if (!fnode)
	fnode=m;
      else
	if (fnode!=m)
	  {
	    *ambigous=TRUE;
	    MsgErrors(line,msg_errors[ERR_AMBIG_FIELD],FName.to_char(),
		      fnode->getname().to_char(),m->getname().to_char());
	    return NULL;
	  }
    }

  return fnode;
}

int CompileUnit::IsAccessibleField(STNode *FNode, STNode *SClass)
{
  if (SClass!=current_class)
    if (current_class->is_subclass(SClass))
      if (!IS_PUBLIC(FNode->getaccess()) && 
	  !IS_PROTECTED(FNode->getaccess()) && 
	  (SClass->getdescriptor().to_packagename()!=
	   current_class->getdescriptor().to_packagename()))
	return FALSE;
      else
	if (SClass->getdescriptor().to_packagename() ==
	    current_class->getdescriptor().to_packagename())
	  {
	    if (IS_PRIVATE(FNode->getaccess()))
	      return FALSE;
	  }
	else
	  if (!IS_PUBLIC(FNode->getaccess()))
	    return FALSE;

  return TRUE;
}

/*
 * CompileUnit::ParseMethodCall
 * CompileUnit::ParseMethodCall2
 * CompileUnit::ParseArgsExpr
 * CompileUnit::ConvertArgs
 * CompileUnit::IsMostSpecific
 * CompileUnit::IsApplicable
 * CompileUnit::IsApplicable2
 * CompileUnit::IsAccessible
 * CompileUnit::SelectMethod
 * CompileUnit::SelectMethodInLink
 * CompileUnit::SelectMethodInSuperClass
 * CompileUnit::SelectMethodInSuperInterfaces
 *
 * ParseMethodCall e ParseMethodCall2 effettuano il parsing delle invocazioni 
 * a metodi. Essi si differenziano per il fatto che ParseMethodCall effettua
 * una preelaborazione del parse-tree qualora anche il nome della classe di
 * riferimento e' "unresolved", mentre ParseMethodCall2 lavora quando il nome
 * della classe di riferimento e' "resolved", e bisogna risolvere il riferimen-
 * to al metodo.
 * Questa fase e' la piu' delicata e complessa dell'intero parser. 
 * Affinche' l'algoritmo sia comprensibile, iniziamo con il formulare alcune 
 * definizioni.
 *
 * 1) Sia m un metodo definito nella classe D. m e' accessibile da una classe C
 *    se e solo se una delle seguenti condizioni e' vera:
 *
 *          - C=D;  
 *          - C extends D;  
 *          - C!=D ma il metodo e' PUBLIC; 
 *          - C!=D ma sono definite nello stesso pacchetto.
 *
 * 2) L'invocazione di un metodo m(t1,...tK) e' applicabile a una definizione 
 *    m(u1,...,un) se e solo se:
 *
 *          - k=n;
 *          - ti e' assegnabile a ui secondo l'algoritmo di method conversion,
 *            per ogni i=1..k;
 *
 * Data l'invocazione di un metodo a partire da una classe C, verranno ricerca-
 * ti tutti i metodi accessibili e applicabili all'invocazione nella classe
 * C e nelle sue superclassi (trascurando metodi sovrascritti). Sia S l'insieme
 * ottenuto.
 * Se S e' vuoto, allora ci sara' un errore di compilazione. Se S contiene
 * un solo elemento allora questi sara' quello voluto. Se la cardinalita' di 
 * S e' maggiore di 0, allora si sceglie la definizione "piu' specifica".
 *
 * Sia m(u1..un) definito in T
 *     m(t1..tn) definito in Y
 * allora m(t1..tn) e' piu' specifico se e solo se Y e' assegnabile a T (secon-
 * do l'algoritmo di assign conversion) e per ogni j tj e' method conversion
 * in uj.
 * A supporto di ParsemethodCall, vengono utilizzati altri metodi privati di
 * supporto come ConvertArgs che prende l'albero degli argomenti ed esegue
 * eventuali conversioni implicite rispetto ai parametri, c''e poi
 * IsApplicable e IsAccessible utilizzati per trovare metodi applicabili e
 * accessibili. Se di questi metodi c'e' ne sono piu' di uno, con 
 * SearchMostSpecific andremo a selezionare il metodo piu' specifico.
 * Si osservi che IsApplicable invoca IsApplicable2, cio' viene fatto perche'
 * quando tra gli argomenti compare null, non si sa il descrittore da associare
 * per cui in base ai parametri si trova il tipo per null e si aggiorna il
 * descrittore generale degli argomenti. Magari in futuro miglioreremo questa
 * modalita' di gestione utilizzando un solo metodo (IsApplicable) iterativo
 * anziche' ricorsivo.
 */

TreeNode *CompileUnit::ParseMethodCall(TreeNode *root)
{
  String Name;

  TreeNode *Exp0=root->GetLeftC();

  if (Exp0->is_UNAMENode())
    {
      STNode *StartClass;

      STNode *node, *start_class;

      Name=((UNAMENode *)Exp0)->GetName();

      if (Name.is_singlename())
	{
	  TreeNode *t=new THISNode();
	  t->SetDescriptor(current_class->getdescriptor());
	  Exp0=new EXPNode(CommaOp,t,Exp0);
	}
      else
	{
	  
	  /*
	   * Possono verificarsi 3 casi:
	   *
	   * 1) se e' un qualified name del tipo PackageName.Iden, allora si 
	   *    verifichera' un errore di compilazione.
	   * 2) Altrimenti, se esso e' un qualified name della forma 
	   *    TypeName.Iden, allora il nome del metodo e' Iden, mentre 
	   *    TypeName denota la classe di appartenenza. Se TypeName e' il 
	   *    nome di un'interfaccia allora ci sara' un errore di compila-
	   *    zione, perche' con tale forma si puo' invocare solo metodi 
	   *    statici.
	   * 3) In tutti gli altri casi, allora la forma e' FieldName.Iden,
	   *    il nome del metodo e' Iden, mentre l'interfaccia o classe da 
	   *    ricercare e' data dal tipo del campo.
	   */

	  if (Name.to_package()==package_name)
	    MsgErrors(root->GetLine(),msg_errors[ERR_UNDEF_VAR],
		      package_name.to_char());
	  else
	    if ((StartClass=LoadClass(Name.to_package(),++LastIndex))!=NULL)
	      {
		delete Exp0;
		Exp0=new EXPNode (CommaOp, 
				  new IDNode(StartClass), 
				  new UNAMENode(Name.to_name()));
	      }
	    else
	      {
		delete Exp0;
		Exp0=
		 new EXPNode(CommaOp,
			     ParseNameExpr(new UNAMENode(Name.to_package())),
			     new UNAMENode(Name.to_name()));
	      }
	}
    }

  root->SetLeftC(Exp0);

  /*
   * A questo punto il nome dell'eventuale classe di appartenenza del meto-
   * do e' stato risolto. Ora l'albero ha il seguente formato:
   *
   *                              [ ]
   *                           ____|____
   *                          |         |
   *                      [CommaOp]    args
   *                      ____|____
   *                     |         |
   *               Primary Expr.  UNAMENode
   *                     o
   *                 SUPERNode
   *
   * A questo punto bisognera' procedere con la risoluzione del nome del 
   * metodo.
   */
  
  return ParseMethodCall2(root);
}

TreeNode *CompileUnit::ParseMethodCall2(TreeNode *root)
{
  STNode *StartClass, *MNode;
  Descriptor DesExp0;
  String     Name;

  /*
   * Se l'albero in input ha il formato
   *
   *                              [ ]
   *                           ____|____
   *                          |         |
   *                      [CommaOp]    args
   *                      ____|____
   *                     |         |
   *               Primary Expr.  IDNode
   *                     o
   *                 SUPERNode   
   * 
   * allora e' inutile effettuare il parsing, perche' l'espressione e' gia'
   * risolta.
   */

  if (root->GetLeftC()->GetRightC()->is_IDNode()) return root;

  Name=((UNAMENode*)root->GetLeftC()->GetRightC())->GetName();

  if (Name=="<init>") 
    return ParseConstrCall(root);

  delete root->GetLeftC()->GetRightC();

  TreeNode *Exp0=root->GetLeftC()->GetLeftC();
  TreeNode *Exp1=root->GetRightC();

  if (!Exp0->is_IDNode())
    if (!Exp0->is_SUPERNode())
      Exp0=ParsePrimaryExpr(Exp0);
    else
      Exp0->SetDescriptor(current_class->get_superclass()->getdescriptor());

  Exp1=ParseArgsExpr(Exp1);

  Exp1->SetLine(root->GetLine());

  DesExp0=Exp0->GetDescriptor();

  if (DesExp0.is_reference())
    {
      if (DesExp0.is_array())
	{
	  /*
	   * NON GESTITO!!!
	   *
	   * Anche gli array dispongono di alcuni metodi predefiniti. Penso
	   * che in questa versione non verranno proprio gestiti.
	   */
	}
      else
	{
	  StartClass=LoadClass(DesExp0.to_fullname(),++LastIndex);

	  /*
	   * Possono verificarsi 2 casi:
	   * 
	   * a) StartClass e' una classe.
	   * b) StartClass e' un'interfaccia.
	   */

	  if ((MNode=SelectMethod(StartClass,Name,Exp1))!=NULL)
	    {
	      STNode *node=Exp0->is_IDNode() ? ((IDNode*)Exp0)->GetIden() :
		           (STNode*) NULL;
	      
	      if ((node) && (node->is_class() || node->is_interface()))
		
		/*
		 * Riferimento statico al metodo.
		 */
		
		if (!IS_STATIC(MNode->getaccess()))
		  MsgErrors(root->GetLine(),
			    msg_errors[ERR_NO_STATIC_METH_ACCESS],
			    Name.to_char(), StartClass->getname().to_char());
	      
	      TreeNode *ptree=
		MNode->getmyheader()->GetLeftC()->GetLeftC()->GetRightC();
	      Exp1=ConvertArgs(ptree,Exp1);
	    }
	  else
	    {
	      /*
	       * Metodo non trovato o inaccessibile o ambiguo.
	       */
	      
	      Descriptor descriptor;
	      
	      descriptor.build_method(Exp1->GetDescriptor(),DesNull);
	      
	      MNode=table->Install_Id(Name,THIRD_LVL,-1,descriptor);
	      table->Discard(MNode);
	    }
	}
    }
  else
    {
      Descriptor descriptor;

      descriptor.build_method(Exp1->GetDescriptor(),DesNull);
		  
      MNode=table->Install_Id(Name,THIRD_LVL,-1,descriptor);
      table->Discard(MNode);

      MsgErrors(root->GetLine(),msg_errors[ERR_INVALID_METHOD_INVOKE],
		Name.to_char());
    }

  /*
   * Se stiamo in un inizializzatore statico e il metodo non e' anch' esso
   * statico, allora ci sara' un errore di compilazione.
   */

  if (env->is_inContext(IN_STATIC) && !IS_STATIC(MNode->getaccess()))
    MsgErrors(MNode->getline(),msg_errors[ERR_NO_STATIC_METH_ACCESS],
	      Name.to_char(),StartClass->getname().to_char());

  root->GetLeftC()->SetLeftC(Exp0);
  root->GetLeftC()->SetRightC(new IDNode(MNode));
  root->SetDescriptor(MNode->getdescriptor().return_type());

  return root;
}

TreeNode *CompileUnit::ParseArgsExpr(TreeNode *root)
{
  /*
   * In effetti in questo metodo a noi interessa risolvere l'espressioni 
   * che denotano ciascun argomento ed estrarre il descrittore finale.
   */

  Descriptor DesArgs;

  DesArgs=DES_NULL;

  for (TreeNode *arg=root; !arg->IsDummy(); arg=arg->GetRightC())
    {
      arg->SetLeftC(ParseExpr(arg->GetLeftC()));
      DesArgs+=arg->GetLeftC()->GetDescriptor();
    }

  root->SetDescriptor(DesArgs);
  return root;
}

STNode *CompileUnit::SelectMethod(STNode *StartClass, String& Name, 
				  TreeNode *args)
{
  /*
   * a) StartClass puo' essere una classe o un'interfaccia.
   * b) args->GetDescriptor contiene solo il tipo degli argomenti, senza le 
   *    parentesi "()".
   */

  STNode *fmeth=NULL; // conterra' il risultato della ricerca.
  int found=FALSE;
  int line=args->GetLine();

  if ((fmeth=SearchMethodInLink(StartClass,StartClass,Name,args,&found))!=NULL)
    return fmeth;
  else
    {
      /*
       * Bisognera' ricercare il metodo nelle superclassi e superinterfacce di
       * StartClass.
       */

      STNode *n=NULL, *m=NULL;
      int ambigous=FALSE;

      if (StartClass->is_class())
	{
	  m=SearchMethodInSuperClass(StartClass,Name,args,&found,
				     &ambigous,args->GetLine());
	  if (ambigous) return NULL;
	}

      n=SearchMethodInSuperInterfaces(StartClass,StartClass,Name,args,&found,
				      &ambigous,args->GetLine());
      if (ambigous) return NULL;
      
      if (m && n)
	if (IsMostSpecific(m,n))
	  return m;
	else
	  if (IsMostSpecific(n,m))
	    return n;
	  else
	    {
	      MsgErrors(line,msg_errors[ERR_AMBIG_FIELD],Name.to_char(),
			m->getmyclass()->getname().to_char(),
			n->getmyclass()->getname().to_char());
	      return NULL;
	    }
      
      if (m) return m;

      if (n) return n;

      if (!found)
	MsgErrors(line,msg_errors[ERR_UNDEF_METH],Name.to_char(),
		  StartClass->getname().to_char());
      else
	MsgErrors(line,msg_errors[ERR_UNMATCHED_METH],Name.to_char(),
		  StartClass->getname().to_char());
      
      return NULL;
    }
}
	
STNode *CompileUnit::SearchMethodInLink(STNode *SearchClass,STNode *StartClass,
					String& Name, TreeNode *args, 
					int *found)
{
  STNode *fmeth=NULL;
  int index=SearchClass->getindex();

  for (STNode *node=table->LookUpHere(Name,THIRD_LVL,index); node; 
       node=table->NextSym(node,THIRD_LVL,index))
    {
      if (node->is_method())
	{
	  *found=TRUE;
	  
	  TreeNode *ptree=
	    node->getmyheader()->GetLeftC()->GetLeftC()->GetRightC();

	  if (IsAccessible(node,StartClass) && IsApplicable(ptree,args))
	    if (!fmeth) 
	      fmeth=node;
	    else
	      {
		/*
		 * Ci sono due metodi accessibili e applicabili alla
		 * invocazione. Si procede selezionando il piu' specifico.
		 */

		if (node->getdescriptor()!=fmeth->getdescriptor())
		  if (IsMostSpecific(node,fmeth))
		    fmeth=node;
	      }
	}
    }

  return fmeth;
}

STNode *CompileUnit::SearchMethodInSuperClass(STNode *StartClass, String& Name,
					      TreeNode *args, int *found, 
					      int *ambigous, int line)
{
  STNode *fmeth=NULL;

  for (STNode *node=StartClass; node; node=node->get_superclass())
    {
      STNode *m;

      m=SearchMethodInLink(node,StartClass,Name,args,found);

      if (m && fmeth)
	{
	  if (m->getdescriptor()!=fmeth->getdescriptor())
	    if (IsMostSpecific(m,fmeth))
	      fmeth=m;
	    else
	      if (!IsMostSpecific(fmeth,m))
		{
		  *ambigous=TRUE;
		  MsgErrors(line,msg_errors[ERR_AMBIG_FIELD],
			    Name.to_char(),
			    fmeth->getmyclass()->getname().to_char(),
			    m->getmyclass()->getname().to_char());
		  return NULL;		  
		}
	}
      else
	if (!fmeth && m)
	  fmeth=m;
    }

  return fmeth;
}

STNode *CompileUnit::SearchMethodInSuperInterfaces(STNode *SearchClass, 
						   STNode *StartClass, 
						   String& Name, 
						   TreeNode *args,
						   int *found, int *ambigous,
						   int line)
{
  TreeNode *tree;
  STNode *fmeth=NULL;

  if (SearchClass->is_class())
    tree=SearchClass->getmyheader()->GetLeftC()->GetRightC()->GetRightC();
  else
    tree=SearchClass->getmyheader()->GetLeftC()->GetRightC();

  for (;!tree->IsDummy(); tree=tree->GetLeftC())
    {
      STNode *m, *intfnode, *n;

      intfnode=((IDNode *)tree->GetRightC())->GetIden();

      m=SearchMethodInLink(intfnode,StartClass,Name,args,found);

      if (!m)
	{
	  m=SearchMethodInSuperInterfaces(intfnode,StartClass,Name,args,found,
					  ambigous,line);
	  if (*ambigous) return NULL;
	  if (!m) continue;
	}
      
      if (fmeth && m)
	{
	  if (m->getdescriptor()!=fmeth->getdescriptor())
	    if (IsMostSpecific(m,fmeth))
	      fmeth=m;
	    else
	      if (!IsMostSpecific(fmeth,m))
		{
		  *ambigous=TRUE;
		  MsgErrors(line,msg_errors[ERR_AMBIG_FIELD],
			    Name.to_char(),
			    fmeth->getmyclass()->getname().to_char(),
			    m->getmyclass()->getname().to_char());
		  return NULL;		  
		}
	}
      else
	if (!fmeth && m)
	  fmeth=m;
    }
    
  return fmeth;
}

int CompileUnit::IsApplicable(TreeNode *params, TreeNode *args)
{
  Descriptor descriptor;
  int modified=FALSE;
  int result=IsApplicable2(params,args,&modified);

  /*
   * Se tra gli argomenti ci sono valori null, di cui in precedenza non 
   * si sapeva il descrittore, ora e' possibile aggiornarlo.
   */

  if (modified)
    {
      descriptor=DES_NULL;
      
      for (TreeNode *t=args; !t->IsDummy(); t=t->GetRightC())
	descriptor+=t->GetDescriptor();
      
      args->SetDescriptor(descriptor);
    }

  return result;
}


int CompileUnit::IsApplicable2(TreeNode *params, TreeNode *args, int *modified)
{
  if ((params->IsDummy()  && !args->IsDummy()) || 
      (!params->IsDummy() && args->IsDummy()))
    return FALSE;

  if (!params->IsDummy() && !args->IsDummy())
    if (!IsApplicable2(params->GetRightC(),args->GetRightC(),modified))
      return FALSE;
    else
      if ((args->GetLeftC()->is_NULLNode() && 
	  params->GetLeftC()->GetDescriptor().is_reference()))
	{
	  args->GetLeftC()->SetDescriptor(params->GetLeftC()->GetDescriptor());
	  *modified=TRUE;
	}
      else
	if (!is_Method_Conversion(params->GetLeftC()->GetDescriptor(),
				  args->GetLeftC()->GetDescriptor()))
	  return FALSE;
  
  return TRUE;
}

int CompileUnit::IsAccessible(STNode *method, STNode* examined_class)
{
  /*
   * Sia T la classe che contiene l'invocazione del metodo o costruttore 
   * (current_class) e sia C la classe a cui si riferisce l'invocazione del
   * metodo o costruttore. Analizziamo i vari casi:
   *
   * a) se m e' public, allora m e' accessibile;
   * b) se m e' protected, allora m e' accessibile se T e' sottoclasse di C,
   *    o T==C oppure T e C sono definite nello stesso pacchetto;
   * c) se m ha accesso di default (cioe' 0), allora m e' accessibile se e
   *    solo se e' definita nello stesso pacchetto di C;
   * d) se m e' private, allora m e' accessibile se e solo se T==C.
   *
   * Ulteriori notizie e' possibile trovarle in Java Language Specification,
   * pag. 335.
   */
 
  int access=method->getaccess();
  String package;

  package=examined_class->getdescriptor().to_packagename();

  if (IS_PUBLIC(access)) 
    return TRUE;

  if (IS_PROTECTED(access))
    if (examined_class==current_class              || 
	current_class->is_subclass(examined_class) ||
	package==package_name)
      return TRUE;

  if (IS_DEFAULT(access) && package==package_name)
    return TRUE;

  if (IS_PRIVATE(access) && examined_class==current_class)
    return TRUE;

  return FALSE;
}

TreeNode *CompileUnit::ConvertArgs(TreeNode *params, TreeNode *args)
{
  /*
   * Sicuramente gli argomenti sono in numero uguale ai parametri e', inoltre,
   * ciascun argomento e' convertibile, secondo l'algoritmo di Method Conver-
   * sion nel tipo del rispettivo parametro. Il nostro compito qui e' solo
   * rendere esplicite le conversioni.
   */
  
  Descriptor DesParam;
  Descriptor DesArg;

  TreeNode *return_args=args;
  String des_parameter, des_arg;

  if (!params->IsDummy())
    {
      TreeNode *t=ConvertArgs(params->GetRightC(),args->GetRightC());
      args->SetRightC(t);
      DesParam=params->GetLeftC()->GetDescriptor();
      DesArg=args->GetLeftC()->GetDescriptor();

      /*
       * Sicuramente DesArg e' convertibile in DesParam secondo l'algoritmo
       * di Method Conversion, perche' se cosi' non fosse il metodo corrente
       * non sarebbe stato proprio attivato.
       */

      if (DesParam!=DesArg)
	{
	  TreeNode *cast=new EXPNode(CastOp,Dummy,args->GetLeftC());
	  cast->SetDescriptor(DesParam);
	  args->SetLeftC(cast);
	}

      args->SetDescriptor(DesParam+t->GetDescriptor());
      return args;
    }
  return Dummy;
}

/*
 * CompileUnit::IsMostSpecific
 *
 * Restituisce TRUE se meth1 e' piu' specifico di meth2, FALSE altrimenti.
 */

int CompileUnit::IsMostSpecific(STNode *meth1, STNode *meth2)
{
  /*
   * I due metodi, poiche' sono entrambi accessibili e applicabili a una
   * stessa definizione, e' chiaro che hanno un numero uguale di parametri.
   */
  Descriptor DesParam1, DesParam2;    
  Descriptor DesFirst, DesSecond;

  TreeNode *params1=meth1->getmyheader()->GetLeftC()->GetLeftC()->GetRightC();
  TreeNode *params2=meth2->getmyheader()->GetLeftC()->GetLeftC()->GetRightC();

  DesFirst=meth1->getmyclass()->getdescriptor();
  DesSecond=meth2->getmyclass()->getdescriptor();

  if (!is_Method_Conversion(DesSecond,DesFirst))
    return FALSE;
  
  for (; !params1->IsDummy(); params1=params1->GetRightC(),
	                      params2=params2->GetRightC())
    {
      DesParam1=params1->GetLeftC()->GetDescriptor();
      DesParam2=params2->GetLeftC()->GetDescriptor();

      if (!is_Method_Conversion(DesParam2,DesParam1))
	return FALSE;
    }

  return TRUE;
}

/*
 * CompileUnit::ParseAssignment
 *
 * Parsing di un'istruzione di assegnamento semplice.
 */

TreeNode *CompileUnit::ParseAssignment(TreeNode *root)
{
  TreeNode *Exp0=root->GetLeftC();
  TreeNode *Exp1=root->GetRightC();

  if (Exp0->is_UNAMENode())
    Exp0=ParseNameExpr(Exp0);
  else
    if (((EXPNode *)Exp0)->GetNodeOp()==FieldAccessOp)
      Exp0=ParseFieldAccess(Exp0);
    else
      Exp0=ParseArrayAccess(Exp0);

  if (!IsVariable(Exp0))
    MsgErrors(Exp0->GetLine(),msg_errors[ERR_INVALID_LHS_ASSIGNMENT]);
  
  Exp1=ParseExpr(Exp1);
  
  Descriptor DesExp0;
  Descriptor DesExp1;
  Descriptor DesReturn;

  DesExp0=Exp0->GetDescriptor();
  DesExp1=Exp1->GetDescriptor();

  /*
   * Se la variabile a sinistra e' un reference type e a destra c'e' null,
   * e' necessario associare a null, il descrittore della variabile.
   */

  if (DesExp0.is_reference() && Exp1->is_NULLNode())
    { 
      Exp1->SetDescriptor(DesExp0);
      DesExp1=Exp1->GetDescriptor();
    } 

  /*
   * Se l'espressione di sinistra o di destra ha DES_NULL per descrittore,
   * e' chiaro che sulla linea dell'assegnamento e' gia' stato inviato un er-
   * rore, per cui e' inutile inviarne un altro.
   */

  if (DesExp0==DES_NULL || DesExp1==DES_NULL) 
    DesReturn=DES_NULL;
  else
    if (!is_Assign_Conversion(DesExp0,DesExp1))
      {
	if (is_Cast_Conversion(DesExp0,DesExp1))
	  MsgErrors(root->GetLine(),msg_errors[ERR_EXPLICIT_CAST_NEEDED],
		    "=",DesExp1.to_typename(),DesExp0.to_typename());
	else
	  MsgErrors(root->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"=",
		    DesExp1.to_typename(),DesExp0.to_typename());

	DesReturn=DES_NULL;
      }
    else
      {
	if (DesExp0!=DesExp1)
	  {
	    Exp1=new EXPNode(CastOp,Dummy,Exp1);
	    Exp1->SetDescriptor(DesExp0);
	  }
	DesReturn=DesExp0;
      }

  root->SetLeftC(Exp0);
  root->SetRightC(Exp1);
  root->SetDescriptor(DesReturn);
  return root;
}

/*
 * CompileUnit::ParseCreateClassInstance
 *
 * Effettua il parsing della creazione di una nuova classe.
 */

TreeNode *CompileUnit::ParseCreateClassInstance(TreeNode *root)
{
  TreeNode *Exp0=root->GetLeftC()->GetLeftC();
  STNode   *idnode;

  if (Exp0->is_UNAMENode())
    {
      /*
       * Va risolto il nome della classe.
       */

      String Name;

      Name=((UNAMENode *)Exp0)->GetName();
      if ((idnode=LoadClass(Name,++LastIndex))!=NULL)
	{
	  delete Exp0;
	  root->GetLeftC()->SetLeftC(new IDNode(idnode));
	}
      else
	{
	  MsgErrors(root->GetLine(),msg_errors[ERR_CLASS_NOT_FOUND],
		    Name.to_char(),"new");
	  return root;
	}
    }
  else
    idnode=((IDNode *)Exp0)->GetIden();

  if (idnode->is_interface())
    MsgErrors(idnode->getline(),msg_errors[ERR_NEW_INTF],
	      idnode->getname().to_char());

  if (IS_ABSTRACT(idnode->getaccess()))
    MsgErrors(idnode->getline(),msg_errors[ERR_NEW_ABSTRACT],
	      idnode->getname().to_char());

  TreeNode *TreeReturn=ParseConstrCall(root);
  TreeReturn->SetDescriptor(root->GetLeftC()->GetLeftC()->GetDescriptor());

  return TreeReturn;
}

/*
 * CompileUnit::ParseConstrCall
 *
 * Questo metodo viene invocato in due occasioni: 
 *
 * a) nella fase di creazione di un'istanza di una classe;
 * b) in un costruttore quando esplicitamente si chiama un costruttore della
 *    classe corrente o quello della superclasse.
 *
 * Nel caso (a), pero', bisognera' trascurare l'albero che il metodo seguente
 * restituira'. In particolare ParseCreateClassInstance provvedera' a 
 * a deallocare l'albero che il metodo seguente gli restituira', cosi' da
 * non allocare memoria inutilmente. 
 */

TreeNode *CompileUnit::ParseConstrCall(TreeNode *root)
{
  TreeNode *Exp0=root->GetLeftC()->GetLeftC();
  TreeNode *Exp1=root->GetLeftC()->GetRightC();
  TreeNode *Exp2=root->GetRightC();

  /*
   * Exp0, puo' essere IDNode, THISNode e SUPERNode. Nel terzo caso, bisognera'
   * assegnargli il descrittore.
   */

  if (Exp0->is_SUPERNode())
    Exp0->SetDescriptor(current_class->get_superclass()->getdescriptor());

  /*
   * Chi chiama il metodo CompileUnit::ParseConstrCall, ha gia' pensato come 
   * risolvere il nome della classe di cui si vuole invocare un costruttore.
   * Qui noi ci occuperemo di risolvere il nome del costruttore e gli
   * argomenti trasmessi.
   */

  /*
   * NON GESTITO!!!
   *
   * E' importante che l'invocazione non sia ciclica.
   */

  Exp2=ParseArgsExpr(Exp2);

  STNode *examined_class, *fconstr=NULL;

  if (Exp0->is_THISNode())
    examined_class=current_class;
  else
    if (Exp0->is_IDNode())
      examined_class=((IDNode *)Exp0)->GetIden();
    else

      /*
       * NON GESTITO!!!
       *
       * Per ora si suppone che la classe corrente contenga sempre una super-
       * classe. Quindi per ora non e' ammessa la compilazione di 
       * java/lang/Object.
       */

      examined_class=current_class->get_superclass();

  int index=examined_class->getindex();
  STNode *first_node=table->LookUpHere(init_name,THIRD_LVL,index);

  for (STNode *m=first_node; m!=NULL; m=table->NextSym(m,THIRD_LVL,index))
    {
      TreeNode *params=m->getmyheader()->GetLeftC()->GetLeftC()->GetRightC();

      if (IsAccessible(m,examined_class))
	if (IsApplicable(params,Exp2))
	  if (fconstr==NULL)
	    fconstr=m;
	  else
	    if (IsMostSpecific(m,fconstr))
	      fconstr=m;
      
      /*
       * NON GESTITO!!!
       *
       * Si possono avere ambiguita' nell'invocazione di costruttori ?
       * Per ora no!!!
       */
    }

  if (fconstr!=NULL)
    {
      TreeNode *params=
	fconstr->getmyheader()->GetLeftC()->GetLeftC()->GetRightC();
      Exp2=ConvertArgs(params,Exp2);
    }
  else
    {
      MsgErrors(Exp1->GetLine(),msg_errors[ERR_UNMATCHED_CONSTR],
		examined_class->getname().to_char(),
		examined_class->getname().to_char());

      Descriptor descriptor;

      descriptor.build_method(Exp2->GetDescriptor(),DesVoid);

      fconstr=table->Install_Id(init_name,THIRD_LVL,-1,descriptor);
      table->Discard(fconstr);
    }

  delete root->GetLeftC()->GetRightC();

  root->GetLeftC()->SetRightC(new IDNode(fconstr));
  root->SetRightC(Exp2);
  root->SetDescriptor(DesVoid);
  return root;
}

/*
 * CompileUnit::ParseConstExpr
 *
 * Questo metodo in effetti richiama su un parse-tree il parsing per espressio-
 * ni, dopo di che' controlla se l'espressione calcolata e' costante.
 */

TreeNode *CompileUnit::ParseConstExpr(TreeNode *root)
{
  root=ParseExpr(root);

  if (!IsConstExpr(root))
    MsgErrors(root->GetLine(),msg_errors[ERR_CONST_EXPR_REQUIRED]);
  
  return root;
}

/*
 * CompileUnit::ParseCompAssignment
 *
 * Effettua il parsing di un'assegnazione composta, ossia assegnazioni del 
 * tipo: &=; |=; +=; etc.
 *
 * Secondo le specifiche di Java, i tipi di assegnazioni composte ammesse,
 * sono: *=, /=, %=, +=, -=, &=, |=, ^=, <<=, >>=, >>>=.
 *
 * L'espressione E1 op= E2 e' equivalente a E1=(T)(E1 op E2) dove T e' il 
 * tipo di E1. 
 * Secondo le specifiche di Java, deve accadere:
 *
 * a) sia E1 che E2 devono essere tipi primitivi;
 * b) se E1 e' di tipo String, allora E2 puo' essere di qualsiasi tipo;
 * c) e' necessario applicare l'algoritmo di cast-conversion per controllare
 *    se (E1 op E2) e' convertibile al tipo di E1.
 *    Per cio' che riguarda la conversione, sulle specifiche dice che deve
 *    essere o una identita' o un narrowing conversion. Da prove sperimentali,
 *    pero', si e' notato, in realta', che viene applicato un cast conversion,
 *    per cui ci e' sembrato giusto applicare questo tipo di casting.
 */

TreeNode *CompileUnit::ParseCompAssignment(TreeNode *root)
{
  const char *strop;
  int  op=((EXPNode *) root)->GetNodeOp();

  switch (op)
    {
    case AssignMultOp   : strop="*=";
    case AssignDivOp    : strop="/=";
    case AssignModOp    : strop="%=";
    case AssignPlusOp   : strop="+=";
    case AssignMinusOp  : strop="-=";
    case AssignAndOp    : strop="&=";
    case AssignOrOp     : strop="|=";
    case AssignXorOp    : strop="^=";
    case AssignLShiftOp : strop="<<=";
    case AssignRShiftOp : strop=">>=";
    case AssignURShiftOp: strop=">>>=";
    }

  TreeNode *Exp0=root->GetLeftC();
  TreeNode *Exp1=root->GetRightC();

  if (Exp0->is_UNAMENode())
    Exp0=ParseNameExpr(Exp0);
  else
    if (((EXPNode *)Exp0)->GetNodeOp()==FieldAccessOp)
      Exp0=ParseFieldAccess(Exp0);
    else
      Exp0=ParseArrayAccess(Exp0);
  
  if (!IsVariable(Exp0))
    MsgErrors(Exp0->GetLine(),msg_errors[ERR_INVALID_LHS_ASSIGNMENT]);
  
  Exp1=ParseExpr(Exp1);
  
  Descriptor DesExp0;
  Descriptor DesExp1;

  DesExp0=Exp0->GetDescriptor();
  DesExp1=Exp1->GetDescriptor();

  /*
   * Se l'espressione di sinistra e' di tipo java/lang/String e l'opeatore
   * in esame e' +=, allora l'espressione di destra puo' essere di qualsiasi 
   * tipo, altrimenti sia Exp0 che Exp1 devono essere di tipo primitive type 
   * con Exp1.
   */

  if (op==AssignPlusOp && DesExp0==DES_STRING)
    {
      Exp1=new EXPNode(CastOp,Dummy,Exp1);
      Exp1->SetDescriptor(DesString);
      root->SetRightC(Exp1);
      return root;
    }

  if (!DesExp0.is_primitive())
    { 
      MsgErrors(Exp0->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],strop,
		DesExp0.to_typename(),"int");

      if (!DesExp1.is_primitive())
	MsgErrors(Exp1->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],strop,
		  DesExp1.to_typename(),"int");
    }
  else
    if (!DesExp1.is_primitive())
      MsgErrors(Exp1->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],strop,
		DesExp1.to_typename(),DesExp0.to_typename());
    else
      if (is_Cast_Conversion(DesExp0,DesExp1))
	{
	  Exp1=new EXPNode(CastOp,Dummy,Exp1);
	  Exp1->SetDescriptor(DesString);
	  root->SetRightC(Exp1);
	}
      else
	MsgErrors(root->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],strop,
		  DesExp1.to_typename(),DesExp0.to_typename());

  return root;
}

/*
 * CompileUnit::ParseArrayCreation
 *
 * Effettua il parsing di un istruzione di creazione di array.
 */

TreeNode *CompileUnit::ParseArrayCreation(TreeNode *root)
{
  /*
   * Il descrittore finale e' ormai gia' stato calcolato, per cui bisogna
   * eseguire i seguenti passi:
   *
   * a) se stiamo creando un array di reference type, ossia i cui elementi sono
   *    riferimenti a classi, allora bisognera' eventualmente risolvere il
   *    nome della classe o interfaccia;
   * b) controllare che ogni espressione all'interno delle parentesi sia di
   *    tipo integral type;
   * c) applicare l'arithmetic promotion al tipo delle espressioni.
   */

  if (!root->GetLeftC()->IsDummy() && root->GetLeftC()->is_UNAMENode())
    {
      /*
       * caso a)
       */
      
      STNode *node;
      String Name;

      Name=((UNAMENode *)root->GetLeftC())->GetName();

      if ((node=LoadClass(Name,++LastIndex))!=NULL)
	{
	  delete root->GetLeftC();
	  root->SetLeftC(new IDNode(node));	  
	}
      else
	MsgErrors(root->GetLine(),msg_errors[ERR_CLASS_NOT_FOUND],
		  Name.to_char(),"new");
    }

  for (TreeNode *commatree=root->GetRightC(); !commatree->IsDummy();
       commatree=commatree->GetLeftC())
    {
      TreeNode *etree=ParseExpr(commatree->GetRightC());
      Descriptor descriptor;

      if (!etree->GetDescriptor().is_integral())
	MsgErrors(root->GetLine(),msg_errors[ERR_INCOMPATIBLE_TYPE],"new",
		  etree->GetDescriptor().to_typename(),"int");
      else
	{
	  descriptor=UArithmetic_Promotion(etree->GetDescriptor());
	  if (descriptor!=etree->GetDescriptor())
	    {
	      etree=new EXPNode(CastOp,Dummy,etree);
	      etree->SetDescriptor(descriptor);
	    }
	}

      commatree->SetRightC(etree);
    }

  return root;
}

/*****************************************************************************
 * Alcuni semplici controlli sulle espressioni.                              *
 *****************************************************************************/

/*
 * CompileUnit::IsConstexpr
 *
 * Controlla se l'espressione presa in input sotto forma di Parse-Tree e'
 * costante.
 * Questo metodo va completato meglio, perche' un'espressione costante puo'
 * essere data anche da espressioni del genere:
 *
 *                  a+10-11
 *
 * con a variabile final.
 */

int CompileUnit::IsConstExpr(TreeNode *root)
{
  if (root->is_STRINGNode() || root->is_CHARNode() || root->is_BOOLEANode() ||
      root->is_INUMNode()   || root->is_FNUMNode())
    return TRUE;
  else
    return FALSE;
}

/*
 * CompileUnit::IsVariable
 *
 * Questo metodo valuta il parse-tree di un'espressione e dice se essa denota
 * una variabile o un valore.
 * Questo tipo di informazione e' molto utile nella gestione di operatori
 * del tipo: ++, --, =, &=, etc.
 * 
 * Il seguente metodo sara' veramente corretto, quando anche il metodo
 * CompileUnit::IsConstExpr lo sara'.
 */

int CompileUnit::IsVariable(TreeNode *root)
{
  if (IsConstExpr(root)) return FALSE;

  if (root->is_IDNode()) return TRUE;

  if (root->is_EXPNode())
    {
      int op=((EXPNode *)root)->GetNodeOp();

      if (op==FieldAccessOp || op==ArrayAccessOp) return TRUE;
    }

  return FALSE;  
}
