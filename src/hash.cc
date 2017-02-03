/*
 * file hash.cc
 *
 * descrizione: questo file implementa una generica hash table, su cui verra'
 *              costruita poi la symbol table.
 *
 * Questo file e' parte del Compilatore Java.
 * 
 * Giugno 1997, scritto da Michele Risi e-mail micris@zoo.diaedu.unisa.it
 */ 

#include <stdio.h>
#include <cstring.h>
#include <hash.h>
#include <access.h>

/*****************************************************************************
 * classe HTNode.                                                            *
 *****************************************************************************/

/*
 * costruttore/distruttore HTNode. 
 */

HTNode::HTNode(void)
{
  name="";
  next=NULL;
  prev=NULL;
}

HTNode::HTNode(String& _name)
{
  name=_name;
  next=NULL;
  prev=NULL;
}

HTNode::~HTNode() { }

String& HTNode::getname()             { return name;       }
void HTNode::setname(String& _name)   { name=_name;        }
HTNode *HTNode::getnext()             { return next;       }
void HTNode::setnext(HTNode* _next)   { next=_next;        }
HTNode *HTNode::getprev()             { return prev;       }
void    HTNode::setprev(HTNode* _prev){ prev=_prev; } 

/*
 * HTNode::PrintNode
 *
 * Stampa di una nodo della tabella hash.
 */

void HTNode::PrintNode(FILE *stream)
{
  fprintf(stream,"    - Key   :     %s\n",name.to_char());
}

/*****************************************************************************
 * classe HTList.                                                            *
 *****************************************************************************/

/*
 * costruttore/distruttore HTList.
 */

HTList::HTList() { Headlist=NULL; }

