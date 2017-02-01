/*
 * Test 3
 *
 * Questa applet carica un' immagine in formato JPEG.
 */

/*
 * <applet code="SimpleImageLoad" width 248 height=146>
 *  <param name="img" value="disney.gif">
 * </applet>
 */

import java.awt.*;
import java.applet.*;

public class SimpleImageLoad extends Applet 
{
  Image img;

  public void init() {
    img=getImage(getDocumentBase(), getParameter("img"));
  }

  public void paint(Graphics g) {
    g.drawImage(img, 0, 0, this);
  }
}
