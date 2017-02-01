/* Test 75:
 *
 * Vedremo ora la dichiarazione di superinterfacce per un'interfaccia.
 */

package gimnasium;

interface applet {}

public interface view {}

public interface graph extends view, applet {}

public interface mike extends view, applet, view
{
  /* 
   * l'errore di duplicazione di interfaccia non deve essere visualizzata,
   * il compilatore omettera' volontariamente l'interfaccia duplicata.
   */
}







