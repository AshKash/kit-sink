/**
  *         Commmon Internet File System Java API (JCIFS)
  *----------------------------------------------------------------
  *  Copyright (C) 1999  Norbert Hranitzky
  *
  *  This program is free software; you can redistribute it and/or
  *  modify it under the terms of the GNU General Public License as
  *  published by the Free Software Foundation; either version 2 of
  *  the License, or (at your option) any later version.
  *
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
import java.util.Date;


final class FileHandle {

    // File ID
    int         fFID;
    // File access attributes
    int         fFileAttributes;
    int         fLastWriteTime;
    int         fLastWriteDate;
    
    
    long        fFileSize;

    // Access permission actually allowed
    int         fGrantedAccess;

    int         fActionTaken;
    String      fFileName;

    DiskImpl    fDisk;

    long        fOffset = 0;

    boolean     fOpen = true;

    FileHandle(CifsDisk disk) {
        fDisk = (DiskImpl)disk;
    }

    int read(byte[] buf, int off, int len) throws IOException {
        int rsize = fDisk.readFile(this,fOffset,buf,off,len);
        fOffset += rsize;

        return rsize;

    }
    void write(byte[] buf, int off, int len) throws IOException {

        // check how many bytes can we send
        int maxData = fDisk.howManyBytesCanWeSend();
        int wsize ;

        while(len > 0) {
            int ssize = Math.min(len,maxData);

            wsize   = fDisk.writeFile(this,fOffset,buf,off,ssize);

            if (wsize != ssize) {
                if (Debug.debugOn)
                    Debug.println(Debug.WARNING,"We sent " + ssize + ", but server wrote " + wsize);
            }

            len     -= wsize;
            off     += wsize;
            fOffset += wsize;
        }
        
        // 1.1: update file size
        fFileSize = Math.max(fFileSize,fOffset);

    }

	/**
	 * Returns the number of skipped bytes
	 */
    long skip(long n) throws IOException {
    	long oldOffset = fOffset;
    	
        if (fOffset + n > fFileSize) {
            fOffset =  fFileSize;

        } else
            fOffset += n;

        return fOffset - oldOffset;
    }
    
    void append() {
        fOffset =  fFileSize;
    }

    void close(boolean touch) throws IOException {
        if (fOpen) {
            try {

                fDisk.closeFile(this,touch);
            } catch(IOException e) {
            } finally {
                fOpen = false;
            }
        }

    }

    CifsDisk getDisk() {
        return fDisk;
    }


	long getOffset() {
		return fOffset;
	}
	
	void setOffset(long offset) {
		fOffset = offset;
	}
}