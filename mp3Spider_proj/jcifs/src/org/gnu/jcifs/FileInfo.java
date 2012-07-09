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
import java.text.DateFormat;


final class FileInfo implements CifsFileInfo {

    Date            fCreationTime;
    Date            fLastAccessTime;
    Date            fLastWriteTime;

    int             fAttributes;
    String          fFileName;
    String          fDirectory;
    long            fFileSize;

    FileInfo(String dir) {
        fDirectory = dir;
    }

    public String getPathName() {
        return fDirectory + "\\" + fFileName;
    }

    public String getFileName() {
        // relative to share
        return fFileName;
    }
    public Date getCreationTime() {
        return fCreationTime;
    }
    public Date getLastAccessTime() {
        return fLastAccessTime;
    }
    public Date getLastWriteTime() {
        return fLastWriteTime;
    }
    public long length() {
        return fFileSize;
    }
    public boolean isDirectory() {
        return ((fAttributes & FILE_ATTR_DIRECTORY) != 0);
    }
    public boolean isFile() {
        return !isDirectory();
    }
    public boolean isReadOnly() {
         return ((fAttributes & FILE_ATTR_READ_ONLY) != 0);
    }
    public boolean isHidden() {
         return ((fAttributes & FILE_ATTR_HIDDEN_FILE) != 0);
    }
    public boolean isSystem() {
         return ((fAttributes & FILE_ATTR_SYSTEM_FILE) != 0);
    }
    public boolean isArchive() {
         return ((fAttributes & FILE_ATTR_ARCHIVE) != 0);
    }
    /**
     * Returns file attribute flag
     * @return flag
     */
    public int getAttributes() {
        return fAttributes;
    }


    public String toString() {
        DateFormat dt = DateFormat.getDateTimeInstance(DateFormat.SHORT,DateFormat.SHORT);
        StringBuffer buf = new StringBuffer(100);
        if (isDirectory()) buf.append("d"); else buf.append("f");

        if (isReadOnly()) buf.append("r"); else  buf.append("-");

        if (isHidden())  buf.append("h"); else  buf.append("-");

        if (isSystem())  buf.append("s"); else  buf.append("-");

        if (isArchive())  buf.append("a"); else  buf.append("-");

        buf.append(" ");

        buf.append(dt.format(fCreationTime) + " ");
        buf.append(dt.format(fLastAccessTime)+ " ");
        buf.append(dt.format(fLastWriteTime) + " ");

        String ssize ;

        if (!isDirectory()) {

            ssize = " " + fFileSize;

        } else
            ssize =  "<DIR>";

        int len = ssize.length();
        for(int i=len;i<8;i++)
            ssize = " " + ssize;
        buf.append(ssize + " ");

        buf.append(fFileName);
        return buf.toString();
    }
}