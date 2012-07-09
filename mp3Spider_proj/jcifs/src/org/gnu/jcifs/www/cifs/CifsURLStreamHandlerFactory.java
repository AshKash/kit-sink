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
 * This handler must be installed by calling the following static method:
 *<pre>
 *    URL.setURLStreamHandlerFactory(new CifsURLStreamHandlerFactory ()); 
 * </pre>
 * 
 * @since 1.1
 */
public  class CifsURLStreamHandlerFactory implements URLStreamHandlerFactory {
	
	
	public URLStreamHandler createURLStreamHandler(String protocol) {
		
		if (protocol.equalsIgnoreCase("cifs"))
			return new org.gnu.jcifs.www.cifs.Handler();
		
		return null;	
	} 
	
}