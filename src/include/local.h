/*
 * file local.h
 *
 * descrizione: questo file contiene le definizioni di una classe che viene
 *              utilizzata per riservare spazio nello stack frame ai parametri
 *              formali e alle variabili locali.
 *
 * Questo file e' parte del Compilatore Java.
 *
 * Maggio 1997, scritto da Michele Risi, e-mail micris@zoo.diaedu.unisa.it.
 */
#ifndef LOCAL_H
#define LOCAL_H

#include <ostream>

class Local_Count
{
private:
    int count;

public:
  Local_Count();
  Local_Count(int); 
  ~Local_Count();

  int getcount();
  int inc(int);
  int dec(int);

  friend std::ostream& operator <<(std::ostream&,Local_Count&);
  void duplicate(Local_Count *);
};

#endif

