/*
 * Test 3
 *
 * Iniziamo con l'esaminare la generazione di codice per le istruzioni java.
 * Iniziamo con l'if..then..else.
 */

class HelloWorld 
{
  static int field;
  static int field1=3;

  static public void main(String[] a)
  {
    field=5;

    if (field==5)
      System.out.println(field);
    else
      System.out.println(field1);

    if (field!=5)
      System.out.println(field);
    else
      System.out.println(field1);
  }
}

