/* Test 46:
 *
 * Ancora sull'invocazione di metodi.
 *
 * In questo test, facciamo riferimento alla forma:
 * 
 *                      FieldName.MethodName
 */

package gimnasium;

class gemini {
  void pippo() {}
  protected float dodo() {}
  private float cake() {}
}

class minni {
  gemini k;
}

class dora extends minni {
  minni par;    // se metto private avro' qualcosa di strano.
}

class jenny {

  void gabriel ()
  {
    double i;
    minni t;
    gemini m;
    dora d;
    
    m.pippo();
    t.k.pippo();
    d.k.pippo();
    i=d.par.k.cake(); // errore!!! cake e' protected in gemini.
    i=d.par.k.dodo(); // OK!!! dodo e' protected in gemini, quindi e' acces-
                      // sibile all'interno di questo pacchetto.
  }
}











