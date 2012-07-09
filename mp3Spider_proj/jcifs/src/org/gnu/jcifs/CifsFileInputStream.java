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
  *  Contact: norbert.hranitzky@mchp.siemens.de; http://www.hranitzky.purespace.de
  *  History: 1.1: incorrect read from handle
  *                
  */


package org.gnu.jcifs;

import org.gnu.jcifs.util.*;
import java.io.*;
import java.util.*;


/**
 * A <code>CifsFileInputStream</code> is an input stream for reading data
 * from a CIFS file. The default buffer
 * size is 2048. The system property <code>org.gnu.jcifs.io.bufsize</code>
 * specifies the buffer size. The implementation was borrowed from
 * BufferedInputStream.
 *
 * @version 1.1, 22 July 1999
 * @since   1.0
 */

public class CifsFileInputStream extends InputStream {

	
    /**  The buffer where data is stored. */
    private byte        fBuf[];

    /**
     * The index one greater than the index of the last valid byte in
     * the buffer.
     */
    private int         fCount = 0;

    /**
     * The current position in the buffer. This is the index of the next
     * character to be read from the <code>fBuf</code> array.
     */
    private int         fPos = 0;

    /**
     * The value of the <code>fPos</code> field at the time the last
     * <code>mark</code> method was called. The value of this field is
     * <code>-1</code> if there is no current mark.

     */
    private int         fMarkpos   = -1;
    private int         fMarklimit = 0;

    /* File handle. I contains a reference to CifsDisk object, so 
       the GC does cannot close the connection
     */  

    private FileHandle  fHandle = null;


    /**
     * Opens the given file for read
     * @param disk disk object
     * @param file name (relative to share name)
     * @exception  IOException  if an I/O error occurs.
     */
    public CifsFileInputStream(CifsDisk disk,String file) throws IOException {
        open(file,disk);
    }


    /**
     * Opens the given file for read with default buffer size
     * @param sessionname disk session name
     * @param file file name( Syntax: <code>/diskname/path</code> or <code>diskname:\path</code> )
     * @exception  IOException  if an I/O error occurs.
     */

    public CifsFileInputStream(String sessionname,String file) throws IOException {

        CifsDisk disk = CifsSessionManager.lookupConnectedDisk(sessionname);

        if (disk == null)
            throw new CifsIOException("DK2",sessionname);

        open(file,disk);

    }


    /**
     * Opens the given file for read
     * @param file file
     * @exception  IOException  if an I/O error occurs.
     */
    public CifsFileInputStream(CifsFile  file) throws IOException {

        open(file.getPathName(),file.getDisk());
    }


    protected void open(String file,CifsDisk disk) throws IOException {
    
        fBuf    = new byte[CifsFile.getIOBufferSize()];
        fHandle = ((DiskImpl)disk).openFile(file,DiskImpl.O_RDONLY,CifsDisk.SM_DENY_NONE);
    }


    /**
     * Returns the file size
     * @return file size
     */
    public long length() {
        return fHandle.fFileSize;
    }

    /**
     * Returns the disk on which the file resides
     * @return CifsDisk disk object
     * @exception  IOException  if file is closed
     */
    public CifsDisk getDisk() throws IOException  {
        ensureOpen();

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
     * Fills the buffer with more data, taking into account
     * shuffling and other tricks for dealing with marks.
     * Assumes that it is being called by a synchronized method.
     * This method also assumes that all data has already been read in,
     * hence fPos > fCount.
     */
    private void fill() throws IOException {
	    if (fMarkpos < 0)
	        fPos = 0;		/* no mark: throw away the buffer */
	    else if (fPos >= fBuf.length)	/* no room left in buffer */
	        if (fMarkpos > 0) {	/* can throw away early part of the buffer */
		        int sz = fPos - fMarkpos;
		        System.arraycopy(fBuf, fMarkpos, fBuf, 0, sz);
		        fPos     = sz;
		        fMarkpos = 0;
	        } else if (fBuf.length >= fMarklimit) {
		        fMarkpos = -1;	/* buffer got too big, invalidate mark */
		        fPos     = 0;	/* drop buffer contents */
	        } else {		/* grow buffer */
		        int nsz = fPos * 2;
		        if (nsz > fMarklimit)
		            nsz = fMarklimit;
		        byte nbuf[] = new byte[nsz];
		        System.arraycopy(fBuf, 0, nbuf, 0, fPos);
		        fBuf = nbuf;
	    }
	    int n = fHandle.read(fBuf, fPos, fBuf.length - fPos);
	    fCount = n <= 0 ? fPos : n + fPos;
    }

    /**
     * Reads the next byte of data from this file. The
     * value byte is returned as an <code>int</code> in the range
     * <code>0</code> to <code>255</code>. If no byte is available
     * because the end of the stream has been reached, the value
     * <code>-1</code> is returned. This method blocks until input data
     * is available, the end of the stream is detected, or an exception
     * is thrown.
     * <p>
     * The <code>read</code> method of <code>CifsFileInputStream</code>
     * returns the next byte of data from its buffer if the buffer is not
     * empty. Otherwise, it refills the buffer from the underlying input
     * stream and returns the next character, if the underlying stream
     * has not returned an end-of-stream indicator.
     *
     * @return     the next byte of data, or <code>-1</code> if the end of the
     *             stream is reached.
     * @exception  IOException  if an I/O error occurs.
     */
    public synchronized int read() throws IOException {
        ensureOpen();

	    if (fPos >= fCount) {
	        fill();
	        if (fPos >= fCount)
		        return -1;
	    }
	    return fBuf[fPos++] & 0xff;
    }

    /**
     * Reads bytes into a portion of an array.  This method will block until
     * some input is available, an I/O error occurs, or the end of the stream
     * is reached.
     *
     * <p> If this stream's buffer is not empty, bytes are copied from it into
     * the array argument.  Otherwise, the buffer is refilled from the
     * underlying input stream and, unless the stream returns an end-of-stream
     * indication, the array argument is filled with characters from the
     * newly-filled buffer.
     *
     * <p> As an optimization, if the buffer is empty, the mark is not valid,
     * and <code>len</code> is at least as large as the buffer, then this
     * method will read directly from the underlying stream into the given
     * array.  Thus redundant <code>CifsFileInputStream</code>s will not copy
     * data unnecessarily.
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

	    int avail = fCount - fPos;
	    if (avail <= 0) {
	        /* If the requested length is larger than the buffer, and if there
	        is no mark/reset activity, do not bother to copy the bytes into
	        the local buffer.  In this way buffered streams will cascade
	        harmlessly. */
	        if (len >= fBuf.length && fMarkpos < 0) {
	        	int r = fHandle.read(b, off, len);

		        return (r > 0 ? r : -1);
	        }
	        fill();
	        avail = fCount - fPos;
	        if (avail <= 0) 
		        return -1;
	        
	    }
	    int cnt = (avail < len) ? avail : len;
	    System.arraycopy(fBuf, fPos, b, off, cnt);
	    fPos += cnt;
	    

