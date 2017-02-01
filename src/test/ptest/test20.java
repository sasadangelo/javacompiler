/* Test 20:
 *
 * In questo test controlliamo la correttezza dell' espressione:
 *	               CondOrExpr ? Expr : CondExpr
 *
 */

class s3 extends s2 {}

class s1 {
  void pippo() 
    {
	double x;
	boolean mh;
	int   i, a, b;
	float j;
	s2 pp;
	s2 kk;
	s3 zz;
	
	/* 
	 * Controllo algoritmo BArithmetic_Promotion().
	 *
	 * 1 - byte, char, short sono sempre convertiti a int.
	 * 2 - dato il seguente ordine prioritario
	 *      
         *       double > float > long > int 
	 *     
         *     dati due descrittori primitive type, il risultato avra' per
	 *     descrittore quello con priorita' maggiore. 
	 */

	x=a > b ? i : j;   	
	
	/*
	 * Controllo l'ottimizzazione da fare nel caso in cui e1 sia TRUE o 
	 * FALSE.
	 */

	x=true ? 'c' : 'd';
	x=true  ? 10 : 20;
	x=false ? 10 : 20;

	/*
	 * Ci sono 4 casi di cui tener conto:
	 *
	 * 1) e2 - e3 sono entrambi primitive type.
	 * 2) e2 - e3 sono entrambi boolean.
	 * 3) e2 - e3 sono entrambi reference type.
	 * 4) nessuno dei punti 1) 2) e 3) si verifica.
	 */ 

	/*
	 * caso 1.
	 */

	short k;	
	byte r;
	
	i=a>b ? 10 : 11;
	i=a>b ? k  : r;
	i=a>b ? r  : k;

	/*
	 * progettualmente e' stato tralasciato il seguente caso:
	 * 
	 * se e2 e' int e e3 e' short, byte, char con e2 rappresentabile in 
	 * tale formato, allora viene eseguito un cast narrow su e2.
	 */

	x=a>b ? i : r;

	/*
	 * Caso 2)
	 */
  
	mh=a>b ? mh : mh; 

	/*
	 * Caso 3)
	 */

	pp=a>b ? null : null;     

	pp=a>b ? null : kk;        /*
				    * non si fanno mai cast su null, non 
				    * avrebbe senso.
				    */
  	pp=a>b ? kk : null;
	
      	pp=a>b ? zz : kk;

	pp=a>b ? zz : zz; 

	zz=a>b ? pp : kk;      /*
			        * errore!!! non posso convertire s2 in s3
				* mediante assign conversion.
			        */
		
    }
}

class s2 {}


