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
import java.util.*;



class FileInfoBuffer extends MarshalBuffer  {

    private short fInfoLevel;
    private int  fSearchAttr;
    private int  fLastOffset   = 0;

    private boolean fHasNext = false;
    private int fCount       = 0;

    FileInfoBuffer(int cap) {
        super(cap);
    }

    final void setSearchAttr(int attr) {
        fSearchAttr = attr;
    }

    final int getSearchAttr() {
        return fSearchAttr;
    }

    final void setInfoLevel( short level) {
        fInfoLevel = level;

        fHasNext = (fInfoLevel != SMBMessage.SMB_INFO_STANDARD &&
                    fInfoLevel != SMBMessage.SMB_INFO_QUERY_EA_SIZE &&
                    fInfoLevel != SMBMessage.SMB_INFO_QUERY_EAS_FROM_LIST);
    }

    void setCount(int count) {

        if (fHasNext)
            fLastOffset = getLast(this,count);

        fCount += count;

    }

    int getCount() {
        return fCount;
    }

    short getInformationLevel() {
        return fInfoLevel;
    }

    void append(MarshalBuffer buf, int count) {



        if (buf.fSize + fSize > fBuffer.length) {
            byte[] nbuf = new byte[buf.fSize + fSize + 100];
            System.arraycopy(fBuffer,0,nbuf,0,fSize);
            fBuffer = nbuf;
        }
        System.arraycopy(buf.fBuffer,0,fBuffer,fSize,buf.fSize);

        if (fHasNext) {
            setIntAt(fLastOffset,fSize - fLastOffset);
            fLastOffset = fSize + getLast(buf,count);
        }
        fCount += count;
        fSize  += buf.fSize;

    }

    private static int getLast(MarshalBuffer buf,int count) {

        if (count == 0)
            return 0;

        int pos = 0;
        int last = 0;

        for(int i=0;i<count;i++) {
            last = pos;
            pos += buf.getIntAt(pos);

        }

        return last;
    }

    void debug(String title) {
        if (Debug.debugOn) {
            Debug.println(Debug.BUFFER,title);
            Debug.println(Debug.BUFFER,"Count= " + fCount);
            super.debug("  --- Data ---");
        }
    }
}