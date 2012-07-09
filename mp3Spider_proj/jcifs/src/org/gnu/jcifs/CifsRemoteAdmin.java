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
  *----------------------------------------------------------------
  *  Author: Norbert Hranitzky
  *  Email : norbert.hranitzky@mchp.siemens.de
  *  Web   : http://www.hranitzky.purespace.de
  */

package org.gnu.jcifs;

import org.gnu.jcifs.util.*;

import java.io.*;
import java.util.*;
import java.text.*;

/*

 The ***parameters section** of the Transact SMB request contains thefollowing
 (in the order described)
 - The function number: an unsigned short 16 bit integer identifying the
   function being remoted
 - The parameter descriptor string: a null terminated ASCII string
 - The data descriptor string: a null terminated ASCII string.
 - The request parameters, as described by the parameter descriptor  string,
   in the order that the request parameter descriptor characters  appear in
   the parameter descriptor string
 - An optional auxiliary data descriptor string:  a null terminated ASCII  string.
   It will be present if there is an auxiliary data structure  count in the primary
   data struct (an "N" descriptor in the data  descriptor string).RAP requires
   that the length of the return parameters be less than orequal to the length
   of the parameters being sent; this requirement ismade to simply buffer management
   in implementations. This is reasonableas the functions were designed to return
   data in the data section anduse the return parameters for items like data length,
   handles, etc. Ifneed be, this restriction can be circumvented by filling in
   some padbytes into the parameters being sent.

   The Data section for the transaction request is present if the parameter description
   string contains an "s" (SENDBUF) descriptor. If present, itcontains:
 - A primary data struct, as described by the data descriptor string

 - Zero or more instances of the auxiliary data struct, as described by
   the auxiliary data descriptor string. The number of instances is  determined
   by the value of the an auxiliary data structure count  member of the primary
   data struct, indicated by the "N" (AUXCOUNT)  descriptor. The auxiliary data
   is present only if the auxiliary data  descriptor string is non null.

 - Possibly some pad bytes
 - The heap: the data referenced by pointers in the primary and  auxiliary data structs.


 */

/**
 * The Remote Administration Protocol (RAP) provides operations<p>
 * - to get list of share names;<br>
 * - to get user informations;<br>
 * - to get workstation informations;<br>
 * - to get informations about print jobs;<br>
 * - to manage print jobs.<p>
 *
 * @author  Norbert Hranitzky
 * @version 1.0, 20 Nov 1998
 * @since   1.0
 */
public interface CifsRemoteAdmin extends CifsSession {




    /**
     * Returns the list of shares on the computer
     * @param sort if true the names are sorted
     * @return  list of <code>CifsShareInfo</code> objects
     * @exception  IOException  if an I/O error occurs.
     */
    public  CifsShareInfo[] listSharesInfo(boolean sort) throws IOException ;


    /**
     * Returns detailed information about a workstation.
     * @exception  IOException  if an I/O error occurs.
     */
    public CifsWorkstationInfo getWorkstationInfo() throws IOException ;


    /**
     * Returns detailed information about a particular user
     * @param user user name
     * @return user inforations
     * @exception  IOException  if an I/O error occurs.
     */

    public CifsUserInfo getUserInfo(String user) throws IOException;

    /**
     * @exception  IOException  if an I/O error occurs.
     */
    public  CifsPrinterQueueInfo[] listPrinterQueues() throws IOException ;

    /**
     * Lists print jobs in the specified printer queue
     * @param queuename printer queue name
     * @return CifsPrintJobInfo
     * @exception  IOException  if an I/O error occurs.
     */
    public  CifsPrintJobInfo getPrintJobInfo(int jobId) throws IOException ;


    /**
     * Lists print jobs in the specified printer queue
     * @param queuename printer queue name
     * @return CifsPrintJobInfo array
     * @exception  IOException  if an I/O error occurs.
     */
    public  CifsPrintJobInfo[] listPrintJobs(String queuename) throws IOException ;



    /**
     * Pauses a print job in a printer queue
     * @param jobId print job id
     * @exception  IOException  if an I/O error occurs.
     */
    public  void pausePrintJob(int jobId) throws IOException ;

    /**
     * Resumes a paused print job
     * @param jobId print job id
     * @exception  IOException  if an I/O error occurs.
     */
    public  void continuePrintJob(int jobId) throws IOException ;


    /**
     * Deletes a print job
     * @param jobId print job id
     * @exception  IOException  if an I/O error occurs.
     */
    public  void deletePrintJob(int jobId) throws IOException ;



    /**
     * Lists all computers of the specified type or types that are visible
     * in the specified domain. It may also enumerate domains.
     * @param domain the name of the workgroup in which to enumerate computers
     *               of the specified type or types. If domain is null, servers
     *               are enumerated for the current domain of the computer
     * @param types  the type or types of computer to enumerate. Computer that
     *               match at least one of the specified types are returned (SV_*)
     * @exception  IOException  if an I/O error occurs.
     */
    public CifsServerInfo[] listServersInfo(String domain, int types) throws IOException;

    /**
     * Lists all computers of the specified type or types that are visible
     * in the specified domain. It may also enumerate domains.
     * @param domain the name of the workgroup in which to enumerate computers
     *               of the specified type or types. If domain is null, servers
     *               are enumerated for the current domain of the computer
     * @param types  the type or types of computer to enumerate. Computer that
     *               match at least one of the specified types are returned (SV_*)
     * @return  <code>java.lang.String</code> (sorted)
     * @exception  IOException  if an I/O error occurs.
     */
    public  String[] listServersNames(String domain, int types) throws IOException;

    /**
     * Returns information about the current  server
     * @return server informations
     * @exception  IOException  if an I/O error occurs.
     */
    public  CifsServerInfo getServerInfo() throws IOException;

    /*
     * Changes password on the server
     * @param user user name
     * @param oldpwd old password
     * @param newpwd new password
     * @exception  IOException  if an I/O error occurs.
     */
    public  void changePassword(String user, String oldpwd, String newpwd) throws IOException;


}