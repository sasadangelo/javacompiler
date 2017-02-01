/* Test 35:
 *
 * Da questo test inizia il controllo delle espressioni unarie.
 *	                  ++ expr o --expr
 */

package gimnasium;

class s1 {
  void pippo() 
   {
	double x;
	int a;
	double b;
	boolean c;
	byte d;

	x=++10;        // errore!!! si possono incrementare solo variabili
	x=++10.3;      // errore!!! si possono incrementare solo variabili
	x=++a;
	x=++b;
	x=++c;         // errore!!! c e' boolean
	x=++d;

	x=--10;        // errore!!! si possono decrementare solo variabili
	x=--10.3;      // errore!!! si possono decrementare solo variabili
	x=--a;
	x=--b;
	x=--c;         // errore!!! c e' boolean
	x=++d;
   }
}







