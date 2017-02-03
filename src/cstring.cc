/*
 * file cstring.cc
 *
 * descrizione: questo file implementa la classe String, utilizzata per 
 *              gestire in maniera astratta le stringhe.
 *
 * Questo file e' parte del Compilatore Java.
 *
 * Maggio 1997, scritto da Salvatore D'Angelo e-mail xc0261@xcom.it.
 * Dicembre 1997, modificato da Salvatore D'Angelo.
 */

#include <stdio.h>
#include <iostream>
#include <string.h>
#include <globals.h>
#include <cstring.h>
using namespace std;

/*****************************************************************************
 * classe String                                                             *
 *****************************************************************************/

/*
 * Costruttori di una stringa nulla o derivante da un (char*)
 */
String::String(const char *_str)
{
  length=strlen(_str);
  str=new char [length+1];
  strcpy(str,_str);
  /*
   * probabilmente l'istruzione che segue e' inutile, pero' senza di essa
   * ci sono stati problemi nella gestione di JVM.
   */
  str[length]='\0';
}

String::String(const char *s, int num)
{
  length=num;
  str=new char [length+1];
  for (int i=0; i<length; i++)
    str[i]=s[i];
  str[length]='\0';
}

String::String()
{
  length=0;
  str=new char [1];
  str[0]='\0';
}

/*
 * Distruttore di una stringa. Si occupa di deallocare la stringa dalla
 * memoria.
 */

String::~String() { delete str; }

/*
 * String::getlength
 *
 * Restituisce la lunghezza della stringa corrente.
 */

int String::getlength() { return length; }

/*
 * String::to_char
 *
 * Converte la stringa in caratteri.
 */

const char *String::to_char() { return str; }

/*
 * String::operator []
 *
 * restituisce l'i-esimo carattere della stringa.
 */

char String::operator[](int i) 
{ 
  if (i>=1 && i<=length)
    return str[i-1]; 
}

/*
 * String::operator +
 * 
 * Calcola la concatenazione tra due stringhe.
 */

String& String::operator+(String& s1)
{
  String *s=new String();
  s->length=length+s1.length;
  s->str=new char [s->length+1];
  strcpy(s->str,str);
  strcat(s->str,s1.str);
  return *s;
}

String& String::operator+(const char * s1)
{
  String *s=new String();
  s->length=length+strlen(s1);
  s->str=new char [s->length+1];
  strcpy(s->str,str);
  strcat(s->str,s1);
  return *s;
}

/*
 * String::operator +=
 *
 * Calcola prima una concatenazione tra la stringa in input e quella corrente.
 * Il risultato verra' riposto nella stringa corrente.
 */

String& String::operator+=(String& s1)
{
  char *p=str;
  length+=s1.length;
  str=new char [length+1];
  strcpy(str,p);
  strcat(str,s1.str);
  delete p;
  return *this;
}

String& String::operator+=(const char *s1)
{ 
 char *p=str;
 length+=strlen(s1);
 str=new char [length+1];
 strcpy(str,p);
 strcat(str,s1);
 delete p;
 return *this;
}

/*
 * String::operator =
 * 
 * Assegna la stringa in input alla stringa corrente. Ovviamente la nuova 
 * stringa sara' situata in una regione diversa della memoria.
 */

String& String::operator=(String& s1)
{
  length=s1.length;
  delete str;
  str=new char [length+1];
  strcpy(str,s1.str);
  return *this;
}

String& String::operator=(const char *s1)
{
  length=strlen(s1);
  delete str;
  str=new char [length+1];
  strcpy(str,s1);
  return *this;
}

/*
 * String::operator ==
 *
 * Confronta due stringhe, se sono uguali allora viene restituito 1, altri-
 * menti 0.
 * Nel secondo metodo, il confronto e' eseguito con una sequenza di caratteri.
 */

int String::operator==(String& s1) { return (strcmp(s1.str,str)==0); }  
int String::operator==(const char * s1)  { return (strcmp(s1,str)==0);     }  

/*
 * String::operator !=
 *
 * Come sopra, solo che questa volta se le due stringhe sono diverse viene
 * restituito 1, altrimenti 0.
 */

int String::operator!=(String& s1) { return (strcmp(s1.str,str)!=0); }  
int String::operator!=(const char * s1)  { return (strcmp(s1,str)!=0); }  

/*
 * String::operator <<
 *
 * Invia la stringa a uno stream.
 */

ostream& operator<<(ostream &os, String &s) { return os << s.str; }

/*
 * String::cut
 *
 * Restituisce una sottostringa della stringa corrente, a partire dal caratte-
 * re di indice init fino a quello di indice final.
 * Esempio:
 * 
 *          per "Java Compiler"  cut(3,8) restituisce "va Com".
 */

String& String::cut(int init, int final)
{
  String *s;
  char *c;

  c=new char[final-init+2];
  strncpy(c,str+init-1,final-init+1);
  c[final-init+1]='\0';
  s=new String(c);
  return *s;
}

/*
 * String::is_singlename
 * String::getpackegename
 * String::getsinglename
 *
 * Questi metodi, come descritto nel file cstring.h nella versione del 03/09/97
 * erano situate nella classe CompileUnit. In effetti, da un punto di vista
 * Object Oriented, sarebbe stato piu' corretto creare una classe Name, figlia
 * di string contenente questi metodi. Comunque questa modifica non ci e'
 * sembrata urgente e soprattutto necessaria.
 *
 * Per definizione, diciamo che per un nome di tipo: "java/lang/String"; la
 * prima parte e' "java/lang" la seconda "String".
 */

int String::is_singlename()
{
  for (int i=0; i<strlen(str); i++)
    if (str[i]=='/')
      return FALSE;
  return TRUE;
}

String& String::to_package()
{
  int i;
  for (i=strlen(str)-1; i>=0; i--)
    if (str[i]=='/')
      break;
  return this->cut(1,i);
}

String& String::to_name()
{
  int i;  

  for (i=strlen(str)-1; i>=0; i--)
    if (str[i]=='/')
      break;
  return this->cut(i+2,strlen(str));
}
