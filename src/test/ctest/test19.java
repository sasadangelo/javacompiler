/*
 * Test 19
 *
 * Testiamo ora l'operatore ||.
 */

class HelloWorld {
  static public void main(String[] argv)
  {
    boolean a;
    
    int x=10;
    int b=15;

    int c=17;
    int d=13;

    a=(x<b) || (c>d);

    System.out.println(a);  // stampo true.    
  }
}
