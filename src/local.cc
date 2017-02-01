/*
 * file local.cc
 *
 * descrizione: questo file implementa uno stack necessario per riservare
 *              spazio alle variabili locali e ai parametri formali sullo
 *              stack frame.
 *
 * Questo file e' parte del Compilatore Java.
 * 
 * Maggio 1997, scritto da Michele Risi, e-mail micris@zoo.diaedu.unisa.it
 */ 

#include <stdio.h>
#include <../stack.cc>
#include <iostream.h>
#include <fstream.h>
#include <local.h>

int max_local=1;    /*
		     * variabile globale utilizzata per calcolare la taglia max
		     * della sezione variabili locali sullo stack frame.
		     */

/*****************************************************************************
 * classe Local_Count.                                                       *
 *****************************************************************************/

/*
 * Costruttore/distruttore dello stack.
 */
 
Local_Count::Local_Count() 
{
  count=0;          // 0 is reserved for this.
}

Local_Count::Local_Count(int i)
{
  count=i;
}

Local_Count::~Local_Count() {}

/*
 * Metodi che ci permettono di accedere ai dati privati della struttura.
 */

int Local_Count::getcount()   { return count;  }

/*
 * Local_Count::inc
 *
 * Metodo di incremento del contatore. Grazie a questo meccanismo di incre-
 * mento, riserviamo spazio a parametri e variabili locali sullo stack frame.
 * Per ulteriori delucidazioni, si faccia riferimento alla documentazione 
 * allegata.
 */

int Local_Count::inc(int size) 
{ 
  count+=size;
  if (count >= max_local) max_local=count+1;
  return count;
}

/*
 * Local_Count:dec
 *
 * utilizzata solo nella gestione dell'istruzione try..catch..finally.
 */

int Local_Count::dec(int size) 
{ 
  count-=size;
  return count;
}

/*
 * Local_Count::duplicate
 *
 * Entrando in un nuovo blocco, si rende necessario inserire al top dello
 * stack un duplicato del top corrente. Il metodo seguente, esegue la
 * duplicazione.
 */

void Local_Count::duplicate(Local_Count *old)
{
  count=old->getcount();
}

/*
 * Local_Count::operator <<
 *
 * input : output stream , reference to class
 * output: none
 *
 * This method print the class on a stream.
 */

ostream& operator <<(ostream& os , Local_Count& lc)
{
  os << " count   : " << lc.count   << "  ";
  return os;
}


