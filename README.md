# Java Compiler

This is a Java Compiler I built for my College Compiler course with other two classmates in 1997, two years after Java 1.0 was released. In 1998 I continued to work alone on this compiler for my thesis, extending it with some researches on compilers. The compiler was able to take in input .java file and convert it in .class in order to be run on a JVM (basically the job done by javac program). The compiler run a lot of example programs using several Java Apis. It was also able to run several Applet in a Netscape browser (the father of Mozilla Firefox).

In 2017 I found this code on an old floppy disk in a drawer of my old desk. To avoid to lost it forever I decided to copy it on my Github profile. I tried to compile it again with modern C++ compiler but the task wasn't that easy. C++ language and compiler changed a lot in 20 years. For this reason I decided to keep it as it is for future memory. If someday I'll decide to try to make it working again and I'll be successful I'll update this page.

# Update 2/3/2017

After few hours of work I was able to get it compile again, but lot of warnings appears during compilation. The compiler run properly if you use commands like:

    ./jcc
    ./jcc -h
    ./jcc -c <class file>
    ./jcc -p <class file>

but it dump giving a .java in input. I hope that with few hours of additional work I will be it running again.


