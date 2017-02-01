/*
 * test 82:
 *
 * Controlleremo ora l'inizializzazione di campi o variabili locali.
 */

public class corinto
{
  int k=1;
  final int g;
  double f=1;     /* 1 viene convertito in double. */

  final int m=k;  /* errore!!! una variabile final va inizializzata con 
		   * valore costante,
		   */

  

  void medio()
  {
    double j=1;   /* 1 e' convertito in double */ 

    int k=j;      /* errore!!! j non e' assegnabile a k. */
    
  }
}


