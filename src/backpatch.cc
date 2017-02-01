/*
 * file backpatch.cc
 *
 * descrizione: questo file implementa una lista di backpatching. Questa lista
 *              e' fondamentale per gestire con efficienza le istruzioni 
 *              booleane e istruzioni come break e continue.
 *
 * Questo file e' parte del Compilatore Java.
 *
 * Marzo 1998, scritto da Salvatore D'Angelo, e-mail xc0261@xcom.it.
 */

#include <stdio.h>
#include <iostream.h>
#include <globals.h>
#include <backpatch.h>

/*****************************************************************************
 * classe BPNode.                                                            *
 *****************************************************************************/

/*
 * Costruttore/distruttore della classe.
 */

BPNode::BPNode(_u4 i)
{
  address=i;
  next=NULL;
  prev=NULL;
}

BPNode::~BPNode() { }

/*
 * Alcuni semplici metodi per l'accesso a dati privati della classe.
 */

_u4     BPNode::getaddress()          { return address; }
BPNode *BPNode::getnext()             { return next;    }
void    BPNode::setnext(BPNode *node) { next=node;      }
BPNode *BPNode::getprev()             { return prev;    }
void    BPNode::setprev(BPNode *node) { prev=node;      }

void    BPNode::PrintNode(FILE *stream) { cout << address << endl; } 

/****************************************************************************
 * classe BPList.                                                           *
 ****************************************************************************/

/*
 * Costruttore / distruttore della lista di backpatching.
 */

BPList::BPList() { Headlist=NULL; }

BPList::~BPList() 
{
  BPNode *t, *p=Headlist;

  /*
   * cancello tutti i nodi della lista.
   */

  while (p!=NULL)
    {
      t=p->getnext();
      delete p;
      p=t;
    }
}

/*
 * BPList::GetHeadlist
 * BPList::SetHeadlist
 *
 * restituisce/imposta il nodo iniziale della lista.
 */

BPNode *BPList::GetHeadlist()             { return Headlist; }
void    BPList::SetHeadlist(BPNode *node) { Headlist=node;   }

/*
 * BPList::Add
 * 
 * Aggiunge un item alla lista di backpatching. Per ragioni di efficienza 
 * ogni nodo viene aggiunto in testa alla lista.
 */

void BPList::Add(_u4 addr)
{
  BPNode *node=new BPNode(addr);
  BPList *app=this;

  node->setprev(NULL);
  if (app->Headlist==NULL)
    {
      app->Headlist=node;
      node->setnext(NULL);
    }
  else
    {
      node->setnext(app->Headlist);
      app->Headlist->setprev(node);
      app->Headlist=node;
    }
}

/*
 * BPList::Merge
 *
 * Pre-condizione: la lista corrente e' NULL.
 *
 * Associa alla lista corrente, il merge delle due liste di backpatching in
 * input.
 */

void BPList::Merge(BPList *list1, BPList *list2)
{
  if (!list1 && !list2) { Headlist=NULL; return; }

  if (!list1)           { Headlist=list2->GetHeadlist(); return; }
  if (!list2)           { Headlist=list1->GetHeadlist(); return; }

  BPNode *n=list1->GetHeadlist();
  Headlist=list1->GetHeadlist();
  
  for (; n->getnext(); n=n->getnext());

  n->setnext(list2->GetHeadlist());
}

/*
 * BPList::PrintBPList
 *
 * stampa una lista di backpatching su stream.
 */

void BPList::PrintBPList(FILE *stream)
{
  for (BPNode *p=Headlist; p!=NULL; p=p->getnext())
    p->PrintNode(stream);
}

