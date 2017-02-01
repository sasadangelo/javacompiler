/* Test 59:
 *
 * Istruzione switch.
 */

package newpack;

class ravanelli {
	void vialli()
          {
		int a;
		long b;

/*
 * Nei case mettiamo per ora solo nodi costanti, perche' non calcoliamo
 * ancora a tempo di compilazione espressioni contenenti identificatori
 * final con valore costante.
 */

/*
 * Altra cosa per ora non gestita e' controllare che in uno switch tutte le
 * espressioni nei case siano assegnabile all'espressione dello stesso, secondo
 * l'algoritmo di assign conversion.
 */

/*		switch(a)
		{
		}
*/


	/*	switch(b)*/  /* errore!!! l'espressione deve essere un integral
			       type ma non long. */    
/*		{
		}	    



		switch(a)
		{
		case 10:
		  if (true)
			return;
		case 11:
		  break;			
		}	



		switch(a)
		{
		case 10:case 11:
		  if (true)
			return;
		}



		switch(a)
		{
		case 10:
		  if (true)
			return;
		case 11:
		  break;
		case 10:   */      /* errore!!! etichetta duplicata. */
/*			return;			
		}	



		switch(a)
		{
		case 10:case 11:case 10:*/ /* errore!!! etichetta duplicata. */
/*		  if (true)
			return;
		}



		switch(a)
		{
		case 10:
			return;
		default:
			break;
		}



		switch(a)
		{
		case 10:
			return;
		default:
			break;
		case 11:
			if (true)
				return;
		default: */                /* errore!!! default duplicato. */
	/*		return;
		}


*/
		switch(a)
		{
		case 10:case 13:
		  if (true)
			return;
		case 12:
			return;
		case 13:                 /* errore!!! etichetta duplicata. */
			return;
		}


          }	
}















