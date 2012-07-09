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
  *  Web   : http://www.hranitzky.purespace.de
  *  Changes: 1999.08.28 new method getDetail()
  */


package org.gnu.jcifs;

import org.gnu.jcifs.util.*;
import java.util.*;
import java.text.*;

/**
 * Runtime exception class<p>
 */
 

public  class CifsRuntimeException extends java.lang.RuntimeException {

    private Throwable fDetail = null;
    protected String  fText   = null;
    protected String  fCode   = null;

    private static ResourceBundle fResourceBundle ;

    public CifsRuntimeException(String msg,boolean key) {
        if (key) {
            fCode = msg;
            fText = fResourceBundle.getString(msg);
        } else
            fText = msg;

    }

    public CifsRuntimeException(String key) {
        this(key,true);

    }
    public CifsRuntimeException(String key, Object i1) {
        Object[] ins = {i1};
        fText = MessageFormat.format(fResourceBundle.getString(key),ins);
        fCode = key;
    }

    public CifsRuntimeException(String key, Object i1,Object i2) {
        Object[] ins = {i1,i2};
        fText = MessageFormat.format(fResourceBundle.getString(key),ins);
        fCode = key;
    }



    public CifsRuntimeException setDetail(Throwable detail) {
        fDetail = detail;
        return this;
    }

	/**
	 * Returns the encapsulated exception (optional)
	 * @return exception
	 */
	public Throwable getDetail() {
		return fDetail;
	}
	
    public static String getMessage(String key) {

        return fResourceBundle.getString(key);
    }

    public static String getMessage(String key, Object i1) {
        Object[] ins = {i1};
        return MessageFormat.format(fResourceBundle.getString(key),ins);
    }
    public static String getMessage(String key, Object i1,Object i2) {
        Object[] ins = {i1,i2};
        return MessageFormat.format(fResourceBundle.getString(key),ins);
    }

   /**
     * Produce the message, include the message from the nested
     * exception if there is one.
     */
    public String getMessage() {
	    if (fDetail == null)
	        return '[' + fCode + "] " + fText;
	    else
	        return '[' + fCode + "] " + fText + "[" + fDetail.toString() + "]";

    }

    static  {
        try  {
            fResourceBundle = ResourceBundle.getBundle("org.gnu.jcifs.i18n.Resources");

        } catch(Exception e) {
            System.err.println("Cannot load resource bundle:" + e);
            System.exit(1);
        }

    }
}