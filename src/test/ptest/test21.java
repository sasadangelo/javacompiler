/* Test 21:
 *
 * In questo test controlliamo la correttezza dell' espressione:
 *	               expr1 || expr2
 *
 */

class s1 {
  void pippo(float j) 
    {
	int i;
	boolean b,k;
	
	if (i || true);    // errore!!! i non e' boolean.
	if (false || j);   // errore!!! j non e' boolean.
	if (true || b);    // shortcircuits. Il risultato e' true.    
	if (false || b);   // il risultato di expr e' pari a quello di b. 

	/*
	 * Il risultato non e' direttamente true, anche se si intuisce che lo
	 * sara', infatti, l'espressione b dovra' essere calcolata.
	 */

	if (b || true); 
	if (b || false);   // Il risultato e' ovviamente b.
	if (b || k);       // costruzione albero.
    }
}
