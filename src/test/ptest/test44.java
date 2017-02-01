/* Test 44:
 *
 * Ancora sull'invocazione di metodi.
 *
 * In questo test, facendo sempre riferimento alle forme del test 50, si con-
 * trolla la gestione degli accessi.
 */

package gimnasium;

class minni {
  private void cake() { }
  protected void frak(int i, float j) { }
}

class pluto extends minni {
  private void pino() {}

  void cane(int i) 
  {
    float a;

    a=(float)10.3;       // senza cast siverifica un errore, perche' 10.3 e'
                         // double.

    this.pino();
    super.cake();        // errore!!! cake e' private per minni.
    super.frak(10,a);
    this.cake();         // errore!!! cake e' private per minni.
  } 
}






