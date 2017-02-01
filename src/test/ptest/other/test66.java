/* Test 77:
 *
 * Inizieremo ora l'analisi del comportamento del compilatore sui costruttori.
 */


/*
 * JVM da un segmantation fault.
 */
	 	
package gimnasium;

class pluto {
}

class minni extends pluto {

minni() {}
minni(int i, float k) {}
cane() {}                /* 
			  * errore!!! il costruttore non ha lo stesso nome
			  * della classe. Viene installato come metodo con 
			  * un valore di ritorno di default, in realta' dovremo
			  * introdurre un descrittore Jolly da adattare poi 
			  * in tutti i contesti.
			  */

minni() {}               /* errore!!! duplicazione costruttore. */

minni(int k)
  {
    this();
  }

minni(double k)
  {
    this(10,10);     /* il secondo parametro e' convertito a float. */
  }

minni(double k, double m)
  {
    this("chiamata errata");
  }

minni(boolean k, double m)
  {
    super();               /* 
			    * per ora posso usare super in un costruttore,
			    * solo se effettivamente la classe corrente ha
			    * una superclasse.
			    */
  }

}







