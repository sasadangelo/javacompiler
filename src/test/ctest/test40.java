/*
 * Test 40
 *
 * Questo test controlla la funzionalita' del compilatore con due if annidati.
 */

class HelloWorld {
  static int off=101;
  static int width=100;
  static int maxWidth=200;

  static public void main(String[] argv)
  {
    if (off<0) {
      off = width - maxWidth;
    } else if (off + maxWidth > width) {
      off=0;
    }
    System.out.println("Off vale:"+off); // stampo "Off vale:0".
  }
}
