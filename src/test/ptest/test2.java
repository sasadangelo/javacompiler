/* Test 1:
 *
 * Questo test in realta' e' un test lessicale che controlla gli errori che
 * si verificano quando un EOF compare in una stringa.
 * Qui il parser fallisce, perche' la stringa pur diventando:
 *
 *                      "new line in string"
 *
 * non e' seguita da niente.
 */

package pippo;

class pluto {
    static public void main() {
        java.lang.String p;
        
        p="eof in string





