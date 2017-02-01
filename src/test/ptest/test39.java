/* Test 39:
 *
 * Il test controlla le espressioni unarie:
 *	                  +expr o -expr 
 */

package gimnasium;

class s1 {
  void pippo() 
   {
     double x;
     int a,b;
     boolean p;
     short f;
     s1 z;

     x=+10;
     x=+10.2;
     x=+p;      // errore di compilazione, p e' di tipo boolean.
     x=+f;      
     p=+z;      // errore di compilazione, z non e' di tipo numerico.

     x=-10;
     x=-10.2;
     x=-p;      // errore di compilazione, p e' di tipo boolean.
     x=-f;      
     p=-z;      // errore di compilazione, z non e' di tipo numerico.
   }
}







