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



public class CifsShareInfo  {

    // Share types

    /**  Disk Directory Tree */
    public final static short STYPE_DISKTREE = 0;
    /**  Printer Queue */
    public final static short STYPE_PRINTQ   = 1;
    /**  Communications device */
    public final static short STYPE_DEVICE   = 2;
    /**  Inter process communication (IPC) */
    public final static short STYPE_IPC      = 3;

    String fHostName;
    String fShareName;
    int    fShareType;
    String fRemark;

    CifsShareInfo(String host) {
        fHostName = host;
    }

    public String getShareName() {
        return fShareName;
    }

    public String getRemark() {
        return fRemark;
    }
    public int getType() {
        return fShareType;
    }

    public String getUNC() {
        return "\\\\" + fHostName + "\\" + fShareName;
    }

    public String toString() {
        StringBuffer buf = new StringBuffer();
        buf.append(fShareName);
        for(int i=fShareName.length();i<15;i++)
            buf.append(' ');
        switch(fShareType) {
            case STYPE_DISKTREE:
                buf.append("DISK   ");
                break;
            case STYPE_PRINTQ:
                buf.append("PRINT  ");
                break;
            case STYPE_DEVICE:
                buf.append("DEVICE ");
                break;
            case STYPE_IPC:
                buf.append("IPC    ");
                break;
            default:
                buf.append("?????? ");
                break;
        }
        buf.append(fRemark);
        return buf.toString();
    }
}