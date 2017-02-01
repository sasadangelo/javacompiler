/*
 * Test12
 *
 * Testiamo l'istruzione switch.
 */

class HelloWorld 
{
  static public void main(String[] a)
  {
    int i;

    i=1;

    System.out.println("Analisi primo switch");

    switch (i)
      {
      case 1: System.out.println("i==1");
      case 2: System.out.println("i==2");
      case 3: System.out.println("i==3");
      }

    i=2;

    System.out.println("Analisi secondo switch, con istruzioni break");

    switch (i)
      {
      case 1: System.out.println("i==1"); break;
      case 2: System.out.println("i==2"); break;
      case 3: System.out.println("i==3"); break;
      }
  }
}
