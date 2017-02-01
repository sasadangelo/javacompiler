/* Test 55:
 *
 * Istruzione return. Implicitamente, tale test controlla anche la validita'
 * dell'algoritmo di assign conversion.
 */

package gimnasium;

class s1 { }

class s2 extends s1 { }

final class s4 { } 

final class s5 { }

class s3 {
  void pippo() 
  {
     return 10;    // errore!!! non deve essere restituito un valore.
  }

  int pluto()
  {
    return;      // errore!!! deve essere restituito un valore.
  } 

  int minni()
  {
    return 10;
  }

  int paperino()
  {
    return 10.2;  // errore!!! eventuale perdita di precisione. 
  }

  float gardena()
  {
    return 10;    // Ok!!! viene fatto un cast implicito. 
  }


  s1 orazio()
  {
    s4 a;
    return a;
    
    // errore!!! non posso assegnare valore di tipo s4 a uno di tipo s1.      
  }

  s1 clarabella()
  {
    s2 var1;
    
    return var1;
  }

  s4 paperoga()
  {
    s5 b;
    
    return b;     
    
    // errore!!! non posso assegnare valore di tipo s5 a uno di tipo s4. 
  }

  s4 nonnapapera()
  {
    s4 k1;
    
    return k1;
  }

  s4 paperone()
  {	
    s1 l;
    
    return l; 
    
    // errore!!! s1 deve essere la stessa classe di s4, dato che una e' 
    // final e l'altra no.  
  }

  int[] jenny()[]
    {
      return 10; 
      
      // errore!!! 10 e' int, mentre il metodo vuole un int[].
    } 
  
  s4 caronne()
  {
    int [] a;  
    
    return a;  
    
    // errore!!! a e' int[], mentre il metodo vuole un tipo s4.
  }

  int[] pino()[]
  {
    s4 kim;
    
    return kim; 
    
    // errore!!! kim e' di tipo s4, mentre il metodo vuole un int[].
    }

  int[] karate()[]
  {
    s1 kim;
    
    return kim;
  
    //errore!!! kim e' di tipo s1, mentre il metodo vuole un int[].
  }

  int[] kelly()
    {
	char[] p;

	return p; 

	// errore!!! p e' un array di char e non puo' essere assegnato a un 
	// array di interi. 
    }

  s1 karambeu()[]
    {	
	s2 l[];
	
      	return l; 
    }

/*
 * Resta da testare per Assign Conversion, la parte Reference e esattamente le
 * celle (0,2), (0,3), (1,2), (2,0), (2,1), (2,2), (2,3), (3,2).
 */
  	
}

















