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


package org.gnu.jcifs.www.cifs;

import org.gnu.jcifs.util.*;
import org.gnu.jcifs.*;


import java.io.*;
import java.net.*;

import java.util.*;
/**
  Protocol handler for the "cifs" protocol.
  @since 1.1
 
 */
public  class Handler extends URLStreamHandler {
	
	/**
     * Opens a connection to the object referenced by the 
     * <code>URL</code> argument. 
     * @param      u   the URL that this connects to.
     *
     * @return a <code>URLConnection</code> object for the
     * <code>URL</code>.
     *
     * @exception  IOException  if an I/O error occurs while opening the
     *               connection.
     */
     protected URLConnection openConnection(URL u) throws IOException {
     	return new CifsURLConnection(u);
     }


    /**
     * Parses the string representation of a <code>URL</code> into a
     * <code>URL</code> object.
     * <p>
     * If there is any inherited context, then it has already been
     * copied into the <code>URL</code> argument.
     * <p>
     * The <code>parseURL</code> method of <code>URLStreamHandler</code>
     * parses the string representation as if it were an
     * <code>http</code> specification. Most URL protocol families have a
     * similar parsing. A stream protocol handler for a protocol that has
     * a different syntax must override this routine.
     *
     * @param   u       the <code>URL</code> to receive the result of parsing
     *                  the spec.
     * @param   spec    the <code>String</code> representing the URL that
     *                  must be parsed.
     * @param   start   the character index at which to begin parsing. This is
     *                  just past the '<code>:</code>' (if there is one) that
     *                  specifies the determination of the protocol name.
     * @param   limit   the character position to stop parsing at. This is the
     *                  end of the string or the position of the
     *                  "<code>#</code>" character, if present. All information
     *                  after the sharp sign indicates an anchor.
     */

	protected void parseURL(URL url, String spec, int start, int limit)  {
        
        String host = null;
        String user = null;
        String pwd  = null;
       
       
        spec = spec.substring(start + 2, limit);
        
        int p1 = spec.indexOf('/');
        
        if (p1 < 0) 
        	throw new NullPointerException("No share name");

		String part1 = spec.substring(0,p1);
		String part2 = spec.substring(p1);
        
        int p2 = part1.indexOf('@');
        
        if (p2 < 0)
        	host = part1;
       	else {
       		host          = part1.substring(p2+1);
       		String auth   = part1.substring(0,p2);
       		int p3        = auth.indexOf(':');
       		
       		if (p3 > 0) {
       			user = auth.substring(0,p3);
       			pwd  = auth.substring(p3+1);
       		} else
       			pwd  = auth;
       	}
       				
       
        setURL(url, "cifs", part1, -1, part2, null);
        
    }
    
  

}