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
import java.util.*;
import java.text.*;

public abstract class Util {

    private static Calendar fCalendar = Calendar.getInstance();

    public final static StringComparator fStringComparator = new StringComparator();

    private final static long  MSEC_BASE_1601AD = 11644477199910L;



  
	public static String decode(String s)  {
        StringBuffer sb = new StringBuffer();
        for(int i=0; i<s.length(); i++) {
            char c = s.charAt(i);
            switch (c) {
                case '+':
                    sb.append(' ');
                    break;
                case '%':
                    try {
                        sb.append((char)Integer.parseInt(
                                        s.substring(i+1,i+3),16));
                    }
                    catch (NumberFormatException e) {
                        throw new IllegalArgumentException();
                    }
                    i += 2;
                    break;
                default:
                    sb.append(c);
                    break;
            }
        }
        // Undo conversion to external encoding
        String result = sb.toString();
        byte[] inputBytes;
        
        try {
        	inputBytes = result.getBytes("8859_1");
        } catch(UnsupportedEncodingException e) {
        	throw new IllegalArgumentException();
        }
        return new String(inputBytes);
    }

    /**
     * Converts base 1601 time (in msec) to 1970 based time
     */
    public static long convert1601Time1970(long msec) {
        return msec - MSEC_BASE_1601AD;
    }

    public static byte[]  getZTStringBytes(String s) {

        try {
            s +=  ' ';
            byte[] r  = s.getBytes("8859_1");
            r[r.length-1] = 0;
            return r;
        } catch(UnsupportedEncodingException e) {
            throw new InternalError(e.toString());
        }
    }

    public static byte[]  getStringBytes(String s) {

        try {

            return  s.getBytes("8859_1");


        } catch(UnsupportedEncodingException e) {
            throw new InternalError(e.toString());
        }
    }

    public static byte[]  getUnicodeBytes(String s, boolean nullterm) {

        if (nullterm)
            s += ' ';

        int len = s.length();
        byte[] p = new byte[2*len];

        for(int i=0;i<len-1;i++) {
            int c = s.charAt(i) & 0xffff;

            p[2*i]   = (byte)(c  & 0xff);
            p[2*i+1] = (byte)((c >> 8) & 0xff);
        }

        p[2*(len-1)]   = (byte)0;
        p[2*(len-1)+1] = (byte)0;
        return p;
    }

    public static final boolean equals (byte[] a, byte[] b) {
        if (a.length != b.length)
            return false;
        else {
            for (int i = 0; i < a.length; ++ i)
                if (a[i] != b[i])
                    return false;
            return true;
        }
    }

    public static final void zero (byte[] a, int aoffset, int len) {
        fill ((byte)0, a, aoffset, len);
    }

    public static final void fill (byte a, byte[] b, int boffset, int len)  {
        for (int i = 0; i < len; ++i)
            b[boffset + i] = a;
    }


    public static final void intsToBytes (int[] a, int aoffset, int len, byte[] b, int boffset)  {

        for (int i = 0; i < len; ++i)
            intToBytes (a[aoffset + i], b, boffset + (i << 2));
    }

    public static final void intToBytes (int a, byte[] b, int bo) {
        b[bo]   = (byte)  ((a  >> 24) & 0xff);
        b[bo+1] = (byte)  ((a >> 16) & 0xff);
        b[bo+2] = (byte)  ((a >> 8) & 0xff);
        b[bo+3] = (byte)  ((a ) & 0xff);

    }

