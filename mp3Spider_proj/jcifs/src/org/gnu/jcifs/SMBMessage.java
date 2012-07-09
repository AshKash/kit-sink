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
import java.util.Enumeration;


final class SMBMessage extends MarshalBuffer implements NBTOutput , NBTInput {


    public final static byte SMB_COM_CREATE_DIRECTORY	    = (byte)0x00;
    public final static byte SMB_COM_DELETE_DIRECTORY	    = (byte)0x01;
    public final static byte SMB_COM_OPEN	                = (byte)0x02;
    public final static byte SMB_COM_CREATE	                = (byte)0x03;
    public final static byte SMB_COM_CLOSE	                = (byte)0x04;
    public final static byte SMB_COM_FLUSH	                = (byte)0x05;
    public final static byte SMB_COM_DELETE	                = (byte)0x06;
    public final static byte SMB_COM_RENAME	                = (byte)0x07;
    public final static byte SMB_COM_QUERY_INFORMATION	    = (byte)0x08;
    public final static byte SMB_COM_SET_INFORMATION	    = (byte)0x09;
    public final static byte SMB_COM_READ	                = (byte)0x0A;
    public final static byte SMB_COM_WRITE	                = (byte)0x0B;
    public final static byte SMB_COM_LOCK_BYTE_RANGE	    = (byte)0x0C;
    public final static byte SMB_COM_UNLOCK_BYTE_RANGE	    = (byte)0x0D;
    public final static byte SMB_COM_CREATE_TEMPORARY	    = (byte)0x0E;
    public final static byte SMB_COM_CREATE_NEW	            = (byte)0x0F;
    public final static byte SMB_COM_CHECK_DIRECTORY	    = (byte)0x10;
    public final static byte SMB_COM_PROCESS_EXIT	        = (byte)0x11;
    public final static byte SMB_COM_SEEK	                = (byte)0x12;
    public final static byte SMB_COM_LOCK_AND_READ	        = (byte)0x13;
    public final static byte SMB_COM_WRITE_AND_UNLOCK	    = (byte)0x14;
    public final static byte SMB_COM_READ_RAW	            = (byte)0x1A;
    public final static byte SMB_COM_READ_MPX	            = (byte)0x1B;
    public final static byte SMB_COM_READ_MPX_SECONDARY	    = (byte)0x1C;
    public final static byte SMB_COM_WRITE_RAW	            = (byte)0x1D;
    public final static byte SMB_COM_WRITE_MPX	            = (byte)0x1E;
    public final static byte SMB_COM_WRITE_COMPLETE	        = (byte)0x20;
    public final static byte SMB_COM_SET_INFORMATION2	    = (byte)0x22;
    public final static byte SMB_COM_QUERY_INFORMATION2	    = (byte)0x23;
    public final static byte SMB_COM_LOCKING_ANDX	        = (byte)0x24;
    public final static byte SMB_COM_TRANSACTION	        = (byte)0x25;
    public final static byte SMB_COM_TRANSACTION_SECONDARY	= (byte)0x26;
    public final static byte SMB_COM_IOCTL	                = (byte)0x27;
    public final static byte SMB_COM_IOCTL_SECONDARY	    = (byte)0x28;
    public final static byte SMB_COM_COPY	                = (byte)0x29;
    public final static byte SMB_COM_MOVE	                = (byte)0x2A;
    public final static byte SMB_COM_ECHO	                = (byte)0x2B;
    public final static byte SMB_COM_WRITE_AND_CLOSE	    = (byte)0x2C;
    public final static byte SMB_COM_OPEN_ANDX	            = (byte)0x2D;
    public final static byte SMB_COM_READ_ANDX	            = (byte)0x2E;
    public final static byte SMB_COM_WRITE_ANDX	            = (byte)0x2F;
    public final static byte SMB_COM_CLOSE_AND_TREE_DISC	= (byte)0x31;
    public final static byte SMB_COM_TRANSACTION2	        = (byte)0x32;
    public final static byte SMB_COM_TRANSACTION2_SECONDARY = (byte)0x33;
    public final static byte SMB_COM_FIND_CLOSE2	        = (byte)0x34;
    public final static byte SMB_COM_FIND_NOTIFY_CLOSE	    = (byte)0x35;
    public final static byte SMB_COM_TREE_CONNECT	        = (byte)0x70;
    public final static byte SMB_COM_TREE_DISCONNECT	    = (byte)0x71;
    public final static byte SMB_COM_NEGOTIATE	            = (byte)0x72;
    public final static byte SMB_COM_SESSION_SETUP_ANDX	    = (byte)0x73;
    public final static byte SMB_COM_LOGOFF_ANDX	        = (byte)0x74;
    public final static byte SMB_COM_TREE_CONNECT_ANDX	    = (byte)0x75;
    public final static byte SMB_COM_QUERY_INFORMATION_DISK = (byte)0x80;
    public final static byte SMB_COM_SEARCH	                = (byte)0x81;
    public final static byte SMB_COM_FIND	                = (byte)0x82;
    public final static byte SMB_COM_FIND_UNIQUE	        = (byte)0x83;
    public final static byte SMB_COM_NT_TRANSACT	        = (byte)0xA0;
    public final static byte SMB_COM_NT_TRANSACT_SECONDARY	= (byte)0xA1;
    public final static byte SMB_COM_NT_CREATE_ANDX	        = (byte)0xA2;
    public final static byte SMB_COM_NT_CANCEL	            = (byte)0xA4;
    public final static byte SMB_COM_OPEN_PRINT_FILE	    = (byte)0xC0;
    public final static byte SMB_COM_WRITE_PRINT_FILE	    = (byte)0xC1;
    public final static byte SMB_COM_CLOSE_PRINT_FILE	    = (byte)0xC2;
    public final static byte SMB_COM_GET_PRINT_QUEUE	    = (byte)0xC3;


