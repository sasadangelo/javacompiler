/* Test 28:
 *
 * In questo test controlliamo la correttezza dell' espressione:
 *	               expr1 > expr2
 *                     expr1 >= expr2
 *                     expr1 < expr2
 *                     expr1 <= expr2
 *
 */

package gimnasium;

class s1 {
  void pippo() 
   {
	double var0,var1;
	short var2;
	boolean var3;
	gloria var5;

	if (var0 > var1);
	if (var0 < var1);
	if (var0 >= var1);
	if (var0 <= var1);

	/*
         * Controllo l' Arithmetic Promotion.
         */

	if (var0 > var2);
	if (var0 < var2);
	if (var2 >= var0);
	if (var2 <= var0);

	if (10 >  11);     // Il risultato e' false.
	if (10 <  11);     // Il risultato e' true.
	if (10 >= 11);     // Il risultato e' false.
	if (10 <= 11);     // Il risultato e' true.

	if (10.5 >  11.0); // Il risultato e' false.
	if (10.8 <  11.0); // Il risultato e' true.
	if (10.3 >= 10.3); // Il risultato e' true.
	if (10 <= 11);     // Il risultato e' true.

  	if (10.5 >  11);   // Il risultato e' false.
	if (11.0 <  11);   // Il risultato e' false.
	if (11 >= 10.3);   // Il risultato e' true.
	if (10.0 <= 10);   // Il risultato e' true.

 	if (var0 < var3);  // errore!!!	

	if (var5 >= var5); // errore!!!
    }
}







