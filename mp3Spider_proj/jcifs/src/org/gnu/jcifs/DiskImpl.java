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
import java.text.*;

/**
 * <code>CifsDisk</code> implements the disk operations.
 * <code>CifsDisk</code> cannot be instantiated directly.
 *
 * @see CifsServiceManager#connectDisk
 * @see CifsService
 * @author  Norbert Hranitzky
 * @version 1.0, 21 Nov 1998
 * @since   1.0
 */
class DiskImpl extends SessionImpl implements CifsDisk {

    /*   Sharing modes
     * ---- ---- -SSS ----
     */

    /** Deny read/write/execute (exclusive) */
    public final static int  SM_EXCLUSIVE      = 1;
    /** Deny write */
    public final static int  SM_DENY_WRITE     = 2;
    /** Deny read/execute */
    public final static int  SM_DENY_READ_EXEC = 3;
    /** Allow all actions */
    public final static int  SM_DENY_NONE      = 4;

    /*   Access modes
     * ---- ---- ---- -AAA
     */

    /* Open for reading */
    private final static int  AM_READ           = 0;
    /* Open for writing */
    private final static int  AM_WRITE          = 1;
    /* Open for reading and writing */
    private final static int  AM_READ_WRITE     = 2;

    /*   Open function
     *  ---- ---- ---C --OO
     */

    /** Fail  if file exists */
    private final static int  OF_FAIL   = 0;
    /** Open if file exists */
    private final static int  OF_OPEN   = 1;
    /** Truncate if file exists */
    private final static int  OF_TRUNC  = 2;
    /** Create if file does not exists */
    private final static int  OF_CREATE = 1;

    /** open for reading only */
    final static int  O_RDONLY        = 0x0000;
    /** open for writing only */
    final static int  O_WRONLY        = 0x0001;
    /** open for reading and writing */
    final static int  O_RDWR          = 0x0002;
    /** create and open file */
    final static int  O_CREAT         = 0x0010;
    /** open and truncate */
    final static int  O_TRUNC         = 0x0020;
    /** open only if file doesn't already exist */
    final static int  O_EXCL          = 0x0040;


    // maximal number of search return elements
    private final static int   MAX_SEARCH_COUNT             = 512;


    // Information Level for TRANS2_QUERY_FS_INFORMATION
    private final static short SMB_INFO_ALLOCATION	        = 1;
    private final static short SMB_INFO_VOLUME	            = 2;
    private final static short SMB_QUERY_FS_VOLUME_INFO	    = 0x102;
    private final static short SMB_QUERY_FS_SIZE_INFO	    = 0x103;
    private final static short SMB_QUERY_FS_DEVICE_INFO	    = 0x104;
    private final static short SMB_QUERY_FS_ATTRIBUTE_INFO  = 0x105;


    private CifsFileSystemInfo  fFSInfo  = new CifsFileSystemInfo();
    //private String              fCurrentDir = "\\";

    private boolean             fSupports_SMB_QUERY_FS_DEVICE_INFO = true;
    private boolean             fSupports_SMB_QUERY_FS_SIZE_INFO   = true;


    protected static FileInfoComparator fFileInfoComparator = new FileInfoComparator();

    DiskImpl(String sessionname,int prot,Share share,NBTSession nbt, SMBMessage msg) throws IOException {
        super(sessionname,prot,share,nbt,msg);

    }

    /**
     * Gets the  file from the server and writes the content to the given
     * output stream. The file will not  not locked
     * @param file remote file name (relativ to share)
     * @param out  output stream
     * @return number of bytes
     * @exception  IOException  if an I/O error occurs.
     */
    public synchronized long getFile(String file, OutputStream out) throws IOException {

        if (Debug.debugOn && Debug.debugLevel >= Debug.INFO)
            Debug.println(Debug.INFO,"getFile:" + file );

        FileHandle handle = openFile(file,DiskImpl.O_RDONLY,CifsDisk.SM_DENY_NONE);

        long pos  = 0;
        long size = 0;

        int maxRead = howManyBytesCanWeSend();

        try {
            while(true) {
                readFile(handle,pos,maxRead);

                int datalen = fMsg.getShortParameterAt(10);
                int dataoff = fMsg.getShortParameterAt(12);

                if (datalen == 0)
                    return size;

                fMsg.writeTo(dataoff,out,datalen);
                size += datalen;
                pos  += datalen;
            }
        } finally {

            closeFile(handle,false);
        }

    }

    /**
     * Gets the  file from the server and writes the content to the given
     * output stream. The file will not  not locked
     * @param file remote file name (relativ to share)
     * @param out  writer
     * @return number of bytes
     * @exception  IOException  if an I/O error occurs.
     */
    public synchronized long getFile(String file, Writer out) throws IOException {

        if (Debug.debugOn && Debug.debugLevel >= Debug.INFO)
            Debug.println(Debug.INFO,"getFile:" + file );

        FileHandle handle = openFile(file,DiskImpl.O_RDONLY,CifsDisk.SM_DENY_NONE);

        long pos  = 0;
        long size = 0;

        int maxRead = howManyBytesCanWeSend();

        try {
            while(true) {
                readFile(handle,pos,maxRead);

                int datalen = fMsg.getShortParameterAt(10);
                int dataoff = fMsg.getShortParameterAt(12);

                if (datalen == 0)
                    return size;

                fMsg.writeTo(dataoff,out,datalen);
                size += datalen;
                pos  += datalen;
            }
        } finally {

            closeFile(handle,false);
        }

    }

    /**
     * Puts the data from the input stream to the given remote file.
     * The file will not  not locked
     * @param file remote file name (relativ to share)
     * @param in  input reader
     * @return number of bytes
     * @exception  IOException  if an I/O error occurs.
     */
    public synchronized long putFile(String file, Reader in) throws IOException {

        if (Debug.debugOn && Debug.debugLevel >= Debug.INFO)
            Debug.println(Debug.INFO,"putFile:" + file );

        FileHandle handle = openFile(file,O_WRONLY | O_CREAT,CifsDisk.SM_DENY_NONE);

        int  roffset  = 0;
        long woffset  = 0;
        long size = 0;

        int max = howManyBytesCanWeSend();
        char[] buf = new char[max];

        try {
            while (true) {
                int rsize = in.read(buf,0,buf.length);
                if (rsize == -1)
                    return size;

                roffset = 0;
                while(rsize > 0) {
                    int wsize = writeFile(handle,woffset,buf,roffset,rsize);
                    size    += wsize;
                    rsize   -= wsize;
                    woffset += wsize;
                    roffset += wsize;
                }
            }
        } finally {
            closeFile(handle,true);
        }

    }

