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
import java.text.*;

/**
 * <code>CifsDisk</code> declares the disk operations.<p>
 * How to access the disk service?<p>
 * <pre>
 *   CifsDisk disk = CifsSessionManager.connectDisk("sessionname","\\server\share");
 * </pre>
 *
 * @see org.gnu.jcifs.CifsSessionManager#connectDisk
 * @version 1.0, 21 Nov 1998
 * @since   1.0
 */
public interface CifsDisk extends CifsSession {

    /*   Sharing modes
     * ---- ---- -SSS ----
     */

    /** Deny read/write/execute (exclusive) */
    public final static int  SM_EXCLUSIVE      = 1;
    /** Deny write */
    public final static int  SM_DENY_WRITE     = 2;
    /** Deny read/execute */
    public final static int  SM_DENY_READ_EXEC = 3;
    /** Allow all actions */
    public final static int  SM_DENY_NONE      = 4;



    /**
     * Gets the  file from the server and writes the content to the given
     * output stream. The file will not  be locked
     * @param file remote file name (relativ to share)
     * @param out  output stream
     * @return number of bytes
     * @exception  IOException  if an I/O error occurs.
     */
    public  long getFile(String file, OutputStream out) throws IOException;

    /**
     * Gets the  file from the server and writes the content to the given
     * output stream. The file will not  be locked
     * @param file remote file name (relativ to share)
     * @param out  writer
     * @return number of bytes
     * @exception  IOException  if an I/O error occurs.
     */
    public  long getFile(String file, Writer out) throws IOException ;

    /**
     * Puts the data from the input stream to the given remote file.
     * The file will not  be locked
     * @param file remote file name (relativ to share)
     * @param in  input reader
     * @return number of bytes
     * @exception  IOException  if an I/O error occurs.
     */
    public  long putFile(String file, Reader in) throws IOException ;

    /**
     * Puts the data from the input stream to the given remote file.
     * The file will not  not locked
     * @param file remote file name (relativ to share)
     * @param in  input stream
     * @return number of bytes
     * @exception  IOException  if an I/O error occurs.
     */
    public  long putFile(String file, InputStream in) throws IOException ;
    /**
     * Deletes the given file
     * @param file remote file name
     * @exception  IOException  if an I/O error occurs.
     */

    public  void deleteFile( String  file) throws IOException ;

    /**
     * Renames the old file to the new file (also hidden and system files)
     * @param oldfile old file
     * @param newfile new file
     * @exception  IOException  if an I/O error occurs.
     */

    public void renameFile(String  oldfile, String newfile) throws IOException ;

    /**
     * Renames file corresponding to the search attributes
     * @param oldfile old file
     * @param newfile new file
     * @param searchattr search attributes (see CifsFileInfo.FILE_ATTR_*)
     * @exception  IOException  if an I/O error occurs.
     * @see org.gnu.jcifs.CifsFile
     * @see org.gnu.jcifs.CifsFileInfo
     */
    public  void renameFile(String  oldfile, String newfile, int searchattr) throws IOException;



    /**
     * Sets or resets file attributes (CifsFileInfo.FILE_ATTR_*)
     * @param file file name
     * @param attr new file attributes
     * @param set if true sets the given attributes otherwise resets it
     * @tbd   set date
     * @exception  IOException  if an I/O error occurs.
     * @see org.gnu.jcifs.CifsFile
     * @see org.gnu.jcifs.CifsFileInfo
     */
    public  void setFileAttribute(String  file, int attr, boolean set) throws IOException ;


    /**
     * Creates the given directory on the server
     * @param dirname directory name
     * @exception  IOException  if an I/O error occurs.
     */
    public  void mkdir(String  dirname) throws IOException;

    /**
     * Removes directory
     * @param dirname directory name
     * @exception  IOException  if an I/O error occurs.
     */
    public  void rmdir( String  dirname) throws IOException ;
    /**
     * Checks if directory exists
     * @param dirname directory name
     * @return true if directory exits
     * @exception  IOException  if an I/O error occurs.
     */
    public boolean directoryExists(String  dirname) throws IOException;


    /**
     * Returns informations about the filesystem
     * @return CifsFileSystemInfo filesystem informations
     * @exception  IOException  if an I/O error occurs.
     */
    public  CifsFileSystemInfo getFileSystemInfo() throws IOException;


    /**
     * Returns information about the given file
     * @param file file name
     * @return CifsFileInfo
     * @exception  IOException  if an I/O error occurs.
     */
    public  CifsFileInfo  getFileInfo(String  file) throws IOException ;


    /**
     * Enumerates informations about the files
     * @param file file name (with wildcards)
     * @param searchattr file attributes (see CifsFileInfo.FILE_ATTR_*)
     * @param sort  if true names will be sorted
     * @return  array of <code>CifsFileInfo</code> elements
     * @exception  IOException  if an I/O error occurs.
     */
    public CifsFileInfo[] listFilesInfo(String  file, int searchattr, boolean sort) throws IOException;

    /**
     * Enumerates file names
     * @param file file name (with wildcards)
     * @param searchattr file attributes
     * @param sort  if true names will be sorted
     * @return array of <code>java.lang.String</code> elements
     * @exception  IOException  if an I/O error occurs.
     */
    public  String[] listFilesName(String  file, int searchattr, boolean sort) throws IOException ;


}