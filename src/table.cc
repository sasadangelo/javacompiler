/*
 * file table.cc
 *
 * descrizione: questo file implementa le classi STable e STNode che rappresen-
 *              tano symbol-table e rispettivi nodi.
 *
 * Questo file e' parte del Compilatore Java.
 * 
 * Maggio 1997, scritto da Salvatore D'Angelo e-mail xc0261@xcom.it.
 */ 

#include <stdio.h>
#include <globals.h>
#include <access.h>
#include <cstring.h>
#include <descriptor.h>
#include <hash.h>
#include <table.h>
#include <tree.h>

extern int Index;
extern int yylineno;

/*****************************************************************************
 * classe STNode                                                             *
 *****************************************************************************/

/*
 * costruttori della classe STNode.
 */

STNode::STNode(void)
{
  myheader=NULL;
  access=0;
  descriptor=DES_NULL;
  full_name="";
  level=0;
  index=0;
  unresolved=FALSE;
  line=yylineno;
  used=TRUE;
  local_index=-1;
  generable=TRUE;
}

STNode::STNode(String& _name, int _level, int _index, 
	       Descriptor& _descriptor) : HTNode(_name)
{
  myheader=NULL;
  access=0;
  descriptor=_descriptor;
  full_name=_name;
  level=_level;
  index=_index;
  unresolved=FALSE;
  line=yylineno;
  used=TRUE;
  local_index=-1;
  generable=TRUE;
}

/*
 * Distruttore della classe STNode.
 */

STNode::~STNode() { myheader=NULL; }

/*
 * STNode::getmyheader
 * STNode::setmyheader
 *
 * restituisce/imposta la radice del parse-tree che descrive l'identificatore.
 */

TreeNode *STNode::getmyheader()              { return myheader;    }
void      STNode::setmyheader(TreeNode *n)   { myheader=n;         }

/*
 * STNode::getaccess
 * STNode::setaccess
 *
 * restituisce/imposta gli accessi di un identificatore.
 */

int       STNode::getaccess()                { return access;      }
void      STNode::setaccess(int a)           { access=a;           }

/*
 * STNode::getdescriptor
 * STNode::setdescriptor
 *
 * restituisce/imposta il descrittore di un identificatore.
 */

Descriptor& STNode::getdescriptor()         { return descriptor;  }
void STNode::setdescriptor(Descriptor& des) { descriptor=des;     }

/*
 * STNode::getfullname
 * STNode::setfullname
 *
 * restituisce/imposta il nome per intero di un identificatore.
 */

String&   STNode::getfullname()              { return full_name;   }
void      STNode::setfullname(String& fname) { full_name=fname;    }

/*
 * STNode::getlevel
 * STNode::getindex
 * STNode::setlevel
 * STNode::setindex
 *
 * restituisce/imposta il livello e l'indice di scoping per un identificatore.
 */

int       STNode::getlevel()                 { return level;       }
void      STNode::setlevel(int _level)       { level=_level;       }
int       STNode::getindex()                 { return index;       }
void      STNode::setindex(int _index)       { index=_index;       }

/*
 * STNode::setunresolved
 * STNode::setresolved
 *
 * imposta un identificatore come "unresolved" o "resolved".
 */

void      STNode::setunresolved()            { unresolved=TRUE;        }
void      STNode::setresolved()              { unresolved=FALSE;       }

/*
 * STNode::getlocalindex
 * STNode::setlocalindex
 *
 * utilizzati nella gestione delle variabili locali nell JVM.
 */

_u1       STNode::getlocalindex()            { return (_u1)local_index;  }
void      STNode::setlocalindex(_u1 _lindex) { local_index=(int)_lindex; }

/*
 * STNode::getmyclass
 * STNode::setmyclass
 *
 * utilizzato per campi e metodi, per sapere, in ogni istante, la loro classe 
 * di appartenenza.
 */

STNode *STNode::getmyclass()             { return myclass; }
void    STNode::setmyclass(STNode *node) { myclass=node;   }

/*
 * STNode::getline
 *
 * restituisce la linea del codice sorgente dove il simbolo e' stato definito.
 */

int  STNode::getline()                   { return line;    }
void STNode::setline(int _line)          { line=_line;     }

/*
 * STNode::set_unused
 * STNode::is_used
 *
 * Esistono nodi della symbol-table che sono allocati, portati nel cestino
 * e utilizzati solo per fare in modo che la compilazione proceda anche in
 * caso di errori. Questi nodi sono inutilizzati, per cui non possono essere
 * scartati, perche' gia' stanno nel cestino, non possono essere recuperati.
 * Es. i parametri dei metodi delle classi esterne caricate.
 */

