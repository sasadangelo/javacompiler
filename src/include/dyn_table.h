/*
 * file dyn_table.h
 * 
 * descrizione: Questo file definisce tabelle dinamiche usate nel compilatore
 *              per implementare: constant pool, contenitore di bytecode, ecc.
 *
 * Questo file e' parte del Compilatore Java.
 * 
 * Maggio 1997, scritto da Salvatore D'angelo, e-mail xc0261@xcom.it.
 */

#ifndef DYN_TABLE_H
#define DYN_TABLE_H

#include <stdio.h>
#include <fstream>
using namespace std;

template<class T> class DYNTable {

private:
    T *dtable;
    unsigned dnum_element;
    unsigned dsize;
    
public:
  
    DYNTable();
    DYNTable(unsigned int);
    ~DYNTable();
    unsigned DYNgetdim(); 
    unsigned DYNbegin();
    void DYNinsert(T);
    void DYNdelete();
    void DYNprint();
    T    DYNget(unsigned);
    void DYNset(unsigned,T); 
    friend ostream& operator << (ostream&,DYNTable<T>&);
};

#endif




