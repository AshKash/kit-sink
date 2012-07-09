/**
  *         Commmon Internet File System Java API (JCIFS)
  *----------------------------------------------------------------
  *  Copyright (C) 1999  Norbert Hranitzky
  *
  *  This program is free software; you can redistribute it and/or
  *  modify it under the terms of the GNU General Public License as
  *  published by the Free Software Foundation; either version 2 of
  *  the License, or (at your option) any later version.
  *
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
  *  Web   : http://www.hranitzky.purespace.de
  */


package org.gnu.jcifs.util;

import java.io.*;
import java.util.Date;
import java.awt.*;
import java.awt.event.*;


/**
 * To enable the debug version, set the field debugOn to true and compile the
 * sources again
 */
public class Debug  {

    public final static int            NONE     = 0;
    public final static int            ERROR    = 1;
    public final static int            WARNING  = 2;
    public final static int            INFO     = 3;
    public final static int            BUFFER   = 4;
    public final static int            METHOD   = 5;

    public final static boolean        assertOn   = true;


	public final static boolean        debugOn    = false;
    public static int                  debugLevel = NONE;

    private static PrintWriter         fStdOutput = new PrintWriter(System.err,true);
    private static PrintWriter         fOutput    = fStdOutput;


    private final static String        NL = System.getProperty("line.separator");

    private final static char          CHARS[] ={ '0','1','2','3','4','5','6','7',
                                                  '8','9','A','B','C','D','E','F'};

    public static void setDebugLevel(int level) {
        debugLevel = level;
    }




    public static synchronized void setDebugFile(String fname)  {

        close();

        if(fname != null) {

            try {
                FileWriter fw = new FileWriter(fname, false);
                fOutput       = new PrintWriter(fw, true);
                fOutput.println(" **** JCIFS debug output ***");
                fOutput.println("Open timestamp=" + (new Date().toString()));
            } catch(IOException e) {
                System.err.println("Cannot open Debug-File:" + fname);
                e.printStackTrace();
            }

        }
    }

    public static void close()  {
        if (fOutput != fStdOutput) {
            fOutput.flush();
            fOutput.close();
            fOutput = fStdOutput;
        }
    }
    public static final void method() {
        if (debugOn && debugLevel >= METHOD)
            fOutput.println("[method]:" + getTracedMethod());

    }

    public static final void println(int level,byte[] data,int off, int len) {
        if (debugOn && debugLevel >= level) {
            fOutput.println(dumpBytes(data,off,len));
            fOutput.flush();
        }
    }

    public static final void print(int level,String data) {
        if (debugOn && debugLevel >= level) {
            fOutput.print(data);
            fOutput.flush();
        }
    }
    public static final void println(int level,String data) {
        if (debugOn && debugLevel >= level) {
            fOutput.println(data);
            fOutput.flush();
        }
    }

    public static final void assert(boolean expr) {
        if (assertOn) {
            if (!expr) {
                fOutput.println("[assert]: Assertion failed:" + getTracedMethod());
                System.err.println("Assertion failed:" + getTracedMethod());
                System.exit(1);
            }
        }
    }



    private static String getTracedMethod()  {
        // new Throwable().printStackTrace();

        try {
            ByteArrayOutputStream bout = new ByteArrayOutputStream();
            PrintStream           pout = new PrintStream(bout);
            new Throwable().printStackTrace(pout);

            BufferedReader        bin  = new BufferedReader(new StringReader(bout.toString()));

            // skip "java.lang.Throwable"
            bin.readLine();

            // skip current method
            String s = bin.readLine();

            if (s != null) {
                if(s.trim().equals("at java.lang.Throwable.<init>(Compiled Code)"))
                     // skip current method
                    bin.readLine();

                 // skip caller of "getStackTrace"
                bin.readLine();
                return bin.readLine().trim().substring(3);
            }

        } catch(IOException e) {


        }
        return "<no stack trace>";
    }


    public static String dumpBytes(byte[] b, int off, int len)  {


        StringBuffer buf = new StringBuffer();
        StringBuffer hex = new StringBuffer();
        StringBuffer chr = new StringBuffer();

        int p;
        char c;
        int n = 0;
        for(int i=0;i<len;i++) {

            p = b[off+i] & 0xff;
            if (p >= 20 && p <= 125)
                c = (char)p;
            else
                c = '.';

            int x = ((p & 0xf0) >> 4) & 0x0f;
            hex.append(CHARS[x]);

            x = p & 0x0f;
            hex.append(CHARS[x] + " ");

            chr.append(c);

            if( n++ == 15) {
                n = 0;
                buf.append(hex.toString() + "    " + chr.toString() + NL);
                hex.setLength(0);
                chr.setLength(0);
            }
        }

        if (n < 15) {
            for(;n<16;n++) {
                hex.append("   " );
                chr.append(' ');
            }

        }
        buf.append(hex.toString() + "    " + chr.toString());
        return buf.toString();

    }

}