    /**
     * Puts the data from the input stream to the given remote file.
     * The file will not  not locked
     * @param file remote file name (relativ to share)
     * @param in  input stream
     * @return number of bytes
     * @exception  IOException  if an I/O error occurs.
     */
    public synchronized long putFile(String file, InputStream in) throws IOException {

        if (Debug.debugOn && Debug.debugLevel >= Debug.INFO)
            Debug.println(Debug.INFO,"putFile:" + file );

        FileHandle handle = openFile(file,O_WRONLY | O_CREAT,CifsDisk.SM_DENY_NONE);

        int  roffset  = 0;
        long woffset  = 0;
        long size = 0;

        int max = howManyBytesCanWeSend();
        byte[] buf = new byte[max];

        try {
            while (true) {
                int rsize = in.read(buf,0,buf.length);
                if (rsize == -1)
                    return size;

                roffset = 0;
                while(rsize > 0) {
                    int wsize = writeFile(handle,woffset,buf,roffset,rsize);
                    size    += wsize;
                    rsize   -= wsize;
                    woffset += wsize;
                    roffset += wsize;
                }
            }
        } finally {
            closeFile(handle,true);
        }

    }
    /**
     * Deletes the given file
     * @param file remote file name
     * @exception  IOException  if an I/O error occurs.
     */

    public synchronized void deleteFile( String  file) throws IOException {


        file = getAbsPathName(file,false);

        if (Debug.debugOn && Debug.debugLevel >= Debug.INFO)
            Debug.println(Debug.INFO,"deleteFile:" + file + " (SMB_COM_DELETE)" );

        setupSMBMessage(fMsg,SMBMessage.SMB_COM_DELETE);


        /*
           UCHAR WordCount;	Count of parameter words = 1
        0: USHORT SearchAttributes;
            SHORT ByteCount;	Count of data bytes;    min = 2
            UCHAR BufferFormat;	0x04
            STRING FileName[];	File name
        */

        // set accept long names
        fMsg.setCanHandleLongNames();
        fMsg.setWordCount(1);
        fMsg.setShortParameterAt(0,0);

        MarshalBuffer data = new MarshalBuffer(file.length() +  10);

        int pos  = 0;
        pos += data.setByteAt(pos,SMBMessage.DT_ASCII);
        pos += data.setZTAsciiStringAt(pos,file);
        data.setSize(pos);

        fMsg.setContent(data);

        fMsg.sendAndReceive(fNBTSession,fMsg);

        int errorclass = fMsg.getErrorClass();

        if (errorclass != CifsIOException.SUCCESS)
            throw new CifsIOException(errorclass,fMsg.getErrorCode());


    }


    /**
     * Renames the old file to the new file (also hidden and system files)
     * @param oldfile old file
     * @param newfile new file
     * @exception  IOException  if an I/O error occurs.
     */

    public void renameFile(String  oldfile, String newfile) throws IOException {
        short searchAttr = CifsFileInfo.FILE_ATTR_HIDDEN_FILE | CifsFileInfo.FILE_ATTR_SYSTEM_FILE;

        renameFile(oldfile, newfile, searchAttr);

    }

    /**
     * Renames file corresponding to the search attributes
     * @param oldfile old file
     * @param newfile new file
     * @param searchattr search attributes (see CifsFileInfo.FILE_ATTR_*)
     * @exception  IOException  if an I/O error occurs.
     * @see CifsFile
     */
    public synchronized void renameFile(String  oldfile, String newfile, int searchattr) throws IOException {

        if (Debug.debugOn)
            Debug.println(Debug.INFO,"renameFile:" + oldfile + "(SMB_COM_RENAME)");

        oldfile = getAbsPathName(oldfile,true);
        newfile = getAbsPathName(newfile,true);

        setupSMBMessage(fMsg,SMBMessage.SMB_COM_RENAME);


        /*
           UCHAR WordCount;	Count of parameter words = 0

            SHORT ByteCount;	Count of data bytes;    min = 2
            UCHAR BufferFormat;	0x04
            STRING DirName[];	File name
        */

        // set accept long names

        fMsg.setWordCount(1);
        fMsg.setShortParameterAt(0,searchattr);

        MarshalBuffer buf = new MarshalBuffer(oldfile.length() + newfile.length() + 10);

        int pos  = 0;
        pos += buf.setByteAt(pos,SMBMessage.DT_ASCII);
        pos += buf.setZTAsciiStringAt(pos,oldfile);
        pos += buf.setByteAt(pos,SMBMessage.DT_ASCII);
        pos += buf.setZTAsciiStringAt(pos,newfile);
        buf.setSize(pos);

        fMsg.setContent(buf);

        fMsg.sendAndReceive(fNBTSession,fMsg);

        int errorclass = fMsg.getErrorClass();

        if (errorclass != CifsIOException.SUCCESS)
            throw new CifsIOException(errorclass,fMsg.getErrorCode());


    }


    /**
     * Sets or resets file attributes (Cifs.FILE_ATTR_*)
     * @param file file name
     * @param attr new file attributes
     * @param set if true sets the given attributes otherwise resets it
     * @tbd   set date
     * @exception  IOException  if an I/O error occurs.
     * @see CifsFile
     */
    public synchronized void setFileAttribute(String  file, int attr, boolean set) throws IOException {
        if (Debug.debugOn)
            Debug.println(Debug.INFO,"setFileAttribute:" + file );

        CifsFileInfo info = getFileInfo(file);

        int oldattr = info.getAttributes();
        int newattr ;

        newattr = set ? (oldattr | attr) : (oldattr & ~attr);

        if (info.isDirectory())
            file = getAbsPathName(file,false);
        else
            file = getAbsPathName(file,true);
        setFileAttribute(file,newattr,null);

    }

    /**
     * Sets file attributes (Cifs.FILE_ATTR_*)
     * @param file file name
     * @param attributes new file attributes
     * @tbd   set data
     * @exception  IOException  if an I/O error occurs.
     */

