/*
 * file environment.cc
 *
 * descrizione: questo file contiene l'implementazione della classe Environ-
 *              ment, utilizzata per sapere, in fase di compilazione, il
 *              contesto attuale in cui ci troviamo, la dove la grammatica non
 *              ce lo permette.
 *
 * Questo file e' parte del Compilatore Java.
 *
 * Maggio 1997,  scritto da Michele Risi e-mail micris@diaedu.unisa.it.
 */

#include <stdlib.h>
#include <globals.h>
#include <cstring.h>
#include <environment.h>

/*****************************************************************************
 * classe Environment                                                        *
 *****************************************************************************/

/*
 * Costruttore/Distruttore della classe Environment.
 */

Environment::Environment()
{
  class_path=getenv("CLASSPATH");
  s=new Stack<int>(STACK_ENV_SIZE);
}

Environment::~Environment()               { delete s; }

/*
 * Environment::GetClassPath
 * Environment::SetClassPath
 *
 * restituisce/imposta il classpath delle librerie Java.
 */

String& Environment::GetClassPath()       { return class_path;    }
void Environment::SetClassPath(String& c) { class_path=c;         }

/*
 * Environement::is_inContext
 *
 * Restituisce 1 se ci troviamo nell'ambiente env, 0 altrimenti.
 * env puo' assumere valori come: IN_SWITCH, IN_WHILE, etc.
 */ 

int Environment::is_inContext(int env)
{
  for (int i=s->StackSize()-1; i>=0; i--)
    if (((*s)[i] & env)!=0)
      return TRUE;
  return FALSE;
}

/*
 * Environment::openContext
 * Environment::closeContext
 *
 * Il primo apre un determinato contesto, il secondo metodo chiude l'ultimo 
 * attivato.
 */

void Environment::openContext(int env) { s->Push(env); }
void Environment::closeContext()       { s->Pop();     }
