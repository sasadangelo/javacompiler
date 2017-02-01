/* 
 * Test 9:
 *
 * Questo test controlla l' uso di superclassi.
 */

package packname;

class men extends s1 {} // estende una classe definita piu' avanti

class s1 {}

final class s2 {}

class classname extends pippo {  // errore!!! pippo non esiste.
}

class giak extends s1 {
}

class tad extends s2 {
	// errore non posso estendere una classe final.
}

class kkkk extends Exception { }

class zzzz extends java.lang.Exception { }

class ffff extends java.util.Date { }
