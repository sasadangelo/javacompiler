/* Test 34:
 *
 * In questo test controlliamo la correttezza dell' espressioni:
 *	               expr1 % expr2
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

	x=10%2;        
	x=10%2.2;      
	x=10.2%2;      
	x=10.2%2.1;    
	x=var1%var2;
	x=var3%var1;
	x=var4%var3;
	x=var5%var4;   // errore !!!
   }
}







