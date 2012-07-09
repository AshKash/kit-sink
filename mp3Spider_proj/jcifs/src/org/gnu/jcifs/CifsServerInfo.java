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


public final class CifsServerInfo  {

    /** Enum all server types */
    public final static int SV_TYPE_ALL     	    = 0xFFFFFFFF;
    /** Enum all workstations */
    public final static int SV_TYPE_WORKSTATION	    = 0x00000001;
    /** Enum servers */
    public final static int SV_TYPE_SERVER	        = 0x00000002;
    /** Enum any server running with SQL server */
    public final static int SV_TYPE_SQLSERVER	    = 0x00000004;
    /** Primary domain controller */
    public final static int SV_TYPE_DOMAIN_CTRL	    = 0x00000008;
    /**  Backup domain controller */
    public final static int SV_TYPE_DOMAIN_BAKCTRL	= 0x00000010;
    /** Server running the timesource service */
    public final static int SV_TYPE_TIME_SOURCE	    = 0x00000020;
    /**  Apple File Protocol servers */
    public final static int SV_TYPE_AFP	            = 0x00000040;
    /** Novell servers */
    public final static int SV_TYPE_NOVELL	        = 0x00000080;
    /** Domain Member */
    public final static int SV_TYPE_DOMAIN_MEMBER	= 0x00000100;
    /** Server sharing print queue */
    public final static int SV_TYPE_PRINTQ_SERVER	= 0x00000200;
    /** Server running dialin service. */
    public final static int SV_TYPE_DIALIN_SERVER	= 0x00000400;
    /** Xenix server */
    public final static int SV_TYPE_XENIX_SERVER	= 0x00000800;
    /** NT server */
    public final static int SV_TYPE_NT	            = 0x00001000;
    /** Server running Windows for Workgroups */
    public final static int SV_TYPE_WFW	            = 0x00002000;
    /** Windows NT non DC server */
    public final static int SV_TYPE_SERVER_NT	    =  0x00008000;
    /** Server that can run the browser service */
    public final static int SV_TYPE_POTENTIAL_BROWSER=  0x00010000;
    /** Backup browser server */
    public final static int SV_TYPE_BACKUP_BROWSER	=  0x00020000;
    /**  Master browser server*/
    public final static int SV_TYPE_MASTER_BROWSER	=  0x00040000;
    /** EDomain Master Browser server */
    public final static int SV_TYPE_DOMAIN_MASTER	=  0x00080000;
    /**  Enumerate only entries marked "local" */
    public final static int SV_TYPE_LOCAL_LIST_ONLY = 0x40000000;
    /** Enumerate Domains */
    public final static int SV_TYPE_DOMAIN_ENUM	    = 0x80000000	;


    String fComputerName;
    int    fMajorVersion;
    int    fMinorVersion;
    int    fType;
    String fComment;

    /**
     * the name of a computer
     */
    public String getComputerName() {
        return fComputerName;
    }
    /**
     * If the getType() indicates that the entry is for a domain,
     * this specifies the name of the domain master browser;
     * otherwise, it specifies a comment describing the server.
     * The comment can be a null string
     */
    public String getComment() {
        return fComment;
    }
    /**
     * the type of software the computer is running (see SW_*)
     */
    public int getType() {
        return fType;
    }
    /**
     * the major version number of the networking
     * software the workstation is running.
     */
    public int getMajorVersion() {
        return fMajorVersion;
    }
    /**
     * the minor version number of the networking
     * software the workstation is running.
     */
    public int getMinorVersion() {
        return fMinorVersion;
    }

}