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
  *  Date  : 1999-06-20
  *  Email : norbert.hranitzky@mchp.siemens.de
  *  Web   : http://www.hranitzky.purespace.de
  */


package org.gnu.jcifs.www.cifs;

import org.gnu.jcifs.util.*;
import org.gnu.jcifs.*;


import java.io.*;
import java.net.*;

import java.util.*;
/**
 <code>CifsURLConnection</code> implements the "cifs" protocol connection. 
 The URL syntax is:<p>
 <code>
 cifs://[user[:password]@]host/path
 </code>
 @since 1.1
 */
public  class CifsURLConnection extends URLConnection {
	
	private CifsDisk     fDisk = null;
	private CifsFileInfo fInfo = null;
	private String       fFile = null;
	private String       fContentType = null;
	private InputStream  fIn = null;
	
	protected CifsURLConnection(URL url) {
		super(url);
    }

	/**
     * Opens a communications link to the CIFS file referenced by this 
     * URL, if such a connection has not already been established. 
     * <p>
     * If the <code>connect</code> method is called when the connection 
     * has already been opened (indicated by the <code>connected</code> 
     * field having the value <code>true</code>), the call is ignored. 
     * <p>
     * CifsURLConnection objects go through two phases: first they are
     * created, then they are connected.  After being created, and
     * before being connected, various options can be specified
     * (e.g., doInput).  After connecting, it is an
     * error to try to set them.  Operations that depend on being
     * connected, like getContentLength, will implicitly perform the
     * connection, if necessary.
     *
     * @exception  IOException  if an I/O error occurs while opening the
     *               connection.
     */
    public void connect() throws IOException {
    	if(connected)
            return;
            
        String host   = null;
  		String share  = null;
  		String user   = null;
  		String pwd    = null;
  	 	// get share name 
  	 	String part1 = url.getHost();
  		String part2 = url.getFile();
  		
        // get user and password
        int p = part1.indexOf('@');
        
        if (p < 0)
        	host = part1;
       	else {
       		host          = part1.substring(p+1);
       		String auth   = part1.substring(0,p);
       		p             = auth.indexOf(':');
       		
       		if (p > 0) {
       			user = auth.substring(0,p);
       			pwd  = auth.substring(p+1);
       		} else
       			pwd  = auth;
       	}
		
  		// get share and file name
  	
  		if (part2 == null)
  			throw new CifsShareNameException("MN1",url.toString());
  		
  		if (part2.startsWith("/"))
  			part2 = part2.substring(1);
  				
  		p = part2.indexOf('/');
  		
  		if (p < 0)
  			throw new CifsShareNameException("MN1",url.toString());

		fFile = part2.substring(p+1);
		share = part2.substring(0,p);
		
	
        if (Debug.debugOn & Debug.debugLevel >= Debug.INFO) {
           Debug.println(Debug.INFO,"URLConnection: Share=" + share);
           Debug.println(Debug.INFO,"URLConnection: File=" + fFile);
           Debug.println(Debug.INFO,"URLConnection: User=" + user);
		   Debug.println(Debug.INFO,"URLConnection: Pass=" + pwd);
		   Debug.println(Debug.INFO,"URLConnection: Host=" + host);


        }
        

		host = Util.decode(host);
		share = Util.decode(share);
		if (user != null)
			user = Util.decode(user);
        if (pwd != null)
			pwd = Util.decode(pwd);
		fFile = Util.decode(fFile);

	
        CifsLogin login;
        
        if (user != null || pwd != null)
        	login = new CifsLogin(user,pwd);
        else
        	login = new CifsLogin();

        String usn = CifsSessionManager.createUSN();
        
        String uri = "cifs://" + url.getHost() + "/" + share;

        fDisk = CifsSessionManager.connectDisk(usn,uri,login);
        
        
    }
     /**
     * Returns an input stream that reads from this open connection.
     *
     * @return     an input stream that reads from this open connection.
     * @exception  IOException              if an I/O error occurs while
     *               creating the input stream.
     */

    public InputStream getInputStream() throws IOException {
    	
        if(!doInput)
        	throw new CifsIOException("WWW1");

		if (fIn != null)
            return fIn;

        if(!connected)
            connect();
        
        fIn = new CifsFileInputStream(fDisk,fFile);
        
        fContentType = super.guessContentTypeFromName(fFile);
        if(fContentType == null && fIn.markSupported())
        	fContentType = super.guessContentTypeFromStream(fIn);
        
      	return fIn;      
    }

	/**
     * Returns an output stream that writes to this connection. The method
     * <code>setDoOutput(true)</code> must be called to allow writing. 
     *
     * @return     an output stream that writes to this connection.
     * @exception  IOException              if an I/O error occurs while
     *               creating the output stream.
     */
    public OutputStream getOutputStream() throws IOException {
    	if(!doOutput)
        	throw new CifsIOException("WWW2");

    	if (fIn != null)
        	throw new CifsIOException("WWW3");
            			
    	if(!connected)
            connect();
            
		return new CifsFileOutputStream(fDisk,fFile,false);
    }

	/**
     * Returns the value of the <code>content-type</code> 
     *
     * @return  the content type of the resource that the URL references,
     *          or <code>null</code> if not known.
     */
    public String getContentType() {
		return fContentType;
    }

	public int getContentLength() {
		if (fInfo != null) {
			try {
        		fInfo = fDisk.getFileInfo(fFile);
			} catch(Exception e) {
			}
		}
		if (fInfo != null) 
			return (int)fInfo.length();
		return -1;
	}
	
	
}