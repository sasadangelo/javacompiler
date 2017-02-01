/* Test 5:
 *
 * Questo test controlla che non venga mai duplicato in una dichiarazione un
 * modificatore. Il Compilatore in tal caso trascura i mod. ripetuti, consi-
 * derandoli una volta sola.
 */

public class name {
  public public float field1;
  protected protected int method1() {}
}
