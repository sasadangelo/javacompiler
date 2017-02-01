/*
 * file backpatch.h
 *
 * descrizione: questo file implementa la classe BPList (BackPatch List) ossia
 *              una lista necessaria per gestire le espressioni booleane e
 *              istruzioni come break e continue.
 *              Una lista di backpatch e' una lista contenente indirizzi di
 *              istruzioni JVM che vanno completate con l'indice di jump a una
 *              istruzione. Per maggiori dettagli sulle liste di backpatching,
 *              vedi documentazione allegata e Aho-Ullman (Principles, Techni-
 *              ques and tools pag. 500).
 *
 * Questo file e' parte del Compilatore Java.
 *
 * Marzo 1998, scritto da Salvatore D'Angelo, e-mail xc0261@xcom.it.
 */

#ifndef BACKPATCH_H
#define BACKPATCH_H

/*
 * Definizione di un nodo della lista di backpatching.
 */

class BPNode {

protected:

  _u4    address;
  BPNode *next;
  BPNode *prev;

public:

  BPNode(_u4);
  ~BPNode();
  _u4 getaddress();

  BPNode *getnext();
  void    setnext(BPNode *);
  BPNode *getprev();
  void    setprev(BPNode *);

  void  PrintNode(FILE *);

};

/*
 * Definizione lista di backpatching.
 */

class BPList
{

private:

  BPNode *Headlist;

public:

  BPList();
  ~BPList();

  BPNode *GetHeadlist();
  void    SetHeadlist(BPNode *);

  void    Add(_u4);
  void    Merge(BPList *, BPList *);

  void PrintBPList(FILE *);
};

#endif