	    return cnt;
    }

    /**
     * Skips over and discards <code>n</code> bytes of data from the
     * input stream. The <code>skip</code> method may, for a variety of
     * reasons, end up skipping over some smaller number of bytes,
     * possibly zero. The actual number of bytes skipped is returned.
     * <p>
     * The <code>skip</code> method of <code>CifsFileInputStream</code>
     * compares the number of bytes it has available in its buffer,
     * <i>k</i>, where <i>k</i>&nbsp;= <code>fCount&nbsp;- fPos</code>,
     * with <code>n</code>. If <code>n</code>&nbsp;&le;&nbsp;<i>k</i>,
     * then the <code>fPos</code> field is incremented by <code>n</code>.
     * Otherwise, the <code>fPos</code> field is incremented to have the
     * value <code>fCount</code>, and the remaining bytes are skipped by
     * calling the <code>skip</code> method on the underlying input
     * stream, supplying the argument <code>n&nbsp;-</code>&nbsp;<i>k</i>.
     *
     * @param      n   the number of bytes to be skipped.
     * @return     the actual number of bytes skipped.
     * @exception  IOException  if an I/O error occurs.
     */
    public synchronized long skip(long n) throws IOException {
    	ensureOpen();
     

	    if (n < 0) {
	        return 0;
	    }
	    long avail = fCount - fPos;
     
        if (avail <= 0) {
            // If no mark position set then don't keep in buffer
            if (fMarkpos <0) 
                return fHandle.skip(n);
            
            // Fill in buffer to save bytes for reset
            fill();
            avail = fCount - fPos;
            if (avail <= 0)
                return 0;
        }
        
        long skipped = (avail < n) ? avail : n;
        fPos += skipped;
        return skipped;

    }

    /**
     * Returns the number of bytes that can be read from this input
     * stream without blocking.
     * <p>
     * The <code>available</code> method of
     * <code>CifsFileInputStream</code> returns the sum of the the number
     * of bytes remaining to be read in the buffer
     * (<code>fCount&nbsp;- fPos</code>)
     * and the result of calling the <code>available</code> method of the
     * underlying input stream.
     *
     * @return     the number of bytes that can be read from this input
     *             stream without blocking.
     * @exception  IOException  if an I/O error occurs.
     */
    public synchronized int available() throws IOException {
	    return (fCount - fPos) ; // + in.available();
    }

    /**
     * Marks the current position in this input stream. A subsequent
     * call to the <code>reset</code> method repositions the stream at
     * the last marked position so that subsequent reads re-read the same
     * bytes.
     * <p>
     * The <code>readlimit</code> argument tells the input stream to
     * allow that many bytes to be read before the mark position gets
     * invalidated.
     *
     * @param   readlimit   the maximum limit of bytes that can be read before
     *                      the mark position becomes invalid.
     */
    public synchronized void mark(int readlimit) {
	    fMarklimit = readlimit;
	    fMarkpos   = fPos;
    }

    /**
     * Repositions this stream to the position at the time the
     * <code>mark</code> method was last called on this input stream.
     * <p>
     * If the stream has not been marked, or if the mark has been invalidated,
     * an IOException is thrown. Stream marks are intended to be used in
     * situations where you need to read ahead a little to see what's in
     * the stream. Often this is most easily done by invoking some
     * general parser. If the stream is of the type handled by the
     * parser, it just chugs along happily. If the stream is not of
     * that type, the parser should toss an exception when it fails. If an
     * exception gets tossed within readlimit bytes, the parser will allow the
     * outer code to reset the stream and to try another parser.
     *
     * @exception  IOException  if this stream has not been marked or
     *               if the mark has been invalidated.
     */
     public synchronized void reset() throws IOException {
	    if (fMarkpos < 0)
	        throw new CifsIOException("FS0");
	    fPos = fMarkpos;
    }

    /**
     * Tests if this input stream supports the <code>mark</code>
     * and <code>reset</code> methods. The <code>markSupported</code>
     * method of <code>CifsFileInputStream</code> returns
     * <code>true</code>.
     *
     * @return  a <code>boolean</code> indicating if this stream type supports
     *          the <code>mark</code> and <code>reset</code> methods.

     */
    public boolean markSupported() {
	    return true;
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