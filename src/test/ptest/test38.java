/* Test 38:
 *
 * Il test controlla le espressioni unarie:
 *	                  ! expr o ~expr 
 */

package gimnasium;

class s1 {
  void pippo() 
   {
	int a,b;
	boolean p;
	double c;

	if (!(10>11));
	if (!(a>b));
	if (!p);
	if (!a);       // errore !!! a non e' di tipo boolean.
	if (!false);
	if (!true);

	if (a>~10);
	if (b<~10.2);  // errore!!! e' richiesto un cast esplicito per 10.2.
	if (~a>b); 
   }
}