    public static final void shortToBytes (int a, byte[] b, int bo) {

        b[bo]   = (byte)  ((a >> 8) & 0xff);
        b[bo+1] = (byte)  ((a ) & 0xff);

    }
    public static final int bytesToInt (byte[] a, int aoffset)  {
        return (a[aoffset]             << 24) + ((a[aoffset + 1] & 0xff) << 16) +
              ((a[aoffset + 2] & 0xff) << 8)  +  (a[aoffset + 3] & 0xff);

    }
    public static final void bytesToInts (byte[] a, int ao, int[] b, int bo, int len)  {

        for (int i = 0; i < len; ++i)
            b[bo + i] = bytesToInt (a, ao + (i << 2));
    }
    public static final void longToBytes (long a, byte[] b, int bo) {
        b[bo]   = (byte)  (a >> 56);
        b[bo+1] = (byte)  (a >> 48);
        b[bo+2] = (byte)  (a >> 40);
        b[bo+3] = (byte)  (a >> 32);
        b[bo+4] = (byte)  (a >> 24);
        b[bo+5] = (byte)  (a >> 16);
        b[bo+6] = (byte)  (a >> 8);
        b[bo+7] = (byte)  (a);
    }
    public static final long bytesToLong (byte[] a, int ao) {
        return ((a[ao]   & 0xffL)  << 56) | ((a[ao+1] & 0xffL)  << 48) |
               ((a[ao+2] & 0xffL)  << 40) | ((a[ao+3] & 0xffL)  << 32) |
               ((a[ao+4] & 0xffL)  << 24) | ((a[ao+5] & 0xffL)  << 16) |
               ((a[ao+6] & 0xffL)  << 8)  |  (a[ao+7] & 0xffL) ;
    }

    public static final String bytesToHex (byte[] a) {

        return bytesToHex(a,a.length);
    }

    public static final String bytesToHex (byte[] a, int len) {
        StringBuffer s = new StringBuffer ();

        for (int i = 0; i < len; ++ i)  {
            s.append (Character.forDigit ((a[i] >> 4) & 0xf, 16));
            s.append (Character.forDigit (a[i] & 0xf, 16));
        }
        return s.toString ();
    }
    public static final String byteToHex (byte a) {
         StringBuffer s = new StringBuffer ();
         s.append (Character.forDigit ((a >> 4) & 0xf, 16));
         s.append (Character.forDigit (a & 0xf, 16));
          return s.toString ();
    }

    public static final String longToHex (long a)  {
        byte[] b = new byte[8];
        longToBytes (a, b, 0);
        return bytesToHex (b);
    }

    public static final String intToHex (int a)  {
        byte[] b = new byte[4];
        intToBytes (a, b, 0);
        return bytesToHex (b);
    }
    public static final String shortToHex (int a)  {
        byte[] b = new byte[2];
        shortToBytes (a, b, 0);
        return bytesToHex (b);
    }

    public static final String byteToBits(byte b) {
        StringBuffer buf = new StringBuffer();

        int n = 256;
        for(int i=0;i<8;i++) {
            n /= 2;
            if ((b & n) != 0) buf.append('1'); else buf.append('0');
        }
        return buf.toString();
    }

    /**
     * Reverses the order of the bits in the byte
     */
    public  static byte  swab(byte b) {

        return (byte)(((b         << 7) | ((b & 0x02) << 5) | ((b & 0x04) << 3) | ((b & 0x08) << 1) |
                      ((b & 0x80) >> 7) | ((b & 0x40) >> 5) | ((b & 0x20) >> 3) | ((b & 0x10) >> 1)) & 0xff);
    }

    public static void swab(byte[] b) {
        for(int i=0;i<b.length;i++)
            b[i] = swab(b[i]);
    }

    public static String replaceString(String s, String pattern, String replace) {
        int p;
        while ( (p = s.indexOf(pattern)) >= 0) {
            s = s.substring(0,p) + replace + s.substring(p+pattern.length());
        }
        return s;
   }

   /**
     * Normalizes file name: replaces \\ ,\..\, \.\
     */
   public static String normalizePathName(String name) {
        name = Util.replaceString(name,"\\\\","\\");
        int p;

        while ( (p = name.indexOf("\\..\\")) >= 0) {
            /*    abc\..\def
                     |
                     p
               s1 = abc
               s2 = \def
            */
            String s1 = name.substring(0,p);
            String s2 = name.substring(p+3);

            p = s1.lastIndexOf("\\");


            if (p >= 0)
                //  xxx\abc\..\def =>  remove xxx
                s1 = s1.substring(0,p);
            else
                //  abc\..\def => remove abc
                s1 = "";
            name = s1 + s2;
        }

        // cut \.. from the end
        if (name.endsWith("\\.."))
            name = name.substring(0,name.length() - 3);

        name = Util.replaceString(name,"\\.\\","\\");

        return name;


    }

