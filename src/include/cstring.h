/*
 * file cstring.h
 *
 * descrizione: in questo file viene definita la classe String. Essa e' una
 *              classe molto generica per la manipolazione di stringhe, 
 *              studiata per essere utilizzata anche in altri programmi.
 *              A questa classe sono aggiunti alcuni metodi per la gestione
 *              dei nomi in Java.
 *
 * Questo file e' parte del Compilatore Java.
 *
 * Maggio 1997, scritto da Salvatore D'Angelo e-mail xc0261@xcom.it.
 */

#ifndef CSTRING_H
#define CSTRING_H

#include <iostream>
using namespace std;

class String {

protected:
  char *str;
  int length;

public:
  String(const char *);
  String(const char *,int);
  String();
  ~String();

  int     getlength();
  const char*   to_char();
  String& cut(int,int);

  char    operator[](int);

  String& operator+(String&);
  String& operator+(const char *);

  String& operator+=(String& );
  String& operator+=(const char *);

  String& operator=(String& );
  String& operator=(const char *);

  int     operator==(String& );
  int     operator==(const char *);

  int     operator!=(String& );
  int     operator!=(const char *);

  friend  ostream& operator<<(ostream&, String&);

  /*
   * Nella versione del 03/09/97 nella classe CompileUnit utilizzavamo 3 
   * metodi per la gestione dei nomi Java. Per maggiore modularita' abbiamo
   * preferito spostare questi metodi nella classe String, pur sapendo che
   * da un punto di vista del design Object Oriented sarebbe stato piu' 
   * corretto l'uso di una classe Name, figlia di String, come fatto per i
   * descrittori.
   */

  int     is_singlename();
  String& to_name();
  String& to_package();
};

#endif
