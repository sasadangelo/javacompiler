/*
 * Test 35
 *
 * Testiamo la creazione di istanze di array (per ora monodimensionali) 
 * e l'accesso ai componenit di questi.
 */

import java.awt.*;

class HelloWorld 
{
  static public void main(String[] arg)
  {
    int k[]= new int [10];

    Image p[] = new Image[3];

    for (int i=0; i<10; i++) k[i]=i;

    for (int i=9; i>=0; i--) System.out.println(k[i]);
  }
}









