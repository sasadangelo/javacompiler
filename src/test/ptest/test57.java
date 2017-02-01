/* Test 57:
 *
 * Istruzione try.
 */

class Errore extends java.lang.Throwable { }

class MyTest {
  void f()
  {
    int i;

    /*
     * Il parametro definito in una clausola catch e' allocato a livello
     * di scoping (4,index) e viene rimosso dalla symbol-table, non appena 
     * termina la clausola catch.
     */

    try { 
      i=0; 
    } catch (Errore pp) {
      i=1;
    } catch (Filly zz)  {    // errore!!! Filly non esiste.  
      i=2;
    } catch (Err zz)    {    // errore!!! Err non e' sottoclasse di Throwable.
      i=2;
    }
  }

}

class Err { }



