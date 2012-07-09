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
import java.util.Date;

/**
 * File attributes information
 * @version 1.0
 */

public interface CifsFileInfo  {


    public final static int FILE_ATTR_ANY         = 0x00;
    /** Read only file */
    public final static int FILE_ATTR_READ_ONLY   = 0x01;
    /** Hidden file */
    public final static int FILE_ATTR_HIDDEN_FILE = 0x02;
    /** System file */
    public final static int FILE_ATTR_SYSTEM_FILE = 0x04;
    /** Volume */
    public final static int FILE_ATTR_VOLUME      = 0x08;
    /** Directory */
    public final static int FILE_ATTR_DIRECTORY   = 0x10;
    /** Archive file */
    public final static int FILE_ATTR_ARCHIVE     = 0x20;

    /** All attributes (for find ) */
    public final static int FILE_ATTR_ALL         = 0x3f;

    /**
     * Returns the path name
     * @return path name
     */
    public String getPathName();

    /**
     * Returns file name
     * @return file name
     */
    public String getFileName();

    /**
     * Returns creation time of the file/directory
     * @return date
     */
    public Date getCreationTime() ;

    /**
     * Returns last access time of the file/directory
     * @return date
     */
    public Date getLastAccessTime() ;

    /**
     * Returns last write  time of the file/directory
     * @return date
     */
    public Date getLastWriteTime() ;

    /**
     * Returns file size
     * @return size
     */
    public long length() ;

    /**
     * Checks if file is a directory
     * @return true if directory
     */
    public boolean isDirectory();

    /**
     * Checks if is a file
     * @return true if file
     */
    public boolean isFile();

    /**
     * Returns true if file is read only
     * @return true if read only
     */
    public boolean isReadOnly() ;

    /**
     * Returns if file is a hidden file
     * @return true if hidden file
     */
    public boolean isHidden() ;

    /**
     * Returns if file is a system file
     * @return true if system file
     */
    public boolean isSystem() ;

    /**
     * Returns if file is a archive file
     * @return true if archive file
     */
    public boolean isArchive() ;

    /**
     * Returns file attribute flag
     * @return flag
     */
    public int getAttributes();

}