/*
 * Test 31
 *
 * Testiamo ora l'operatore di resto. 
 */

class HelloWorld {
  static public void main(String[] argv)
  {
    short a=(short)10;
    int i=2;
    long k=3, m;
    float z, t=3;

    m=a%i;

    System.out.println(m);  // stampo 0.

    z=k%2;

    System.out.println(z);  // stampo 1.

    z=t%2;

    System.out.println(z);  // stampo 1.
  }
}
