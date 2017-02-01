/*
 * file environment.h
 *
 * descrizione: questo file definisce la classe Environment, che mantiene
 *              memorizzato il contesto attuale di esecuzione.
 *
 * Questo file e' parte del Compilatore Java.
 *
 * Maggio 1997, scritto da Salvatore D'Angelo e-mail xc0261@xcom.it.
 */

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <../stack.cc>

#define STACK_ENV_SIZE 500

#define IN_CLASS       0x0001
#define IN_INTERFACE   0x0002
#define IN_METHOD      0x0004
#define IN_CONSTRUCTOR 0x0008
#define IN_SWITCH      0x0010
#define IN_FOR         0x0020
#define IN_WHILE       0x0040
#define IN_DOWHILE     0x0080
#define IN_TRY         0x0100
#define IN_PARAM       0x0200
#define IN_STATIC      0x0400

#define IN_LOOP        IN_FOR | IN_WHILE | IN_DOWHILE

class Environment
{
private:

    Stack<int> *s;
    String class_path;

public:

    Environment();
    ~Environment();
    String& GetClassPath();
    void SetClassPath(String&);
    int  is_inContext(int);
    void openContext(int);
    void closeContext();
 };

#endif


