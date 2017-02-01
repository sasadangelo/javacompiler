/* file access.h
 * 
 * descrizione: definizione degli accessi nel compilatore.
 *
 * Questo file e' parte del Compilatore Java.
 * 
 * Maggio 1997, scritto da Salvatore D'Angelo e-mail xc0261@xcom.it.
 */

#ifndef ACCESS_H
#define ACCESS_H

#define ACC_PUBLIC        0x0001
#define ACC_PRIVATE       0x0002
#define ACC_PROTECTED     0x0004
#define ACC_STATIC        0x0008
#define ACC_FINAL         0x0010
#define ACC_SYNCHRONIZED  0x0020
#define ACC_SUPER               /* 0x0020 */
#define ACC_VOLATILE      0x0040
#define ACC_TRANSIENT     0x0080
#define ACC_NATIVE        0x0100
#define ACC_INTERFACE     0x0200
#define ACC_ABSTRACT      0x0400

#define IS_PUBLIC(m)       ((m & ACC_PUBLIC)==ACC_PUBLIC)
#define IS_PRIVATE(m)      ((m & ACC_PRIVATE)==ACC_PRIVATE)
#define IS_PROTECTED(m)    ((m & ACC_PROTECTED)==ACC_PROTECTED)
#define IS_STATIC(m)       ((m & ACC_STATIC)==ACC_STATIC)
#define IS_FINAL(m)        ((m & ACC_FINAL)==ACC_FINAL)
#define IS_SYNCHRONIZED(m) ((m & ACC_SYNCHRONIZED)==ACC_SYNCHRONIZED)
#define IS_SUPER(m)        ((m & ACC_SUPER)==ACC_SUPER)
#define IS_VOLATILE(m)     ((m & ACC_VOLATILE)==ACC_VOLATILE)
#define IS_TRANSIENT(m)    ((m & ACC_TRANSIENT)==ACC_TRANSIENT)
#define IS_NATIVE(m)       ((m & ACC_NATIVE)==ACC_NATIVE)
#define IS_INTERFACE(m)    ((m & ACC_INTERFACE)==ACC_INTERFACE)
#define IS_ABSTRACT(m)     ((m & ACC_ABSTRACT)==ACC_ABSTRACT)
#define IS_DEFAULT(m)      ((m & (ACC_PUBLIC|ACC_PRIVATE|ACC_PROTECTED))==0)

#endif
