/* Test 32:
 *
 * In questo test controlliamo la correttezza dell' espressioni:
 *	               expr1 * expr2
 */

package gimnasium;

class s1 {
  void pippo() 
   {
	double x;
	int var1, var2;
	short var3;
	double var4;
	boolean var5;

	x=10*2;
	x=10.1*2;
	x=2*10.1;
	x=2.4*10.1;
	x=var1*var2;
	x=var3*var4;
	x=var4*var1;
	x=var5*var1;    // errore !!!
   }
}