    private  void setFileAttribute(String  file, int attributes, Date lastwrite) throws IOException {


        if (Debug.debugOn)
            Debug.println(Debug.INFO,"setFileAttribute:" + file  + "(SMB_COM_SET_INFORMATION)");

        //file = getAbsPathName(file,true);

        setupSMBMessage(fMsg,SMBMessage.SMB_COM_SET_INFORMATION);


        /*
           UCHAR WordCount;	Count of parameter words = 8

            SHORT ByteCount;	Count of data bytes;    min = 2
            UCHAR BufferFormat;	0x04
            STRING DirName[];	File name
        */


        fMsg.setWordCount(8);
        fMsg.setShortParameterAt(0,attributes);

        if (lastwrite == null) {
            // last write time
            fMsg.setShortParameterAt(2,0);
            // last write date
            fMsg.setShortParameterAt(4,0);
        } else {
            //int stamp = 0xffffffff; // (int)(lastwrite.getTime() / 1000) & 0xffffffff;
            //fMsg.setIntParameterAt(2,stamp);

            // last write date
            // TBD: es funktioniert nicht auf WIn95
            fMsg.setShortParameterAt(4,Util.getSMBTime(lastwrite));
            fMsg.setShortParameterAt(2,Util.getSMBDate(lastwrite));
        }

        // reserved[5]
        fMsg.setShortParameterAt(6,0);
        fMsg.setShortParameterAt(8,0);
        fMsg.setShortParameterAt(10,0);
        fMsg.setShortParameterAt(12,0);
        fMsg.setShortParameterAt(14,0);


        MarshalBuffer data = new MarshalBuffer(file.length() +  10);

        int pos  = 0;
        pos += data.setByteAt(pos,SMBMessage.DT_ASCII);
        pos += data.setZTAsciiStringAt(pos,file);
        data.setSize(pos);


        fMsg.setContent(data);

        fMsg.sendAndReceive(fNBTSession,fMsg);

        int errorclass = fMsg.getErrorClass();

        if (errorclass != CifsIOException.SUCCESS)
            throw new CifsIOException(errorclass,fMsg.getErrorCode());


    }

    /**
     * Creates the given directory on the server
     * @param dirname directory name
     * @exception  IOException  if an I/O error occurs.
     */
    public synchronized void mkdir(String  dirname) throws IOException {

        dirname = getAbsPathName(dirname,false);

        if (Debug.debugOn)
            Debug.println(Debug.INFO,"mkdir:" + dirname + "(SMB_COM_CREATE_DIRECTORY)" );


        setupSMBMessage(fMsg,SMBMessage.SMB_COM_CREATE_DIRECTORY);


        /*
           UCHAR WordCount;	Count of parameter words = 0

            SHORT ByteCount;	Count of data bytes;    min = 2
            UCHAR BufferFormat;	0x04
            STRING DirName[];	File name
        */


        fMsg.setWordCount(0);


        MarshalBuffer buf = new MarshalBuffer(dirname.length() +  10);

        int pos  = 0;
        pos += buf.setByteAt(pos,SMBMessage.DT_ASCII);
        pos += buf.setZTAsciiStringAt(pos,dirname);
        buf.setSize(pos);

        fMsg.setContent(buf);

        fMsg.sendAndReceive(fNBTSession,fMsg);

        int errorclass = fMsg.getErrorClass();


        if (errorclass != CifsIOException.SUCCESS)
            throw new CifsIOException(errorclass,fMsg.getErrorCode());


    }

    /**
     * Removes directory
     * @param dirname directory name
     * @exception  IOException  if an I/O error occurs.
     */
    public synchronized void rmdir( String  dirname) throws IOException {

        dirname = getAbsPathName(dirname,false);

        if (Debug.debugOn)
            Debug.println(Debug.INFO,"rmdir:" + dirname  + "(SMB_COM_DELETE_DIRECTORY)");

        setupSMBMessage(fMsg,SMBMessage.SMB_COM_DELETE_DIRECTORY);


        /*
           UCHAR WordCount;	Count of parameter words = 0

            SHORT ByteCount;	Count of data bytes;    min = 2
            UCHAR BufferFormat;	0x04
            STRING DirName[];	File name
        */


        fMsg.setWordCount(0);


        MarshalBuffer buf = new MarshalBuffer(dirname.length() +  10);

        int pos  = 0;
        pos += buf.setByteAt(pos,SMBMessage.DT_ASCII);
        pos += buf.setZTAsciiStringAt(pos,dirname);
        buf.setSize(pos);

        fMsg.setContent(buf);

        fMsg.sendAndReceive(fNBTSession,fMsg);

        int errorclass = fMsg.getErrorClass();

        if (errorclass != CifsIOException.SUCCESS)
            throw new CifsIOException(errorclass,fMsg.getErrorCode());


    }

    /**
     * Checks if directory exists
     * @param dirname directory name
     * @return true if directory exits
     * @exception  IOException  if an I/O error occurs.
     */
    public boolean directoryExists(String  dirname) throws IOException {

        return checkDirectory(getAbsPathName(dirname,true));
    }

    /**
     * Checks directory (name not changed)
     * @param dirname directory name
     * @return true if yes
     */
    protected synchronized boolean checkDirectory(String dirname) throws IOException {


        if (Debug.debugOn)
            Debug.println(Debug.INFO,"checkDirectory:" + dirname + "(SMB_COM_CHECK_DIRECTORY)" );


        setupSMBMessage(fMsg,SMBMessage.SMB_COM_CHECK_DIRECTORY);


        /*
           UCHAR WordCount;	Count of parameter words = 0

            SHORT ByteCount;	Count of data bytes;    min = 2
            UCHAR BufferFormat;	0x04
            STRING DirName[];	File name
        */


        fMsg.setWordCount(0);


        MarshalBuffer buf = new MarshalBuffer(dirname.length() +  10);

        int pos  = 0;
        pos += buf.setByteAt(pos,SMBMessage.DT_ASCII);
        pos += buf.setZTAsciiStringAt(pos,dirname);
        buf.setSize(pos);

        fMsg.setContent(buf);

        fMsg.sendAndReceive(fNBTSession,fMsg);

        int errorclass = fMsg.getErrorClass();
        int errorcode  = fMsg.getErrorCode();

        if (errorclass != CifsIOException.SUCCESS) {
            if (errorclass == CifsIOException.ERROR_DOS && errorcode == CifsIOException.DOS_BAD_PATH )
                return false;

            throw new CifsIOException(errorclass,fMsg.getErrorCode());
        }

        return true;

    }





