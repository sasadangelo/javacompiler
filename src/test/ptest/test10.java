/* Test 10:
 *
 * Questo test controlla la dichiarazioni di campi in una classe.
 */

package packname;

class s1 {
  int field1, field2;
  String field3;
}

class s2 {
  float field3, field3, field4; // errore! non posso dichiarare field3 2 volte.
  java.lang.Throwable field5;
}

