/*
 * Test10
 *
 * Inserisco un carattere c e lo stampo per n volte, con n==10.
 */

import java.io.*;

class HelloWorld 
{
  static public void main(String[] a) throws IOException
  {
    int n;
    char c;

    n=10;

    System.out.println("inserisci un carattere:");

    c=(char) System.in.read();

    for (int j=1; j<=n; ++j) System.out.println(c);

  }
}
