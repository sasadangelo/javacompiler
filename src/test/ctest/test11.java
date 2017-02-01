/*
 * Test11
 *
 * Testiamo la chiamata a un metodo definito in questo file (non di sistema).
 */

class HelloWorld 
{
  static void MyMethod() 
  {
    System.out.println("Sto in MyMethod");
  }

  static public void main(String[] a)
  {
    MyMethod();
  }
}
