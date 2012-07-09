#
# @(#)Makefile	1.1 97/03/27
#
# Makefile for win32 jre demo.
#

SRC=..\..\src
CP=classes
WIN=..\..\src\win32
# point this at a *stable* JDK
JDK=g:\jdk1.1.6


all: jcifssh jcifssh_g

.SUFFIXES: .java .class

classdir:
	@if not exist classes mkdir classes


jar:
	cd classes
	$(JDK)\bin\jar.exe cvfM classes.jar .
	cd ..
	mv classes\classes.jar lib





jcifssh: jcifssh.c
	cl -I$(JDK)\include -I$(JDK)\include\win32 -MD \
	-DDEBUG -DJAVA_ARGS="{\"-ms6m\", \"org.gnu.jcifs.shell.Main\"}" \
	 $(JDK)\lib\javai.lib setargv.obj jcifssh.c -Fejcifssh.exe


clobber :: FORCE
	deltree classes


FORCE :



