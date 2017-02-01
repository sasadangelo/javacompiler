/*
 * file dyn_table.cc
 * 
 * descrizione: questo file implementa tabelle dinamiche polimorfe.
 *
 * Questo file e' parte del Compilatore Java.
 * 
 * Maggio 1997, scritto by Salvatore D'Angelo, e-mail xc0261@xcom.it.
 */

#ifndef DYN_TABLE_CC
#define DYN_TABLE_CC

#include <stdio.h>
#include <dyn_table.h>
#include <fstream.h>

template<class T>
ostream& operator << (ostream& os,DYNTable<T>& dyn)
{
  for(int i=0;i< dyn.dnum_element;i++)
    os << dyn.DYNget(i);
  return os;
}

template<class T>
DYNTable<T>::DYNTable()
{
  dtable=NULL;
  dsize=0;
  dnum_element=0;
}

template<class T>
DYNTable<T>::DYNTable(unsigned dim)
{
  dtable=new T[dim];
  dsize=dim;
  dnum_element=0;
}

template<class T>   
DYNTable<T>::~DYNTable()
{
  delete dtable;
}

template<class T>   
unsigned DYNTable<T>::DYNgetdim() { return dnum_element; }

template<class T>   
void DYNTable<T>::DYNinsert(T element)
{
  T *t;
  if (dsize==0) 
    {
      dtable=new T[1];
      dsize=1;
    }
  else
    if (dnum_element==dsize) 
      {
	t=new T [dsize*2];
	for (int i=0;i < dnum_element; i++) 
	  t[i]=dtable[i];
	 
	delete dtable;
	dtable=t;
	dsize=dsize*2;
      }
  dtable[dnum_element]=element;
  dnum_element++;
}

template<class T>   
void DYNTable<T>::DYNdelete() 
  {
  T *t;
  if (num_elemet!=0) {
    if (num_element==1) {
      delete dtable;
      dsize=0;
  }
  else
    if (dnum_element==1/4*dsize) {
      t=new T [dsize/2];
      for (int i=0; i< dnum_element; i++) 
	*t[i]=*dtable[i];
      delete dtable;
      dtable=t;
      dsize=dsize/2;
    }
    dnum_element--;
  }
}

template<class T>   
T DYNTable<T>::DYNget(unsigned index)
{
  if ((index>=0) && (index <dnum_element))
    return dtable[index];
 }

template<class T>
void DYNTable<T>::DYNset(unsigned index, T element)
{
  if ((index>=0) && (index < dnum_element))
    dtable[index]=element;
 }


template <class T>
void DYNTable<T>::DYNprint()
{
  for (int i=0; i<dnum_element; i++)
    fprintf(stderr,"Numero:%d\n",dtable[i]);
}

#endif