void STNode::set_unused() { used=FALSE;   }
int  STNode::is_used()    { return used;  }

/*
 * STNode::PrintNode
 *
 * stampa di un nodo su stream.
 */

void STNode::PrintNode(FILE *stream)
{
  fprintf(stream,"    - Name  :     %s\n",name.to_char());
  fprintf(stream,"    - Access:     "); 
  if (IS_PUBLIC(access))       fprintf(stream,"Pu");
  if (IS_PROTECTED(access))    fprintf(stream,"Pt");
  if (IS_PRIVATE(access))      fprintf(stream,"Pr");
  if (IS_STATIC(access))       fprintf(stream,"St");
  if (IS_ABSTRACT(access))     fprintf(stream,"A");
  if (IS_FINAL(access))        fprintf(stream,"F");
  if (IS_NATIVE(access))       fprintf(stream,"N");
  if (IS_SYNCHRONIZED(access)) fprintf(stream,"Sy");
  if (IS_TRANSIENT(access))    fprintf(stream,"T");
  if (IS_VOLATILE(access))     fprintf(stream,"V");
  if (IS_INTERFACE(access))    fprintf(stream,"I");
  fprintf(stream,"\n");
  fprintf(stream,"    - Descriptor: %s\n",descriptor.to_char());
  fprintf(stream,"    - Full Name:  %s\n",full_name.to_char());
  fprintf(stream,"    - Level:      (%d,%d)\n",level,index);
  fprintf(stream,"\n");
}

/*
 * STNode::is_local
 *
 * restituisce TRUE se il nodo denota una var. locale o parametro.
 */

int STNode::is_local() { return level==FOURTH_LVL; }

/*
 * STNode::is_local_on_stackframe
 *
 * Restituisce TRUE se la variabile locale ha un posto assegnato sullo stack
 * frame. Questo metodo e' utilizzato in fase di generazione di codice.
 */

int STNode::is_local_on_stackframe() { return local_index!=-1; }

/*
 * STNode::is_resolved
 * 
 * Restituisce TRUE se il nodo corrente e' "resolved", FALSE altrimenti.
 */

int STNode::is_resolved()
{
  if (unresolved) 
    return FALSE;
  else
    return TRUE;
} 

/*
 * STNode::is_class
 *
 * restituisce TRUE se nel nodo corrente e' definito un identificatore che e' 
 * il nome di una classe.
 */

int STNode::is_class()
{
  if (level==SECOND_LVL && ((access & ACC_INTERFACE)==0))
    return TRUE;
  else
    return FALSE;
} 

/*
 * STNode::is_method
 *
 * restituisce TRUE se il nodo corrente contiene il nome di un metodo.
 */


int STNode::is_method()
{
  return (descriptor[1]=='(');
}

/*
 * STNode::is_field
 *
 * come sopra solo che per un campo.
 */

int STNode::is_field()
{
  return (level==3 && !is_method());
}

/*
 * STNode::is_interface
 *
 * come sopra, solo che per interfacce.
 */

int STNode::is_interface()
{
  if (level==SECOND_LVL && ((access & ACC_INTERFACE)!=0))
    return TRUE;
  else
    return FALSE;
}

/*
 * STNode::is_subclass
 *
 * restituisce TRUE se nel nodo corrente e' definita una classe che e' sotto-
 * classe di un'altra classe definita nel nodo preso in input.
 */

int STNode::is_subclass(STNode *super)
{
  TreeNode *init=myheader->GetLeftC()->GetRightC()->GetLeftC()->GetRightC();

  for (TreeNode *t=init; !t->IsDummy();)
    {
      STNode *node=((IDNode *)t)->GetIden();
      if (node==super)
	return TRUE;
      t=node->getmyheader()->GetLeftC()->GetRightC()->GetLeftC()->GetRightC();
    }
  return FALSE;
}

/*
 * STNode::is_subinterface
 *
 * come sopra, solo che vale per le interfacce.
 */

int STNode::is_subinterface(STNode *super)
{
  TreeNode *head=myheader;

  if (!(head->GetLeftC()->GetRightC())->IsDummy())
    {
       head=head->GetLeftC()->GetRightC();
       while (!head->IsDummy())
	 {
	   if (((IDNode*)head->GetRightC())->GetIden()==super)
	     return TRUE;
	   else
	     if (is_subinterface(((IDNode *)head->GetRightC())->GetIden()))
	       return TRUE;
	   head=head->GetLeftC();
	 }
    }  
  else
    return FALSE;
}

