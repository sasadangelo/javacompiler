/* Test 47:
 *
 * Ancora sull'invocazione di metodi.
 *
 * In questo test, controlliamo gli algoritmi di applicabilita' di un metodo e
 * quello sulla scelta del metodo piu' specififco.
 */

package gimnasium;

class attila {
  int cashmire(long i, long j) {}	
  float fabio() {}	
  double udik(long i, long j) {}	
}


class agamennone extends attila {
  int plutargo(int k) {}
  int plutargo(long k) {}

  int cashmire(int i, int j) {}
  int fabio() {}
  double udik(long i, double j) { }		
}

class first extends agamennone {
  void pippo(int j) {}
  
  void cake(float h) {}

  void jack(int i, float k)
  {
    short j;

    pippo();         // errore!!!
    pippo(10);
    pippo(10.3);     // errore!!!
    pippo(j);
    plutargo(10);
    cashmire(5,5);   // chiama cashmire della classe attila.

    fabio();	     // Viene esegiuto l'overriding.

    cashmire(j,j);
    udik(10,10);     // errore!!! rif. ambiguo.
  }
}














































