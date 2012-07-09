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

import java.util.Date;

public final class CifsUserInfo  {

    /** User is  guest */
    public final static int USER_PRIV_GUEST  = 0 ;

    /** User privilege  */
    public final static int USER_PRIV_USER   = 1;

    /** User has Administrator privilege */
    public final static int USER_PRIV_ADMIN  = 2 ;


    /** Print operator */
    public final static int AF_OP_PRINT      = 0 ;

    /** Communications operator */
    public final static int AF_OP_COMM       = 1 ;

    /** Server operator */
    public final static int AF_OP_SERVER     = 2 ;

    /**  Accounts operator */
    public final static int AF_OP_ACCOUNTS   = 3 ;


    String fUserName;
    String fComment;
    String fUserComment;
    String fUserFullName;
    int    fUserPriv;
    int    fOperatorPriv;
    int    fPasswordAge;
    String fHomeDir;
    long   fLastLogon;
    long   fLastLogoff;
    int    fBadLogons;
    int    fNumLogons;
    String fLogonServer;
    int    fCountryCode;


    /**
     * Protected constructor
     */
    CifsUserInfo() {

    }

    /**
     * Gets the user name for which information is retrieved
     * @return user name
     */
    public String getUserName() {
        return fUserName;
    }

    /**
     * Gets comment
     * @return comment
     */
    public String getComment() {
        return fComment;
    }

    /**
     * Gets comment about the user
     * @return comment
     */
    public String getUserComment() {
        return fUserComment;
    }
    /**
     * Gets the full name  of the user
     * @return full user name
     */
    public String getFullUserName() {
        return fUserFullName;
    }
    /**
     * Gets the level of the privilege assigned to the user
     * @return privilege (see USER_*)
     */
    public int getUserPrivilege() {
        return fUserPriv;
    }
    /**
     * Gets the account operator privileges.
     * @return privilege (see AF_OP_*)
     */
    public int getOperatorPrivileges() {
        return fOperatorPriv;
    }
    /**
     * Gets how many seconds have elapsed since the password was last changed
     * @return number of seconds
     */
    public int getPasswordAge() {
        return fPasswordAge;
    }
    /**
     * Gets the path name of the user's home directory
     * @return home directory
     */
    public String getHomeDir() {
        return fHomeDir;
    }
    /**
     * Gets the time when the user last logged on
     * @return date or null if last logon is unknown
     */
    public Date getLastLogon() {
        if (fLastLogon == 0)
            return null;
        return new Date(fLastLogon);
    }
    /**
     * Gets the time when the user last logged off.
     * A value of null means the last logoff time is unknown.
     * @return date
     */
    public Date getLastLogoff() {
        if (fLastLogoff == 0)
            return null;
        return new Date(fLastLogoff);
    }
    /**
     * Gets the number of incorrect passwords entered since the last successful logon
     * @return bad logon counter
     */
    public int getBadLogons() {
        return fBadLogons;
    }
    /**
     * Gets the number of times this user has logged on.
     * A value of -1 means the number of logons is unknown
     * @return logon counter
     */
    public int getLogons() {
        return fNumLogons;
    }
    /**
     * Gets the name of the server to which logon requests are sent.
     * A null string indicates logon requests should be sent to the
     * domain controller
     * @return server name
     */
    public String getLogonServer() {
        return fLogonServer;
    }
    /**
     * Gets the country code for the user's language  of choice.
     * @return country code
     */
    public int getCountryCode() {
        return fCountryCode;
    }


}