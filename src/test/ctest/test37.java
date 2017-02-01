/*
 * Test 37
 *
 * In questo file testiamo l'istruzione try..catch, generando una divisione per
 * 0.
 */

class HelloWorld 
{
  static public void main(String[] args) {
    int d, a;
    
    try {
      d=0;
      a=42/d;
      System.out.println("Questo messaggio non verra' visualizzato.");
    } catch (ArithmeticException e) {
      System.out.println("Salvatore wrote: divisione per 0");
    }

    System.out.println("Eccezione catturata");
  }
}

