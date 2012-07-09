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
 * The <code>CifsRandomAccessFile</code> class allows reading and writing
 * data from and to any specified location in a file. This class does not
 * use any buffers, all read and write operations are performed remotely.
 *
 * @author  Norbert Hranitzky
 * @version 1.1, 28 Aug 1999
 * @since   1.1
 */

public class CifsRandomAccessFile {


    /** File handle */
    private FileHandle  fHandle = null;

	/**
     * Creates a random access file stream to read from, and optionally 
     * to write to, a file with the specified name.
     * <p>
     * The mode argument must either be equal to <code>"r"</code> or 
     * <code>"rw"</code>, indicating that the file is to be opened for 
     * input only or for both input and output, respectively. The 
     * write methods on this object will always throw an 
     * <code>IOException</code> if the file is opened with a mode of 
     * <code>"r"</code>. If the mode is <code>"rw"</code> and the 
     * file does not exist, then an attempt is made to create it.
     *
     * @param      disk   disk session
     * @param      file   file name
     * @param      mode   the access mode.
     * @exception  IllegalArgumentException  if the mode argument is not equal
     *               to <code>"r"</code> or to <code>"rw"</code>.
     * @exception  IOException  if an I/O error occurs.
     */

	public CifsRandomAccessFile(CifsDisk disk,String file, String mode) throws IOException {
	
	 	open(file,disk,mode);
		
	}

    /**
     * Creates a random access file stream to read from, and optionally 
     * to write to, a file with the specified name.
     * <p>
     * The mode argument must either be equal to <code>"r"</code> or 
     * <code>"rw"</code>, indicating that the file is to be opened for 
     * input only or for both input and output, respectively. The 
     * write methods on this object will always throw an 
     * <code>IOException</code> if the file is opened with a mode of 
     * <code>"r"</code>. If the mode is <code>"rw"</code> and the 
     * file does not exist, then an attempt is made to create it.
     *
     * @param      sessionname   disk session name
     * @param      file   file name
     * @param      mode   the access mode.
     * @exception  IllegalArgumentException  if the mode argument is not equal
     *               to <code>"r"</code> or to <code>"rw"</code>.
     * @exception  IOException  if an I/O error occurs.
     */

    public CifsRandomAccessFile(String sessionname,String file, String mode) throws IOException {

        CifsDisk disk = CifsSessionManager.lookupConnectedDisk(sessionname);

        if (disk == null)
            throw new CifsIOException("DK2",sessionname);

        open(file,disk,mode);

    }


    /**
     * Creates a random access file stream to read from, and optionally 
     * to write to, a file with the specified name.
     * <p>
     * The mode argument must either be equal to <code>"r"</code> or 
     * <code>"rw"</code>, indicating that the file is to be opened for 
     * input only or for both input and output, respectively. The 
     * write methods on this object will always throw an 
     * <code>IOException</code> if the file is opened with a mode of 
     * <code>"r"</code>. If the mode is <code>"rw"</code> and the 
     * file does not exist, then an attempt is made to create it.
     *
     * @param      file   file object
     * @param      mode   the access mode.
     * @exception  IllegalArgumentException  if the mode argument is not equal
     *               to <code>"r"</code> or to <code>"rw"</code>.
     * @exception  IOException  if an I/O error occurs.
     */
    public CifsRandomAccessFile(CifsFile  file, String mode) throws IOException {

        open(file.getPathName(),file.getDisk(),mode);
    }


    protected void open(String file,CifsDisk disk, String mode) throws IOException {
    	boolean rw = mode.equals("rw");
		if (!rw && !mode.equals("r"))
			throw new IllegalArgumentException(CifsRuntimeException.getMessage("FS2"));
			
    
    	int dmode;
    	if (rw)
    		// Create the file if does not exists
    		dmode = DiskImpl.O_RDWR | DiskImpl.O_CREAT;
    	else
    		dmode = DiskImpl.O_RDONLY;
        fHandle = ((DiskImpl)disk).openFile(file,dmode,CifsDisk.SM_DENY_NONE);
    }


    
	/**
     * Returns the length of this file.
     *
     * @return     the length of this file, measured in bytes.
     * @exception  IOException  if an I/O error occurs.
     */
  
    public long length() {
        return fHandle.fFileSize;
    }


