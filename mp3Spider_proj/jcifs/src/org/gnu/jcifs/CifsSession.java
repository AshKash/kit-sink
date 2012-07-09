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
import java.net.InetAddress;

/**
 * Interface of all service sessions
 * @author  Norbert Hranitzky
 * @version 1.0, 20 Nov 1998
 * @since   1.0
 */
public interface  CifsSession  {



    /**
     * Sets automatic reconnection
     * @param on true if automatic reconnection allowed
     */
     public void setAllowAutoReconnection(boolean on);


    /**
     * Returns the name of this session
     * @return session name
     */
    public  String getShareName () ;

    /**
     * Returns share name
     * @reteurn share name
     */
    public  String getSessionName ();

    /**
     * Returns server OS name
     * @return os name or blank if unknown
     */
    public  String getServerOS() ;

    /**
     * Returns LAN Manager of the server
     * @return LAN Manager or blank if unknown
     */
    public  String getServerLanMan() ;


    /**
     * Returns the primary domain of the server
     * @return primary domain or blank if unknown
     */
    public  String getServerPrimaryDomain() ;

    /**
     * Gets NetBIOS name
     * @return NetBIOS name of the server
     */
    public  String getNetBIOSName() ;

    /**
     * Gets the address of the server
     * @return InetAddress address
     */
    public  InetAddress getServerAddress();

    /**
     * Time zone of server (min from UTC)
     * @return minutes
     */
    public  int getServerTimeZone() ;

    /**
     * Returns the server time (from 1970 in msec)
     * @return msec
     */
    public  long getServerTime() ;

    /**
     * Checks if the server is connected
     * @return true if the connection is alive
     */
    public  boolean isConnected() ;

    /**
     * Sets an API-user property. The value is not interpreted by
     * CifsService
     * @param key property name
     * @param value property value;
     * @see #getProperty(String)
     */
    public  void setProperty(String key, Object value) ;

    /**
     * Gets an API-user property
     * @param key property name
     * @return  property value;
     * @see #setProperty(String)
     */
    public  Object getProperty(String key) ;

    /**
     * Returns true if the share has user level security
     * @return true user level, false share level
     */
    public  boolean isUserLevelSecurity() ;

    /**
     * Returns the connect time in milliseconds (base:  January 1, 1970 UTC )
     * @return time in milliseconds
     */

     public  long getConnectTime() ;

    /**
     * Reconnects server if disconnected
     * @exception IOException  if an I/O error occurs
     */
    public  void reconnect() throws IOException ;


    /**
     * Disconnects session
     */
    public void  disconnect();


    /**
     * Ping the server to test the connection to the server and to
     * see if the server is still responding
     * @param  text text to send
     * @return text returned by server (must be the same as the input text)
     * @exception  IOException  if an I/O error occurs.
     */
    public  String echo(String text) throws IOException ;

}