/* 
 * STNode::implements
 *
 * restituisce TRUE se nel nodo corrente e' definita una classe che implementa
 * l'interfaccia definita nel nodo in input.
 */

int STNode::implements(STNode *interface)
{

  for (STNode *m=this; m; m=m->get_superclass())
    {
      TreeNode *head=m->getmyheader();
      STNode *n;

      if (!(head->GetLeftC()->GetRightC())->IsDummy() &&
	  !(head->GetLeftC()->GetRightC()->GetRightC())->IsDummy())
	{
	  head=head->GetLeftC()->GetRightC()->GetRightC();
	  while (!head->IsDummy())
	    {
	      n=((IDNode*)head->GetRightC())->GetIden();
	      if (n==interface)
		return TRUE;
	      else
		if (n->is_subinterface(interface))
		  return TRUE;
	      head=head->GetLeftC();
	    }
	}
      //      else
      // return FALSE;
    }

  return FALSE;
}

/*
 * STNode::get_superclass
 *
 * restituisce l'STNode della superclasse della classe definita nel nodo cor-
 * rente.
 */

STNode *STNode::get_superclass()
{
  TreeNode *t=myheader->GetLeftC()->GetRightC()->GetLeftC()->GetRightC();

  if (t->IsDummy())     

    /*
     * solo se la classe corrente e' java/lang/Object.
     */

    return NULL;
  else
    return ((IDNode *)t)->GetIden();
}

/*
 * STNode::is_generable
 * 
 * Restituisce il valore del campo generable. Se il nodo non denota una
 * classe, allora restituisce FALSE.
 */

int STNode::is_generable() 
{ 
  if (is_class()) 
    return generable; 
  else 
    return FALSE;
}

/*
 * STNode::set_not_generable
 *
 * Pre-condizione: il nodo deve denotare una classe.
 * Imposta la classe come non generabile, perche' magri in essa si e' verifica-
 * to un errore.
 */

void STNode::set_not_generable() { generable=FALSE; }


/*****************************************************************************
 * classe STable.                                                            *
 *****************************************************************************/

/*
 * costruttore della classe STable.
 */

STable::STable(int n) : Hashtable(n) {}

/*
 * distruttore della classe STable.
 */

STable::~STable() {}

/*
 * STable::Install_Id
 *
 * installa un identificatore nella symbol-table.
 */

STNode *STable::Install_Id(String& nam, int lev, int ind, Descriptor& des)
{
  STNode *p=new STNode(nam,lev,ind,des);
  HT[Hash(nam)].Insert((HTNode*)p);
  return (STNode *)p;
}

/*
 * STable::LookUpHere
 *
 * ricerca nella symbol-table l'identificatore avente un dato nome.
 * Se il nome e' un singlename, allora si ricerca proprio quel nome.
 * Se il nome e' un qualified name, allora si ricerca prima il nome singolo
 * della classe e, se non e' trovato, si tenta di ricercare il suo
 * qualified name. Cio' viene fatto perche' classi e interfacce pur essendo
 * memorizzate con singolo nome, in alcuni casi, possono essere memorizzati
 * anche con full-name. Per coprire anche questi casi si e' aggiunto l'ul-
 * teriore lookup.
 * Questo vale per tutte le funzioni di LookUpHere e di conseguenza per
 * quelle di LookUp (dato che utilizzano le LookUpHere).
 */

STNode* STable::LookUpHere(String& _name) 
{
  STNode *p=NULL;

  if (!_name.is_singlename())
    if ((p=(STNode*)HT[Hash(_name.to_name())].Find(_name.to_name())))
      return p;

  return (STNode*)HT[Hash(_name)].Find(_name);
}

/*
 * STable::LookUpHere
 *
 * ricerca nella symbol-table l'identificatore avente un dato nome e livello di
 * scoping.
 */

STNode* STable::LookUpHere(String& _name, int lev)
{
  if (!_name.is_singlename())
    for (STNode *n=(STNode *)HT[Hash(_name.to_name())].Find(_name.to_name()); 
	 n!=NULL; n=NextSym(n))
      if (n->getlevel()==lev) 
	return n;
  
  for (STNode *n=(STNode *) HT[Hash(_name)].Find(_name); n!=NULL; n=NextSym(n))
    if (n->getlevel()==lev) 
      return n;  

  return NULL;
}