    /**
     * Returns informations about the filesystem
     * @return CifsFileSystemInfo filesystem informations
     * @exception  IOException  if an I/O error occurs.
     */
    public synchronized CifsFileSystemInfo getFileSystemInfo() throws IOException {

        if (Debug.debugOn)
            Debug.println(Debug.INFO, "getFileSystemInfo (SMB_QUERY_FS_VOLUME_INFO)");

        int data_count;
        int data_off;
        int str_len;


        if (!fFSInfo.fQueryFSVolumeDone) {
            getFSInformation(SMB_QUERY_FS_VOLUME_INFO);

             //  data count
            data_count = fMsg.getShortParameterAt(12);
            //  data offset
            data_off   = fMsg.getShortParameterAt(14);

            fFSInfo.fVolumeCreationTime = fMsg.getLongAt(data_off);
            data_off += 8;

            fFSInfo.fVolumeSerialNumber = fMsg.getIntAt(data_off);
            data_off += 4;

            str_len = fMsg.getIntAt(data_off);
            data_off += 4;

            // Skip 2 bytes
            data_off +=  2;

            fFSInfo.fVolumeLabel = fMsg.getUnicodeStringAt(data_off,str_len);

            fFSInfo.fQueryFSVolumeDone = true;

        }

        if (!fFSInfo.fQueryDeviceInfoDone && fSupports_SMB_QUERY_FS_DEVICE_INFO) {

            try {
                getFSInformation(SMB_QUERY_FS_DEVICE_INFO);

                 //  data count
                data_count = fMsg.getShortParameterAt(12);
                //  data offset
                data_off   = fMsg.getShortParameterAt(14);

                fFSInfo.fDeviceType = fMsg.getIntAt(data_off);
                data_off += 4;
                fFSInfo.fDeviceCharacteristics = fMsg.getIntAt(data_off);

                fFSInfo.fQueryDeviceInfoDone = true;
            } catch(IOException e) {
                // ignore; Win95 returns exception
                fSupports_SMB_QUERY_FS_DEVICE_INFO = false;

            }
        }

        if (!fFSInfo.fQueryFSAttrDone) {
            getFSInformation(SMB_QUERY_FS_ATTRIBUTE_INFO);

             //  data count
            data_count = fMsg.getShortParameterAt(12);
            //  data offset
            data_off   = fMsg.getShortParameterAt(14);

            fFSInfo.fFSAttributes = fMsg.getIntAt(data_off);
            data_off += 4;
            fFSInfo.fMaxFileComponentLength = fMsg.getIntAt(data_off);

            data_off += 4;
            str_len = fMsg.getIntAt(data_off);

            data_off += 4;
            fFSInfo.fFSName = fMsg.getUnicodeStringAt(data_off,str_len);

            fFSInfo.fQueryFSAttrDone = true;
        }

        if (fSupports_SMB_QUERY_FS_SIZE_INFO) {
            try {
                getFSInformation(SMB_QUERY_FS_SIZE_INFO);

                //  data count
                data_count = fMsg.getShortParameterAt(12);
                //  data offset
                data_off   = fMsg.getShortParameterAt(14);

                fFSInfo.fTotalAllocUnits = fMsg.getLongAt(data_off);
                data_off += 8;
                fFSInfo.fFreeAllocUnits = fMsg.getIntAt(data_off);
                data_off += 8;
                fFSInfo.fSectorsPerUnit = fMsg.getIntAt(data_off);
                data_off += 4;
                fFSInfo.fBytesPerSector = fMsg.getIntAt(data_off);

                fFSInfo.fQueryFSSizeDone = true;

            } catch(IOException e) {
                // Win95 returns exception
                fFSInfo.fQueryFSSizeDone = false;
                fSupports_SMB_QUERY_FS_SIZE_INFO = false;
            }
        }

        return fFSInfo;
    }


    private void getFSInformation(short infolevel) throws IOException {


        setupSMBMessage(fMsg,SMBMessage.SMB_COM_TRANSACTION2);


        /*
            WordCount;	15
            TotalParameterCount;	2 or 4
            MaxSetupCount;	0
            SetupCount;	1 or 2
            Setup[0];	TRANS2_QUERY_FS_INFORMATION

            Parameter Block Encoding

                USHORT Information Level;	Level of information requested
        */


        fMsg.setWordCount(14+1);

        // TotalParameterCount !!!!
        fMsg.setShortParameterAt(0,0);

        // TotalDataCount !!!!
        fMsg.setShortParameterAt(2,0);

        // MaxParameterCount returned by server
        fMsg.setShortParameterAt(4,16);

        // MaxDataCount returned by server
		// Modified by Ashwin, put 15000 instead of 3000
        fMsg.setShortParameterAt(6,15000);

        // MaxSetupCount returned by server
        fMsg.setByteParameterAt(8,(byte)0);

        // Reserved
        fMsg.setByteParameterAt(9,(byte)0);

        // Flags
        fMsg.setShortParameterAt(10,0);

        // Timeout
        fMsg.setIntParameterAt(12,0);

        // Reserved 2
        fMsg.setShortParameterAt(16,0);

        // ParameterCount bytes sent this buffer
        fMsg.setShortParameterAt(18,0);

        // ParameterOffset bytes sent this buffer
        fMsg.setShortParameterAt(20,0);

        // DataCount bytes sent this buffer
        fMsg.setShortParameterAt(22,0);

        // DataOffset bytes sent this buffer
        fMsg.setShortParameterAt(24,0);

        // SetupCount
        fMsg.setByteParameterAt(26,(byte)1);

         // Reserved3
        fMsg.setByteParameterAt(27,(byte)0);

        // Setup[0]
        fMsg.setShortParameterAt(28,SMBMessage.TRANS2_QUERY_FS_INFORMATION);


        // byteCount
        fMsg.setContentSize(0);

        int bytes_off, off;

        bytes_off = off = fMsg.getContentOffset();

        fMsg.zero(off,20);


        // Name of trans (NULL string
        fMsg.setByteAt(off,(byte)0);
        off++;

        // Calculate beginning of Parameters (offset from Header)
        int param_off = off = MarshalBuffer.align(off,4);

        // Information level
        fMsg.setShortAt(off,infolevel);
        off += 2;


        // now set the TotalParameterCount
        // TotalParameterCount !!!!
        fMsg.setShortParameterAt(0,off - param_off);

         // ParameterCount bytes sent this buffer
        fMsg.setShortParameterAt(18,off - param_off);

        // ParameterOffset bytes sent this buffer
        fMsg.setShortParameterAt(20,param_off);

        int data_off = off = MarshalBuffer.align(off,4);


        // byteCount
        fMsg.setContentSize(off - bytes_off);

        fMsg.sendAndReceive(fNBTSession,fMsg);

        int errorclass = fMsg.getErrorClass();

        if (errorclass != CifsIOException.SUCCESS)
            throw new CifsIOException(errorclass,fMsg.getErrorCode());


    }

