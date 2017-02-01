/* Test 24:
 *
 * In questo test controlliamo la correttezza dell' espressione:
 *	               expr1 & expr2
 *
 */

class s1 {
  void pippo(float j) 
    {

	int     i,x,y;
	short   k;
	double  d;
	boolean b,t;

	t=b & i;       // errore!!! b e' boolean mentre i e' int.
	y=k & b;       // errore!!! b e' boolean mentre k e' short.

	x=x & y;
	x=k & y;       // Arithmetic Promotion di short a int.
	x=k & d;       // errore!!! d e' di tipo double.
		
	x=0x03&0x02;   // Il risultato e' 0x02.   

	b=true&true;   // Il risultato e' true. 
	b=false&true;  // Il risultato e' false. 
	b=true&false;  // Il risultato e' false. 
	b=false&false; // Il risultato e' false.       
	
    }
}







