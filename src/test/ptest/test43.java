/* Test 43:
 *
 * Il test controlla l' invocazione di metodi. 
 * In particolare le forme:
 * 
 *                       Primary.MethodName
 *                       SUPER.MethodName                       
 */

package gimnasium;

class minni {

  void cake() { }
  void frak(int i, double j) { }
  short sandokan() {}

}

class pluto extends minni {

  void pino() {}
  void gigi(int i, double k) {}

  void cane(int i) 
  {
    this.pino();
    this.gigi(10,10.3);
    super.cake();
    super.frak(10,10.3);
    minni.frak(10,10);      // errore!!! rif. statico a un metodo non statico.
    this.cake();
    i=this.sandokan();
  }

}





