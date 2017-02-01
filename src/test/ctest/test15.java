/*
 * Test 15
 *
 * Testiamo l' istruzione return.
 */

class HelloWorld 
{
  static int getnumero()
  {
    return 10;
  }

  static public void main(String[] arg)
  {
    int i;

    i=getnumero();

    System.out.println(i);
  }
}
