/* Test 27:
 *
 * In questo test controlliamo la correttezza dell' espressione:
 *	               expr1 != expr2
 *
 */

package gimnasium;

class estesa {}

final class k2 extends estesa {}

final class k1 {}

class gloria extends estesa {}

class s1 {
  void pippo() 
   {
	double var1;
	short var2;
	boolean var3,var4;
	gloria var5,var6;
	estesa var7;
	k2 var8,var9;
	k1 var10;
	int[] var11,var12;
	int[][] var13,var14;
	gloria[] var15,var16;
	estesa[] var17;
	gloria var18[],var19[];

	if (10!=11);       /* l'expr vale true. */
	if (10!=10);       /* l'expr vale false.  */
	if (10!=11.3);     /* l'expr vale true. */
	if (10.8!=11);     /* l'expr vale true. */
	if (10!=10.0);     /* l'expr vale false.  */
	if (10.6!=10.6);   /* l'expr vale false.  */	

	if (var1!=var2);   /* Arithmetic Promotion su var2. */

	if (true!=false);  /* l'expr vale true. */	
	if (false!=false); /* l'expr vale false.  */	

	if (var3!=var4);
	if (var5!=var4);   /* 
			    * errore!!! var5 e' reference type, mentre var4 e' 
                            * bool.
		            */
	
	/*
	 * Testiamo ora anche l'algoritmo di Cast Conversion su reference.
	 */

	if (var5!=var6);
	if (var5!=var7);
	if (var7!=var5);/* 
			 * Attenzione!!! qui e' il tipo di var5 a essere con-
	                 * vertito a quello di var7. Avviene ua restrizione di 
	                 * classe.
			 */
	if (var8!=var7);
	if (var7!=var8);
	if (var8!=var10);/* errore!!! essendo var8 e var10 di tipo classe 
                          * final, allora dovrebbero essere entrambi della 
			  * stessa classe.
			  */
	if (var8!=var9);
	if (var11!=var12);
	if (var13!=var14);
        if (var15!=var16);
	if (var15!=var17);
	if (var17!=var15);
	if (var18!=var19);
	if (var15!=var18); /* 
		            * Controllo che la dichiarazione di array del
		            * tipo:
                            *            int a[];
	                    * sia equivalente a:
                            *            int[] a;
                            */
	if (var17!=var15);

/*
 * Attenzione, per ora non posso completare il test dell'algoritmo di assign 
 * conversion, perche' non ho ancora implementato le interfacce.
 * Nella tabella su appunti, pag. 64 mancano da testare le seguenti celle:
 * (2,0),(3,0),(2,1),(3,1),(0,2),(1,2),(2,2),(3,0),(3,1),(3,2),(0,3),(1,3),
 * (2,3).
 * Le celle (3,0) e (0,3) saranno testabili quando sara' disponibile l'algo-
 * ritmo di LOADCLASS, perche' serve Object.
 */

    }
}