    /**
     * Returns information about the given file
     * @param file file name
     * @return CifsFileInfo
     * @exception  IOException  if an I/O error occurs.
     */
    public synchronized CifsFileInfo  getFileInfo(String  file) throws IOException {

        if (Debug.debugOn && Debug.debugLevel >= Debug.INFO)
            Debug.println(Debug.INFO, "getFileInfo");

        String  searchPath ;
        String  searchFile;

        searchPath =  getAbsPathName(file,true);
        // normalize filename

        int p = searchPath.lastIndexOf("\\");

        if (p > 0)
            searchFile = searchPath.substring(p+1);
        else
            searchFile = searchPath;

        FileInfoBuffer data = new FileInfoBuffer(4096);
        data.setInfoLevel(SMBMessage.SMB_INFO_STANDARD);
        data.setSearchAttr(CifsFile.FILE_ATTR_ALL);

        searchFiles(searchPath,data,1);
        int count = data.getCount();


        if (count != 1)
            throw new CifsIOException("DK5",file);  // File does not exist

        /*
        0:SMB_DATE CreationDate;	Date when file was created
        2:SMB_TIME CreationTime;	Time when file was created
        4:SMB_DATE LastAccessDate;	Date of last file access
        6:SMB_TIME LastAccessTime;	Time of last file access
        8:SMB_DATE LastWriteDate;	Date of last write to the file
        10:SMB_TIME LastWriteTime;	Time of last write to the file
        12:ULONG  DataSize;	File Size
        16:ULONG AllocationSize;	Size of filesystem allocation unit
        20:USHORT Attributes;	File Attributes
        22:UCHAR FileNameLength;	Length of filename in bytes
        23:STRING FileName;	Name of found file

        */

        int len = data.getByteAt(22) & 0xff;

        String f = data.getAsciiStringAt(23,len);

        // Sanity check
        if (!f.equalsIgnoreCase(searchFile))
            throw new CifsIOException("DK5",file);

        FileInfo info = new FileInfo(extractDir(file));

        info.fFileName = extractName(file);

        int d = data.getShortAt(0);
        int t = data.getShortAt(2);

        info.fCreationTime = Util.getDateTime(d,t);

        d = data.getShortAt(4);
        t = data.getShortAt(6);

        info.fLastAccessTime = Util.getDateTime(d,t);

        d = data.getShortAt(8);
        t = data.getShortAt(10);


        info.fLastWriteTime = Util.getDateTime(d,t);

        info.fFileSize = data.getLongAt(12) & 0xffffffff;

        info.fAttributes = data.getShortAt(20);

        return info;

    }

    private int  extractInfo(FileInfoBuffer data, int pos, FileInfo info) throws IOException {


        int d = data.getShortAt(pos + 0);
        int t = data.getShortAt(pos + 2);


        info.fCreationTime = Util.getDateTime(d,t);

        d = data.getShortAt(pos + 4);
        t = data.getShortAt(pos + 6);

        info.fLastAccessTime = Util.getDateTime(d,t);

        d = data.getShortAt(pos + 8);
        t = data.getShortAt(pos + 10);


        info.fLastWriteTime = Util.getDateTime(d,t);

        info.fFileSize = data.getLongAt(pos + 12) & 0xffffffff;

        info.fAttributes = data.getShortAt(pos + 20);

        int len = data.getByteAt(pos + 22) & 0xff;

        info.fFileName =  data.getAsciiStringAt(pos+23,len);

        // string are zero terminated
        return 23 + len + 1;
    }

    /**
     * Enumerates informations about the files
     * @param file file name (with wildcards)
     * @param searchattr file attributes (see CifsFile.FILE_ATTR_*)
     * @param sort  if true names will be sorted
     * @return  array of <code>CifsFileInfo</code> elements
     * @exception  IOException  if an I/O error occurs.
     */
    public CifsFileInfo[] listFilesInfo(String  file, int searchattr, boolean sort) throws IOException {

        if (Debug.debugOn && Debug.debugLevel >= Debug.INFO)
            Debug.println(Debug.INFO, "listFilesInfo:" + file);

        file = getAbsPathName(file,false);

        FileInfoBuffer data = new FileInfoBuffer(4096);
        data.setInfoLevel(SMBMessage.SMB_INFO_STANDARD);
        data.setSearchAttr(searchattr);

        searchFiles(file,data,MAX_SEARCH_COUNT);
        int count = data.getCount();

        if (Debug.debugOn && Debug.debugLevel > Debug.BUFFER)
            data.debug("List files");

        CifsFileInfo[] result = new CifsFileInfo[count];
        int pos = 0;

        String dir = extractDir(file);

        for(int i=0;i<count;i++) {
            FileInfo info  = new FileInfo(dir);

            pos += extractInfo(data,pos,info);

            result[i] = info;
        }
        data = null;

        if (sort)
            Util.sort(result,fFileInfoComparator);

        return result;

    }

    /**
     * Enumerates file names
     * @param file file name (with wildcards)
     * @param searchattr file attributes
     * @param sort  if true names will be sorted
     * @return array of <code>java.lang.String</code> elements
     * @exception  IOException  if an I/O error occurs.
     */
    public  String[] listFilesName(String  file, int searchattr, boolean sort) throws IOException {

        if (Debug.debugOn && Debug.debugLevel >= Debug.INFO)
            Debug.println(Debug.INFO, "listFiles:" + file);

        file = getAbsPathName(file,false);

        FileInfoBuffer info = new FileInfoBuffer(4096);
        info.setInfoLevel(SMBMessage.SMB_FIND_FILE_NAMES_INFO);
        info.setSearchAttr(searchattr);

        searchFiles(file,info,MAX_SEARCH_COUNT);
        int count = info.getCount();

        String[] result = new String[count];


        int pos = 0;

        for(int i=0;i<count;i++) {
            int next = pos + info.getIntAt(pos);
            pos += 8;
            int len = info.getIntAt(pos);
            pos += 4;

            result[i] = info.getAsciiStringAt(pos,len);

            pos = next;
        }

        if (sort)
            Util.sortStrings(result);


        return result;

    }

