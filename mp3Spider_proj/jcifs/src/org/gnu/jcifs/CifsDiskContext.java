/**
  *         Commmon Internet File System Java API (JCIFS) - Client
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
  *  Author : Norbert Hranitzky
  *  Contact: norbert.hranitzky@mchp.siemens.de; http://www.hranitzky.purespace.de
  *  History:
  */


package org.gnu.jcifs;

import org.gnu.jcifs.util.*;

import java.io.*;
import java.util.*;

/**
 * <code>CifsDiskContext</code> is a helper class. The JCIFS runtime does not use it.
 * You can use the class to save user specific data and to implement
 * the current directory concept.<p>
 * After connecting to disk, you can create a <code>CifsDiskContext</code> object and
 * save it as a property:<p>
 * <pre>
 *      CifsDisk disk;
 *      ....
 *      CifsDiskContext ctxt = new CifsDiskContext(disk);
 *      disk.setProperty("mycontext",ctxt);
 * </pre>
 * Whenever you need the context object:<p>
 * <pre>
 *    CifsDiskContext ctxt = (CifsDiskContext)disk.getProperty("mycontext");
 * </pre>
 *
 * @see org.gnu.jcifs.CifsDisk#setProperty
 * @see org.gnu.jcifs.CifsDisk#getProperty
 * @version 1.0, 20 Nov 1998
 * @since   1.0
 */
public  class CifsDiskContext  {

    private DiskImpl fDisk;

    private String   fCurrentDir = "\\";

    /**
     * Creates a new context object
     * @param disk disk object
     */
    public CifsDiskContext(CifsDisk disk) {
        fDisk = (DiskImpl)disk;

    }

    /**
     * Returns the current directory (relative to share)
     * @return directory
     */
    public final String getCurrentDir() {
        return fCurrentDir;
    }

    /**
     * Changes current directory. If the name begins with &#92;, the new directory
     * is relative to the share, otherwise relative the current directory
     * @param newdir new directory name
     * @exception IOException if directory does not exists
     */
    public final synchronized void changeCurrentDir(String newdir) throws IOException {

        String ncd ;

        if (newdir.startsWith("\\"))
            ncd = newdir;
        else
            ncd = fCurrentDir + newdir;

        if (!ncd.endsWith("\\"))
            ncd += "\\";

        ncd = Util.normalizePathName(ncd);

        if (ncd.equals("\\"))
            fCurrentDir = ncd;
        else {
            if (fDisk.checkDirectory(ncd))
                fCurrentDir = ncd;
            else
                throw new CifsIOException("DK6",ncd);
        }

    }

    /**
     * Returns the path name: If the file starts with \ (or) /, the pathname=filename,
     * otherwise pathname = current-dir/filename
     * @param file file name
     * @return path name
     */
     public final String getPathName(String file) {
         file = file.replace('/','\\');

         if (file.startsWith("\\"))
            return file;

         return fCurrentDir + file;


     }

     /**
      * Returns the corresponding disk object
      * @return disk object
      */
    public CifsDisk getDisk() {
        return fDisk;
    }


    public String toString() {
        return "[CifsDiskContext:currentDir=" + fCurrentDir + "]";
    }
}