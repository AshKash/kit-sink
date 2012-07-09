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
final class PrinterImpl extends SessionImpl implements CifsPrinter {

    /** The server may optinally expand tabs to a series of spaces */
    public final static int TEXT_MODE      = 0;

    /** No conversion of data should be done by the server */
    public final static int GRAPHICS_MODE  = 1;



    PrinterImpl(String sessionname,int prot,Share share,NBTSession nbt, SMBMessage packet) throws IOException {
        super(sessionname,prot,share,nbt,packet);

    }




    /**
     * Enumerates the list of the elements currently in the printer queue
     * on the server
     * @return <code>CifsPrinterSpoolInfo</code> array
     */
    public CifsPrinterSpoolInfo[] listPrinterSpoolInfo() throws IOException {

        if (Debug.debugOn && Debug.debugLevel >= Debug.INFO)
            Debug.println(Debug.INFO,"SMB_COM_GET_PRINT_QUEUE"  );

        setupSMBMessage(fMsg,SMBMessage.SMB_COM_GET_PRINT_QUEUE);


        /*
            UCHAR WordCount;	Count of parameter words = 2
            USHORT MaxCount;	Max number of entries to return
            USHORT StartIndex;	First queue entry to return
            USHORT ByteCount;	Count of data bytes = 0


        */



        fMsg.setWordCount(2);

        // MaxCount
        fMsg.setShortParameterAt(0,20);
        // Mode

        fMsg.setShortParameterAt(2,0);

        // WinNT need this bit
        fMsg.setCaselessPathnames();


        fMsg.setContentSize(0);

        fMsg.sendAndReceive(fNBTSession,fMsg);

        int errorclass = fMsg.getErrorClass();

        if (errorclass != CifsIOException.SUCCESS)
            throw new CifsIOException(errorclass,fMsg.getErrorCode());

        /*
        UCHAR WordCount;	Count of parameter words = 2
            0:USHORT Count;	Number of entries returned
            2:USHORT RestartIndex;	Index of entry after last returned
            USHORT ByteCount;	Count of data bytes;  min = 3
            UCHAR BufferFormat;	0x01 -- Data block
            USHORT DataLength;	Length of data
            UCHAR Data[];	Queue elements
        */
        int count = fMsg.getShortParameterAt(0);

        int pos = fMsg.getContentOffset();

        // skip BufferFormat and DataLength
        pos += 3;

        CifsPrinterSpoolInfo[] result = new CifsPrinterSpoolInfo[count];

        for(int i=0;i<count;i++) {
            CifsPrinterSpoolInfo info = new CifsPrinterSpoolInfo();

            // date
            int d = fMsg.getShortAt(pos);
            pos += 2;

            // time
            int t = fMsg.getShortAt(pos);
            pos += 2;

            info.fCreationTime = Util.getDateTime(d,t);

            // status
            info.fStatus = fMsg.getByteAt(pos) & 0xff;
            pos += 1;

            info.fSpoolFileNumber = fMsg.getShortAt(pos);
            pos += 2;

            info.fSpoolFileSize = fMsg.getIntAt(pos) & 0xffffffff;
            pos += 4;

            // reserved
            pos += 1;
            info.fSpoolFileName = fMsg.getZTAsciiStringAt(pos,16);
            pos += 16;

            result[i] = info;
        }
        return result;
    }

    /**
     * Creates a new print spool file. The setup length is 0.
     * @param printid  can be used by the server to provide some sort of per-client
     *                 identifying component to the print file
     * @param mode <code>TEXT_MODE</code> or <code>GRAPHICS_MODE</code>
     * @see #TEXT_MODE and #GRAPHICS_MODE
     */
    public CifsPrintOutputStream openPrintFile(String  printid, int mode) throws IOException {

        return doOpenPrintFile(printid,mode,0);
    }

