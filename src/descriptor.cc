/*
 * file descriptor.cc
 *
 * descrizione: questo file implementa la classe Descriptor utilizzata per
 *              implementare i descrittori della JVM.
 *              Per chi non conosce l'uso dei descrittori, ricordiamo che
 *              essi sono fondamentali per il type checking del compilatore.
 *
 * Questo file e' parte del Compilatore Java.
 *
 * Dicembre 1997, scritto da Salvatore D'Angelo e-mail xc0261@xcom.it.
 */

#include <string.h>
#include <globals.h>
#include <cstring.h>
#include <descriptor.h>

Descriptor DesNull;
Descriptor DesByte;
Descriptor DesChar;
Descriptor DesDouble;
Descriptor DesFloat;
Descriptor DesInt;
Descriptor DesLong;
Descriptor DesShort;
Descriptor DesBoolean;
Descriptor DesVoid;

Descriptor DesString;
Descriptor DesThrowable;
Descriptor DesStringBuffer;

/*****************************************************************************
 * classe Descriptor                                                         *
 *****************************************************************************/

/*
 * Costruttori della classe Descriptor, per lo piu' ereditati dalla classe
 * String.
 */

Descriptor::Descriptor() : String() { }
Descriptor::Descriptor(const char* s) : String (s) { }
Descriptor::Descriptor(String& s) : String (s.to_char()) { }

/*
 * Distruttore della classe.
 */

Descriptor::~Descriptor() { }

/*
 * Descriptor::operator +
 * 
 * implementa la concatenazione tra due descrittori. Cio' che differenzia
 * questo operatore dalla concatenazione tra stringhe e' che anziche'
 * restituire una stringa, l'operatore restituisce un descrittore.
 * Poiche' Descriptor IS A String, e' chiaro che l'operatore puo' ricevere
 * in input anche Descriptor&.
 */

Descriptor& Descriptor::operator+(String& s)
{
  return (Descriptor&)(((String&)(*this))+s);
} 

Descriptor& Descriptor::operator+(const char *c)
{
  return (Descriptor&)(((String&)(*this))+c);
}

/*
 * Descriptor::operator +=
 *
 * Questo operatore esegue la concatenazione tra il descrittore corrente e un
 * descrittore (o stringa o sequenza di caratteri) in input. Il risultato ver-
 * ra' riposto nel descrittore corrente.
 */

Descriptor& Descriptor::operator+=(String& s)
{
  ((String&)(*this))+=s;
  return *this;
}

Descriptor& Descriptor::operator+=(const char *c)
{
  (((String&)(*this))+=c);
  return *this;
}  

/*
 * Descriptor::operator =
 *
 * Questo operatore permette di assegnare al descrittore corrente un descrit-
 * tore (stringa o sequenza di caratteri) in input.
 */

Descriptor& Descriptor::operator=(String& s)
{
  (((String&)(*this))=s);
  return *this;
}

Descriptor& Descriptor::operator=(const char *c)
{
  (((String&)(*this))=c);
  return *this;
}

/*
 * Descriptor::operator <<
 *
 * operatore utilizzato per inviare un descrittore a uno stream.
 */

ostream& operator<<(ostream &os, Descriptor &d) { return os << d.to_char(); }

/*
 * Descriptor::cut
 *
 * Taglia il descrittore corrente da init a end.
 * Esempio:
 *
 * per "Ljava/lang/String;"  cut(2,8) restituisce "java/la"
 */

Descriptor& Descriptor::cut(int init, int end)
{
  return (Descriptor&)(((String&)(*this)).cut(init,end));
}

/*
 * Descriptor::is_primitive
 *
 * Questo metodo restituisce TRUE se il descrittore corrente e' un tipo
 * primitivo (short, byte, char, int, long, float, double), FALSE altrimenti.
 */
  
int Descriptor::is_primitive()
{
  if (is_integral() || is_floating())
    return TRUE;
  else
    return FALSE;
}

/*
 * Descriptor::is_link
 *
 * Restituisce TRUE se il descrittore corrente denota un link a classe o in-
 * terfaccia.
 */

int Descriptor::is_link()
{
  if ((*this)[1]=='L')
    return TRUE;
  else
    return FALSE;
}

