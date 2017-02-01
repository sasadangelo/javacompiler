/*
 * Test 25
 *
 * Testiamo ora gli operatori relazionali. In seguito, qui testeremo anche
 * l'operatore instanceof.
 */

class HelloWorld {
  static public void main(String[] argv)
  {
    boolean a;
    long i, k=1, m=2;
    int j, l=4, u=8;
    
    a=l>=k;

    System.out.println(a);   // stampo true.

    if ((l>=4) || (m<1))
      System.out.println("L'espressione e' vera.");
    else
      System.out.println("L'espressione e' falsa.");
  }
}
