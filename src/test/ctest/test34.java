/*
 * Test 34
 *
 * Testiamo le espressioni di negazione logica e negazione bit a bit.
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
    int i=10;
    long j=16;
    boolean a;

    print(~i);                 // stampo -11.
    print(~j);                 // stampo -17.
    print(~k);                 // stampo -1.

    a=!true;

    System.out.println(a);     // stampo false.

    a=!(j>i && i>k);  

    System.out.println(a);     // stampo false.
  }
}