 	/**
     * Sets the length of this file.
     *
     * <p> If the present length of the file as returned by the
     * <code>length</code> method is greater than the <code>newLength</code>
     * argument then the file will be truncated.  In this case, if the file
     * offset as returned by the <code>getFilePointer</code> method is greater
     * then <code>newLength</code> then after this method returns the offset
     * will be equal to <code>newLength</code>.
     *
     * <p> If the present length of the file as returned by the
     * <code>length</code> method is smaller than the <code>newLength</code>
     * argument then the file will be extended.  In this case, the contents of
     * the extended portion of the file are not defined.
     *
     * @param      newLength    The desired length of the file
     * @exception  IOException  If an I/O error occurs
     */
    public  void setLength(long newLength) throws IOException {
    	throw new InternalError("Not yet implemented");	
    	
    }

    /**
     * Returns the disk on which the file resides
     * @return CifsDisk disk object
     * @exception  IOException  if file is closed
     */
    public CifsDisk getDisk() throws IOException  {
        if (fHandle == null)
            throw new CifsIOException("FS1");

        return fHandle.getDisk();
    }

    /**
     * Closes the input file
     * @exception IOException if an I/O error occurs.
     */
    public synchronized void close() throws IOException {
        if (fHandle == null)
            return;

        try {
            fHandle.close(false);
        } finally {
            fHandle = null;
        }
    }

    /**
     * Returns the current offset in this file. 
     *
     * @return     the offset from the beginning of the file, in bytes,
     *             at which the next read or write occurs.
     * @exception  IOException  if an I/O error occurs.
     */
    public  long getFilePointer() throws IOException {
    	ensureOpen();

    	return fHandle.getOffset();
    }


    /**
     * Reads bytes from the current offset
     *
     * @param      b     destination buffer.
     * @param      off   offset at which to start storing bytes.
     * @param      len   maximum number of bytes to read.
     * @return     the number of bytes read, or <code>-1</code> if the end of
     *             the stream has been reached.
     * @exception  IOException  if an I/O error occurs.
     */
    public synchronized int read(byte b[], int off, int len) throws IOException {
        ensureOpen();
   
	    
	  	int r = fHandle.read(b, off, len);

		return (r > 0 ? r : -1);
	
    }

 	/**
     * Writes <code>len</code> bytes from the specified byte array 
     * starting at offset <code>off</code> to this file. 
     *
     * @param      b     the data.
     * @param      off   the start offset in the data.
     * @param      len   the number of bytes to write.
     * @exception  IOException  if an I/O error occurs.
     */
    public void write(byte b[], int off, int len) throws IOException {
		
		ensureOpen();

		fHandle.write(b,off,len);
    }


 	/**
     * Attempts to skip over <code>n</code> bytes of input discarding the 
     * skipped bytes. 
     * <p>
     * 
     * This method may skip over some smaller number of bytes, possibly zero. 
     * This may result from any of a number of conditions; reaching end of 
     * file before <code>n</code> bytes have been skipped is only one 
     * possibility. This method never throws an <code>EOFException</code>. 
     * The actual number of bytes skipped is returned.  If <code>n</code> 
     * is negative, no bytes are skipped.
     *
     * @param      n   the number of bytes to be skipped.
     * @return     the actual number of bytes skipped.
     * @exception  IOException  if an I/O error occurs.
     */
    public int skipBytes(int n) throws IOException {
    	ensureOpen();

        long pos;
		long len;
		long newpos; 

		if (n <= 0) {
	    	return 0;
		}
		pos = getFilePointer();
		len = length();
		newpos = pos + n;
		if (newpos > len) {
	    	newpos = len;
		}
		seek(newpos);

		/* return the actual number of bytes skipped */
		return (int) (newpos - pos);
    }

    /**
     * Sets the file-pointer offset, measured from the beginning of this 
     * file, at which the next read or write occurs.  The offset may be 
     * set beyond the end of the file. Setting the offset beyond the end 
     * of the file does not change the file length.  The file length will 
     * change only by writing after the offset has been set beyond the end 
     * of the file. 
     *
     * @param      pos   the offset position, measured in bytes from the 
     *                   beginning of the file, at which to set the file 
     *                   pointer.
     * @exception  IOException  if <code>pos</code> is less than 
     *                          <code>0</code> or if an I/O error occurs.
     */
    public  void seek(long pos) throws IOException {
    	ensureOpen();
    	
    	fHandle.setOffset(pos);
  
	}


    /**
     * Finalizes this object
     */
    public void finalize() {

        try {
            close();
        } catch(Throwable e) {
        }
    }
    
     /**
     * Check to make sure that this stream has not been closed
     * @since 1.1
     */
    private void ensureOpen() throws IOException {
		if (fHandle == null)
            throw new CifsIOException("FS1");
    }

}