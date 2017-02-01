/* Test 60:
 *
 * Iniziamo ora il test delle istruzioni NoShortIf.
 */

package gimnasium;

class s1 {
  void pippo() 
   {
	int var1, var2;

	
	if (var1>var2) 
		if (var1<var2)
			return;
		else
			return;
	else
		return;

	if (var1>var2) 
		while (var1<var2)
			return;
	else
		return; 


	if (var1>var2) 
		for (;;)
			return;
	else
		return; 
	

   }
}







