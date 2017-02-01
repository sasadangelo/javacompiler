/* Test 1:
 *
 * Questo test in realta' e' un test lessicale che controlla gli errori che
 * si verificano quando una NEW LINE compare in una stringa.
 * Qui il parser va a buon fine, perche' la stringa diventa:
 *
 *                      "new line in string"
 *
 * corretta ovviamente dallo scanner.
 */

package pippo;

class pluto {
  static public void main() {
    java.lang.String p;
    p="new line in string
;

    }
}












