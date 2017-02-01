/* Test 6:
 *
 * Questo test controlla la correttezza dell'import on demand. In effetti
 * si constrolla che vengono installati nella symbol_table tutti i nomi che
 * si trovano in path/ se abbiamo invocato:
 * 
 *                    import path.*
 *
 * controlliamo inoltre l'import automatico di java.lang.*
 */

import java.io.*; 
