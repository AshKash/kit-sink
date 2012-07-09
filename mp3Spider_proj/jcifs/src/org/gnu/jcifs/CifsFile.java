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
  *  Author : Norbert Hranitzky
  *  Contact: norbert.hranitzky@mchp.siemens.de; http://www.hranitzky.purespace.de
  *  History: 
  */


package org.gnu.jcifs;

import org.gnu.jcifs.util.*;
import java.io.*;
import java.util.*;

/**
 * <code>CifsFile</code> represents a CIFS file
 *
 * @version 1.0, 20 Nov 1998
 * @since   1.0
 */
public final class CifsFile implements CifsFileInfo {


    private CifsDisk  fDisk;
    private String    fFile;

    private CifsFileInfo fInfo = null;


    /**
     * Creates a new <code>CifsFile</code> object.
     * @param disk disk object
     * @param file file name
     * @exception  IOException  if an I/O error occurs.
     */
    public CifsFile(CifsDisk disk, String file) throws IOException {

         fDisk = disk;
         fFile = file;
         refresh();

    }

    /**
     * Creates a new <code>CifsFile</code> object.
     * @param sessionname the name of the connected disk session
     * @param file file name
     * @exception  IOException  if an I/O error occurs.
     */
    public CifsFile(String sessionname, String file) throws IOException {
        fFile = file;

        fDisk = CifsSessionManager.lookupConnectedDisk(sessionname);

        if (fDisk == null)
            throw new CifsIOException("201",sessionname);
        refresh();
    }

    /**
     * Returns the disk object
     * @return disk object
     */
    public CifsDisk getDisk() {
        return fDisk;
    }

    /**
     * Checks if the file exists
     * @return true if file exists
     */
    public boolean exists() {

        return (fInfo != null);
    }

    /**
     * Refresh file informations. If the file was deleted, created or
     * the attributes of the file was changed, you must call this method
     * to update file information hold in this object
     */
    public void refresh() {
        try {
            fInfo = fDisk.getFileInfo(fFile);
         } catch(IOException e) {
            fInfo = null;
         }
    }

    /**
     * Deletes the current file or directory
     * @exception  IOException  if an I/O error occurs.
     */
    public void delete() throws IOException {

        refresh();

        if (fInfo != null) {
            if (fInfo.isFile())
                fDisk.deleteFile(fInfo.getPathName());
            else
                fDisk.rmdir(fInfo.getPathName());
        }
    }

    /**
     * Create directory
     * @exception  IOException  if an I/O error occurs.
     */
    public void mkdir() throws IOException {

        refresh();

        if (exists())
            throw new CifsIOException("DK4",fInfo.getPathName());

        fDisk.mkdir(fFile);
    }

    /**
     * Renames current file/directory to the given file
     * @param file new file name
     * @exception  IOException  if an I/O error occurs.
     */
    public void renameTo(String file) throws IOException {
        refresh();

        if (fInfo == null)
            throw new CifsIOException("DK5",fFile);

        fDisk.renameFile(fInfo.getPathName(),file);
    }

    /**
     * Syntax: diskname:\path or /diskname/path
     */
     /*
    private static void delete(String cifsfile) throws IOException {

        StringBuffer dname = new StringBuffer(10);
        StringBuffer path  = new StringBuffer(30);

        parseNameWithDiskName(cifsfile,dname,path);

        String diskname = dname.toString();
        CifsDisk. validateDiskName(diskname);

        CifsDisk disk = CifsDisk.lookupConnectedDisk(diskname);

        if (disk == null)
            throw new CifsIOException("201",diskname);

        String pn = path.toString().replace('/','\\');
        disk.deleteFile(pn);
    }
    */

    //---------------- CifsFileInfo ------------------
    /**
     * Returns the path name
     * @return path name
     */
    public String getPathName() {
        if (fInfo != null)
            return fInfo.getPathName();
        return fFile;
    }
    /**
     * Returns file name
     * @return file name
     */
    public String getFileName() {
        if (fInfo != null)
            return fInfo.getFileName();
        return fFile;
    }
    /**
     * Returns creation time of the file/directory
     * @return date
     */
    public Date getCreationTime() {
        if (fInfo != null)
            return fInfo.getCreationTime();
        return null;
    }
    /**
     * Returns last access time of the file/directory
     * @return date
     */
    public Date getLastAccessTime() {
        if (fInfo != null)
            return fInfo.getLastAccessTime();
        return null;
    }
    /**
     * Returns last write  time of the file/directory
     * @return date
     */
    public Date getLastWriteTime() {
        if (fInfo != null)
            return fInfo.getLastWriteTime();
        return null;
    }

    /**
     * Returns file size
     * @return size
     */
    public long length() {
        if (fInfo != null)
            return fInfo.length();
        return -1;
    }

    /**
     * Checks if file is a directory
     * @return true if directory
     */
    public boolean isDirectory() {
        if (fInfo != null)
            return fInfo.isDirectory();
        return false;
    }

    /**
     * Checks if is a file
     * @return true if file
     */
    public boolean isFile() {
        if (fInfo != null)
            return fInfo.isFile();
        return false;
    }

    /**
     * Returns true if file is read only
     * @return true if read only
     */
    public boolean isReadOnly() {
        if (fInfo != null)
            return fInfo.isReadOnly();
        return false;
    }

    /**
     * Returns if file is a hidden file
     * @return true if hidden file
     */
    public boolean isHidden() {
        if (fInfo != null)
            return fInfo.isHidden();
        return false;
    }
    /**
     * Returns if file is a system file
     * @return true if system file
     */
    public boolean isSystem() {
        if (fInfo != null)
            return fInfo.isSystem();
        return false;
    }

    /**
     * Returns if file is a archive file
     * @return true if archive file
     */
    public boolean isArchive() {
        if (fInfo != null)
            return fInfo.isArchive();
        return false;
    }
    /**
     * Returns file attribute flag
     * @return flag
     */
    public int getAttributes() {
        if (fInfo != null)
            return fInfo.getAttributes();
        return 0;
    }
    public String toString() {
        if (fInfo != null)
            return fInfo.toString();
        return fFile;
    }



    static int getIOBufferSize() {
        int size = 2048;
        try {
            size = Integer.parseInt(System.getProperty("org.gnu.jcifs.io.bufsize","8196"));
        } catch(NumberFormatException e) {

        }
        if (size < 2048)
            size = 2048;
        return size;
    }
}