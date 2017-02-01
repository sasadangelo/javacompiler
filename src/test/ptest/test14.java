/* Test 14:
 *
 * Questo test controlla l'allocazione nella symbol table delle variabili
 * locali a un metodo. 
 */


package packname;

class s1 {
 
  void pippo() 
    {
      int a,b;
      String a;   // errore!!! variabile locale duplicata.
    }
  void pluto() 
    {
      int a,b;
    }

}


