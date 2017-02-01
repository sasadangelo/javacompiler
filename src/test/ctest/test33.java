/*
 * Test 33
 *
 * Testiamo le espressioni di pre-incremento pre-decremento, post-incremento e
 * post decremento.
 */

class HelloWorld 
{
  static int k=0;

  static void print(float i)
  {
    System.out.println(i);
    return;
  }

  static public void main(String[] arg)
  {
    int i=0;
    long j=10;
    float z=20;

    print(++i);      // stampa 1.
    print(i++);      // stampa 1.
    print(i);        // stampa 2.

    print(--j);      // stampa 9.
    print(j--);      // stampa 9.
    print(j);        // stampa 8.

    print(++k);      // stampo 1. 
    print(k++);      // stampo 1.
    print(k);        // stampo 2.

    print(++z);      // stampo 21. 
    print(z++);      // stampo 21.
    print(z);        // stampo 22.    
  }
}
