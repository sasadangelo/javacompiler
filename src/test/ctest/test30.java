/*
 * Test 30
 *
 * Testiamo ora l'operatore di divisione. 
 */

class HelloWorld {
  static public void main(String[] argv)
  {
    short a=(short)10;
    int i=2;
    long k=3, m;
    float z, t=3;

    m=a/i;

    System.out.println(m);  // stampo 5.

    z=k/2;

    System.out.println(z);  // stampo 1.

    z=t/2;

    System.out.println(z);  // stampo 1.5.
  }
}