/*
 * Descriptor::is_array
 *
 * Restituisce TRUE se il corrente descrittore denota un array.
 */

int Descriptor::is_array()
  {
    if ((*this)[1]=='[')
      return TRUE;
    else
      return FALSE;
  }

/*
 * Descriptor::is_reference
 *
 * Restituisce TRUE se il descrittore e' un reference type.
 */

int Descriptor::is_reference()
  {
    if (is_link() || is_array())
      return TRUE;
    else
      return FALSE;
  }

/*
 * Descriptor::is_integral
 *
 * Restituisce TRUE se il corrente descrittore denota un integral type,
 * cioe': byte, short, char, int, long.
 */

int Descriptor::is_integral()
  {
    if (*this==DES_INT  || *this==DES_SHORT || *this==DES_LONG ||
	*this==DES_CHAR || *this==DES_BYTE)
      return TRUE;
    else
      return FALSE;
  }

/*
 * Descriptor::is_floating
 *
 * Restituisce TRUE se il descrittore corrente e' un FLOAT o DOUBLE.
 */

int Descriptor::is_floating()
  {
    if (*this==DES_FLOAT || *this==DES_DOUBLE)
      return TRUE;
    else 
      return FALSE;    
  }

/*
 * Descriptor::is_16
 * Descriptor::is_32
 *
 * Utilizzato nella gestione della JVM, questi due metodi servono per capire
 * quando un descrittore che denota un primitive type e' a 16 o 32 bit.
 */

int Descriptor::is_16()
  {
    if (*this==DES_INT   || *this==DES_FLOAT || *this==DES_CHAR ||
	*this==DES_SHORT || *this==DES_BYTE)
      return TRUE;
    else 
      return FALSE;
  }

int Descriptor::is_32()
  {
    if (*this==DES_LONG || *this==DES_DOUBLE)
      return TRUE;
    else 
      return FALSE;    
  }

/*
 * Descriptor::to_fullname
 *
 * Questo metodo e' utilizzato quando il descrittore e' un link type.
 * In tal caso, allora esso preleva l'intera stringa eccetto il carattere
 * iniziale 'L' e quello finale ';'.
 * Esempio:
 *
 * sia il descrittore  "Ljava/lang/String;" allora il metodo restituisce
 * "java/lang/String".
 *
 * Da invocare solo se il descrittore e' un link type.
 */

String& Descriptor::to_fullname()
  {
    return cut(2,getlength()-1);
  }

/*
 * Descriptor::to_singlename
 *
 * Analogo a sopra, solo che questa volta viene restituita solo l'ultima parte
 * del nome.
 * Esempio:
 * 
 *       "Ljava/lang/String;" ---> "String".
 *
 * Da invocare solo se il descrittore e' un link a classe o interfaccia.
 */

String& Descriptor::to_singlename()
  {
    int i;
    
    for (i=getlength(); i>0; i--)
      if ((*this)[i]=='/')
	break;
    if (i==0)
      return cut(2,getlength()-1);
    else
      return cut(i+1,getlength()-1);
  }

/*
 * Descriptor::to_packagename
 * 
 * Come sopra, solo che questa volta viene estratto il nome del pacchetto.
 *
 * Esempio:
 *
 *        "Ljava/lang/String;" --> "java/lang"
 *
 * Ovviamente nel compilatore e' necessario che tale metodo venga invocato
 * solo su descrittori che denotano un link type.
 */

String& Descriptor::to_packagename()
  {
    int i;

    for (i=getlength(); i>0; i--)
      if ((*this)[i]=='/')
	break;
    if (i==0)
      return *(new String(DES_NULL));
    else
      return cut(2,i-1);
  }

/*
 * Descriptor::to_signature
 * 
 * Se nei metodi to_fullname, to_singlename e to_packagename la precondizione
 * era che il descrittore fosse un link type, qui il descrittore deve essere
 * quello di un metodo, il cui formato e'
 *
 *             '('<tipo argomenti>')'<tipo di ritorno>
 *
 * Si noti che il valore di ritorno e' una stringa e non un descrittore, questo
 * perche' una signature non e' un descrittore valido secondo le specifiche
 * di Java.
 * La stringa restituita, comprende anche le 2 parentesi di apertura e 
 * chiusura.
 */

