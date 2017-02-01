/*
 * Test17
 *
 * Testiamo l'istruzione switch con lookupswitch.
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
      case 10: System.out.println("i==10");
      case 1: System.out.println("i==1");
      case 5: System.out.println("i==5");
      }

    i=2;

    System.out.println("Analisi secondo switch, con istruzioni break");

    switch (i)
      {
      case 7: System.out.println("i==7"); break;
      case 2: System.out.println("i==2"); break;
      case 22: System.out.println("i==22"); break;
      }
  }
}
