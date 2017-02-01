/*
 * Test 29
 *
 * Testiamo ora l'operatore di moltiplicazione. 
 */

class HelloWorld {
  static public void main(String[] argv)
  {
    short a=(short)10;
    int i=2;
    long k=3, m;

    m=a*i;

    System.out.println(m);  // stampo 20.

    m=k*3;

    System.out.println(m);  // stampo 9.

    k=k*2;

    System.out.println(k);  // stampo 6.
  }
}