    /*----------------- SMB_COM_TRANSACTION2 Subcommand codes ----------------*/

    // Create file with extended attributes
    public final static short TRANS2_OPEN2	=(short)0x00;

    // Begin search for files
    public final static short TRANS2_FIND_FIRST2	=(short)0x01;

    // Resume search for files
    public final static short TRANS2_FIND_NEXT2	=(short)0x02;

    // Get file system information
    public final static short TRANS2_QUERY_FS_INFORMATION	=(short)0x03;

    // Get information about a named file or directory
    public final static short TRANS2_QUERY_PATH_INFORMATION	=(short)0x05;

    // Set information about a named file or directory
    public final static short TRANS2_SET_PATH_INFORMATION	=(short)0x06;

    // Get information about a handle
    public final static short TRANS2_QUERY_FILE_INFORMATION	=(short)0x07;

    // Set information by handle
    public final static short TRANS2_SET_FILE_INFORMATION	=(short)0x08;

    // Create directory with extended attributes
    public final static short TRANS2_CREATE_DIRECTORY	=(short)0x0D;

    // Session setup with extended security information
    public final static short TRANS2_SESSION_SETUP	=(short)0x0E;

    // Get a DFS referral
    public final static short TRANS2_GET_DFS_REFERRAL	=(short)0x10;

    // Report a DFS knowledge inconsistency
    public final static short TRANS2_REPORT_DFS_INCONSISTENCY	=(short)0x11	;


    // InformationLevels
    public final static short SMB_INFO_STANDARD	                = 1;
    public final static short SMB_INFO_QUERY_EA_SIZE	        = 2;
    public final static short SMB_INFO_QUERY_EAS_FROM_LIST	    = 3;
    public final static short SMB_FIND_FILE_DIRECTORY_INFO	    = 0x101;
    public final static short SMB_FIND_FILE_FULL_DIRECTORY_INFO	= 0x102;
    public final static short SMB_FIND_FILE_NAMES_INFO	        = 0x103;
    public final static short SMB_FIND_FILE_BOTH_DIRECTORY_INFO	= 0x104;

    /*
     * Data portion types
     */
    public final static  byte DT_DATA_BLOCK     = 1;
    public final static  byte DT_DIALECT        = 2;
    public final static  byte DT_PATHNAME       = 3;
    public final static  byte DT_ASCII          = 4;
    public final static  byte DT_VARIABLE_BLOCK = 5;


    /*
     *  All reserved fields in the SMB header must be zero.
     *  All quantities are sent in native Intel format.
     */

    // Contains 0xff 'SMB'
    private final static int  MAGIC_4       = 0;