    /**
     * Converts SMB date and time to time in milliseconds (base  1.1.1970)
     */
    public synchronized static Date getDateTime(int smbdate, int smbtime) {


        int d = (smbdate & 0x0ffff);
        int t = (smbtime & 0x0ffff);

        // day:5 month:4 year:7
        // yyyy yyym mmmd dddd
        int day  = (d & 0x1f);
        int mon  = (d >> 5) & 0x0f;
        int year = (d >> 9) & 0x007F ;


        // twosec:5 min:6: hours:5
        int sec  = (t & 0x1f)*2;
        int min  = (t >> 5) & 0x03F;
        int hour = (t >> 11) & 0x001F ;

        // The seconds are in SMB tow second increments within the minute


        fCalendar.clear();
        fCalendar.set(year + 1980, mon - 1, day, hour, min, sec);
        return fCalendar.getTime();

    }

    public synchronized static int getSMBDate(Date date) {

        fCalendar.setTime(date);
        int day  = fCalendar.get(Calendar.DAY_OF_MONTH);
        int mon  = fCalendar.get(Calendar.MONTH) + 1;
        int year = fCalendar.get(Calendar.YEAR) - 1980;
        return ((year & 0x007F) << 9) + (( mon & 0x0F) << 5) + ( day &  0x1F);
    }
    public synchronized static int getSMBTime(Date date) {

        fCalendar.setTime(date);
        int hour  = fCalendar.get(Calendar.HOUR_OF_DAY);
        int min   = fCalendar.get(Calendar.MINUTE);
        int sec   = fCalendar.get(Calendar.SECOND) / 2;
        return ((hour & 0x001F) << 11) + ((min & 0x3F) << 5) + ( sec &  0x1F);
    }
    public static Comparator getStringComparator() {
        return fStringComparator;
    }


    public static void sortStrings(String[] array) {
        sort(array,array.length,getStringComparator());
    }

    public static void sort(Object[] array,Comparator comparator) {
        sort(array,array.length,comparator);
    }

    /**
     * Sorts array (Heapsort)
     */
    public static void sort(Object[] array,int count,Comparator comparator) {

	    int i,top,t,largest,l,r,here;
	    Object temp;

        int elementCount = count;

	    if (elementCount<=1) {
	        return;
	    }

	    top = elementCount - 1;
	    t   = elementCount / 2;


	    do {
	        --t;
	        largest=t;

	        /* heapify */

	        do {
		        i = largest;
		        l = left(largest);
		        r = right(largest);

		        if (l<=top) {

		            if (comparator.compare(array[l],array[i]) > 0)
		                largest = l;

		        }
		        if (r <= top) {

		            if (comparator.compare(array[r],array[l]) > 0)
		                largest = r;

		        }
		        if (largest != i) {
		            temp           = array[largest];
		            array[largest] = array[i];
		            array[i]       = temp;
		        }
	        }  while (largest != i);
	    } while (t>0);


	    t=elementCount;

	    do {
	        --top;
	        --t;

	        here=t;

	        temp       = array[here];
	        array[here]= array[0];
	        array[0]   = temp;

	        largest=0;

	        do {
		        i=largest;
		        l=left(largest);
		        r=right(largest);

		        if (l<=top) {

		            if (comparator.compare(array[l],array[i])>0)
		                largest=l;

		        }
		        if (r<=top) {

		            if (comparator.compare(array[r],array[largest])>0)
		                largest=r;

		        }
		        if (largest != i) {
		            temp           = array[largest];
		            array[largest] = array[i];
		            array[i]       = temp;
		        }
	        } while (largest!=i);

	    } while (t>1);

    }

    private static int left(int i) {return 2*i+1;}
    private static int right(int i) {return 2*i+2;}


    public static String getIPAddress(byte[] b ,int off) {
        StringBuffer addr = new StringBuffer();

        addr.append((int)b[off++] & 0xff);
        addr.append('.');
        addr.append((int)b[off++] & 0xff);
        addr.append('.');
        addr.append((int)b[off++] & 0xff);
        addr.append('.');
        addr.append((int)b[off++] & 0xff);
        return addr.toString();
    }

}


class StringComparator implements Comparator {

    private Collator fCollator = Collator.getInstance();

    public int compare(Object o1, Object o2) {

        return fCollator.compare((String)o1,(String)o2);

    }
}