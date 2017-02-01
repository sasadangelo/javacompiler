/*
 * Test 84:
 *
 * Data la disponibilita' delle procedure di deallocazione delle variabili
 * locali, testeremo ora il Nesting di variabili locali. 
 */


class Kanu {}

public class corinto
{

  void proc()
  {	
	int i=1;

	{
	  float i=1;
	  int k=10;

	  {
		Kanu k;  /*
			  * Variabile locale duplicata, k del blocco precedente
	 		  * e' a livello 5 cosi' come questa.
			  */
		int m;
	  }
	
	  {
		float m=1.1; /*
			      * Ok. m del blocco precedente e' ormai scartata
			      * dalla symbol-table.
			      */

		for (m=1; m!=0; m--); /*
				       * Ok. il for usa m dichiarata in 
				       * questo blocco.
				       */

		for (int m=1; m==0; m++); /*
				           * errore!!! m duplicata. 
					   */
	  }

	}	
  }	
}