    // Command code
    private final static int  COMMAND_1     = 4;       // uchar

    // Status
        // NT-style 32-bit error code
    private  final static int  NT_STATUS_4  = 5;;      // ulong (little-endian)

    // Error Class
    private final static int  ERROR_CLASS_1 = 5;    // uchar
    // Reserved
    private final static int  RESERVED1_1   = 6;     // uchar (0x00)
    // Error
    private final static int  ERROR_CODE_2  = 7;     // ushort
    // Flags
    private final static int  FLAGS_1       = 9;      // uchar
    // Flags2
    private final static int  FLAGS2_2      = 10;     // ushort

    /*
     * Connectionless (12 bytes)
     *
     * CONNECTIONLESS. SID, and CONNECTIONLESS.SEQUENCENUMBER are
     * used when the client to server connection is on a datagram
     * oriented protocol such as IPX.
     */

     // High part of PID (used by NTCREATEANDX )
    private final static int  PID_HIGH_2       = 12;     // ushort
    private final static int  SECURITY_SIG_8   = 14;     // uchar[8]
    // Reserved
    private final static int  RESERVED2_2      = 22;     // uchar[2] (0x00)


    // Tree identifier
    // TID identifies the subdirectory, or "tree", on the server
    // which the client is accessing.  SMBs which do not reference
    // a particular tree should set TID to 0xFFFF
    private final static int   TID_2            = 24;     // ushort

    // Caller's process id
    // PID is the caller's process id, and is generated by the client
    // to uniquely identify a process within the client computer.

    private final static int   PID_2            = 26;      // ushort

    // Unauthenticated user id
    private final static int   UID_2            = 28;      // ushort

    // multiplex id
    // o MID is reserved for multiplexing multiple messages on a
    // single Virtual Circuit (VC).  A response message will always
    // contain the same value as the corresponding request message.
    private final static int   MID_2            = 30;      // ushort
        // ushort
    // Count of parameter words
    private final static int  PARAMETER_COUNT_1 = 32;      // uchar
     // The parameter words
     // ushort[PARAMETER_COUNT_1] Paramter
     // ushort ByteCount
     // uchar buffer[ByteCount]


    // When on, this SMB is being sent from the server
    private final static int FLAG_IS_RESPONSE        = 0x80;

    // When on, all pathnames in this SMB must be treated as caseless
    private final static int FLAG_CASELESS_PATHNAMES = 0x08;

    // If set, any strings in this SMB message are encoded as UNICODE, otherwise ASCII
    private final static int FLAG2_STRING_AS_UNICODE = 0x8000;

    private final static int FLAG2_CLIENT_EXT_ATTR   = 0x0002;

    private final static int FLAG2_CAN_LONG_NAMES    = 0x0001;


    private final static String NL = System.getProperty("line.separator");

    // helper array for initialization
    private final static byte[] ZEROS = new byte[PARAMETER_COUNT_1];

    //--------------------------- Object fields ---------------------



    SMBMessage(int capacity) {
        super(capacity);

        reset();
    }

    private void reset() {

        System.arraycopy(ZEROS,0,fBuffer,0,ZEROS.length);

        /*
        for(int i=0;i<PARAMETER_COUNT_1;i++)
            fBuffer[i] = 0;
        */

        fBuffer[MAGIC_4]   = (byte)0xFF;
        fBuffer[MAGIC_4+1] = (byte)'S';
        fBuffer[MAGIC_4+2] = (byte)'M';
        fBuffer[MAGIC_4+3] = (byte)'B';
        setShortAt(TID_2, (short)(0xFFFF));
    }




    /**
     * Set command type and resets packet
     * @param cmd command
     */
    public void setCommand(byte cmd) {
        reset();
        fBuffer[COMMAND_1] = cmd;
    }

    public byte getCommand() {
        return fBuffer[COMMAND_1];
    }

    public int getErrorClass() {
        return fBuffer[ERROR_CLASS_1] & 0xFF;

    }
    public int getErrorCode() {
        return getShortAt(ERROR_CODE_2);
    }

    public int getNTErrorCode() {
        return getIntAt(NT_STATUS_4);
    }

    private int getFlags() {
        return fBuffer[FLAGS_1] & 0xFF;
    }



