/* Test 19:
 *
 * Questo test controlla l'uso della clausola Throws per un metodo.
 * Controlliamo i seguenti casi:
 *
 * a) la classe non esiste;
 * b) la classe e' sottoclasse di Throwable;
 * c) la classe esiste ma non e' sottoclasse di Throwable;
 */

package packname;

class  errcerchio  { }

class s1
{
  void pippo(int a) throws errcerchio, errquadrato {} // errore!!!
  void pippo() throws clara, errquadrato {}           // errore!!!
}

class errquadrato extends Throwable { }
