/*
 * Test 24
 *
 * Testiamo ora l'operatore ==.
 */

class HelloWorld {
  static public void main(String[] argv)
  {
    boolean a, z=false, x=false;
    long i, k=1, m=2;
    int j, l=4, u=8;
    
    a=l==k;

    System.out.println(a);   // stampo false.

    a=z==x;

    System.out.println(a);   // stampo true.

    if ((l==4) && (m==2))
      System.out.println("L'espressione e' vera.");
    else
      System.out.println("L'espressione e' falsa.");
  }
}