    public boolean isResponse() {
        return ((getFlags() & FLAG_IS_RESPONSE) != 0);
    }

    /**
     * Strings in SMB are UNICODE encoded (Flag2)
     */
    public void setStringsAsUnicode() {
        int flag = getFlags2() | FLAG2_STRING_AS_UNICODE;
        setFlags2(flag);
    }

    /**
     * Strings in SMB are UNICODE encoded (Flag2)
     */
    public boolean isStringsAsUnicode() {
        return ((getFlags2() & FLAG2_STRING_AS_UNICODE) != 0);
    }

    /**
     * We can handle long components in path names in the response
     */
    public void setCanHandleLongNames() {
        int flag = getFlags2() | FLAG2_CAN_LONG_NAMES;
        setFlags2(flag);
    }

    /**
     * If set the client is aware of extended attr
     */
    public void setExtendedAttributes() {
        int flag = getFlags2() | FLAG2_CLIENT_EXT_ATTR;
        setFlags2(flag);
    }

    public void setCaselessPathnames() {
        int flag = getFlags() | FLAG_CASELESS_PATHNAMES;
        fBuffer[FLAGS_1] = (byte) (flag & 0xff);
    }

    private int getFlags2() {
        return getShortAt(FLAGS2_2);
    }

    private void setFlags2(int flag) {
        setShortAt(FLAGS2_2, (short)(flag & 0xFFFF));
    }

    public int getTID() {
         return getShortAt(TID_2);
    }

    public void setTID(int tid) {
        setShortAt(TID_2, tid);
    }

    public int getPID() {
         return getShortAt(PID_2);
    }

    public void setPID(int tid) {
        setShortAt(PID_2, tid);
    }

    public int getUID() {
         return getShortAt(UID_2);
    }

    public void setUID(int tid) {
        setShortAt(UID_2, tid);
    }

    public int getMID() {
         return getShortAt(MID_2);
    }

    public void setMID(int mid) {
        setShortAt(MID_2, mid);
    }


    public void setWordCount(int num) {
        fBuffer[PARAMETER_COUNT_1] = (byte) (num & 0xff);
    }
    public int getWordCount() {
        return fBuffer[PARAMETER_COUNT_1]  & 0xff;
    }


    public int getParameter(int index) {
        int offset =  PARAMETER_COUNT_1 + 1 + 2*index ;
        return getShortAt(offset);
    }

    public int getShortParameterAt(int pos) {
        int offset =  PARAMETER_COUNT_1 + 1 + pos ;
        return getShortAt(offset);

    }

    public short getSignedShortParameterAt(int pos) {
        int offset =  PARAMETER_COUNT_1 + 1 + pos ;
        return getSignedShortAt(offset);

    }
    public void setShortParameterAt(int pos, int val) {
        int offset =  PARAMETER_COUNT_1 + 1 + pos ;
        setShortAt(offset,val);

    }
    public int getIntParameterAt(int pos) {
        int offset =  PARAMETER_COUNT_1 + 1 + pos ;
        return getIntAt(offset);
    }
    public void setIntParameterAt(int pos, int val) {
        int offset =  PARAMETER_COUNT_1 + 1 + pos ;
        setIntAt(offset,val);

    }
    public void setByteParameterAt(int pos, byte val) {
        int offset =  PARAMETER_COUNT_1 + 1 + pos ;
        fBuffer[offset] = val;

    }
    public byte getByteParameterAt(int pos) {
        int offset =  PARAMETER_COUNT_1 + 1 + pos ;
        return fBuffer[offset];

    }

    public void setContent(byte[] content, int offset, int len) {
        // pos points to ByteCount
        int pos = getContentOffset() - 2;
        setShortAt(pos, (len & 0xFFFF));
        pos += 2;
        for(int i=0;i<len;i++)
            fBuffer[pos+i] = content[offset+i];

    }

    public void setContent(byte[] content) {
        setContent(content,0,content.length);
    }

    public void setContentSize(int size) {
        int pos = getContentOffset() - 2;
        setShortAt(pos, size);
    }

    public void setContent(MarshalBuffer content) {
        setContent(content.getBytes(),0,content.getSize());
    }
    public int getContentSize() {
        int pos = getContentOffset() - 2;
        return getShortAt(pos) ;

    }

