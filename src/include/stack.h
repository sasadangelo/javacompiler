/*
 * file stack.h
 * 
 * descrizione: questo file definisce la classe polimorfa stack.
 *
 * Questo file e' parte del Compilatore Java.
 * 
 * Maggio 1997, scritto da Salvatore D'Angelo e-mail xc0261@xcom.it.
 */

#ifndef STACK_H
#define STACK_H

#include <fstream.h>

template<class T> class Stack {

private:
    T *vector;
    unsigned num_element;
    unsigned size;

public:

    Stack(unsigned);
    ~Stack();
    unsigned StackSize();
    void Push(T);
    T Pop();
    T Top();
    friend ostream& operator << (ostream&,Stack<T>&);

    T operator[](int i);         // operatore non legale per uno stack.
    void Dup();                  // operatore ad hoc per il compilatore.
};

#endif
