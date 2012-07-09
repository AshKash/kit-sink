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
 * The class implements a CIFS print output stream.
 * The data is written into a buffer,
 * and then written to the underlying stream if the buffer reaches
 * its capacity, the buffer output stream is closed, or the buffer
 * output stream is explicity flushed. The default buffer
 * size is 2048. The system property <code>org.gnu.jcifs.io.bufsize</code>
 * specifies the buffer size.
 *
 * @author  Norbert Hranitzky
 * @version 1.0, 20 Nov 1998
 * @since   1.0
 */
public class CifsPrintOutputStream extends OutputStream {


    int             fFID        = 0;
    PrinterImpl     fPrinter    = null;
    private byte    fBuf[];

    //The number of valid bytes in the buffer.
    private int   fCount;


    /**
     * Creates a file output stream to write to the specified file
     * @param disk disk
     * @param file relativ to share
     * @param append if true append data otherwise overwrite
     * @exception  IOException  if an I/O error occurs.
     */
    CifsPrintOutputStream(CifsPrinter printer,String printid, int fid) throws IOException {
        fPrinter = (PrinterImpl)printer;
        fFID     = fid;
        fBuf     = new byte[CifsFile.getIOBufferSize()];
    }



    /** Flush the internal buffer */
    private void flushBuffer() throws IOException {
         if (fPrinter == null)
            throw new CifsIOException("FS1");

        if (fCount > 0) {
	        fPrinter.writePrintFile(fFID,fBuf, 0, fCount);
	        fCount = 0;
        }
    }

    /**
     * Writes the specified byte to this buffered output stream.
     *
     * @param      b   the byte to be written.
     * @exception  IOException  if an I/O error occurs.
     */
    public synchronized void write(int b) throws IOException {
	   writeByte(b);
    }

    private final void writeByte(int b) throws IOException {
	    if (fCount >= fBuf.length) {
	     flushBuffer();
	    }
	    fBuf[fCount++] = (byte)b;
    }

    /**
     * Writes <code>len</code> bytes from the specified byte array
     * starting at offset <code>off</code> to this buffered output stream.
     *
     * <p> Ordinarily this method stores bytes from the given array into this
     * stream's buffer, flushing the buffer to the underlying output stream as
     * needed.  If the requested length is at least as large as this stream's
     * buffer, however, then this method will flush the buffer and write the
     * bytes directly to the underlying output stream.
     * @param      b     the data.
     * @param      off   the start offset in the data.
     * @param      len   the number of bytes to write.
     * @exception  IOException  if an I/O error occurs.
     */
    public synchronized void write(byte b[], int off, int len) throws IOException {
        if (fPrinter == null)
            throw new CifsIOException("FS1");

	    if (len >= fBuf.length) {
	        /* If the request length exceeds the size of the output buffer,
    	       flush the output buffer and then write the data directly.
    	       In this way buffered streams will cascade harmlessly. */
	        flushBuffer();
	        fPrinter.writePrintFile(fFID,b, off, len);
	        return;
	    }
	    if (len > fBuf.length - fCount) {
	        flushBuffer();
	    }
	    System.arraycopy(b, off, fBuf, fCount, len);
	    fCount += len;
    }

    public synchronized long write(InputStream in) throws IOException {
        long written = 0;
        while(true) {
            if (fCount >= fBuf.length) {
	            flushBuffer();
	        }

            int r = in.read(fBuf,fCount,fBuf.length - fCount);
            if (r == -1)
                return written;
            fCount  += r;
            written += r;
        }
    }

    public  synchronized long write(Reader in) throws IOException {

        int c;
        long written = 0;
        while( (c = in.read()) != -1) {
            writeByte(c);
            written++;
        }
        return written;
    }

    /**
     * Flushes this buffered output stream. This forces any buffered
     * output bytes to be written out to the underlying output stream.
     *
     * @exception  IOException  if an I/O error occurs.
     */
    public synchronized void flush() throws IOException {
        flushBuffer();

    }

    /**
     * Closes this output stream and releases any system resources
     * associated with the stream.

     */
    public void close() throws IOException {
	    try {
	        flush();
	    } catch (IOException ignored) {
	    } finally {
	        fPrinter.closePrintFile(fFID);
	        fPrinter = null;
	    }
    }

    public void finalize() {
        if (fPrinter != null) {

            try {
                close();
            } catch(Throwable t) {
            }
            fPrinter = null;
        }
    }
}