/* Test 37:
 *
 * Il test controlla la correttenza dei cast:
 *	                  ( cast ) expr
 *
 * Magari questo file ha bisogno di test un po' piu' complessi.
 */

package gimnasium;
class s3 {}

class s2 extends s3 {}

class s1 {
  void pippo() 
   {
	double x;
	float a;
	s3 pp;
	s2 tt;

	x=(int) 10;	
	x=(short) 10;	
	x=(short) a;	
	x=(boolean) a;  // errore!!! non posso convertire un float a boolean.

	tt=(s2)pp;

   }
}
