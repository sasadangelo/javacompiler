/*
 * Test 9
 *
 * Qui proviamo a testare l'input da tastiera. Inoltre testeremo anche il
 * costrutto do..while.
 */

import java.io.*;

class HelloWorld 
{
  static public void main(String[] a) throws IOException
  {
    char c;

    System.out.println("Edit characters. 'q' to quit.");

    do {
      c=(char) System.in.read();
      System.out.println(c);
    } while (c!='q');
 
  }
}
