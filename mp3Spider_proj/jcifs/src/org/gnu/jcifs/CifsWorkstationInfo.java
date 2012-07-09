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
 * Informations about the workstation
 *
 * @see CifsRemoteAdmin#getWorkstationInfo
 *
 * @author  Norbert Hranitzky
 * @version 1.0, 21 Nov 1998
 * @since   1.0
 */
public final class CifsWorkstationInfo  {

    String fWorkstationName;
    String fUserName;
    String fDomain;
    int    fMajorVersion;
    int    fMinorVersion;
    String fLogonDomain;
    String fAllDomains;



    CifsWorkstationInfo() {

    }

    /**
     * Returns the name of the workstation
     * @return the name of the workstation
     */
    public String getWorkstationName() {
        return fWorkstationName;
    }
    /**
     * Returns user who is logged on at the workstation
     * @return user name
     */
    public String getUserName() {
        return fUserName;
    }
    /**
     * Returns the domain to which the workstation belongs
     * @return domain name
     */
    public String getDomain() {
        return fDomain;
    }
    /**
     * Returns the major version number of the networking
     * software the workstation is running.
     * @return major version number
     */
    public int getMajorVersion() {
        return fMajorVersion;
    }
    /**
     * Returns the minor version number of the networking
     * software the workstation is running.
     * @return minor version number
     */
    public int getMinorVersion() {
        return fMinorVersion;
    }

    /**
     * Returns the domain for which a user is logged on
     * @return domain name
     */
    public String getLogonDomain() {
        return fLogonDomain;
    }
    /**
     * Returns all domains in which the computer is enlisted
     * @return domain list
     */
    public String getAllDomains() {
        return fAllDomains;
    }

}