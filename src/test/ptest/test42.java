/* Test 42:
 *
 * Il test controlla l' espressione di assegnamento normale.
 */

package gimnasium;

class s2 
{
  int k;
}

class agamennone {

	void vergigetorige()
	{	
		int x;
		int a;
		double b;
		short z[];
		byte n;
		s2 gigi,pippo;

		x=10;
		x=a;
	        x=b;           // errore!!! non posso convertire b a x.
		x=z[10];
		z[11]=n;
		gigi.k=n;
		s2.k=10;       /* 
			        * errore!!! faccio un rif. static a una var. 
				* non static.
 			        */

		gigi=n;   /*
			   * errore!!! n e' primitive type, mentre gigi e' 
			   * reference type.
			   */

		n=gigi;	  /*
			   * errore!!! n e' primitive type, mentre gigi e' 
			   * reference type.
			   */

		gigi=pippo;
	}

}







