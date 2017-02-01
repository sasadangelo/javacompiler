/* Test 31:
 *
 * In questo test controlliamo la correttezza dell' espressioni:
 *	               expr1 - expr2
 */

package gimnasium;

class s1 {
  void pippo() 
   {
	double x;
	int var1, var2;
	double var3;
	boolean var4;

	x=12-11;
	x=11.5-12;
	x=12-11.5;
	x=12.3-11.5;
	x=var1-var2;
	x=var3-var2;
	x=var2-var3;
	x=var2-var4;  // errore !!!
   }
}







