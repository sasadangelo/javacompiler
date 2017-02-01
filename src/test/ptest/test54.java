/* Test 54:
 *
 * Istruzione continue. 
 */

package gimnasium;

class s1 {
  void pippo() 
   {
	double i,j;
	boolean t;
	int a;

	for (;;)
	  if (i>j)
	    continue;

	while (i>j)
	  {
	    if (t)
	      continue;
	  }

	do {
	    if (t)
	      continue;
	} while (i>j);

	continue;         // errore!!! continue deve stare in un loop.

	
	switch (a)
	{
	case 10:
	  continue;       // errore!!! continue deve stare in un loop.
	case 11:
	  continue;       // errore!!! continue deve stare in un loop.
	}

	/*
	 * L'introduzione nello switch di label costanti e' possibile solo
	 * quando il Constant Folding e' completo al 100 %
         */
  }
}







