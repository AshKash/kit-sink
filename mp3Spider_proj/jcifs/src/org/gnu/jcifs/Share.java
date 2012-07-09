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
import java.util.Enumeration;

/**
 * Universal Naming Convention<p>
 * UNC: \\machine\share
 * URL: cifs://machine/share
 *
 */
final class Share {

    private String      fName;
    private String      fNodeName;
    private String      fHostName;
    private String      fShareName;
    private CifsLogin   fLogin;
    final static int    DISK    = 0;
    final static int    IPC     = 1;
    final static int    PRINTER = 2;
    private int         fType = DISK;

    private final static String UNC_PREFIX = "\\\\";
    private final static String URL_PREFIX = "cifs://";



    Share(CifsLogin login)  {


        fLogin = login;
    }

    Share(String name, int type) throws CifsShareNameException {
        setName(name);
        fType  = type;
        fLogin = new CifsLogin();
    }

    Share(String name, int type,CifsLogin login) throws CifsShareNameException {
        setName(name);
        fType  = type;
        fLogin = login;
    }


    void set(int type,String host, String sharename) {
        fType      = type;
        fHostName  = host;
        fShareName = sharename;
        setNodeName();
    }

    public CifsLogin getLogin() {
        return fLogin;
    }

    public String getNodeName() {
        return fNodeName;
    }

    public String getHostName() {
        return fHostName;
    }

    public String getShareName() {

        StringBuffer buf = new StringBuffer(255);
        buf.append("\\\\");
        buf.append(fNodeName);
        buf.append("\\");
        buf.append(fShareName);

        return buf.toString();
    }



    public final void setName(String name) throws CifsShareNameException {

        name = name.trim();

        if (name.startsWith(UNC_PREFIX) && name.length() > 5)
            setUNCName(name);
        else if (name.startsWith(URL_PREFIX) && name.length() > 8)
            setURLName(name);
        else
            throw new CifsShareNameException("MN1",name);

        // make node name
        setNodeName();

    }

    private void setNodeName() {
        int p = fHostName.indexOf('.');

        if (p < 0)
            fNodeName = fHostName;
        else
            fNodeName = fHostName.substring(0,p);
    }

    private final void setURLName(String name) throws CifsShareNameException {

        int off = URL_PREFIX.length();

        /*
         * cifs://server/share
         *        |     |
         *       off    p
         */
        int p = name.indexOf('/',off);

        if (p <= 0)
            throw new CifsShareNameException("MN1",name);

        fHostName = name.substring(off,p);

        off = p + 1;

        if (off >= name.length() )
            throw new CifsShareNameException("MN1",name);

        fShareName = name.substring(off);

    }

    private final void setUNCName(String name) throws CifsShareNameException {

        int off = UNC_PREFIX.length();

        int p = name.indexOf("\\", off);
        if(p < 0)
            throw new CifsShareNameException("MN1",name);

        /* \\server\share
         *   |      |
         *  off     p
         */

        fHostName = name.substring(off, p);

        off = p + 1;

        if (off >= name.length() )
            throw new CifsShareNameException("MN1",name);

        fShareName = name.substring(off);

    }

    public String toString() {
        return getShareName();
    }

    int getType() {
        return fType;
    }

}