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


package org.gnu.jcifs;

import org.gnu.jcifs.util.*;
import java.io.*;
import java.util.*;


class MarshalBuffer {

    protected byte[] fBuffer;

    protected int    fSize;

    private final static int MASK  = 0xFF;

    public  final static String ISO8859_1 = "8859_1";


    MarshalBuffer(int capacity) {
        fBuffer = new byte[capacity];
    }

    int getCapacity() {
        return fBuffer.length;
    }

    int getSize() {
        return fSize;
    }

    void setSize(int size) {
        fSize = size;
    }

    byte[] getBytes() {
        return fBuffer;
    }

    void setCapacity(int size) {
        if (size > fBuffer.length) {
            byte[] nbuf = new byte[size];

            fBuffer = nbuf;
        }
    }

    void zero(int len) {
        if (len > fBuffer.length)
            len = fBuffer.length;

        for(int i=0;i<len;i++)
            fBuffer[i] = 0;
    }

    public  int setIntAt(int pos, int val) {


        // use little-endian encoding (Intel)
        fBuffer[pos]   = (byte)(val       & MASK);
        fBuffer[pos+1] = (byte)((val >>  8) & MASK);
        fBuffer[pos+2] = (byte)((val >> 16) & MASK);
        fBuffer[pos+3] = (byte)((val >> 24) & MASK);

        return 4;
    }

    public static int setIntAt(int pos, byte[] buf,int val) {


        // use little-endian encoding (Intel)
        buf[pos]   = (byte)(val       & MASK);
        buf[pos+1] = (byte)((val >>  8) & MASK);
        buf[pos+2] = (byte)((val >> 16) & MASK);
        buf[pos+3] = (byte)((val >> 24) & MASK);

        return 4;
    }

    public int getIntAt( int pos) {

        if (pos + 4 > fBuffer.length )
            throw new ArrayIndexOutOfBoundsException();



        return ((fBuffer[pos]   & MASK)        +
                  ((fBuffer[pos+1] & MASK) <<  8) +
                  ((fBuffer[pos+2] & MASK) << 16) +
                  ((fBuffer[pos+3] & MASK) << 24));


    }

    public  long getLongAt( int pos) {

        if (pos + 8 > fBuffer.length )
            throw new ArrayIndexOutOfBoundsException();




        long val = ((fBuffer[pos]   & MASK)        +
                  ((fBuffer[pos+1] & MASK) <<  8) +
                  ((fBuffer[pos+2] & MASK) << 16) +
                  ((fBuffer[pos+3] & MASK) << 24) +
                  ((fBuffer[pos+3] & MASK) << 32) +
                  ((fBuffer[pos+3] & MASK) << 40) +
                  ((fBuffer[pos+3] & MASK) << 48) +
                  ((fBuffer[pos+3] & MASK) << 56) );



        return val;
    }

    public int setShortAt(int pos, short val) {


        // use little-endian encoding (Intel)
        fBuffer[pos]   = (byte)(val & MASK);
        fBuffer[pos+1] = (byte)((val >>  8) & MASK);

        return 2;
    }

    public int setShortAt(int pos, int val) {

        return setShortAt(pos,(short)( val & 0xffff));


    }

    public  int getShortAt( int pos) {

        return ( (fBuffer[pos]   & MASK)        +
               ((fBuffer[pos+1] & MASK)  <<  8) ) & 0xFFFF;

    }

    public  short getSignedShortAt( int pos) {

        return (short)( (fBuffer[pos]  & MASK)        +
                      ((fBuffer[pos+1] & MASK)  <<  8) );

    }

    public  short getBigEndianShortAt( int pos) {

        if (pos + 2 > fBuffer.length )
            throw new ArrayIndexOutOfBoundsException();

        return (short)( ((fBuffer[pos]   & MASK)  << 8 )      +
                        ((fBuffer[pos+1] & MASK)) );

    }

    /**
     * Read Zero Terminated Ascii String
     */
    public   String getZTAsciiStringAt(int pos,  int maximum) {
        int maxpos = pos + maximum;
        int endpos = pos;
        while(fBuffer[endpos] != 0 && endpos < maxpos)
            endpos++;

        try {
            if(endpos < maxpos)
                return new String(fBuffer, pos, endpos - pos,ISO8859_1);
            else
                return null;
        } catch(UnsupportedEncodingException e) {
            throw new InternalError(e.toString());
        }
    }


    public   String getUnicodeStringAt(int pos,  int bytes) {
        int n = bytes/2;

        char[] chars = new char[n];
        for(int i=0;i<n;i++) {
            int val = getShortAt(pos+i*2);
            chars[i] = (char)(val & 0xffff);
        }

        return new String(chars);

    }

    public   String getAsciiStringAt(int pos,  int len) {

        try {
            return new String(fBuffer, pos, len,ISO8859_1);
        } catch(UnsupportedEncodingException e) {
            throw new InternalError(e.toString());
        }
    }

    public  int setZTAsciiStringAt(int pos, String s) {
        int i=0;

        for( i=0;i<s.length();i++)
            fBuffer[pos+i] = (byte)(s.charAt(i) & 0xff);
        fBuffer[pos+i] = (byte)0;

        return s.length()+1;

    }

    public  int setAsciiStringAt(int pos, String s) {
        int i=0;

        for( i=0;i<s.length();i++)
            fBuffer[pos+i] = (byte)(s.charAt(i) & 0xff);


        return s.length();

    }



    public int setByteAt(int off, byte val) {
        fBuffer[off] = val;
        return 1;
    }


    public byte getByteAt(int off) {
        return fBuffer[off] ;
    }

    public int setBytesAt(int pos, byte[] bytes, int off, int len) {
        System.arraycopy(bytes,off,fBuffer, pos,len);
        return len;

    }

    public int setBytesAt(int pos, char[] bytes, int off, int len) {
        for(int i=0;i<len;i++)
            fBuffer[pos+i]= (byte)(bytes[off+i] & 0xff);

        return len;

    }


    public void setBytesAt(int pos,MarshalBuffer bytes, int from, int  len) {
        System.arraycopy(bytes.fBuffer,from,fBuffer, pos,len);

    }

    void debug(String title) {

        if (!Debug.debugOn || Debug.debugLevel < Debug.BUFFER)
            return;

        Debug.println(Debug.BUFFER,title);

        Debug.println(Debug.BUFFER,fBuffer,0,fSize);

    }

    /**
     * aligns p to a bytes<br>
     * @param p current position
     * @param a alignment (2,4,8)
     * @return aligned p
     */
    final protected static int align( int p,  int a) {
        return (((int)(((a)-1)+(int)(p))) & (( int)(~((a)-1))));
    }

}