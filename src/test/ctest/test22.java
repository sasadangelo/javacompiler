/*
 * Test 22
 *
 * Testiamo ora l'operatore &.
 */

class HelloWorld {
  static public void main(String[] argv)
  {
    boolean a, z=true, x=false;
    long i, k=1, m=2;
    int j, l=4, u=8;
    
    i=l&k;
    j=l&u;
    a=z&x;

    System.out.println(i);  // stampo 0.
    System.out.println(j);  // stampo 0.
    System.out.println(a);  // stampo false.
    
  }
}