Descriptor& Descriptor::to_signature()
{
  int i;
  for (i=getlength(); i>0; i--)
    if ((*this)[i]==')')
      break;
  return cut(1,i);
}

/*
 * Descriptor::array_comp_type
 *
 * La precondizione qui e' che il descrittore sia quello di un array.
 * Sotto tale ipotesi il metodo restituisce il tipo dei componenti.
 * Esempio:
 *
 *      "[I" --> "I"
 *
 * ossia se il descrittore e' un array di interi, allora l'output del metodo
 * e' il descrittore degli interi.
 */

Descriptor& Descriptor::array_comp_type()
  {
    return cut(2,getlength());    
  }

/*
 * Descriptor::return_type
 *
 * Precondizione: il descrittore e' quello di un metodo.
 * L'output e' il descrittore del valore di ritorno.
 *
 * Esempio:
 *
 *          int method pippo (int i, int j, long k)
 * ha per descrittore "(IIJ)I" il cui valore di ritorno e' "I".
 */
 
Descriptor& Descriptor::return_type()
  {
    for (int i=getlength(); i>0; i--)
      if ((*this)[i]==')')
	return cut(i+1,getlength());    
  }

/*
 * Descriptor::build_link
 *
 * A partire da una stringa o sequenza di caratteri, si costruisce un descrit-
 * tore che denotera' un riferimento a classe o interfaccia.
 */

void Descriptor::build_link(String& s)
{
  delete str;
  length=s.getlength()+2;
  str=new char [length+1];
  str[0]='L';
  for (int i=1; i<=length-2; i++) str[i]=s[i];
  str[length-1]=';';
  str[length]='\0';
}

void Descriptor::build_link(const char *c)
{
  delete str;
  length=strlen(c)+2;
  str=new char [length+1];
  str[0]='L';
  for (int i=1; i<=length-2; i++) str[i]=c[i-1];
  str[length-1]=';';
  str[length]='\0';
}

/*
 * Descriptor::build_array
 *
 * Sia il descrittore corrente di tipo D, allora il descrittore diventera'
 * un array di tipo D.
 */

void Descriptor::build_array()
{
  char *tempstr=str;
  length=strlen(str)+1;
  str=new char [length+1];
  str[0]='['; str[1]='\0';
  strcat(str,tempstr);
  str[length+1]='\0';
  // delete tempstr;  inspiegabilmente questo delete mi da un seg. fault
}

/*
 * Descriptor::build_signature
 *
 * Sia D il descrittore corrente. Come pre-condizione, esso deve essere la con-
 * catenazione di piu' descrittori (che denotano parametri o argomenti di un 
 * metodo). Il descrittore D viene trasformato in una signature come mostrato
 * nel seguente esempio.
 * Esempio:
 *
 *                          in: IJB ---> out: (IJB)
 */

void Descriptor::build_signature()
{
  char *tempstr=str;

  length=strlen(str)+2;
  str=new char[length+1];
  str[0]='(';
  for (int i=1;i<=length-2;i++)
    str[i]=tempstr[i-1];
  str[length-1]=')';
  str[length]='\0';
  delete tempstr;
}

/*
 * Descriptor::build_method
 *
 * Costruisce un descrittore per un metodo. In input esso riceve 2 descrittori,
 * uno che denota la signature e l'altro che denota il valore di ritorno.
 */

void Descriptor::build_method(Descriptor& signature, Descriptor& ret)
{
  char *tempstr=str;

  length=signature.getlength()+ret.getlength()+2;
  str=new char[length+1];
  str[0]='(';
  for (int i=1; i<=signature.getlength(); i++) 
    str[i]=signature[i];
  str[signature.getlength()+1]=')';
  for (int i=signature.getlength()+2; i<length; i++) 
    str[i]=ret[i-signature.getlength()-1];
  str[length]='\0';
  delete tempstr;
}

/*
 * Descriptor::to_typename
 *
 * Questo metodo e' utilizzato principalmente nella stampa di messaggi di 
 * errore, dove per un dato descrittore, e' richiesto il nome per esteso in
 * formato char*.
 */

