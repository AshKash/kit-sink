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
import java.util.Enumeration;


public final class CifsFileSystemInfo  {

    public final static int FILE_DEVICE_BEEP	            = 0x00000001;
    public final static int FILE_DEVICE_CD_ROM	            = 0x00000002;
    public final static int FILE_DEVICE_CD_ROM_FILE_SYSTEM	= 0x00000003;
    public final static int FILE_DEVICE_CONTROLLER	        = 0x00000004;
    public final static int FILE_DEVICE_DATALINK	        = 0x00000005;
    public final static int FILE_DEVICE_DFS	                = 0x00000006;
    public final static int FILE_DEVICE_DISK	            = 0x00000007;
    public final static int FILE_DEVICE_DISK_FILE_SYSTEM	= 0x00000008;
    public final static int FILE_DEVICE_FILE_SYSTEM	        = 0x00000009;
    public final static int FILE_DEVICE_INPORT_PORT	        = 0x0000000a;
    public final static int FILE_DEVICE_KEYBOARD	        = 0x0000000b;
    public final static int FILE_DEVICE_MAILSLOT	        = 0x0000000c;
    public final static int FILE_DEVICE_MIDI_IN	            = 0x0000000d;
    public final static int FILE_DEVICE_MIDI_OUT	        = 0x0000000e;
    public final static int FILE_DEVICE_MOUSE	            = 0x0000000f;
    public final static int FILE_DEVICE_MULTI_UNC_PROVIDER	= 0x00000010;
    public final static int FILE_DEVICE_NAMED_PIPE	        = 0x00000011;
    public final static int FILE_DEVICE_NETWORK	            = 0x00000012;
    public final static int FILE_DEVICE_NETWORK_BROWSER	    = 0x00000013;
    public final static int FILE_DEVICE_NETWORK_FILE_SYSTEM	= 0x00000014;
    public final static int FILE_DEVICE_NULL	            = 0x00000015;
    public final static int FILE_DEVICE_PARALLEL_PORT	    = 0x00000016;
    public final static int FILE_DEVICE_PHYSICAL_NETCARD	= 0x00000017;
    public final static int FILE_DEVICE_PRINTER	            = 0x00000018;
    public final static int FILE_DEVICE_SCANNER	            = 0x00000019;
    public final static int FILE_DEVICE_SERIAL_MOUSE_PORT	= 0x0000001a;
    public final static int FILE_DEVICE_SERIAL_PORT	        = 0x0000001b;
    public final static int FILE_DEVICE_SCREEN	            = 0x0000001c;
    public final static int FILE_DEVICE_SOUND	            = 0x0000001d;
    public final static int FILE_DEVICE_STREAMS	            = 0x0000001e;
    public final static int FILE_DEVICE_TAPE	            = 0x0000001f;
    public final static int FILE_DEVICE_TAPE_FILE_SYSTEM	= 0x00000020;
    public final static int FILE_DEVICE_TRANSPORT	        = 0x00000021;
    public final static int FILE_DEVICE_UNKNOWN	            = 0x00000022;
    public final static int FILE_DEVICE_VIDEO	            = 0x00000023;
    public final static int FILE_DEVICE_VIRTUAL_DISK	    = 0x00000024;
    public final static int FILE_DEVICE_WAVE_IN	            = 0x00000025;
    public final static int FILE_DEVICE_WAVE_OUT	        = 0x00000026;
    public final static int FILE_DEVICE_8042_PORT	        = 0x00000027;
    public final static int FILE_DEVICE_NETWORK_REDIRECTOR	= 0x00000028;
    public final static int FILE_DEVICE_BATTERY	            = 0x00000029;
    public final static int FILE_DEVICE_BUS_EXTENDER	    = 0x0000002a;
    public final static int FILE_DEVICE_MODEM	            = 0x0000002b;
    public final static int FILE_DEVICE_VDM	                = 0x0000002c;

    /** Device characteristics */
    public final static int FILE_REMOVABLE_MEDIA	    = 0x00000001;
    public final static int FILE_READ_ONLY_DEVICE	    = 0x00000002;
    public final static int FILE_FLOPPY_DISKETTE	    = 0x00000004;
    public final static int FILE_WRITE_ONE_MEDIA	    = 0x00000008;
    public final static int FILE_REMOTE_DEVICE	        = 0x00000010;
    public final static int FILE_DEVICE_IS_MOUNTED	    = 0x00000020;
    public final static int FILE_VIRTUAL_VOLUME	        = 0x00000040;

    /** File system attributes */
    public final static int FS_ATTR_CASE_SENSITIVE_SEARCH	= 0x00000001;
    public final static int FS_ATTR_CASE_PRESERVED_NAMES	= 0x00000002;
    public final static int FS_ATTR_PERSISTENT_ACLS	        = 0x00000004;
    public final static int FS_ATTR_COMPRESSION	            = 0x00000008;
    public final static int FS_ATTR_VOLUME_QUOTAS	        = 0x00000010;
    public final static int FS_ATTR_DEVICE_IS_MOUNTED	    = 0x00000020;
    public final static int FS_ATTR_VOLUME_IS_COMPRESSED	= 0x00008000;

    // SMB_QUERY_FS_VOLUME

    boolean fQueryFSVolumeDone = false;
    long    fVolumeCreationTime = 0;
    long    fVolumeSerialNumber = 0;
    String  fVolumeLabel        = "";

    // SMB_QUERY_FS_SIZE_INFO
    boolean fQueryFSSizeDone = false;
    long    fTotalAllocUnits    = 0;
    long    fFreeAllocUnits     = 0;
    long    fSectorsPerUnit     = 0;
    long    fBytesPerSector     = 0;

    //  SMB_QUERY_FS_DEVICE_INFO
    boolean fQueryDeviceInfoDone = false;
    int     fDeviceType         = 0;
    int     fDeviceCharacteristics = 0;

    // SMB_QUERY_FS_ATTRIBUTE_INFO
    boolean fQueryFSAttrDone = false;
    int     fFSAttributes;
    int     fMaxFileComponentLength ;

    // Name of the file system
    String  fFSName;

    CifsFileSystemInfo() {
    }

    public String getVolumeLabel() {
        return fVolumeLabel;
    }
    public long getTotalAllocUnits() {
        return fTotalAllocUnits;
    }

    public boolean isSizeInfoValid() {
        return fQueryFSSizeDone;
    }

    public long getFreeAllocUnits() {
        return fFreeAllocUnits;
    }
    public long getSectorsPerUnit() {
        return fSectorsPerUnit;
    }
    public long getBytesPerSector() {
        return fBytesPerSector;
    }
    public boolean isDeviceInfoValid() {
        return fQueryDeviceInfoDone;
    }

    public int getDeviceType() {
        return fDeviceType;
    }
    public int getDeviceCharacteristics() {
        return fDeviceCharacteristics;
    }
    public String getFileSystemName() {
        return fFSName;
    }
    public int getFileSystemAttr() {
        return fFSAttributes;
    }
}