/* Test 13:
 *
 * Questo test controlla l'allocazione nella symbol table dei parametri 
 * formali. 
 */

package packname;

class s1 {

  void pippo(int a, float b, double c) {}
  void pluto(int x, float a, double x) {} // errore!!! id. duplicato.

}
