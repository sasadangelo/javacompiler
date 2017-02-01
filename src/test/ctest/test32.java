/*
 * Test 32
 *
 * Testiamo ora l'operatore di negazione. 
 */

class HelloWorld {
  static public void main(String[] argv)
  {
    short a=(short)10;
    int i=2;
    long k=3, m;
    float t=(float)3.2;
    double x, y=10.3;

    m=-i;

    System.out.println(m);  // stampo -2.

    m=-a;

    System.out.println(m);  // stampo -10.

    x=-t;

    System.out.println(x);  // stampo -3.2.
    
    x=-y;

    System.out.println(x);  // stampo -10.3.
  }
}
