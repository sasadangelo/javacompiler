/* Test 29:
 *
 * In questo test controlliamo la correttezza dell' espressioni:
 *	               expr1 >> expr2
 *                     expr1 << expr2
 *                     expr1 >>> expr2
 *
 */

package gimnasium;

class s1 {
  void pippo() 
   {
	double x;
	int var1,var2;
	boolean var3;
	float var4;
	short var5;

	x=10 >> 2;        // equivale a x=2.
	x=10 << 3;        // equivale a x=80.
	x=10 >>> 2;       // equivale a x=2.
	x=-10 >>> 2; 

	x=10.2 >> 3;      // errore!!!
	x=3 << 10.2;      // errore!!!

	x=var3 >>> var4;  // errore!!!

	x=var1 >> var2;

	x=var5 >> var2;
	x=var2 >> var5;
   }
}







