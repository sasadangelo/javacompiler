/* Test 23:
 *
 * In questo test controlliamo la correttezza dell' espressione:
 *	               expr1 | expr2
 *
 */

class s1 {
  void pippo(float j) 
    {

	int     i,x,y;
	short   k;
	double  d;
	boolean b,t;


	x=b | i;       // errore!!! b e' boolean mentre i e' int.
	y=k | b;       // errore!!! b e' boolean mentre k e' short.

	x=x | y;
	x=k | y;       // Arithmetic Promotion di short a int.
	x=k | d;       // errore!!! d e' di tipo double.
		
	x=0x01|0x02;   // Il risultato e' 0x03.   

	b=true|true;   // il risultato e' true. 
	b=false|true;  // Il risultato e' true. 
	b=true|false;  // Il risultato e' true. 
	b=false|false; // Il risultato e' false.
    }
}







