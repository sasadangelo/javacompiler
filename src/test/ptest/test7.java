/* Test 7:
 *
 * Questo test controlla l'istallazione del nome di una classe nella symbol-
 * table e l'individuazione di id. duplicato.
 * Si tenga conto che il full name di nameclass e' packname/classname.
 */

package packname;

class classname {

}

class classname { // errore!!!

}