    private synchronized int searchFiles(String  path, FileInfoBuffer info, int max) throws IOException {


        int     loopCount   = 0;
        int     endOfSearch = 0;
        int     searchCount = 0;
        int     searchHandle;
        int     flags;


        if (max > 1)
            // close search if end + continue search
            flags = 0x02 | 0x08;
        else
            // if max = 1 , close after this search
            flags = 0x01;



        /*
            SetupCount	1
            Setup[0]	TRANS2_FIND_FIRST2

            -------------------- Parameter block ----------------------
            0: USHORT SearchAttributes;
            2: USHORT SearchCount;	Maximum number of entries to return
            4: USHORT Flags;	Additional information:
 	            Bit 0 - close search after this request
	            Bit 1 - close search if end of search reached
	            Bit 2 - return resume keys for each entry found
	            Bit 3 - continue search from previous ending place
	            Bit 4 - find with backup intent
            6: USHORT InformationLevel;	See below
            8: ULONG SearchStorageType;
            12:STRING FileName;	Pattern for the search

            UCHAR Data[ TotalDataCount ]	FEAList if InformationLevel is QUERY_EAS_FROM_LIST
        */


        int    lparam = 12 + path.length() + 1;
        MarshalBuffer param = new MarshalBuffer(lparam+10);

        int    pos = 0;

        // SearchAttributes
        param.setShortAt(pos,info.getSearchAttr());
        pos += 2;
        // SearchCount
        param.setShortAt(pos,(short)max);
        pos += 2;
        // Flags
        param.setShortAt(pos,(short)flags);
        pos += 2;

        // InformationLevel
        param.setShortAt(pos,info.getInformationLevel());
        pos += 2;

        // SearchStorageType
        param.setIntAt(pos, (short)0);
        pos += 4;


        pos += param.setZTAsciiStringAt(pos,path);

        param.setSize(pos);

        sendTransaction2(SMBMessage.TRANS2_FIND_FIRST2, param,null,0,0xffff);


        receiveTransaction(param,info);


        /*
            Parameter Block:

            0:USHORT Sid;	Search handle
            2:USHORT SearchCount;	Number of entries returned
            4:USHORT EndOfSearch;	Was last entry returned?
            6:USHORT EaErrorOffset;	Offset into EA list if EA error
            8:USHORT LastNameOffset;	Offset into data to file name of last entry,
                        if server needs it to resume search; else 0
            10:UCHAR Data[ TotalDataCount ]	Level dependent info about the matches
                    found in the search
        */



        searchHandle = param.getShortAt(0);

        // Search count
        searchCount  = param.getShortAt(2);

        info.setCount(searchCount);

        if (searchCount == 0)
            return info.getCount();

         // End of Search
        endOfSearch  = param.getShortAt(4);


        MarshalBuffer data = new MarshalBuffer(4096);

        while (endOfSearch == 0) {
            loopCount++;

            if (loopCount > 200)
                break;

            /*
                WordCount	15
                SetupCount	1
                Setup[0]	TRANS2_FIND_NEXT2
                Parameter Block Encoding

                0:USHORT Sid;	Search handle
                2:USHORT SearchCount;	Maximum number of entries to return
                4:USHORT InformationLevel;	Levels described in TRANS2_FIND_FIRST2 request
                6:ULONG ResumeKey;	Value returned by previous find2 call
                10:USHORT Flags;	Additional information: bit set-
                    0 - close search after this request
                    1 - close search if end of search reached
                    2 - return resume keys for each entry found
                    3 - resume/continue from previous ending place
                    4 - find with backup intent
                12:STRING FileName;	Resume file name
            */


            // Search handle
            param.setShortAt(0,searchHandle);

            // SearchCount
            param.setShortAt(2,(short)max);
            // InformationLevel
            param.setShortAt(4,info.getInformationLevel());
            // ResumeKey
            param.setIntAt(6,0);
            // Flags resum/cont +
            param.setShortAt(10,(short)flags);

            pos = 12;
            /*
             If Bit3 of Flags is set, then FileName may be the NULL string,
             since the search is continued from the previous TRANS2_FIND request.
            */
            pos += param.setZTAsciiStringAt(pos,"");

            param.setSize(pos);



            sendTransaction2(SMBMessage.TRANS2_FIND_NEXT2, param,null,0,0xffff);
            receiveTransaction(param,data);


            /*
            0:USHORT SearchCount;	Number of entries returned
            2:USHORT EndOfSearch;	Was last entry returned?
            4:USHORT EaErrorOffset;	Offset into EA list if EA error
            6:USHORT LastNameOffset;	Offset into data to file name of last entry, if server needs it to resume search; else 0
              UCHAR Data[TotalDataCount]	Level dependent info about the matches found in the search
            */


            // Search count
            searchCount  = param.getShortAt(0);

            if (searchCount == 0)
                break;

            info.append(data,searchCount);

            // End of Search
            endOfSearch  = param.getShortAt(2);


        }

        return info.getCount();


    }


    synchronized FileHandle openFile(String  file,int flags, int sharemode) throws IOException {

        file = getAbsPathName(file,false);



        if (Debug.debugOn && Debug.debugLevel >= Debug.INFO)
            Debug.println(Debug.INFO,"SMB_COM_OPEN_ANDX:" + file  );

        setupSMBMessage(fMsg,SMBMessage.SMB_COM_OPEN_ANDX);


        /*
            UCHAR WordCount;	Count of parameter words = 15
            0:UCHAR AndXCommand;	Secondary (X) command;  0xFF = none
            1:UCHAR AndXReserved;	Reserved (must be 0)
            2:USHORT AndXOffset;	Offset to next command WordCount
            4:USHORT Flags;	Additional information: bit set-
	            0 - return additional info
	            1 - exclusive oplock requested
	            2 - batch oplock requested
            6:USHORT DesiredAccess;	File open mode
            8:USHORT SearchAttributes;
            10:USHORT FileAttributes;
            12:SMB_TIME CreationTime;
            14:SMB_DATE CreationDate;
            16:USHORT OpenFunction;	Action to take if file exists
            18:ULONG AllocationSize;	Bytes to reserve on create or truncate
            22:ULONG Reserved[2];	Must be 0
            USHORT ByteCount;	Count of data bytes;    min = 1
            UCHAR BufferFormat	0x04
            STRING FileName;


        */

        int accessmode = 0;
        int openfn     = 0;

        if ((flags & O_CREAT) != 0)
		    openfn |= (OF_CREATE <<4);

	    if ((flags & O_EXCL) == 0) {

		    if ((flags & O_TRUNC) != 0)
			    openfn |= OF_TRUNC;
		    else
			    openfn |= OF_OPEN;
	    } else {
	        // open only if file doesn't already exist
        }

	    accessmode = (sharemode << 4);

	    if ((flags & O_RDWR) == O_RDWR) {
		    accessmode |= AM_READ_WRITE;
	    } else if ((flags & O_WRONLY) == O_WRONLY) {
		    accessmode |= AM_WRITE;
	    }



        fMsg.setWordCount(15);

        // AndXCommand
        fMsg.setByteParameterAt(0,(byte)0xff);
        // AndXReserved
        fMsg.setByteParameterAt(1,(byte)0);
        // AndXOffset
        fMsg.setShortParameterAt(2,0);
        // Flags: return additional info
        fMsg.setShortParameterAt(4,(short)0x01);
        // DesiredAccess
        fMsg.setShortParameterAt(6,(short)(accessmode & 0xffff));
        // SearchAttributes
        fMsg.setShortParameterAt(8,0);

        // FileAttributes (????)
        fMsg.setShortParameterAt(10,(short)0);
        // CreationTime
        fMsg.setShortParameterAt(12,0);
        //fMsg.setShortParameterAt(12,(short)0);
        // CreationDate
        fMsg.setShortParameterAt(14,0);
        //fMsg.setShortParameterAt(14,(short)0);
        // Open function
        fMsg.setShortParameterAt(16,(short)(openfn & 0xffff));
        // Allocation size
        fMsg.setIntParameterAt(18,0);
        // Reserved
        fMsg.setIntParameterAt(22,0);
        fMsg.setIntParameterAt(26,0);

        MarshalBuffer data = new MarshalBuffer(file.length() +  10);

        int pos  = 0;

        pos += data.setZTAsciiStringAt(pos,file);
        data.setSize(pos);

        fMsg.setContent(data);

        fMsg.sendAndReceive(fNBTSession,fMsg);

        int errorclass = fMsg.getErrorClass();

        if (errorclass != CifsIOException.SUCCESS)
            throw new CifsIOException(errorclass,fMsg.getErrorCode());


        /*
        UCHAR WordCount;	Count of parameter words = 15
        0:UCHAR AndXCommand;	Secondary (X) command;  0xFF = none
        1:UCHAR AndXReserved;	Reserved (must be 0)
        2:USHORT AndXOffset;	Offset to next command WordCount
        4:USHORT Fid;	File handle
        6:USHORT FileAttributes;
        8:SMB_TIME LastWriteTime;
        10:SMB_DATE LastWriteDate;
        12:ULONG DataSize;	Current file size
        16:USHORT GrantedAccess;	Access permissions actually allowed
        18:USHORT FileType;	Type of file opened
        20:USHORT DeviceState;	State of the named pipe
        22:USHORT Action;	Action taken
        24:ULONG ServerFid;	Server unique file id
        28:USHORT Reserved;	Reserved (must be 0)
        USHORT ByteCount;	Count of data bytes = 0
        */

        FileHandle handle = new FileHandle(this);

        // Fid
        handle.fFID = fMsg.getShortParameterAt(4);

        handle.fFileAttributes = fMsg.getShortParameterAt(6);
        handle.fLastWriteTime = fMsg.getShortParameterAt(8);
        handle.fLastWriteDate = fMsg.getShortParameterAt(10);
        handle.fFileSize      = fMsg.getIntParameterAt(12) & 0xffffffff;
        handle.fGrantedAccess = fMsg.getShortParameterAt(16);
        handle.fActionTaken   = fMsg.getShortParameterAt(22);
        handle.fFileName      = file;


        return handle;
    }