const char *Descriptor::to_typename()
{ 
  Descriptor des;
  char *str=new char[1];
  char *oldstr=NULL;
  const char *primitivestr;
  char *brackets;

  des=*this;
  str[0]='\0';
  brackets=new char[3];
  brackets[0]='['; brackets[1]=']'; brackets[2]='\0';
  for(;;)
    if (des.is_array())
      {
	oldstr=str;
	str=new char[strlen(str)+2+1];
	str[0]='\0';
	strcat(str,oldstr);
	strcat(str,brackets);
	des=des.cut(2,des.getlength());
	delete oldstr;
      }
    else
      break;
  if (des.is_reference())
    {
      oldstr=str;
      str=new char[strlen(str)+des.getlength()-2+1];
      str=strcat((char *)(des.cut(2,des.getlength()-1).to_char()),oldstr);
      str[des.getlength()-2+strlen(oldstr)]='\0';
      delete oldstr;
    }
  else
    if (des.is_primitive() || des==DES_BOOLEAN || des==DES_NULL)
      {
	if (des==DES_BYTE)         primitivestr="byte";
	else if (des==DES_SHORT)   primitivestr="short";
	else if (des==DES_CHAR)    primitivestr="char";
	else if (des==DES_INT)     primitivestr="int";
	else if (des==DES_LONG)    primitivestr="long";
	else if (des==DES_FLOAT)   primitivestr="float";
	else if (des==DES_DOUBLE)  primitivestr="double";
	else if (des==DES_BOOLEAN) primitivestr="boolean";
	else if (des==DES_NULL)    primitivestr="(error)";
	
	oldstr=str;
	str=new char[strlen(str)+strlen(primitivestr)+1];
	str[0]='\0';
	strcat(str,primitivestr);
	strcat(str,oldstr);
	str[strlen(oldstr)+strlen(primitivestr)]='\0';
	delete oldstr;
      }
  return str;
}

/*
 * I due metodi che seguono, vengono utilizzati esclusivamente nel loadclass,
 * affinche' dal descrittore di un metodo si riesca a costruire il parse-tree
 * dei parametri da sfruttare nell'algoritmo di Applicabilita' di un'invo-
 * cazione di metodo.
 *
 * Attenzione!!!
 *
 * La soluzione di utilizzare questi due metodi aggiuntivi non mi soddisfa
 * molto, comunque utilizzeremo questa in attesa di trovarne una migliore.
 */

/*
 * Descriptor::num_args
 *
 * Precondizione: il descrittore in input e' quello di un metodo.
 *
 * Questo metodo, viene utilizzato quando dal descrittore di un metodo, si
 * vuole reperire i descrittori dei singoli argomenti. Il metodo
 */

int Descriptor::num_args()
{
  int count=0;

  for (int i=2; i<=this->getlength(); i++)
    {
      int j;

      if ((*this)[i]==')') return count;

      if ((*this)[i]=='[') continue;

      if ((*this)[i]=='L') 
	{
	  for (j=i; (*this)[j]!=';'; j++);
	  count++;
	  i=j;
	}
      else
	count++;
    }
}

/*
 * Descriptor::get_arg
 *
 * Precondizione: il descrittore in input e' quello di un metodo e i< num.
 * argomenti.
 *
 * Restituisce il descrittore dell'i-esimo argomento del metodo.
 */

Descriptor& Descriptor::get_arg(int i)
{
  int low, upper, k=2, j;

  for (int count=1; count <i;)
    {
      if ((*this)[k]=='L')
	{
	  for (j=k; (*this)[j]!=';';j++);
	  count++;
	  k=j+1;
	}
      else
	if ((*this)[k]=='[')
	  k++;
	else
	  { 
	    count++;
	    k++;
	  }
    }
  low=k;
  
  for (;;)
    {
      if ((*this)[k]=='L')
	{
	  for (j=k; (*this)[j]!=';'; j++);
	  upper=j;
	  break;
	}
      else
	if ((*this)[k]=='[') 
	  k++;
	else
	  {
	    upper=k;
	    break;
	  }
    }

  return this->cut(low,upper);
}
