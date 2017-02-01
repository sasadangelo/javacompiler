/* Test 30:
 *
 * In questo test controlliamo la correttezza dell' espressioni:
 *	               expr1 + expr2
 */

package gimnasium;

class s1 {
  void pippo() 
   {
	double x;
	String y;
	int    var1, var2;
	double var3, var4;
	short  var5;

	/*
	 * Attenzione!!!
	 *
	 * in futuro cercare di fare l'assegnazione delle varie stringhe
	 * a una variabile di tipo String anziche' a x.
	 */

	x=10+11;
	x=10+11.1;
	x=10.1+11;
	x='c'+11;
	x=10.1+10.2;
	x=var1+var2;
	x=var3+var4;
	x=var1+var5;
	x=var5+var3;
	x=true+false;          // errore !!! 
	y="pippo"+"pluto";
	y="pippo"+10;
	y=10+"pippo";
        y="pippo"+10.3;		
	y=10.3+"pippo";
	y="pluto"+'s';
	y='s'+"pluto";
	y="vomero"+null;
	y=null+"vomero";
	y="napoli"+true;
	y="napoli"+false;
	y=true+"napoli";
	y=false+"napoli";
	y="toyota"+var3;
	y=var3+"toyota";
   }
}







