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
 * <code>CifsPrinter</code> implements the printer operations.
 * <code>CifsPrinter</code> cannot be instantiated directly
 *
 * @see org.gnu.jcifs.CifsServiceManager
 * @author  Norbert Hranitzky
 * @version 1.0, 21 Nov 1998
 * @since   1.0
 */
public  interface CifsPrinter extends CifsSession {

    /** The server may optinally expand tabs to a series of spaces */
    public final static int TEXT_MODE      = 0;

    /** No conversion of data should be done by the server */
    public final static int GRAPHICS_MODE  = 1;




    /**
     * Enumerates the list of the elements currently in the printer queue
     * on the server
     * @return <code>CifsPrinterSpoolInfo</code> array
     */
    public CifsPrinterSpoolInfo[] listPrinterSpoolInfo() throws IOException ;


    /**
     * Creates a new print spool file. The setup length is 0.
     * @param printid  can be used by the server to provide some sort of per-client
     *                 identifying component to the print file
     * @param mode <code>TEXT_MODE</code> or <code>GRAPHICS_MODE</code>
     * @see #TEXT_MODE and #GRAPHICS_MODE
     */
    public CifsPrintOutputStream openPrintFile(String  printid, int mode) throws IOException ;


}