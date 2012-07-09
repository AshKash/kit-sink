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
 * A <code>CifsPrintJobInfo</code> contains informations about a print job.
 * (Corresponds to the structure PSJINFO_2).
 *
 * @author  Norbert Hranitzky
 * @version 1.0, 20 Nov 1998
 * @since   1.0
 */
public final class CifsPrintJobInfo  {

    //----------- Print queue status (Bit 0-1) --------------
    /** Print job is queued */
    public final static int PRJ_QS_QUEUED	= 0;
    /** Print job is paused */
    public final static int PRJ_QS_PAUSED	= 1;
    /** Print job is spooling */
    public final static int PRJ_QS_SPOOLING	= 2;
    /** Print job is printing, Print Job Status is valid */
    public final static int PRJ_QS_PRINTING	= 3;


    /** Print job is complete */
    public final static int PRJ_COMPLETE	= 0x0004;
    /** An error occurred */
    public final static int PRJ_INTERV	    = 0x0008;
    /** Print job is spooling */
    public final static int PRJ_ERROR	    = 0x0010;
    /** The print destination is offline */
    public final static int PRJ_DESTOFFLINE	= 0x0020;
    /** The print destination is paused */
    public final static int PRJ_DESTPAUSED	= 0x0040;
    /** An alert is raised */
    public final static int PRJ_NOTIFY	    = 0x0080;
    /** The print destination is out of paper */
    public final static int PRJ_DESTNOPAPER	= 0x0100;
    /** The printer is waiting for a form change */
    public final static int PRJ_DESTFORMCHG	= 0x0200;
    /** The printer is waiting for a cartridge change */
    public final static int PRJ_DESTCRTCHG	= 0x0400;
    /** The printer is waiting for a pen change */
    public final static int PRJ_DESTENCHG	= 0x0800;
    /** An alert indicates the job was deleted */
    public final static int PRJ_PRINTING	= 0x8000;

    /* ???
    public final static int STOPPED      = 1;
    public final static int PRINTING     = 2;
    public final static int AWAITING     = 3;
    public final static int IN_INTERCEPT = 4;
    public final static int FILE_ERROR   = 5;
    public final static int PRINTER_ERROR= 6;
    */

    CifsPrintJobInfo () {

    }

    int     fJobId;
    int     fPriority;
    String  fUserName;
    int     fPosition;
    int     fStatus;

    // secodns since 1970
    long    fSubmitted;

    long    fSize;
    String  fComment;
    String  fDocument;

    /**
     * JobId  uniquely specifies a print job within a printer queue.
     * The JobID is unique on a server. A combination of the server
     * name and JobId is sufficient to uniquely identify a particular print job.
     * @return jobId
     */
    public int getJobId() {
        return fJobId;
    }
    /**
     * Priority  specifies the print job priority.
     * This varies from a value of 1 (lowest priority) to 99
     * (highest priority. Higher priority jobs print first.
     * When 2 jobs have the same priority, the older job prints first.
     * @return priority value
     */
    public int getPriority() {
        return fPriority;
    }

    /**
     * The user name specifies the name of the user who submitted the print job
     * @return user name
     */
    public String getUserName() {
        return fUserName;
    }

    /**
     * Position specifies the position of the print job within the print queue.
     * If the value is 1, this print job prints next.
     * @return position
     */
    public int getPosition() {
        return fPosition;
    }

    /**
     * Returns the status of the print queue. If the status is
     * <code>PRJ_QS_PRINTING</code>, the call <code>getPrintJobStatus()</code>
     * returns detailed status informations.
     * @return status (see PRJ_QS_*)
     */
    public int getPrintJobQueueStatus() {
        return (fStatus & 0x03);
    }

    /**
     * Returns the status of the print job.
     * @return status (see PRJ_*)
     */
    public int getPrintJobStatus() {
        return (fStatus & 0xFFFC);
    }

    /**
     * Return the time when the user submitted the job.
     * This is stored as the number of seconds elapsed since
     * 00:00:00 Jan 1st, 1970
     * @return time in seconds
     */
    public long getSubmittedTime() {
        return fSubmitted;
    }

    /**
     * Returns the size of the print job in terms of number of bytes.
     * @return size in bytes
     */
    public long getSize() {
        return fSize;
    }
    /**
     * Returns the comment about the print job
     * @return comment text
     */
    public String getComment() {
        return fComment;
    }

    /**
     * Returns the name of the document
     * @return document name
     */
    public String getDocument() {
        return fDocument;
    }

     /**
     * Compares if the two object are the same (same Job ID)
     * @param obj the object to test
     * @return true of obj is the same <code>CifsPrintJobInfo</code> object
     */
    public boolean equals(Object obj) {

        if ((obj != null) && (obj instanceof CifsPrintJobInfo)) {

            CifsPrintJobInfo anobj = (CifsPrintJobInfo)obj;

            return (anobj.fJobId == fJobId);

        }
        return false;
    }
}