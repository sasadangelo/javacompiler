/*
 * Test 14
 *
 * Testiamo l' istruzione continue.
 */

class HelloWorld 
{
  static public void main(String[] arg)
  {
    boolean a=true;
    int i;

    i=0;

    while (a) 
      {
	if (++i<100) continue;
	break;
      }

    System.out.println(i);

    do {
      if (++i<200) continue;
      break;
    } while (!a);      // non deve eseguire mai questo test.
    
    System.out.println(i);

    for (;;) 
      {
	if (++i<300) continue;
	break;
      }

    System.out.println(i);

    switch (i)
      {
      case 299: System.out.println("ERRORE"); break;
      case 300: System.out.println("OK"); break;
      case 301: System.out.println("ERRORE"); break;
      }

    System.out.println("FINE");
  }
}
