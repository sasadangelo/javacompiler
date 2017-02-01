/* Test 15:
 *
 * Questo test controlla l'invio di un messaggio di errore nel caso si tenti
 * di fare l'overriding di un mettodo final o static.
 *
 * Automaticamente i metodi static, vengono settati final.
 * Automaticamente i metodi private sono final.
 *
 * Dichiarando un metodo abstract e final e' errato, pero' il compilatore non 
 * segnala l'errore finche' non si tenta di fare l'overriding.
 *
 * Dichiarando un metodo abstract e private e' errato, pero' il compilatore 
 * non segnala l'errore finche' non si tenta di fare l'overriding del metodo
 * che sara' implicitamente final.
 *
 * Controllo che un metodo non venga dichiarato abstract e static, non venga
 * dichiarato transient o volatile.
 */

package packname;

class s2 extends s1 {

  void pippo(int a, float b, double c) {}  // errore!!!
  void pluto(int a, float b, double c) {}  // errore!!!
  void minni() {}                          // errore!!! 
  void clarabella() {}                     // errore!!!
}

class s1 {

  abstract final void minni();
  final void pippo(int a, float b, double c) {}
  static void pluto(int a, float b, double c) {} 
  abstract private int clarabella();
  abstract static float genny();   // errore!!!
  transient void figaro() {}       // errore!!!
  volatile int attila() {}         // errore!!!
}