   synchronized  void closeFile(FileHandle handle, boolean touch) throws IOException {


        if (Debug.debugOn && Debug.debugLevel >= Debug.INFO)
            Debug.println(Debug.INFO,"SMB_COM_CLOSE" );

        setupSMBMessage(fMsg,SMBMessage.SMB_COM_CLOSE);


        /*
            UCHAR WordCount;	Count of parameter words = 3
            0:USHORT Fid;	File handle
            2:UTIME   LastWriteTime	Time of last write

            USHORT ByteCount;	Count of data bytes = 0

        */

        int  stamp = 0;

        fMsg.setWordCount(3);

        // Fid
        fMsg.setShortParameterAt(0,handle.fFID);

        if (touch) {
            stamp = (int)(new Date().getTime() / 1000) & 0xffffffff;
            fMsg.setIntParameterAt(2,stamp);
        } else {
            // Bei 0 wird der Zeitstempel geändert, obwohl in der Dok.
            // 0 vorgeschrieben wird!!!
            fMsg.setIntParameterAt(2,0xffffffff);
        }
        fMsg.setContentSize(0);

        fMsg.sendAndReceive(fNBTSession,fMsg);

        int errorclass = fMsg.getErrorClass();

        // ignore close error
        if (errorclass != CifsIOException.SUCCESS)
            Debug.println(Debug.WARNING,"Close error:" + fMsg.getNTErrorCode());


    }


    synchronized int readFile(FileHandle handle,long offset, byte[] buf, int obuf, int len) throws IOException {


        if (Debug.debugOn && Debug.debugLevel >= Debug.INFO)
            Debug.println(Debug.INFO,"SMB_COM_READ_ANDX" );

        readFile(handle,offset,len);


        /*
        UCHAR WordCount;	Count of parameter words = 12
        0:UCHAR AndXCommand;	Secondary (X) command;  0xFF = none
        1:UCHAR AndXReserved;	Reserved (must be 0)
        2:USHORT AndXOffset;	Offset to next command WordCount
        4:USHORT Remaining;	Bytes remaining to be read
        6:USHORT DataCompactionMode;
        8:USHORT Reserved;	Reserved (must be 0)
        10:USHORT DataLength;	Number of data bytes (min = 0)
        12:USHORT DataOffset;	Offset (from header start) to data
        14:USHORT Reserved[5];	Reserved (must be 0)
        USHORT ByteCount;	Count of data bytes
        UCHAR Pad[];
        UCHAR Data[ DataLength];	Data from resource

        */

        int datalen = fMsg.getShortParameterAt(10);
        int dataoff = fMsg.getShortParameterAt(12);

        fMsg.copyTo(dataoff,buf,obuf,datalen);

        return datalen;
    }


    synchronized void readFile(FileHandle handle,long offset, int len) throws IOException {


        if (Debug.debugOn && Debug.debugLevel >= Debug.INFO)
            Debug.println(Debug.INFO,"SMB_COM_READ_ANDX" );

        setupSMBMessage(fMsg,SMBMessage.SMB_COM_READ_ANDX);


        /*
            UCHAR WordCount;	Count of parameter words = 10
            0:UCHAR AndXCommand;	Secondary (X) command;  0xFF = none
            1:UCHAR AndXReserved;	Reserved (must be 0)
            2:USHORT AndXOffset;	Offset to next command WordCount
            4:USHORT Fid;	File handle
            6:ULONG Offset;	Offset in file to begin read
            10:USHORT MaxCount;	Max number of bytes to return
            12:USHORT MinCount;	Min number of bytes to return
            14:ULONG Reserved;	Must be 0
            18:USHORT Remaining;	Bytes remaining to satisfy request
            USHORT ByteCount;	Count of data bytes = 0

        */

        fMsg.setWordCount(10);

        // AndXCommand
        fMsg.setByteParameterAt(0,(byte)0xff);
        // AndXReserved
        fMsg.setByteParameterAt(1,(byte)0);
        // AndXOffset
        fMsg.setShortParameterAt(2,0);
        // FID
        fMsg.setShortParameterAt(4,handle.fFID);
        // Offset
        fMsg.setIntParameterAt(6,(int)offset);
        // MaxCount
        fMsg.setShortParameterAt(10,len);
        // MinCount (only for pipe)
        fMsg.setShortParameterAt(12,0);
        // Reserved
        fMsg.setIntParameterAt(14,0);
        // Remaining (pipes)
        fMsg.setShortParameterAt(18,0);

        fMsg.setContentSize(0);

        fMsg.sendAndReceive(fNBTSession,fMsg);

        int errorclass = fMsg.getErrorClass();

        if (errorclass != CifsIOException.SUCCESS)
            throw new CifsIOException(errorclass,fMsg.getErrorCode());
    }


