/* Test 16:
 *
 * Questo test controlla che in un metodo cosi' come per altre parti, sia
 * possibile porre:
 *
 *           public protected private == public
 *
 * ossia se uso insieme i modificatori public, protected e  private viene
 * preso quello che da' maggiori liberta' di accesso:
 *
 *                   public > protected > private
 */

package packname;

class s1 {
 
  public protected private void pippo(int a, float b, double c) {}
  protected private void pluto(int a, float b, double c) {}
  public private int minni() {} 

}




