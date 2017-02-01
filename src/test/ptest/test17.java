/* Test 17:
 *
 * Questo test controlla che 2 metodi possano avere lo stesso nome, nello
 * stesso spazio nomi, a patto che la firma sia diversa.
 * 
 * Attenzione!!! per firma, qui intendiamo il descrittore meno il valore di 
 * ritorno. Lo schema di traduzione deve controllare tutti i metodi esistenti 
 * nel livello di scoping.
 */

package packname;

class s1 {
  void pluto() {}
  void pluto(int c) {}  // corretto!!!
  int  pluto() {}       // errore  !!!
}
