/* Test 22:
 *
 * In questo test controlliamo la correttezza dell' espressione:
 *	               expr1 && expr2
 *
 */

class s1 {
  void pippo(float j) 
    {

	int i;
	boolean b,k;

	if (i && true) ;  // errore!!! i non e' boolean.
	if (false && j);  // errore!!! j non e' boolean. 
	if (true && b);   // il risultato di expr e' pari a quello di b.
	if (false && b);  // shortcut. Il risultato e' false.

	/*
	 * Il risultato non e' direttamente false, anche se si intuisce che lo
	 * sara', infatti, l'espressione b dovra' essere calcolata.
	 */

	if (b && false); 
	if (b && true);   // Il risultato e' ovviamente b.
	if (b && k);
    }
}




