/**
  *         Commmon Internet File System Java API (JCIFS)
  *----------------------------------------------------------------
  *  Copyright (C) 1999  Norbert Hranitzky
  *
  *  This program is free software; you can redistribute it and/or
  *  modify it under the terms of the GNU General Public License as
  *  published by the Free Software Foundation; either version 2 of
  *  the License, or (at your option) any later version.

  *  This program is distributed in the hope that it will be useful,
  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  *  General Public License for more details.
  *
  *  You should have received a copy of the GNU General Public License
  *  along with this program; if not, write to the Free Software
  *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  *
  *  The full copyright text: http://www.gnu.org/copyleft/gpl.html
  *
  *----------------------------------------------------------------
  *  Author: Norbert Hranitzky
  *  Email : norbert.hranitzky@mchp.siemens.de
  *  Web   : http://members.tripod.de/Hranitzky
  */


#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys_api.h>

#ifdef JAVA_ARGS
#define NUM_ARGS (sizeof(java_args) / sizeof(char *))
char *java_args[] = JAVA_ARGS;
#else
#define NUM_ARGS 0
#endif

#ifdef DEBUG
// #define JAVA "java_g"
#define MY_JAR  "\\jcifs_g.jar"
#define JAVA "java"
#else
#define JAVA "java"
#define MY_JAR  "\\jcifs.jar"
#endif

#ifdef WINMAIN
#define main _main
__declspec(dllimport) char **__initenv;
#endif





void  setClassPath() {
    char loaddir[MAX_PATH], *cp;
    char *cl = getenv("CLASSPATH");

    GetModuleFileName(NULL, loaddir, MAX_PATH);
    *(strrchr(loaddir, '\\')) = '\0';

    cp = (char *)malloc(5 * strlen(loaddir) + 200);
    sprintf(cp,"%s=%s%s;%s","CLASSPATH",loaddir,MY_JAR,cl);
    /*
    sprintf(cp, "%s=%s\\..\\lib\\classes.jar;%s\\..\\lib\\rt.jar;"
	    "%s\\classes;%s\\..\\lib\\i18n.jar"
	    "%s\\..\\lib\\classes.zip",
	    "CLASSPATH", loaddir, loaddir, loaddir, loaddir, loaddir);
	*/
    putenv(cp);

    putenv("JAVA_HOME=");
}

int
main(int argc, char *argv[])
{
    char **newargv, **ap;
    int i;

    setClassPath();

    newargv = ap = (char **) calloc(argc + NUM_ARGS + 1, sizeof(char *));

    *ap++ = JAVA;

#ifdef JAVA_ARGS
    for (i = 0; i < NUM_ARGS; i++) {
	*ap++ = java_args[i];
    }
#endif

    for (i = 1; i < argc; i++) {
	*ap++ = argv[i];
    }

    return java_main(argc + NUM_ARGS, newargv);
}

#ifdef WINMAIN

int WINAPI
WinMain(HINSTANCE inst, HINSTANCE previnst, LPSTR cmdline, int cmdshow)
{
    int mainret;

    __initenv = _environ;
    mainret = main(__argc, __argv);
    sysExit(mainret);
}

#endif
