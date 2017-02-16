/*
 * file cphash.cc
 *
 * descrizione: questo file implementa la tabella Constant Pool Hashtable,
 *              utilizzata per evitare duplicazioni di item nella Constant
 *              Pool.
 *
 * Questo file e' parte del Compilatore Java.
 *
 * Maggio 1997, scritto da Michele Risi e-mail micris@zoo.diaedu.unisa.it
 *                         Salvatore D'Angelo e-mail xc0261@xcom.it
 */ 

#include <stdio.h>
#include <globals.h>
#include <cstring.h>
#include <descriptor.h>
#include <hash.h>
#include <cphash.h>
#include <access.h>
#include <tree.h>
#include <jvm.h>

/****************************************************************************
 * classe CPHNode.                                                          *
 ****************************************************************************/

/*
 * Costruttori/distruttore del nodo.
 */

CPHNode::CPHNode(void):HTNode()                           { index=0;  }
CPHNode::CPHNode(String& _name, int _tag) : HTNode(_name) { tag=_tag; }
CPHNode::~CPHNode() {}

/*
 * Altri semplici metodi.
 */

int  CPHNode::getindex()        { return index; }
void CPHNode::setindex(int ind) { index=ind;    }
int  CPHNode::gettag()          { return tag;   }

void CPHNode::PrintNode(FILE *stream)
{
  const char *tagstr;

  switch (tag)
    {
    case CONSTANT_Class       : tagstr="CONSTANT_Class";        break;
    case CONSTANT_Fieldref    : tagstr="CONSTANT_Fieldref";     break;
    case CONSTANT_Methodref   : tagstr="CONSTANT_Methodref";    break;
    case CONSTANT_IntMethodref: tagstr="CONSTANT_IntMethodref"; break;
    case CONSTANT_String      : tagstr="CONSTANT_String";       break;
    case CONSTANT_Integer     : tagstr="CONSTANT_Integer";      break;
    case CONSTANT_Float       : tagstr="CONSTANT_Float";        break;
    case CONSTANT_Long        : tagstr="CONSTANT_Long";         break;
    case CONSTANT_Double      : tagstr="CONSTANT_Double";       break;
    case CONSTANT_NameAndType : tagstr="CONSTANT_NameAndType";  break;
    case CONSTANT_Utf8        : tagstr="CONSTANT_Utf8";         break;
    }

  fprintf(stream," Name  :  %s\n",name.to_char());
  fprintf(stream," Tag   :  %s\n",tagstr);
  fprintf(stream," Index :  %d\n",index);
}

/****************************************************************************
 * classe CPHash.                                                           *
 ****************************************************************************/

/*
 * Costruttore/distruttore della tabella. In input al costruttore, viene
 * passato il numero di bucket della tabella.
 */

CPHash::CPHash(int n) : Hashtable(n) { }
CPHash::~CPHash()                    { }

/*
 * CPHash::Install_Id
 *
 * Installa un nodo nella tabella avente un certo nome e un certo tag.
 */

CPHNode *CPHash::Install_Id(String& _name,int tag)
{
  CPHNode *p=new CPHNode(_name,tag);
  HT[Hash(_name)].Insert((HTNode*)p);
  return p;
}

/*
 * CPHash::LookUp
 *
 * Le ricerche sulla tabella a nooi interessa farle sulla chiave (name,tag).
 */

CPHNode *CPHash::LookUp(String& _name, int tag) 
{
  CPHNode *p;
  for (p=(CPHNode *)HT[Hash(_name)].Find(_name); p; p=(CPHNode *)NextSym(p))
    if (p->gettag()==tag)
      break;
  return p;
}
