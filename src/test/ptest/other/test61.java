/* Test 72:
 *
 * Iniziamo ora il test sulle interfacce.
 */

package gimnasium;

interface applet {
}

abstract public interface graph {
}

final interface demo {

  /* errore!!! un'interfaccia non puo' essere final. */

}

synchronized native interface gui {

  /* errore!!! un'interfaccia non puo' essere synchronized, native, pro-
    tected, static. */

}

interface demo {

  /* errore!!! duplicazione nome interfaccia. */

}

interface view extends demo, applet {


}

class pluto {}

interface galaxy extends pluto {


}