    public int getContentOffset() {
        int params = getWordCount();
        return (PARAMETER_COUNT_1 + 1 + params*2 + 2);

    }


    public void copyTo(int pos,byte[] buf, int off, int len) {
        System.arraycopy(fBuffer,pos,buf,off,len);
    }

    public int getMessageSize() {
           return getContentOffset() + getContentSize();
    }


    public byte[] getMessageBuffer() {
        return fBuffer;
    }


    public void zero(int pos, int len) {

        if (pos + len > fBuffer.length)
            len = fBuffer.length - pos;

        for(int i=0;i<len;i++)
            fBuffer[pos+i] = (byte)0;
    }


    public void send(NBTSession nbt) throws IOException {


        if (Debug.debugOn)
            dumpPacket("Send SMB buffer");

        nbt.doSend(this);

    }

    public  void receive(NBTSession nbt) throws IOException {

        nbt.doReceive(this);

        if (Debug.debugOn)
            dumpPacket("Receive SMB buffer");

    }
    public  void sendAndReceive(NBTSession nbt,SMBMessage reply) throws IOException {


        nbt.doSend(this);

        if (Debug.debugOn)
            dumpPacket("Send SMB buffer");

        nbt.doReceive(reply);

        if (Debug.debugOn)
            dumpPacket("Receive SMB buffer");

    }

    //------------------------- NetBIOSOutput -------------------------
    public int  getSize() {
        return getMessageSize();

    }

    public void writeTo(Writer out, int size) throws IOException {

        for(int i=0;i<size;i++)
            out.write(fBuffer[i] & 0xff);
    }

     public void writeTo(OutputStream out, int size) throws IOException {

        out.write(fBuffer,0,size);
    }

    /**
     * Writes the buffer to output stream
     * @param pos offset in buffer
     * @param out output stream
     * @param size size to write
     */
    public void writeTo(int pos,OutputStream out, int size) throws IOException {
        out.write(fBuffer,pos,size);

    }

    public void writeTo(int pos,Writer out, int size) throws IOException {
        for(int i=0;i<size;i++)
            out.write(fBuffer[pos+i] & 0xff);

    }
    //------------------------- NetBIOSInput -------------------------
    public int readFrom(InputStream in, int size) throws IOException {

        if (size <= 0)
            return 0;

        if (fBuffer.length < size)
            setCapacity(size);

        int count = 0;

        while(size > 0) {

		    int result = in.read(fBuffer, count, size);

		    if(result <= 0)
                throw new EOFException();

		    count += result;
		    size  -= result;
	    }
        return count;
    }



    private void dumpPacket(String title) {

        if (!Debug.debugOn || Debug.debugLevel < Debug.BUFFER)
            return;

        Debug.println(Debug.BUFFER,title);


        StringBuffer buf = new StringBuffer(300);

        buf.append("smb_com  = 0x"  + Util.byteToHex(getCommand())+ NL);
        buf.append("smb_rcls = "  + getErrorClass()+ NL);
        buf.append("smb_reh  = "  + getErrorCode()+ NL);
        buf.append("smb_flg  = 0x"  + Util.intToHex(getFlags())+ NL);
        buf.append("smb_flg2 = 0x"  + Util.intToHex(getFlags2())+ NL);
        buf.append("smb_tid  = 0x"  + Util.shortToHex(getTID())+ NL);
        buf.append("smb_pid  = 0x"  + Util.shortToHex(getPID())+ NL);
        buf.append("smb_uid  = 0x"  + Util.shortToHex(getUID())+ NL);
        buf.append("smb_mid  = 0x"  + Util.shortToHex(getMID())+ NL);

        int wc = getWordCount();
        buf.append("smb_wct  = "  + wc + NL);


        for(int i=0;i<wc;i++) {
            short par = (short)getParameter(i);
            buf.append("smb_vwv[" + i + "]= 0x"  + Util.shortToHex(par));
            buf.append(" (" + par + ")"+ NL);
        }
        int len = getContentSize();
        buf.append("smb_bcc  = "  + len+ NL);



        int off = getContentOffset();
        buf.append("smb_boff = "  + off+ NL);
        Debug.print(Debug.BUFFER,buf.toString());
        Debug.println(Debug.BUFFER,fBuffer,off,len);
    }
}