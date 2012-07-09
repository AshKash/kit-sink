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
  */


package org.gnu.jcifs;

import org.gnu.jcifs.util.*;

import java.io.*;
import java.util.*;


/**
 * Spool queue entry
 */
public  class CifsPrinterSpoolInfo  {

    public final static int STOPPED      = 1;
    public final static int PRINTING     = 2;
    public final static int AWAITING     = 3;
    public final static int IN_INTERCEPT = 4;
    public final static int FILE_ERROR   = 5;
    public final static int PRINTER_ERROR= 6;

    CifsPrinterSpoolInfo () {

    }

	// Date file was queued
    Date    fCreationTime;

	// Entry status
    int     fStatus;

	// Assigned by the spooler
    int     fSpoolFileNumber;
    // Number of bytes in spool file
    long    fSpoolFileSize;
    
    // Client which created the spool file
    String  fSpoolFileName;

	/**
	 * Returns the time the file was queued
	 * @return date
	 */
	public Date getCreationTime() {
		return fCreationTime;
	}
	
	/**
	 * Returns the spool status
	 * @return status (see constants)
	 */
	public int getStatus() {
		return fStatus;
	}
	
	/**
	 * Returns the number of bytes in the spool file
	 * @return number of bytes
	 */
	public int getSpoolFileNumber() {
		return fSpoolFileNumber;
	}
	
	/**
	 * Returns the spool file name
	 * @return spool file name
	 */
	public String getSpoolFileName() {
		return fSpoolFileName;
	}
	
	
}