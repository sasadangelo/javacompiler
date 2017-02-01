/* Test 76:
 *
 * Vedremo ora un test sulle classi,tralasciato in precedenza perche' non erano
 * ancora disponibili le interfacce.
 */

package gimnasium;

interface applet {}

public interface view {}

class genio {}

class mino extends genio implements view {}

class pino extends genio implements view, applet {}

class cake implements view, applet, view 
{
  /* erore!!! Interfaccia view ripetuta. */
}






