/*
 * Test 36
 *
 * Testiamo l'utilizzo di istruzioni per la creazione di istanze di classi.
 */

import java.awt.*;

class HelloWorld 
{
  int i = 10;

  void print()
  {
    ++i;    
    System.out.println(i);
  }

  static public void main(String[] arg)
  {
    HelloWorld p=new HelloWorld();

    p.print();                // stampo 11.
  }
}
