/* Test 48:
 *
 * Controlliamo l' efficacia degli accessi sui costruttori+Creazione di
 * istanze di classi.
 */

package gimnasium;

class gigi {
  final gigi () {}   /* 
		      * errore!!! i costruttori non possono essere: final
                      * abstract, native, sync. e static.
		      */ 
}

class minni
{
  minni(int k) {}
  protected minni(boolean j) {}
}

class pippo 
{
  void mino()
  {
    minni k;
    short n;

    k=new minni (n);         // n e' convertito a int.
    k=new minni ();          // errore!!! minni non ha costruttore minni().
    k=new minni(true);       // il costruttore e' protetto ma e' definito
                             // nello stesso pacchetto, quindi e' accessibile.
    k=new minni(true,true);  // errore!!! costruttore inesistente.
  }
}



