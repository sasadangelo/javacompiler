/*
 * Test 26
 *
 * Testiamo ora gli operatori di shift.
 */

class HelloWorld {
  static public void main(String[] argv)
  {
    short a=(short)10, b=(short)-16;
    int i;

    i=a >> 2;

    System.out.println(i);  // stampo 2.

    i=a << 2;

    System.out.println(i);  // stampo 40.

    i=b >> 2;

    System.out.println(i);  // stampo -4.

    i=a >>> 1;

    System.out.println(i);  // stampo 5.
  }
}
