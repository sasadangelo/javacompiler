/* Test 45:
 *
 * Ancora sull'invocazione di metodi.
 *
 * In questo test, facciamo riferimento alla forma:
 * 
 *                      TypeName.MethodName
 */

package gimnasium;

class gemini {
  static void pippo() {}
  int pluto() { }
  private static void cake() { }
  public static void paperino() { } 
  protected static int hugo() { }
}

class jenny {

  void gabriel ()
  {
    int i;

    gemini.pippo();
    i=gemini.pluto(); // errore!!! pluto in gemini non e' static.
    gemini.cake();    // errore!!! cake in gemini e' private.
    gemini.paperino();
  }
}

class dino extends gemini {
  void sandokan()
  {
    float k;

    k=gemini.hugo();
  }
}











