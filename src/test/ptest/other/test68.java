/* Test 79:
 *
 * Controlliamo l' efficacia degli accessi sui costruttori.
 * Il riferimento a costruttori di classi non collegate gerarchicamente a
 * quella in cui l'invocazione viene fatta, e' trattata nel test55.java.
 */

package gimnasium;

class pluto {

  private pluto() {}  
  protected pluto(double k) {}

}

class minni extends pluto {

minni(int j)
  {
    super(10);
  }

minni() 
  {
    super();   /* errore!!! il costruttore pluto() di pluto e' private. */  
  }

}