    /**
     * Creates a new print spool file
     * @param printid  can be used by the server to provide some sort of per-client
     *                 identifying component to the print file
     * @param mode <code>TEXT_MODE</code> or <code>GRAPHICS_MODE</code>
     * @param setuplen the number bytes in the first part of the resulting print
     *                 spool file which contains printer-specific control strings.
     *                 The caller must write first the setupm data.
     * @see #TEXT_MODE and #GRAPHICS_MODE
     */
    private CifsPrintOutputStream doOpenPrintFile(String  printid, int mode, int setuplen) throws IOException {
        if (Debug.debugOn && Debug.debugLevel >= Debug.INFO)
            Debug.println(Debug.INFO,"SMB_COM_OPEN_PRINT_FILE:" + printid  );

        setupSMBMessage(fMsg,SMBMessage.SMB_COM_OPEN_PRINT_FILE);


        /*
            UCHAR WordCount;	Count of parameter words = 2
            0:USHORT SetupLength;	Length of printer setup data
            2:USHORT Mode;	0 = Text mode (DOS expands TABs)
	                        1 = Graphics mode
            USHORT ByteCount;	Count of data bytes;  min = 2
            UCHAR BufferFormat;	0x04
            STRING IdentifierString[];	Identifier string

        */



        fMsg.setWordCount(2);

        // SetupLength
        fMsg.setShortParameterAt(0,setuplen);
        // Mode

        fMsg.setShortParameterAt(2,mode);

        // WinNT need this bit
        fMsg.setCaselessPathnames();

        MarshalBuffer data = new MarshalBuffer(printid.length() +  10);

        int pos  = 0;

        pos += data.setByteAt(pos,SMBMessage.DT_ASCII);
        pos += data.setZTAsciiStringAt(pos,printid);
        data.setSize(pos);

        fMsg.setContent(data);

        fMsg.sendAndReceive(fNBTSession,fMsg);

        int errorclass = fMsg.getErrorClass();

        if (errorclass != CifsIOException.SUCCESS)
            throw new CifsIOException(errorclass,fMsg.getErrorCode());

        int fid = fMsg.getShortParameterAt(0);

        CifsPrintOutputStream handle = new CifsPrintOutputStream( this,printid,fid);

        return handle;
    }


    void closePrintFile(int fid) throws IOException {


        if (Debug.debugOn && Debug.debugLevel >= Debug.INFO)
            Debug.println(Debug.INFO,"SMB_COM_CLOSE_PRINT_FILE" );

        setupSMBMessage(fMsg,SMBMessage.SMB_COM_CLOSE_PRINT_FILE);


        /*
            UCHAR WordCount;	Count of parameter words = 1
            USHORT Fid;	File handle
            USHORT ByteCount;	Count of data bytes = 0


        */



        fMsg.setWordCount(1);

        // Fid
        fMsg.setShortParameterAt(0,fid);

        fMsg.setContentSize(0);

        fMsg.sendAndReceive(fNBTSession,fMsg);

        int errorclass = fMsg.getErrorClass();

        // ignore close error
        if (errorclass != CifsIOException.SUCCESS)
            Debug.println(Debug.WARNING,"Close error:" + fMsg.getNTErrorCode());


    }


    void writePrintFile(int fid,byte[] buf, int off, int len) throws IOException {

        // check how many bytes can we send
        int maxData = howManyBytesCanWeSend();
        int wsize ;

        while(len > 0) {
            int ssize = Math.min(len,maxData);

            doWritePrintFile(fid,buf,off,ssize);



            len     -= ssize;
            off     += ssize;

        }

    }

    private void doWritePrintFile(int fid,byte[] data, int off, int len) throws IOException {


        if (Debug.debugOn && Debug.debugLevel >= Debug.INFO)
            Debug.println(Debug.INFO,"SMB_COM_WRITE_PRINT_FILE" );

        setupSMBMessage(fMsg,SMBMessage.SMB_COM_WRITE_PRINT_FILE);


        /*
            UCHAR WordCount;	Count of parameter words = 1
            USHORT Fid;	File handle
            USHORT ByteCount;	Count of data bytes = 0


        */



        fMsg.setWordCount(1);

        // Fid
        fMsg.setShortParameterAt(0,fid);

        int start = fMsg.getContentOffset();

        int pos = start;

        pos += fMsg.setByteAt(pos,SMBMessage.DT_DATA_BLOCK);
        pos += fMsg.setShortAt(pos,len);
        pos += fMsg.setBytesAt(pos,data,off,len);

        fMsg.setContentSize(pos - start);

        fMsg.sendAndReceive(fNBTSession,fMsg);

        int errorclass = fMsg.getErrorClass();

        // ignore close error
        if (errorclass != CifsIOException.SUCCESS)
             throw new CifsIOException(errorclass,fMsg.getErrorCode());


    }


    public String toString() {

        return "Session:" + fSessionName + ", Type=Printer, Share=" + fShare;

    }

    int getSortPosition() {
        return 3;
    }


}