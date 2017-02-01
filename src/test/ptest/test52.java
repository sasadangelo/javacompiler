/* Test 52:
 *
 * Istruzione for .. to .. do 
 */

package gimnasium;

class s1 {
  void pippo() 
   {
	double j;

	for (int i=10; i>10; i++) ;

	for (;;);

	for (;j;);            // errore!!! j non e' di tipo boolean.
	
	for (int k=10; k < 11; k++)
	  {
	    boolean z;
	    
	    z=true;
	  }
  }
}







