/* Test 12:
 *
 * Questo test controlla che se un metodo non ha corpo, allora deve essere 
 * dichiarato ABSTRACT. Al contrario se il metodo e' ABSTRACT, deve verificar-
 * si un errore, se metto il corpo.
 */

package packname;

class s1 { 
  void pippo();             // errore!!!
  abstract int pluto() {}   // errore!!! 
}