/*
 * STable::LookUpHere
 *
 * ricerca nella symbol-table l'identificatore avente un dato nome, livello e 
 * indice di scoping.
 */

STNode* STable::LookUpHere(String& _name, int lev, int ind)
{
  if (!_name.is_singlename())
    for (STNode *n=(STNode *) HT[Hash(_name.to_name())].Find(_name.to_name()); 
	 n!=NULL; n=NextSym(n))
      if (n->getlevel()==lev && n->getindex()==ind) 
	return n;

  for (STNode *n=(STNode *) HT[Hash(_name)].Find(_name); n!=NULL; n=NextSym(n))
    if (n->getlevel()==lev && n->getindex()==ind) 
      return n;

  return NULL;
}

/*
 * STable::LookUpHere
 *
 * come sopra, solo che in piu' viene dato anche il descrittore.
 */

STNode* STable::LookUpHere(String& _name, int lev, int ind, Descriptor& des)
{
  if (!_name.is_singlename())
    for (STNode *n=(STNode *) HT[Hash(_name.to_name())].Find(_name.to_name()); 
	 n!=NULL; n=NextSym(n))
      if (n->getlevel()==lev && n->getindex()==ind && n->getdescriptor()==des) 
	return n;

  for (STNode *n=(STNode *) HT[Hash(_name)].Find(_name); 
       n!=NULL; n=NextSym(n))
    if (n->getlevel()==lev && n->getindex()==ind && n->getdescriptor()==des) 
      return n;

  return NULL;
}

/*
 * STable::LookUp
 *
 * Questo tipo di ricerca si diferenzia dal fatto che un nome e' ricercato
 * a partire dal livello lev fino a 0 andando all'indietro.
 */

STNode* STable::LookUp(String& _name, int lev)
{
  STNode *p=NULL;

  for (int i=lev; i>0; i--)
    if ((p=LookUpHere(_name,i))!=NULL)
      break;
  return p;
}

STNode* STable::LookUp(String& _name, int lev, int index)
{
  STNode *p=NULL;

  for (int i=lev; i>0; i--)
    if ((p=LookUpHere(_name,i,index))!=NULL)
      break;
  return p;
}

/*
 * STable::NextSym
 *                
 * Restituisce il simbolo successivo avente la stessa chiave del nodo preceden-
 * temente trovato.
 */
 
STNode* STable::NextSym(STNode *node)
{

  /*
   * Modificare l'algoritmo affinche' sfrutti le potenzialita' di NextSym di 
   * Hashtable.
   */

  if (node!=NULL)
    {
      for (HTNode *p=node->getnext(); p!=NULL; p=p->getnext())
	if (p->getname()==node->getname())
	  return (STNode*)p;
    }
  return NULL;    
}

/*
 * STable::NextSym
 *                
 * Restituisce il simbolo successivo avente la stessa chiave e livelli di
 * scoping del nodo precedentemente trovato.
 */
 
STNode* STable::NextSym(STNode *node, int lev, int ind)
{
  for (STNode *n=NextSym(node); n!=NULL; n=NextSym(n))
    if (n->getlevel()==lev && n->getindex()==ind)
      return n;
  return NULL;
}

/*
 * STable::PrintTable
 *
 * Stampa su stream della symbol-table.
 */

void STable::PrintTable(FILE *stream)
{
  HTList *app=HT;
  fprintf(stream,"\n\nStampa Hash:\n\n");

  for(int i=0; i<num_bucket; i++)
    {
      fprintf(stream,"bucket #%d:\n",i);
      app->PrintList(stream);
      app++;
    }
  fprintf(stream,"\nBasket:\n\n");
  basket->PrintList(stream);
}

/*
 * STable::Recover
 *
 * Recupera un nodo dal cestino. Il Recover della symbol-table si differenzia
 * da quello della hash table, per il fatto che esistono alcuni nodi definiti
 * "non utilizzabili" i quali risiedono perennemente nel cestino e che non
 * e' possibile recuperare.
 */

void STable::Recover(STNode *node)
{
  if (node && node->is_used())
    ((Hashtable *)this)->Recover(node);
}

/*
 * STable::Discard
 *
 * Come Recover anche qui noi sovrascriviamo il metodo Discard per Hashtable,
 * dato che solo i nodi in uso possono essere scartati dato che quelli
 * "unused" lo sono per definizione.
 */

void STable::Discard(STNode *node)
{
  if (node && node->is_used())
    ((Hashtable *)this)->Discard(node);
}
