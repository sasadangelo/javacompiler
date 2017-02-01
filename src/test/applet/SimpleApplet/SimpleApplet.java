/*
 * Test 1
 *
 * Con questo test inizia il controllo della libreria AWT per l'uilizzo di
 * applet. Qui noi stamperemo un semplice messaggio.
 */

import java.awt.*;
import java.applet.*;

/*
  <applet code="SimpleApplet" width=200 height=60>
  </applet>
  
*/

public class SimpleApplet extends Applet {
  public void paint(Graphics g) {
    g.drawString("A simple Applet", 20, 20);
  }
}
