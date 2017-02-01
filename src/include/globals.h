/*
 * file globals.h
 *
 * descrizione: qui sono definite alcuna costanti di uso globale.
 *
 * Questo file e' parte del Compilatore Java.
 *
 * Dicembre 1997, scritto da Salvatore D'Angelo e-mail xc0261@xcom.it.
 */

#ifndef GLOBALS_H
#define GLOBALS_H

/*
 * Livelli di scoping.
 *
 * Java Language Specification 1.0 versione beta, inizialmente ci ha indotti
 * ad un errore molto grave. In realta' sul documento si parla di 6 livelli
 * di scoping:
 *
 *     0: package name
 *     1: type import on demand
 *     2: classi e interfacce
 *     3: metodi e campi
 *     4: loc. variabili e parametri
 *     5: loc. variabili in blocchi annidati e in cicli for.
 *
 * Queste distinzioni, pero', sono rivolte all'utente finale. Dopo uno scambio
 * di mail con alcuni membri del Java Team, mi sono reso conto che, in realta',
 * i livelli 4 e 5 sono lo stesso livello di scoping. Quindi il programma:
 * 
 *                    class xxx {
 *                       void f() {
 *                          int i=10; ---> lev. 4 
 *                          {
 *                            int i=5; --> lev. 5  (realmente e' a liv. 4) 
 *                                         errore!!!
 *                          }
 *                       }
 *                     }
 *
 * che inizialmente mi pareva corretto, in realta' produce un errore nella 
 * seconda dichiarazione, perche' sono entrambi a liv. 4.
 * Invece,
 *
 *                    class xxx {
 *                       void f() {
 *                          {
 *                            int i=5; --> lev. 4
 *                          }
 *                          int i=10; ---> lev. 4 
 *                       }
 *                     }
 *
 * e' corretto dato che terminato il blocco io elimino la prima i dalla 
 * symbol-table.
 */

#define ZEROTH_LVL    0     // livello di package.
#define FIRST_LVL     1     // livello import on demand.
#define SECOND_LVL    2     // livello della classe o interfaccia.
#define THIRD_LVL     3     // livello campi o metodi.
#define FOURTH_LVL    4     // livello var. locali e parametri.

#define STABLE      Unit->GetTable()
#define ENVIRONMENT Unit->GetEnvironment()

#define TRUE        1
#define FALSE       0

#define _u1  unsigned char
#define _u2  unsigned short int
#define _u4  unsigned int

#define MAX_LOCAL 200

#define S_BRK_SIZE 100
#define S_CON_SIZE 100

#endif
