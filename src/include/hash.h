/*
 * file hash.h
 *
 * descrizione: Questo file contiene la definizione delle strutture necessarie
 *              alla implementazione di una completa Hash Table.
 *
 * Questo file e' parte del Compilatore Java.
 *
 * Maggio 1997, scritto da Michele Risi e-mail micris@zoo.diaedu.unisa.it
 */

#ifndef HASH_H
#define HASH_H

#define NUM_BUCKET 20

class HTNode {

protected:

  String name;
  HTNode *next;
  HTNode *prev;

public:

  HTNode();
  HTNode(String&);
  virtual ~HTNode();
  String& getname();
  void  setname(String&);
  HTNode *getnext();
  void  setnext(HTNode *);
  HTNode *getprev();
  void  setprev(HTNode *);
  virtual void  PrintNode(FILE *);
};

/* 
 * This is the definition of class HTList which rapresents the list of
 * on the buckets.
 */

class HTList
{
private:
  HTNode *Headlist;

public:

  HTList();
  ~HTList();
  HTNode *GetHeadlist();
  void    SetHeadlist(HTNode *);
  HTNode *Find(String&);
  void    Insert(HTNode *);
  void    Discard(HTNode *);
  void    Delete(HTNode *);
  virtual void PrintList(FILE *);
};

/* 
 * This is the definition of Hash Table class. The Symbol-Table is implements 
 * by an hash table.
 */

class Hashtable
{
protected:
  static int num_bucket;
  HTList *HT;
  int Hash(String&);
  HTList *basket;

public:

  Hashtable();
  Hashtable(int);
  virtual ~Hashtable();
  HTNode *Install_Id(String&);
  void Discard(HTNode*);
  void Recover(HTNode*);
  void Delete(HTNode *);
  HTNode* LookUpHere(String&);
  HTNode* NextSym(HTNode *);
  virtual void PrintTable(FILE *);
};

#endif
