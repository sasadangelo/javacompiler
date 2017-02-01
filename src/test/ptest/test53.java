/* Test 53:
 *
 * Istruzione break. 
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
	    break;

	while (i>j)
	  {
	    if (t)
	      break;
	  } 

	do {
	    if (t)
	      break;
	} while (i>j);

	break;         // errore!!! break deve stare o in uno switch o in un 
	               // loop.

	switch(a)
	{
	case 10:
		break;
	case 11:
		break;
	}

	/*
	 * L'introduzione nello switch di label costanti e' possibile solo
	 * quando il Constant Folding e' completo al 100 %
         */
  }
}







