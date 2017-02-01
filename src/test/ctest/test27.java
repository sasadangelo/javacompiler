/*
 * Test 27
 *
 * Testiamo ora l'operatore di somma. Per ora non e' gestita la generazione
 * di codice per la somma tra stringhe.
 */

class HelloWorld {
  static public void main(String[] argv)
  {
    short a=(short)10;
    int i=2;
    long k=3, m;

    m=a+i;

    System.out.println(m);  // stampo 12.

    m=k+3;

    System.out.println(m);  // stampo 6.

    k=k+2;

    System.out.println(k);  // stampo 5.
  }
}
