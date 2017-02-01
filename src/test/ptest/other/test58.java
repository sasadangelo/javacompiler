/* Test 58:
 *
 * Istruzione throw.
 *
 * La validita' dell'istruzione throw verra' fatta controllando le seguenti 
 * cose:
 * 
 * 1) Caricare la classe Throwable se per caso non lo si e' fatto in
 *    precedenza;
 * 2) fare il parser dell'espressione e controllare che essa sia assegnabile,
 *    secondo l'algoritmo di Assign Conversion, altrimenti si avra' un
 *    errore di compilazione;
 * 3) Una delle seguenti 3 condizioni deve essere vera o ci sara' un errore 
 *    di compilazione:
 *
 *    a) l'espressione denota la classe RuntimeException o una sua sotto-
 *       classe; oppure l'espressione denota la classe Error o una sua 
 *       sottoclasse;
 *    b) l'istruzione "throw" e' invocata all'interno di un'istruzione
 *       "try" e l'espressione e' assegnabile a uno dei parametri delle
 *       clausole "catch";
 *    c) "throw" e' lanciato in un metodo o costruttore e l'espressione e'
 *       assegnabile a una delle classi disposte nella clausola "throws"
 *       di questi.
 *
 * Di quest'ultime tre condizioni solo la (b) non viene gestita dal nostro
 * compilatore, per scelta progettuale, perche' non ritenuta complessa e
 * significativa per il proseguimento del lavoro.
 */

class MiniTower extends RuntimeException { }

class ModeTower extends Error { }

class Student { } 

class MyTest {

  void prova()
  {
    Driver pp;
    java.lang.Throwable zz;
    RuntimeException ss;
    MiniTower dd;
    java.lang.Error aa;
    ModeTower kk;
    
    /*    throw new Driver();
    throw new Throwable();
    throw new RuntimeException();
    throw new MiniTower();
    throw new Error();
    throw new ModeTower();*/
  }

  void g() throws Driver, ModeTower {
    Gino jj;
    
    throw jj;
  }
}

class Gino extends Driver { }

class Driver extends Throwable { }




















