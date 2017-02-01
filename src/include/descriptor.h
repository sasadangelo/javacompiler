/*
 * file descriptor.h
 *
 * descrizione: in questo file oltre ad essere definita la classe Descriptor,
 *              sono riportati anche le dichiarazioni di alcuni descrittori
 *              standard.
 *
 * Questo file e' parte del Compilatore Java.
 *
 * Dicembre 1997, scritto da Salvatore D'Angelo e-mail xc0261@xcom.it.
 */

#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

#define DES_NULL      ""
#define DES_BYTE      "B"
#define DES_CHAR      "C"
#define DES_DOUBLE    "D"
#define DES_FLOAT     "F"
#define DES_INT       "I"
#define DES_LONG      "J"
#define DES_SHORT     "S"
#define DES_BOOLEAN   "Z"
#define DES_VOID      "V"

#define DES_DIM       "[" 
#define DES_LABEL     "E"

#define DES_STRING    "Ljava/lang/String;"
#define DES_THROWABLE "Ljava/lang/Throwable;"
#define DES_STRINGBUFFER "Ljava/lang/StringBuffer;";

class Descriptor : public String {

public:
  Descriptor();
  Descriptor(char*);
  Descriptor(String&);
  ~Descriptor();

  Descriptor& operator+(String&);
  Descriptor& operator+(char *);

  Descriptor& operator+=(String&);
  Descriptor& operator+=(char *);

  Descriptor& operator=(String&);
  Descriptor& operator=(char *);

  friend  ostream& operator<<(ostream&, String&);
  Descriptor& cut(int,int);

  int is_primitive();
  int is_link();
  int is_array();
  int is_reference();
  int is_integral();
  int is_floating();
  int is_16();
  int is_32();

  String&     to_fullname();
  String&     to_singlename();
  String&     to_packagename();
  Descriptor& to_signature();
  char       *to_typename();

  Descriptor& array_comp_type();
  Descriptor& return_type();

  void build_link(String&);
  void build_link(char *);
  void build_array();
  void build_signature();
  void build_method(Descriptor&,Descriptor&);
  
  /*
   * Spero in futuro di riuscire a fare a meno dei seguenti due metodi.
   */

  int  num_args();
  Descriptor& get_arg(int);
}; 

#endif
