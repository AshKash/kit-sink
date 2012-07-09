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



public class CifsPrinterQueueInfo  {

    public final static int PRQ_ACTIVE	= 0; //	Active
    public final static int PRQ_PAUSE	= 1; //	Paused
    public final static int PRQ_ERROR	= 2; //	Error Occurred
    public final static int PRQ_PENDING	= 3; //	Deletion pending

    String fName;
    int    fPriority;
    int    fStartTime;
    int    fUntilTime;
    String fPrintDestination;
    String fParams;
    String fComment;
    int    fStatus;
    int    fJobs;
    String fDriver;

    CifsPrinterQueueInfo() {

    }
    /**
     * The queue name
     */
    public String getPrinterName() {
        return fName;
    }
    /**
     * The printer queue priority (1:highest ... 9(lowest))
     */
    public int getPriority() {
        return fPriority;
    }
    /**
     * Starttime contains an unsigned short integer specifying the time
     * of day a printer queue can start sending print jobs to printers.
     * This value represents the number of minutes since midnight (00:00).
     */
    public int getStartTime() {
        return fStartTime;
    }
    /**
    * Untiltime contains an unsigned short integer specifying the time of
    * day a printer queue becomes inactive and stops sending print jobs to printers.
    * This value represents the number of minutes since midnight (00:00).
    */
    public int getUntilTime() {
        return fUntilTime;
    }

    /**
     * A list of print destinations for the print queue.
     * This is a multi-valued property and the values are separated by spaces.
     */
    public String getPrintDestinations() {
        return fPrintDestination;
    }
    /**
     * contains parameters required by printer queues
     */
    public String getParams() {
        return fParams;
    }
    /**
     * a comment about the print queue
     */
    public String getComment() {
        return fComment;
    }
   /**
    * Untiltime contains an unsigned short integer specifying the time of
    * day a printer queue becomes inactive and stops sending print jobs to printers.
    * This value represents the number of minutes since midnight (00:00).
    */
    public int getStatus() {
        return fStatus;
    }
    /**
    * the number of print jobs currently in the print queue
    */
    public int getJobsNumber() {
        return fJobs;
    }

    /**
     * default device driver for the queue.
     * If this field is null
     */
    public String getDriver() {
        return fDriver;
    }
}