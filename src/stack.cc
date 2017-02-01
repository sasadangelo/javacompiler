/*
 * file stack.cc
 * 
 * descrizione: questo file implementa uno stack polimorfo. Questa struttura
 *              sara' molto utilizzata all'interno del compilatore, per
 *              gestire le variabili locali, i parametri formali e varie
 *              operazioni di back-patching.
 *
 * Questo file e' parte del Compilatore Java.
 * 
 * Maggio 1997, scritto da Salvatore D'Angelo, e-mail xc0261@xcom.it.
 */

#ifndef STACK_CC
#define STACK_CC

#include <stdio.h>
#include <stack.h>

/*
 * Stack::operator <<
 *
 * operatore di stampa su stream.
 */

template<class T>
ostream& operator << (ostream& os, Stack<T>& s)
{
  os << "The Stack is:\n\n";
  for(int i=s.num_element-1;i>=0;i--)
    os << *s[i] << "\n";
  return os;
}

/*
 * Stack::operator []
 *
 * Permette di accedere all'i-esimo componente dello stack. Si tenga presente 
 * che quest'operazione non e' legale per uno stack, tuttavia noi l'abbiamo
 * implementata perche' era indispensabile per la gestione del compilatore.
 */

template<class T>
T Stack<T>::operator [](int i)
{
  if (i>=0 && i<num_element)
    return vector[i];
}

/*
 * Costruttore/distruttore dello stack.
 */

template<class T>
Stack<T>::Stack(unsigned stack_size)
{
  size=stack_size;
  vector=new T [stack_size];
  num_element=0;
}

template<class T>   
Stack<T>::~Stack()
{
  delete vector;
}

template<class T>   
unsigned Stack<T>::StackSize() { return num_element; }

/*
 * Stack::Push
 *
 * Effettua il push sullo stack.
 */

template<class T>   
void Stack<T>::Push(T element)
{
  if (num_element <= size -1)
    {
      vector[num_element]=element;
      num_element++;
    }
}

/*
 * Stack::Dup
 *
 * duplica il top dello stack. Si tenga conto che quest'operazione pur se
 * ammissibile per uno stack, e' anch'essa introdotta per facilitare la 
 * gestione del compilatore. Infatti questa operazione e' molto utile nella
 * gestione di variabili locali.
 */

template<class T>   
void Stack<T>::Dup()
{
  if (num_element <= size -1)
    {
      T t = new Local_Count();
      t->duplicate(vector[num_element-1]);
      vector[num_element]=t;
      num_element++;
    }
}

/*
 * Stack::Pop
 *
 * Effettua il pop del top dello stack.
 */

template<class T>   
T Stack<T>::Pop() 
{
  if (num_element > 0)
    return vector[--num_element];
}

/*
 * Stack::Top
 *
 * Restituisce il top dello stack.
 */

template<class T>   
T Stack<T>::Top()
{
  if (num_element > 0)
    return vector[num_element-1];
 }

#endif

