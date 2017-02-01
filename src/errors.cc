/*
 * file errors.cc
 *
 * descrizione: questo file implementa la classe ListErrors che non e' altro
 *              che una lista ordinata per linee di messaggi di errore che
 *              l'utente conservera' durante la fase di parsing e che verra'
 *              sparata a video alla fine di parser e analisi del parse-tree.
 *
 * Questo file e' parte del Compilatore Java
 *
 * Dicembre 1997, scritto da Salvatore D'Angelo e-mail xc0261@xcom.it
 */

#include <iostream.h>
#include <stdio.h>
#include <errors.h>

/*****************************************************************************
 * implementazione classe NodeErrors                                         *
 *****************************************************************************/

/*
 * costruttore classe NodeErrors
 * 
 * costruisce un nodo in cui verra' riposto un messaggio di errore.
 */

NodeErrors::NodeErrors(int aline, char *msg)
{
  line=aline;
  message=new char [strlen(msg)+1];
  // message=msg;
  strcpy(message,msg);
  leftnode=NULL;
  rightnode=NULL;
}

/*
 * distruttore classe NodeErrors
 *
 * si occupa principalmente di deallocare il messaggio di errore.
 */

NodeErrors::~NodeErrors()
{
  delete message;
  delete leftnode;
  delete rightnode;
}

/*
 * NodeErrors::GetLeftNode
 * NodeErrors::SetLeftNode
 *
 * restituisce/imposta il nodo sinistro del nodo corrente.
 */

NodeErrors *NodeErrors::GetLeftNode()          { return leftnode;  }
void NodeErrors::SetLeftNode(NodeErrors *node) { leftnode=node;    }

/*
 * NodeErrors::GetRightNode
 * NodeErrors::SetRightNode
 *
 * restituisce/imposta il nodo destro del nodo corrente.
 */

NodeErrors *NodeErrors::GetRightNode()          { return rightnode; }
void NodeErrors::SetRightNode(NodeErrors *node) { rightnode=node;   }

int NodeErrors::GetLine() { return line; }

/*
 * NodeErrors::Print
 *
 * stampa su stderr del nodo corrente, nel formato:
 *
 *   javac <linea>: <messaggio>
 */ 

void NodeErrors::Print()
{
  NodeErrors *left, *right;

  if ((left=GetLeftNode())!=NULL)
    left->Print();
  if (line)
    cout << "jcc " << line << ": " << message << "\n";
  else
    cout << "jcc " << ": " << message << "\n";
  if ((right=GetRightNode())!=NULL)
    right->Print();
}

/*****************************************************************************
 * implementazione classe ListErrors                                         *
 *****************************************************************************/

/*
 * costruttore/distruttore classe ListErrors
 */

ListErrors::ListErrors()  { root=NULL; count_errors=0; }
ListErrors::~ListErrors() { delete root;               }

/*
 * ListErrors::GetRoot
 *
 * restituisce la radice dell'albero di ricerca.
 */

NodeErrors *ListErrors::GetRoot() { return root; }

/*
 * ListErrors::GetNumErrors
 *
 * restituisce il numero di errori.
 */

int ListErrors::GetNumErrors() { return count_errors; }

/*
 * ListErrors::InsertMsg
 *
 * inserisce un messaggio nella lista (albero).
 */

void ListErrors::InsertMsg(int line, char *msg)
{
  NodeErrors *node=root;
  NodeErrors *prec_node=NULL;

  if (!root)
    root=new NodeErrors(line,msg);
  else
    {    
      while (node!=NULL)
	{
	  if (line >= node->GetLine())
	    {
	      prec_node=node;
	      node=node->GetRightNode();
	    }
	  else
	    {
	      prec_node=node;
	      node=node->GetLeftNode();
	    }
	}

      NodeErrors *n=new NodeErrors(line,msg);

      if (line >= prec_node->GetLine())
	prec_node->SetRightNode(n);
      else
	prec_node->SetLeftNode(n);
    }
  count_errors++;
}

/*
 * ListErrors::Print
 *
 * Stampa il report degli errori di compilazione.
 */

void ListErrors::Print()
{
  if (root) root->Print();
  if (count_errors==1) 
    cout << "\n " << count_errors << " error\n\n";
  else
    if (count_errors > 1)
      cout << "\n " << count_errors << " errors\n\n";
}