HTList::~HTList() 
{
  HTNode *t, *p=Headlist;

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

HTNode *HTList::GetHeadlist()          { return Headlist; }
void    HTList::SetHeadlist(HTNode *l) { Headlist=l;      }

/*
 * HTList::Find
 *
 * Ricerca un nodo avente una certa chiave nella lista.
 */

HTNode *HTList::Find(String& nam)
{
  for (HTNode *p=Headlist; p!=NULL; p=p->getnext())
    if (p->getname()==nam) 
      return p;
  return NULL;
}

/*
 * HTList::Insert
 *
 * Inserisce un nodo nella lista.
 */

void HTList::Insert(HTNode *node)
{
  HTList *app=this;

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
 * HTList::Discard
 *
 * Scarta un nodo dalla lista, senza deallocarlo.
 */
 
void HTList::Discard(HTNode *node)
{
  if (node->getprev()==NULL && node->getnext()==NULL)
     this->Headlist=NULL;
  else
    {
      if (node->getprev()==NULL)
	{
	  node->getnext()->setprev(NULL);
          this->Headlist=node->getnext();
	}
      else
	if (node->getnext()==NULL)
	  node->getprev()->setnext(NULL);
	else 
	  {
	    node->getprev()->setnext(node->getnext());
	    node->getnext()->setprev(node->getprev());
	  } 
    }
}

/*
 * HTList::Delete
 *
 * scarta un nodo dalla lista deallocandolo.
 */

void HTList::Delete(HTNode *node)
{
  if (node->getprev()==NULL && node->getnext()==NULL)
     this->Headlist=NULL;
  else
    {
      if (node->getprev()==NULL)
	{
	  node->getnext()->setprev(NULL);
          this->Headlist=node->getnext();
	}
      else
	if (node->getnext()==NULL)
	  node->getprev()->setnext(NULL);
	else 
	  {
	    node->getprev()->setnext(node->getnext());
	    node->getnext()->setprev(node->getprev());
	  } 
    }
  delete node;
}

/*
 * HTList::PrintList
 *
 * Stampa di una lista.
 */

void HTList::PrintList(FILE *stream)
{
  for (HTNode *p=Headlist; p!=NULL; p=p->getnext())
      p->PrintNode(stream);
}

/*****************************************************************************
 * classe Hashtable.                                                         *
 *****************************************************************************/

int Hashtable::num_bucket=0;

Hashtable::Hashtable() { }

/*
 * costruttore/distruttore di un'hash table avente n bucket.
 */

Hashtable::Hashtable(int n)
{
  HT=new HTList [n];
  num_bucket=n;
  basket=new HTList;
}

Hashtable::~Hashtable()
{
  for (int i=0; i<num_bucket; i++)
    {
      /*
       * Per ogni bucket viene invocato il distruttore della lista a cui
       * esso punta.
       */

      HTList *l=new HTList;
      l->SetHeadlist(HT[i].GetHeadlist());
      delete l;
    }
  /* delete HT; ? */
  delete basket;
}

/*
 * Hashtable::Hash
 *
 * Funzione Hash. Chiunque conosca una funzione di hashing migliore, puo'
 * sostituirla rispettando, pero', i parametri di I/O.
 */

#define MASK_HASH 0xF0000000

int Hashtable::Hash(String& key)
{
  const char *point;
  unsigned h,g;
  
  h=0;
  point=key.to_char();
  while (*point!='\0')
    {
      h=(h << 4)+(*point);
      if (g=h & MASK_HASH) 
	{
	  h=h^(g >> 24);
	  h=h^g;
	}
      point++;
    }
  return (h % num_bucket);
}

/*
 * Hashtable::Discard
 *
 * Scarto un nodo dalla tabella, ponendolo nel cestino. Il cestino e' stato
 * concpito con l'intento di evitare di avere nodi allocati in memoria senza
 * riferimento. Infatti, quando si eseguira' il delete di una tabella, automa-
 * ticamente anche questi nodi verranno eliminati.
 */

void Hashtable::Discard(HTNode *node)
{
  if (node)
    {
      HT[Hash(node->getname())].Discard(node);
      basket->Insert(node);
    }
}

/*
 * Hashtable::Recover
 *
 * Recupero un nodo precedentemente posto nel cestino, reinserendolo nella
 * symbol table. Questo metodo, in combinazione con Discard, puo' essere uti-
 * lizzato in una generica Hash Table. Nel nostro progetto questi due metodi
 * servono per la gestione di variabili locali e parametri formali che vengono
 * continuamente scartati e rimessi nella symbol-table.
 * Per fare un recupero di un nodo dal cestino, bisogna effettuare due passi:
 *
 * - scartare il nodo dal cestino;
 * - inserirlo nella tabella.
 */

void Hashtable::Recover(HTNode *node)
{
  if (node)
    {
      basket->Discard(node);
      HT[Hash(node->getname())].Insert(node);
    }
}

/*
 * Hashtable::Delete
 *
 * Elimina un nodo dalla tabella, deallocandolo pure.
 */

void Hashtable::Delete(HTNode *node)
{
  if (node!=NULL)
    HT[Hash(node->getname())].Delete(node);
}

/*
 * Hashtable::Install_Id
 *
 * Installazione di un nome nella tabella.
 */

HTNode *Hashtable::Install_Id(String& nam)
{
  HTNode *p=new HTNode(nam);
  HT[Hash(nam)].Insert(p);
  return p;
}

/*
 * Hashtable::LookUpHere
 *
 * Procedura di lookup di una chiave.
 */ 

HTNode* Hashtable::LookUpHere(String& _name) 
{
  return HT[Hash(_name)].Find(_name);
}

/*
 * Hashtable::NextSym
 *
 * E' ammesso avere nella tabella nodi con chiave uguale, basti pensare che
 * nel compilatore, si puo' avere due metodi con uguale nome nello stesso
 * livello di scoping. I nodi con nome uguale, capiteranno sicuramente nello
 * stesso bucket e, per una questione di efficienza, si e' pensato di usare
 * questo metodo che restituisce il successivo nodo con quel nome, se esiste,
 * altrimenti NULL.
 */
 
HTNode* Hashtable::NextSym(HTNode *node)
{
  if (node!=NULL)
    {
      for (HTNode *p=node->getnext(); p!=NULL; p=p->getnext())
	if (p->getname()==node->getname())
	  return p;
    }
  return NULL;    
}

/*
 * Hashtable::PrintTable
 *
 * Stampa di una tabella hash.
 */

void Hashtable::PrintTable(FILE *stream)
{
  fprintf(stream,"\n\nStampa Hash:\n\n");
  for (int i=0; i<num_bucket; i++)
    {
      fprintf(stream,"bucket #%d:\n",i);
      HT[i].PrintList(stream);
    }
  fprintf(stream,"\n\nBasket:\n\n");
  basket->PrintList(stderr);
}