    synchronized int writeFile(FileHandle handle,long offset, char[] buf, int obuf, int lbuf) throws IOException {

        initWriteFile( handle, offset,lbuf);

        int contentOffset =  fMsg.getContentOffset();


        // Calculate beginning of Parameters (offset from Header)
        int pos = MarshalBuffer.align(contentOffset,4);

        // data offset
        fMsg.setShortParameterAt(22,(short)pos);
        fMsg.setBytesAt(pos,buf,obuf,lbuf);

        fMsg.setContentSize(pos + lbuf - contentOffset);

        fMsg.sendAndReceive(fNBTSession,fMsg);

        int errorclass = fMsg.getErrorClass();

        if (errorclass != CifsIOException.SUCCESS)
            throw new CifsIOException(errorclass,fMsg.getErrorCode());


        /*
        UCHAR WordCount;	Count of parameter words = 6
        0:UCHAR AndXCommand;	Secondary (X) command;  0xFF = none
        1:UCHAR AndXReserved;	Reserved (must be 0)
        2:USHORT AndXOffset;	Offset to next command WordCount
        4:USHORT Count;	Number of bytes written
        6:USHORT Remaining;	Bytes remaining to be read in pipe
        8:ULONG Reserved;
        USHORT ByteCount;	Count of data bytes = 0


        */

        int datalen = fMsg.getShortParameterAt(4);

        return datalen;

    }

     synchronized int writeFile(FileHandle handle,long offset, byte[] buf, int obuf, int lbuf) throws IOException {

        initWriteFile( handle, offset,lbuf);

        int contentOffset =  fMsg.getContentOffset();


        // Calculate beginning of Parameters (offset from Header)
        int pos = MarshalBuffer.align(contentOffset,4);

        // data offset
        fMsg.setShortParameterAt(22,(short)pos);
        fMsg.setBytesAt(pos,buf,obuf,lbuf);

        fMsg.setContentSize(pos + lbuf - contentOffset);

        fMsg.sendAndReceive(fNBTSession,fMsg);

        int errorclass = fMsg.getErrorClass();

        if (errorclass != CifsIOException.SUCCESS)
            throw new CifsIOException(errorclass,fMsg.getErrorCode());


        /*
        UCHAR WordCount;	Count of parameter words = 6
        0:UCHAR AndXCommand;	Secondary (X) command;  0xFF = none
        1:UCHAR AndXReserved;	Reserved (must be 0)
        2:USHORT AndXOffset;	Offset to next command WordCount
        4:USHORT Count;	Number of bytes written
        6:USHORT Remaining;	Bytes remaining to be read in pipe
        8:ULONG Reserved;
        USHORT ByteCount;	Count of data bytes = 0


        */

        int datalen = fMsg.getShortParameterAt(4);

        return datalen;

    }
    private void initWriteFile(FileHandle handle,long offset, int lbuf) throws IOException {


        if (Debug.debugOn && Debug.debugLevel >= Debug.INFO)
            Debug.println(Debug.INFO,"SMB_COM_WRITE_ANDX" );

        setupSMBMessage(fMsg,SMBMessage.SMB_COM_WRITE_ANDX);


        /*
            UCHAR WordCount;	Count of parameter words = 12
            0:UCHAR AndXCommand;	Secondary (X) command;  0xFF = none
            1:UCHAR AndXReserved;	Reserved (must be 0)
            2:USHORT AndXOffset;	Offset to next command WordCount
            4:USHORT Fid;	File handle
            6:ULONG Offset;	Offset in file to begin write
            10:ULONG Reserved;	Must be 0
            14:USHORT WriteMode;	Write mode:
	            0 - write through
	            1 - return Remaining
	            2 - use WriteRawNamedPipe (n. pipes)
	            3 - "this is the start of the msg"
            16:USHORT Remaining;	Bytes remaining to satisfy request
            18:USHORT Reserved;
            20:USHORT DataLength;	Number of data bytes in buffer (>=0)
            22:USHORT DataOffset;	Offset to data bytes
            USHORT ByteCount;	Count of data bytes
            UCHAR Pad[];	Pad to SHORT or LONG
            UCHAR Data[DataLength];	Data to write

        */

        fMsg.setWordCount(12);

        // AndXCommand
        fMsg.setByteParameterAt(0,(byte)0xff);
        // AndXReserved
        fMsg.setByteParameterAt(1,(byte)0);
        // AndXOffset
        fMsg.setShortParameterAt(2,0);
        // FID
        fMsg.setShortParameterAt(4,handle.fFID);
        // Offset
        fMsg.setIntParameterAt(6,(int)offset);
        // Reserved
        fMsg.setIntParameterAt(10,0);
        // Write mode
        fMsg.setShortParameterAt(14,(short)0x01);
        // Remaining
        fMsg.setShortParameterAt(16,0);
        // Reserved
        fMsg.setShortParameterAt(18,0);
        // DataLength
        fMsg.setShortParameterAt(20,(short)lbuf);


    }

    public String toString() {
        return "Session:" + fSessionName + ", Type=Disk, Share=" + fShare;
    }





    //========================== Protected section =========================
    protected String getAbsPathName(String name, boolean relative) {
        name = name.replace('/','\\');
        if (!name.startsWith("\\"))
            name = "\\" + name;

        name = Util.normalizePathName(name);

        if (relative)
            name = "." + name;
        return name;

    }
    protected static String extractDir(String file) {
        int p = file.lastIndexOf('\\');
        if (p < 0)
            return file;
        return file.substring(0,p);
    }
    protected static String extractName(String file) {
        int p = file.lastIndexOf('\\');
        if (p < 0)
            return file;
        return file.substring(p+1);
    }

    int getSortPosition() {
        return 1;
    }
}

// Modified by Ashwin, full qualified Comparator
class FileInfoComparator implements org.gnu.jcifs.util.Comparator {

     Collator fCollator = Collator.getInstance();

     public int compare(Object o1, Object o2) {
        FileInfo fo1 = (FileInfo)o1;
        FileInfo fo2 = (FileInfo)o2;
        return fCollator.compare(fo1.fFileName,fo2.fFileName);


     }

}
