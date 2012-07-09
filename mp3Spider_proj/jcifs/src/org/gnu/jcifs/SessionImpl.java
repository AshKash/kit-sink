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
import java.net.InetAddress;

/**
 * Abstract base class of all service sessions
 * @author  Norbert Hranitzky
 * @version 1.0, 20 Nov 1998
 * @since   1.0
 */
public abstract class SessionImpl implements  CifsSession {

    /*
     * CIFS servers select the most recent version of the protocol known to both
     * client and server.  Any CIFS server which supports dialects newer than the
     * original core dialect must support all the messages and semantics of the
     * dialects between the core dialect and the newer one.  This is to say that
     * a server which supports the NT LM 0.12 dialect must also support all of the
     * messages of the previous 10 dialects.  It is the client's responsibility to
     * ensure it only sends SMBs which are appropriate to the dialect negotiated.
     * Clients must be prepared to receive an SMB response from an earlier protocol
     * dialect -- even if the client used the most recent form of the request.
     */

    // The original MSNET SMB protocol (otherwise known as the "core protocol")
    final static int  SMB_CORE = 0;

    // This is the first version of the full LANMAN 1.0 protocol
    final static int  LANMAN_1_0 = 1;

	// This is the first version of the full LANMAN 2.0 protocol
	final static int  LM_1_2X002 = 2;


    // The SMB protocol designed for NT networking.  This has special SMBs
    // which duplicate the NT semantics.
    final static int NT_LM_0_12  = 3;


    final static String[] SUPPORTED_DIALECTS = {

              "PC NETWORK PROGRAM 1.0"

             ,"LANMAN1.0"
             ,"LM1.2X002"
             ,"NT LM 0.12"
     };


    // Max buffer size for setup. Cifs
    private final static int JCIFS_MAX_BUFFER_SIZE = 40*1024;

    //----------------- Security mode -----------------------
    private final static int SM_USER_MODE         = 0x01;
    private final static int SM_ENCRYPT_PASSWORDS = 0x02;
    // Security signature ( SMB sequence numbers) enabled
    private final static int SM_SEC_SIG_ENABLED   = 0x04;
    // Security signature ( SMB sequence numbers) required
    private final static int SM_SEC_SIG_REQUIRED  = 0x08;


    //The server supports SMB_COM_READ_RAW and SMB_COM_WRITE_RAW
    final  static int CAP_RAW_MODE	        = 0x0001;
    // The server supports SMB_COM_READ_MPX and SMB_COM_WRITE_MPX
    final  static int CAP_MPX_MODE	        = 0x0002	;
    // 	The server supports Unicode strings
    final  static int CAP_UNICODE	        = 0x0004;
    // The server supports large files with 64 bit offsets
    final  static int CAP_LARGE_FILES	    = 0x0008;
    // 	The server supports the SMBs particular to the NT LM 0.12 dialect
    final  static int CAP_NT_SMBS	        = 0x0010;
    // 	The sever supports remote API requests via RPC
    final  static int CAP_RPC_REMOTE_APIS	= 0x0020;
    // 	The server can respond with 32 bit status codes in Status.Status
    final  static int CAP_STATUS32	        = 0x0040;
    // The server supports level 2 oplocks
    final  static int CAP_LEVEL_II_OPLOCKS	= 0x0080;
    // 	The server supports the SMB_COM_LOCK_AND_READ SMB
    final  static int CAP_LOCK_AND_READ	    = 0x0100;
    final  static int CAP_NT_FIND	        = 0x0200;
    // This server is DFS aware
    final  static int CAP_DFS	            = 0x1000;
    // 	This server supports SMB_BULK_READ, SMB_BULK_WRITE
    final  static int CAP_BULK_TRANSFER	    = 0x20000000;
    // This server supports compressed data transfer
    // (BULK_TRANSFER capability is required in order to support compressed data transfer)
    final  static int CAP_COMPRESSED_DATA	= 0x40000000;
    // 	This server supports extended security validation
    final  static int CAP_EXTENDED_SECURITY	= 0x80000000;

    // Share name
    protected Share          fShare;

    // NetBios over TCP/IP session
    protected NBTSession     fNBTSession;

    // SMB message
    protected SMBMessage     fMsg;

    // User ID (returned by the server)
    protected int            fUID;

    // Tree ID (returned by the server)
    protected int            fTID;

    // Process ID
    protected static int     fPID = new Random(System.currentTimeMillis()).nextInt();
    private   int            fMID = 0;

    // Login dialog
    private   LoginDialog     fLoginDialog = null;

    protected byte            fSecurityMode         = 0;

    // Max pending multiplexed requests
    protected int             fMaxPendingMPRequests = 0;

    // Max VCs between client and server
    protected int             fMaxVCs               = 0;

    // Max transmit buffer size
    protected int             fMaxBufferSize        = 0;

    // Maximum raw buffer size
    protected int             fMaxRawSize           = 0;

    // Unique token identifying this session
    protected int             fSessionKey           = 0;

    // Server capabilities (see CAP_*)
    protected int             fCapabilities         = 0;

    // System time from 1970 in msec
    protected long            fSystemTime           = 0L;

    // Time zone of server (min from UTC)
    protected int             fTimeZone             = 0;

    // Length of challenge encryption key
    protected int             fEncryptionKeyLen     = 0;

    // Encryption challenge key
    private byte[]            fEncryptionKey        = null;

    // Extended security
    protected boolean         fExtendedSecurity     = false;

    // True if user ist logged in as guest
    protected boolean         fLoggedAsGuest        = false;

    // Server OS (may be empty, Win95)
    protected String          fServerOS             = "";

    // Server LAN Manager (may be empty, Win95)
    protected String          fServerLanMan         = "";

    // Server Primary Domain (may be empty, Win95)
    protected String          fServerPrimaryDomain  = "";

    // Service user properties
    protected Hashtable       fCallerProperties     = new Hashtable();

    // True if disconnected
    private  boolean          fConnectionLost = false;

    private boolean           fAutoReconnect  = true;

    // Negotiated protocol index
    protected int             fProtocol = 0;

    // Session table
    private static Hashtable  fSessionTable  = new Hashtable();

    protected String          fSessionName = "";

    private static int        fSessionNameCounter = 1;

    private long              fConnectTime;

    private SessionImpl() {
    }

    /**
     * Constructor
     * @param share share object
     * @param msg   message containing negotiated data
     * @param nbt   NetBios session
     */
    SessionImpl(String sessionname,int protocol,Share share,NBTSession nbt, SMBMessage msg) throws IOException {
        fShare       = share;
        fNBTSession  = nbt;
        fMsg         = msg;
        fProtocol    = protocol;
        setNegotiatedData(msg);
        if (sessionname == null)
            fSessionName = "Session" + fSessionNameCounter++;
        else
            fSessionName = sessionname;
    }

    /**
     * Sets automatic reconnection
     * @param on true if automatic reconnection allowed
     */
     public void setAllowAutoReconnection(boolean on) {
        fAutoReconnect = on;
     }


    /**
     * Returns the name of this session
     * @return session name
     */
    public final String getShareName () {
        return fShare.getShareName();
    }
    /**
     * Returns share name
     * @reteurn share name
     */
    public final String getSessionName () {
        return fSessionName;
    }

    /**
     * Returns server OS name
     * @return os name or blank if unknown
     */
    public final String getServerOS() {
        return fServerOS;
    }

    /**
     * Returns LAN Manager of the server
     * @return LAN Manager or blank if unknown
     */
    public final String getServerLanMan() {
        return fServerLanMan;
    }


    /**
     * Returns the primary domain of the server
     * @return primary domain or blank if unknown
     */
    public final String getServerPrimaryDomain() {
        return fServerPrimaryDomain;
    }

    /**
     * Gets NetBIOS name
     * @return NetBIOS name of the server
     */
    public final String getNetBIOSName() {
        return fNBTSession.getNetBIOSName();
    }

    /**
     * Gets the address of the server
     * @return InetAddress address
     */
    public final InetAddress getServerAddress() {
        return fNBTSession.getInetAddress();
    }
    /**
     * Time zone of server (min from UTC)
     * @return minutes
     */
    public final int getServerTimeZone() {
        return fTimeZone;
    }

    /**
     * Returns the server time (from 1970 in msec)
     * @return msec
     */
    public final long getServerTime() {
        return fSystemTime;
    }

    /**
     * Checks if the server is connected
     * @return true if the connection is alive
     */
    public synchronized boolean isConnected() {
        return (fNBTSession != null && fNBTSession.isAlive());
    }

    /**
     * Sets an API-user property. The value is not interpreted by
     * CifsService
     * @param key property name
     * @param value property value;
     * @see #getProperty(String)
     */
    public final void setProperty(String key, Object value) {
        fCallerProperties.put(key,value);
    }

    /**
     * Gets an API-user property
     * @param key property name
     * @return  property value;
     * @see #setProperty(String)
     */
    public final Object getProperty(String key) {
        return fCallerProperties.get(key);
    }

    /**
     * Returns true if the share has user level security
     * @return true user level, false share level
     */
    public final boolean isUserLevelSecurity() {
        return ((fSecurityMode & SM_USER_MODE) != 0);
    }

    /**
     * Returns the connect time in milliseconds (base:  January 1, 1970 UTC )
     * @return time in milliseconds
     */

     public final long getConnectTime() {
        return fConnectTime;
     }

    /**
     * Reconnects server if disconnected
     * @exception IOException  if an I/O error occurs
     */
    public synchronized void reconnect() throws IOException {
        if (isConnected())
            return;


        if (Debug.debugOn)
            Debug.println(Debug.INFO,"Reconnect session");

        if (fMsg == null)
            fMsg = allocateSMBMessage();

        if (fNBTSession == null)
            fNBTSession  = new NBTSession();

        fConnectionLost = false;

        try {
            fProtocol = negotiate(fNBTSession,fShare.getHostName(),fMsg);
            setNegotiatedData(fMsg);
            connect();

        } catch(IOException e) {
            fNBTSession.doHangup();
            fNBTSession = null;

            throw e;
        }

    }

    protected void checkConnection() throws IOException {
        if (!fConnectionLost && isConnected())
            return;

        if (fConnectionLost && fAutoReconnect)
            reconnect();
    }

    /**
     *  session setup and tree connect
     * @exception IOException  if an I/O error occurs
     */
    void connect() throws IOException {

        trySessionSetup();

        tryTreeConnect();

        addSession(fSessionName,this);


        fConnectTime = System.currentTimeMillis();


        fNBTSession.addSessionListener (new NBTSessionListener () {
            public void connectionLost () {
                fConnectionLost = true;
            }
        }
        );
        fConnectionLost = false;

        if (Debug.debugOn & Debug.debugLevel >= Debug.INFO)
            debug();

    }


    /**
     * Disconnect the connection
     */
    public void  disconnect() {

        doTreeDisconnect();
        //logoff();
        fNBTSession.doHangup();
        fNBTSession = null;
        fMsg  = null;
        removeSession(fSessionName);
    }

	/**
	 * Finalization: disconnect
	 */
	public void finalize() {
		
		disconnect();
	}
	
    /**
     * Try session setup
     * @exception IOException  if an I/O error occurs
     */
    private void trySessionSetup() throws IOException {

        // try first without asking for password
        while(true) {

            if (doSessionSetup())
                return;

            // ask for password
            if (!promptLogin())
                throw new CifsIOException(CifsIOException.ERROR_SRV,CifsIOException.SRV_BAD_PASSWORD);
        }
    }

    /**
     * Try tree connect
     * @exception IOException  if an I/O error occurs
     */
    private void tryTreeConnect() throws IOException {

        CifsLogin login = fShare.getLogin();

        String password = login.getPassword();


        while(true) {

            // Try as is
            if (doTreeConnect(password))
                return;

            // if no password, ask for password
            if (login.getPassword() == null) {
                if (!promptLogin())
                    throw new CifsIOException(CifsIOException.ERROR_SRV,CifsIOException.SRV_BAD_PASSWORD);

                password = login.getPassword();

            } else {
                // check upper case password
                String upper = login.getPassword().toUpperCase();

                if (doTreeConnect(upper))
                    return;

                if (!promptLogin())
                   throw new CifsIOException(CifsIOException.ERROR_SRV,CifsIOException.SRV_BAD_PASSWORD);

                password = login.getPassword();
            }
        }
    }



    /**
     * Allocates SMB message buffer
     * @return SMBMessage SMB message
     */
    static SMBMessage allocateSMBMessage() {
        return new SMBMessage(JCIFS_MAX_BUFFER_SIZE);
    }



   /**
     * Negotiates protocol (we support only NT_LM_0_12). Calls NetBIOS
     * @param nbt NetBios session
     * @param nbtname NetBIOS name
     * @param msg  SMB message
     * @return negotiated protocol
     * @exception IOException  if an I/O error occurs
     */
    static int negotiate(NBTSession nbt,String nbtname,SMBMessage msg) throws IOException {

        if (Debug.debugOn)
            Debug.println(Debug.INFO,"SMB_COM_NEGOTIATE");

        nbt.doCall(nbtname);

        msg.setCommand(SMBMessage.SMB_COM_NEGOTIATE);
        msg.setPID(SessionImpl.fPID);

        /**
         * struct {
	     *     UCHAR BufferFormat;    // DT_DIALECT
	     *     UCHAR DialectName[];   // Null-terminated
         * } Dialects[];
         */

        StringBuffer buf = new StringBuffer();
        for(int i=0;i<SUPPORTED_DIALECTS.length;i++) {
            buf.append((char)SMBMessage.DT_DIALECT);
            buf.append(SUPPORTED_DIALECTS[i]);
            buf.append('\0');
        }

        msg.setContent(buf.toString().getBytes(MarshalBuffer.ISO8859_1));

        msg.sendAndReceive(nbt,msg);

        int protocol = msg.getParameter(0);

        if (protocol == -1)
            throw new CifsIOException("PE1");


        if (protocol != NT_LM_0_12)
            throw new CifsIOException("PE2",SUPPORTED_DIALECTS[protocol]);

        if (msg.getWordCount() != 17)
             throw new CifsIOException("PE2",SUPPORTED_DIALECTS[protocol]);

		if (Debug.debugOn & Debug.debugLevel >= Debug.INFO)
            Debug.println(Debug.INFO,"Negotiated protocol:" + SUPPORTED_DIALECTS[protocol]);
                        	
            	
        return protocol;
    }

    /**
     * Sets negotiated data
     * @param msg SMB message returned by negotiation
     */
    private void setNegotiatedData(SMBMessage msg) throws IOException {

        // Security mode at position 2
        fSecurityMode         = msg.getByteParameterAt(2);

        fExtendedSecurity     = ((fCapabilities & CAP_EXTENDED_SECURITY) != 0);

        fMaxPendingMPRequests = msg.getShortParameterAt(3);

        fMaxVCs               = msg.getShortParameterAt(5);

        fMaxBufferSize        = msg.getIntParameterAt(7);

        fMaxRawSize           = msg.getIntParameterAt(11);

        fSessionKey           = msg.getIntParameterAt(15);

        fCapabilities         = msg.getIntParameterAt(19);

        // System time from 1601 in 100ns
        long  lo_time         = msg.getIntParameterAt(23) & 0xffffffff;
        long  hi_time         = msg.getIntParameterAt(27) & 0xffffffff;


        // System time from 1601 in 100ns -> convert it to msec
        fSystemTime         = Util.convert1601Time1970( ((hi_time << 32) + lo_time)/10000);

        fTimeZone           = msg.getSignedShortParameterAt(31);

        fEncryptionKeyLen   = msg.getByteParameterAt(33) & 0xff;

        int off = msg.getContentOffset();
        byte[] msgbuf = msg.getMessageBuffer();
        int    content_size = msg.getContentSize();

        if (!fExtendedSecurity) {
            // Encryption key
            fEncryptionKey = new byte[fEncryptionKeyLen];

            for(int i=0;i<fEncryptionKeyLen;i++)
                fEncryptionKey[i] = msgbuf[off+i];

        }


    }

    /**
     * Connects to the tree
     * @param password password
     * @return true if ok, false if bad password
     */
    private boolean doTreeConnect(String password) throws IOException {

        if (Debug.debugOn)
            Debug.println(Debug.INFO, "SMB_COM_TREE_CONNECT_ANDX");


        setupSMBMessage(fMsg,SMBMessage.SMB_COM_TREE_CONNECT_ANDX);
        fMsg.setTID((short)0);

        /*
           UCHAR WordCount;	Count of parameter words = 4
        0: UCHAR AndXCommand;	Secondary (X) command; 0xFF = none
        1: UCHAR AndXReserved;	Reserved (must be 0)
        2: USHORT AndXOffset;	Offset to next command WordCount
        4: USHORT Flags;	Additional information
	                    bit 0 set = disconnect Tid
        6: USHORT PasswordLength;	Length of Password[]
            USHORT ByteCount;	Count of data bytes;    min = 3
            UCHAR Password[];	Password
            STRING Path[];	Server name and share name
            STRING Service[];	Service name
        */
        fMsg.setWordCount(4);

        // AndXCommand
        fMsg.setByteParameterAt(0,(byte)0xFF) ;
        // AndXReserved
        fMsg.setByteParameterAt(1,(byte)0) ;
        // AndXOffset
        fMsg.setShortParameterAt(2,0) ;
         // Flags
        fMsg.setShortParameterAt(4,0) ;

        byte[] challenge_response   =  null;


        if ( (fSecurityMode & SM_ENCRYPT_PASSWORDS) != 0)
            challenge_response =  CifsLogin.getNTAuthData(password,fEncryptionKey);
        else
            challenge_response = Util.getZTStringBytes(password);


        //System.out.println("Password=" + Util.bytesToHex(challenge_response));
        fMsg.setShortParameterAt(6,challenge_response.length) ;


        MarshalBuffer data = new MarshalBuffer(100);

        // auth data
        int pos = 0;
        data.setBytesAt(pos,challenge_response,0,challenge_response.length);
        pos += challenge_response.length;

        // share name
        pos += data.setZTAsciiStringAt(pos,fShare.getShareName().toUpperCase());

        String dev;

        switch(fShare.getType()) {
                case Share.DISK:
                    dev = "A:";
                    break;
                case Share.IPC:
                    dev = "IPC";
                    break;
                case Share.PRINTER:
                    dev = "LPT1:";
                    break;
                default:
                    dev = "A:";
                    break;
        }
        pos += data.setZTAsciiStringAt(pos,dev);

        data.setSize(pos);

        fMsg.setContent(data);

        fMsg.sendAndReceive(fNBTSession,fMsg);

        int errorclass = fMsg.getErrorClass();

        if (errorclass != CifsIOException.SUCCESS) {
            int errorcode = fMsg.getErrorCode();

             if ((errorclass == CifsIOException.ERROR_SRV         &&
                  errorcode  == CifsIOException.SRV_BAD_PASSWORD) ||
                 (errorclass == CifsIOException.ERROR_DOS         &&
                  errorcode  == CifsIOException.DOS_NO_ACCESS))
                return false;

            throw new CifsIOException(errorclass,errorcode);
        }

        fUID = fMsg.getUID();
        fTID = fMsg.getTID();

        return true;
    }

    /**
     * Set up the session
     * @return true if ok, false if bad password
     */
    private boolean doSessionSetup() throws IOException {

        if (Debug.debugOn)
            Debug.println(Debug.INFO, "SMB_COM_SESSION_SETUP_ANDX");

        byte[] case_sensitive_passwd   =  null;
        byte[] case_insensitive_passwd =  null;
        String string_passwd = fShare.getLogin().getPassword();


        setupSMBMessage(fMsg,SMBMessage.SMB_COM_SESSION_SETUP_ANDX);


        if ( (fSecurityMode & SM_ENCRYPT_PASSWORDS) != 0) {
           case_sensitive_passwd   = CifsLogin.getNTAuthData(string_passwd,fEncryptionKey);
           case_insensitive_passwd = CifsLogin.getLMAuthData(string_passwd,fEncryptionKey);
        } else {
           case_sensitive_passwd   = CifsLogin.getPasswordBytesUnicode(string_passwd);
           case_insensitive_passwd = CifsLogin.getPasswordBytesAscii(string_passwd);
        }



        /*
                   UCHAR WordCount;	Count of parameter words = 13
                0: UCHAR AndXCommand;	Secondary (X) command;  0xFF = none
                1: UCHAR AndXReserved;	Reserved (must be 0)
                2: USHORT AndXOffset;	Offset to next command WordCount
                4: USHORT MaxBufferSize;	Client's maximum buffer size
                6: USHORT MaxMpxCount;	Actual maximum multiplexed pending requests
                8: USHORT VcNumber;	0 = first (only), nonzero=additional VC number
                10:ULONG SessionKey;	Session key (valid iff VcNumber != 0)
                14:USHORT CaseInsensitivePasswordLength;	Account password size, ANSI
                16:USHORT CaseSensitivePasswordLength;	Account password size, Unicode
                18:ULONG Reserved;	must be 0
                22:ULONG Capabilities;	Client capabilities
                    USHORT ByteCount;	Count of data bytes;    min = 0
                    UCHAR CaseInsensitivePassword[];	Account Password, ANSI
                    UCHAR CaseSensitivePassword[];	Account Password, Unicode
                    STRING AccountName[];	Account Name, Unicode
                    STRING PrimaryDomain[];	Client's primary domain, Unicode
                    STRING NativeOS[];	Client's native operating system, Unicode
                    STRING NativeLanMan[];	Client's native LAN Manager type, Unicode
         */

         fMsg.setWordCount(13);

         // AndXCommand
         fMsg.setByteParameterAt(0,(byte)0xFF) ;
         // AndXReserved
         fMsg.setByteParameterAt(1,(byte)0) ;
         // AndXOffset
         fMsg.setShortParameterAt(2,0) ;
         // MaxBufferSize
         fMsg.setShortParameterAt(4,JCIFS_MAX_BUFFER_SIZE);
         // MaxMpxCount
         fMsg.setShortParameterAt(6,1);
         // VcNumber
         fMsg.setShortParameterAt(8,0);
         // SessionKey
         fMsg.setIntParameterAt(10,0);

         // CaseInsensitivePasswordLength
         fMsg.setShortParameterAt(14,case_insensitive_passwd.length);
         // CaseSensitivePasswordLength
         fMsg.setShortParameterAt(16,case_sensitive_passwd.length);
         // Reserved
         fMsg.setIntParameterAt(18,0);
         // Capabilities
         fMsg.setIntParameterAt(22,CAP_UNICODE | CAP_NT_SMBS);



         MarshalBuffer data = new MarshalBuffer(200);

         int pos = 0;

         // CaseInsensitivePassword
         data.setBytesAt(pos,case_insensitive_passwd,0,case_insensitive_passwd.length);
         pos += case_insensitive_passwd.length;


         // CasSensitivePassword
         data.setBytesAt(pos,case_sensitive_passwd,0,case_sensitive_passwd.length);
         pos += case_sensitive_passwd.length;

         // Account name
         pos += data.setZTAsciiStringAt(pos,fShare.getLogin().getAccount());


         // Primary domain
         String pdomain = System.getProperty("org.gnu.jcifs.PrimaryDomain","?");
         pos += data.setZTAsciiStringAt(pos,pdomain);

         // Native OS
         pos += data.setZTAsciiStringAt(pos,"CIFS JavaVM Client");

         data.setSize(pos);

         fMsg.setContent(data);

         fMsg.sendAndReceive(fNBTSession,fMsg);

         if (!fMsg.isResponse())
            throw new CifsIOException("PE3");


         int errorclass = fMsg.getErrorClass();

         if (errorclass != CifsIOException.SUCCESS) {
            int errorcode = fMsg.getErrorCode();

            // bad password or no access
            if ((errorclass == CifsIOException.ERROR_SRV         &&
                 errorcode  == CifsIOException.SRV_BAD_PASSWORD) ||
                (errorclass == CifsIOException.ERROR_DOS         &&
                 errorcode  == CifsIOException.DOS_NO_ACCESS))
                return false;

            throw new CifsIOException(errorclass,errorcode);
         }

         fUID = fMsg.getUID();

         // System.out.println("UID =" + fMsg.getUID());

         if (fMsg.getWordCount() != 3)
            return true;

        /*
           UCHAR WordCount;	Count of parameter words = 3
        0: UCHAR AndXCommand;	Secondary (X) command;  0xFF = none
        1: UCHAR AndXReserved;	Reserved (must be 0)
        2: USHORT AndXOffset;	Offset to next command WordCount
        4: USHORT Action;	Request mode:
	                bit0 = logged in as GUEST
        6: USHORT SecurityBlobLength	length of Security Blob that follows in a later field
        8: USHORT ByteCount;	Count of data bytes
            UCHAR SecurityBlob[]	SecurityBlob of length specified in field SecurityBlobLength
            STRING NativeOS[];	Server's native operating system
            STRING NativeLanMan[];	Server's native LAN Manager type
            STRING PrimaryDomain[];	Server's primary domain
        */

        byte action = fMsg.getByteParameterAt(4);

        if ( (action & 0x01) != 0)
            fLoggedAsGuest = true;


        int byte_count = fMsg.getContentSize();
        int off        = fMsg.getContentOffset();
        int max_off    = off + byte_count;

        /*
        // Skip Securit Blob
        off += fMsg.getShortParameterAt(6);
        */
        if (off >= max_off)
            return true;

        // Read NativeOS
        fServerOS = fMsg.getZTAsciiStringAt(off,max_off - off);
        off += fServerOS.length() + 1;

        if (off >= max_off)
            return true;

        // Read NativeLanMan
        fServerLanMan = fMsg.getZTAsciiStringAt(off,max_off - off);

        off += fServerLanMan.length() + 1;

        if (off >= max_off)
            return true;

        // Read PrimaryDomain
        fServerPrimaryDomain = fMsg.getZTAsciiStringAt(off,max_off - off);

        return true;
    }

    /**
     * Disconnects Tree
     */
    private void doTreeDisconnect() {

        if (Debug.debugOn)
            Debug.println(Debug.INFO,"SMB_COM_TREE_DISCONNECT");

        if (!fNBTSession.isAlive())
            return;




        try {

            setupSMBMessage(fMsg,SMBMessage.SMB_COM_TREE_DISCONNECT);

            fMsg.setWordCount(0);
            fMsg.setContentSize(0);

            fMsg.sendAndReceive(fNBTSession,fMsg);

            int errorclass = fMsg.getErrorClass();

            // Ignores errors
            if (errorclass != CifsIOException.SUCCESS)
                Debug.println(Debug.WARNING,"SMB_COM_TREE_DISCONNECT: Error=" + fMsg.getNTErrorCode());

        } catch(Exception e) {
            Debug.println(Debug.WARNING,"SMB_COM_TREE_DISCONNECT: Exception=" + e);
        }

    }

    /*
     * Logoff (inverse of Setup session)
     * @tbd Returns always error!!!!
     */

    private void logoff() {
        if (Debug.debugOn & Debug.debugLevel >= Debug.INFO)
            Debug.println(Debug.INFO,"SMB_COM_LOGOFF_ANDX");

        if (!fNBTSession.isAlive())
            return;





        try {
            setupSMBMessage(fMsg,SMBMessage.SMB_COM_LOGOFF_ANDX);

            fMsg.setWordCount(2);
            fMsg.setByteParameterAt(0,(byte)0xff);
            fMsg.setByteParameterAt(1,(byte)0);
            fMsg.setShortParameterAt(2,0);
            fMsg.setContentSize(0);

            fMsg.sendAndReceive(fNBTSession,fMsg);

            int errorclass = fMsg.getErrorClass();

            if (errorclass != CifsIOException.SUCCESS)
                Debug.println(Debug.WARNING,"SMB_COM_LOGOFF_ANDX: Error=" + fMsg.getNTErrorCode());

        } catch(Exception e) {
            Debug.println(Debug.WARNING,"SMB_COM_LOGOFF_ANDX: Exception=" + e);
        }

    }

    /**
     * Ping the server to test the connection to the server and to
     * see if the server is still responding
     * @param  text text to send
     * @return text returned by server (must be the same as the input text)
     * @exception  IOException  if an I/O error occurs.
     */
    public synchronized String echo(String text) throws IOException {

        if (Debug.debugOn)
            Debug.println(Debug.INFO,"ping (SMB_COM_ECHO)" );

        // now we support only 0 or 1

        int echos = 1;

        setupSMBMessage(fMsg,SMBMessage.SMB_COM_ECHO);


        /*
            UCHAR WordCount;	Count of parameter words = 1
            USHORT EchoCount;	Number of times to echo data back
            USHORT ByteCount;	Count of data bytes;    min = 1
            UCHAR Buffer[1];	Data to echo
        */



        // Set WordCount
        fMsg.setWordCount(1);
        // Set echo count
        fMsg.setShortParameterAt(0,echos);

        MarshalBuffer data = new MarshalBuffer(text.length() +  10);

        int pos  = 0;
        pos += data.setAsciiStringAt(pos,text);
        data.setSize(pos);

        fMsg.setContent(data);

        fMsg.sendAndReceive(fNBTSession,fMsg);

        int errorclass = fMsg.getErrorClass();

        if (errorclass != CifsIOException.SUCCESS)
         throw new CifsIOException(errorclass,fMsg.getErrorCode());

        int size = fMsg.getContentSize();

        if (size == 0)
            return "";

        pos = fMsg.getContentOffset();

        return fMsg.getAsciiStringAt(pos,size);
    }


    /**
     * Sends SMB_COM_TRANSACTION message
     * @param setup setup words
     * @param name  name string
     * @param param parameter buffer
     * @param data  data to send
     * @param ldata length of data
     * @exception IOException  if an I/O error occurs
     */
    protected void sendTransaction(short[] setup, String name,MarshalBuffer param,
                            byte[] data, int ldata) throws IOException {

        if (Debug.debugOn)
            Debug.println(Debug.INFO, "Send SMB_COM_TRANSACTION");

        int lparam = param.getSize();
        int lsetup;

        if (setup == null)
            lsetup = 0;
        else
            lsetup = setup.length;

        setupSMBMessage(fMsg,SMBMessage.SMB_COM_TRANSACTION);

        /*
            UCHAR WordCount;	Count of parameter words;   value = (14 + SetupCount)
            USHORT TotalParameterCount;	Total parameter bytes being sent
            USHORT TotalDataCount;	Total data bytes being sent
            USHORT MaxParameterCount;	Max parameter bytes to return
            USHORT MaxDataCount;	Max data bytes to return
            UCHAR MaxSetupCount;	Max setup words to return
            UCHAR Reserved;
            USHORT Flags;	Additional information:
	                        bit 0 - also disconnect TID in TID
            ULONG Timeout;
            USHORT Reserved2;
            USHORT ParameterCount;	Parameter bytes sent this buffer
            USHORT ParameterOffset;	Offset (from header start) to Parameters
            USHORT DataCount;	Data bytes sent this buffer
            USHORT DataOffset;	Offset (from header start) to data
            UCHAR SetupCount;	Count of setup words
            UCHAR Reserved3;	Reserved (pad above to word)
            USHORT Setup[SetupCount];	Setup words (# = SetupWordCount)
            USHORT ByteCount;	Count of data bytes
            STRING Name[];	Must be NULL
            UCHAR Pad[];	Pad to SHORT or LONG
            UCHAR Parameters[ ParameterCount];	Parameter bytes (# = ParameterCount)
            UCHAR Pad1[];	Pad to SHORT or LONG
            UCHAR Data[ DataCount ];	Data bytes (# = DataCount)
        */
        fMsg.setWordCount(14+lsetup);

        // TotalParameterCount !!!!
        fMsg.setShortParameterAt(0,lparam);

        // TotalDataCount !!!!
        fMsg.setShortParameterAt(2,ldata);

        // MaxParameterCount returned by server
        fMsg.setShortParameterAt(4,16);

        // MaxDataCount returned by server
		// Modified by Ashwin, put 15000 instead of 3000
        fMsg.setShortParameterAt(6,15000);

        // MaxSetupCount returned by server
        fMsg.setByteParameterAt(8,(byte)20);

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
        fMsg.setByteParameterAt(26,(byte)lsetup);

         // Reserved3
        fMsg.setByteParameterAt(27,(byte)0);

        for(int i=0;i<lsetup;i++)
        // Setup[0]
            fMsg.setShortParameterAt(28+i,setup[i]);


        // byteCount
        fMsg.setContentSize(0);

        int bytes_off, off;

        bytes_off = off = fMsg.getContentOffset();


        // reserve 1 (Name) + 4 (Pad) + 4 (Pad1)
        // free bytes for Parameters and Data
        int free = Math.min(fMsg.getCapacity() - bytes_off,fMaxBufferSize) - 9;

        int send_lparam = Math.min(free,lparam);
        int send_ldata  = Math.min(free - send_lparam,ldata);

        // Name of trans (NULL string
        off += fMsg.setZTAsciiStringAt(off,name);


        /*----------------- set parameters --------------------*/
        // Calculate beginning of Parameters (offset from Header)
        int param_off = off = MarshalBuffer.align(off,2);

        // set parameter bytes
        fMsg.setBytesAt(off,param,0,send_lparam);


         // ParameterCount bytes sent this buffer
        fMsg.setShortParameterAt(18,send_lparam);

        // ParameterOffset bytes sent this buffer
        fMsg.setShortParameterAt(20,param_off);

        off += send_lparam;

        /*---------------------- set data --------------------*/
        // Calculate beginning of data (offset from Header)
        int data_off = off = MarshalBuffer.align(off,2);

        // set data bytes

        if (send_ldata > 0)
            fMsg.setBytesAt(off,data,0,send_ldata);

        // DataCount bytes sent this buffer
        fMsg.setShortParameterAt(22,send_ldata);

        // DataOffset bytes sent this buffer
        fMsg.setShortParameterAt(24,data_off);

        off += send_ldata;


        // byteCount
        fMsg.setContentSize(off - bytes_off);

        fMsg.send(fNBTSession);


        if (send_lparam < lparam  || send_ldata < ldata) {

            // receive interim response

            fMsg.receive(fNBTSession);


            int errorclass = fMsg.getErrorClass();

            if (errorclass != CifsIOException.SUCCESS)
                throw new CifsIOException(errorclass,fMsg.getErrorCode());

            int tot_ldata  = send_ldata;
            int tot_lparam = send_lparam;

            while (tot_ldata < ldata || tot_lparam < lparam) {
                // Now send the next packet



                setupSMBMessageSecondary(fMsg,SMBMessage.SMB_COM_TRANSACTION_SECONDARY);

                /*
                   Command	SMB_COM_TRANSACTION_SECONDARY

                   UCHAR WordCount;	Count of parameter words = 8
                0: USHORT TotalParameterCount;	Total parameter bytes being sent
                2: USHORT TotalDataCount;	Total data bytes being sent
                4: USHORT ParameterCount;	Parameter bytes sent this buffer
                6: USHORT ParameterOffset;	Offset (from header start) to Parameters
                8: USHORT ParameterDisplacement;	Displacement of these Parameter bytes
                10:USHORT DataCount;	Data bytes sent this buffer
                12:USHORT DataOffset;	Offset (from header start) to data
                14:USHORT DataDisplacement;	Displacement of these data bytes
                16:USHORT Fid;	FID for handle based requests, else 0xFFFF.  This field is present only if this is an SMB_COM_TRANSACTION2 request.
                   USHORT ByteCount;	Count of data bytes
                   UCHAR Pad[];	Pad to SHORT or LONG
                   UCHAR Parameters[ParameterCount];	Parameter bytes (# = ParameterCount)
                   UCHAR Pad1[];	Pad to SHORT or LONG
                   UCHAR Data[DataCount];	Data bytes (# = DataCount)
                */

                fMsg.setWordCount(8);

                bytes_off = off = fMsg.getContentOffset();
                free = Math.min(fMsg.getCapacity() - bytes_off,fMaxBufferSize) - 9;


                send_lparam = Math.min(lparam - tot_lparam,free);

                send_ldata  = Math.min(ldata - tot_ldata,free - send_lparam);

                // TotalParameterCount
                fMsg.setShortParameterAt(0,lparam);

                // TotalDataCount
                fMsg.setShortParameterAt(2,ldata);

                // ParameterCount
                fMsg.setShortParameterAt(4,send_lparam);

                // ParameterDisplacement
                fMsg.setShortParameterAt(8,tot_lparam);

                // DataCount
                fMsg.setShortParameterAt(10,send_ldata);

                // DataDisplacement
                fMsg.setShortParameterAt(14,tot_ldata);

                // NO FID



                /*----------------- set parameters --------------------*/
                // Calculate beginning of Parameters (offset from Header)
                param_off = off = MarshalBuffer.align(off,4);

                // set parameter bytes
                fMsg.setBytesAt(off,param,tot_lparam,send_lparam);

                // ParameterOffset bytes sent this buffer
                fMsg.setShortParameterAt(6,param_off);

                off += send_lparam;

                /*---------------------- set data --------------------*/
                // Calculate beginning of data (offset from Header)
                data_off = off = MarshalBuffer.align(off,4);

                // set data bytes
                if (send_ldata > 0)
                    fMsg.setBytesAt(off,data,tot_ldata,send_ldata);


                // DataOffset bytes sent this buffer
                fMsg.setShortParameterAt(12,data_off);

                off += send_ldata;


                // byteCount
                fMsg.setContentSize(off - bytes_off);

                fMsg.send(fNBTSession);

                tot_lparam += send_lparam;
                tot_ldata  += send_ldata;
            }

        }

    }

    /**
     * Sends SMB_COM_TRANSACTION2 message
     * @param setup setup words
     * @param name  name string
     * @param param parameter buffer
     * @param data  data to send
     * @param ldata length of data
     * @param fid   file id
     * @exception IOException  if an I/O error occurs
     */
    protected final void sendTransaction2(short setup,
                            MarshalBuffer param,
                            byte[] data, int ldata, int fid) throws IOException {

        if (Debug.debugOn)
            Debug.println(Debug.INFO, "Send SMB_COM_TRANSACTION2");

        int lparam = param.getSize();

        setupSMBMessage(fMsg,SMBMessage.SMB_COM_TRANSACTION2);

        /*
            UCHAR WordCount;	Count of parameter words;   value = (14 + SetupCount)
            USHORT TotalParameterCount;	Total parameter bytes being sent
            USHORT TotalDataCount;	Total data bytes being sent
            USHORT MaxParameterCount;	Max parameter bytes to return
            USHORT MaxDataCount;	Max data bytes to return
            UCHAR MaxSetupCount;	Max setup words to return
            UCHAR Reserved;
            USHORT Flags;	Additional information:
	                        bit 0 - also disconnect TID in TID
            ULONG Timeout;
            USHORT Reserved2;
            USHORT ParameterCount;	Parameter bytes sent this buffer
            USHORT ParameterOffset;	Offset (from header start) to Parameters
            USHORT DataCount;	Data bytes sent this buffer
            USHORT DataOffset;	Offset (from header start) to data
            UCHAR SetupCount;	Count of setup words
            UCHAR Reserved3;	Reserved (pad above to word)
            USHORT Setup[SetupCount];	Setup words (# = SetupWordCount)
            USHORT ByteCount;	Count of data bytes
            STRING Name[];	Must be NULL
            UCHAR Pad[];	Pad to SHORT or LONG
            UCHAR Parameters[ ParameterCount];	Parameter bytes (# = ParameterCount)
            UCHAR Pad1[];	Pad to SHORT or LONG
            UCHAR Data[ DataCount ];	Data bytes (# = DataCount)
        */
        fMsg.setWordCount(14+1);

        // TotalParameterCount !!!!
        fMsg.setShortParameterAt(0,lparam);

        // TotalDataCount !!!!
        fMsg.setShortParameterAt(2,ldata);

        // MaxParameterCount returned by server
        fMsg.setShortParameterAt(4,16);

        // MaxDataCount returned by server
		// Modified by Ashwin, put 15000 instead of 3000
        fMsg.setShortParameterAt(6,15000);

        // MaxSetupCount returned by server
        fMsg.setByteParameterAt(8,(byte)20);

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
        fMsg.setShortParameterAt(28,setup);


        // byteCount
        fMsg.setContentSize(0);

        int bytes_off, off;

        bytes_off = off = fMsg.getContentOffset();


        // reserve 1 (Name) + 4 (Pad) + 4 (Pad1)
        // free bytes for Parameters and Data
        int free = Math.min(fMsg.getCapacity() - bytes_off,fMaxBufferSize) - 9;

        int send_lparam = Math.min(free,lparam);
        int send_ldata  = Math.min(free - send_lparam,ldata);

        // Name of trans (NULL string
        fMsg.setByteAt(off,(byte)0);
        off++;

        /*----------------- set parameters --------------------*/
        // Calculate beginning of Parameters (offset from Header)
        int param_off = off = MarshalBuffer.align(off,2);

        // set parameter bytes
        fMsg.setBytesAt(off,param,0,send_lparam);


         // ParameterCount bytes sent this buffer
        fMsg.setShortParameterAt(18,send_lparam);

        // ParameterOffset bytes sent this buffer
        fMsg.setShortParameterAt(20,param_off);

        off += send_lparam;

        /*---------------------- set data --------------------*/
        // Calculate beginning of data (offset from Header)
        int data_off = off = MarshalBuffer.align(off,2);

        // set data bytes

        if (send_ldata > 0)
            fMsg.setBytesAt(off,data,0,send_ldata);

        // DataCount bytes sent this buffer
        fMsg.setShortParameterAt(22,send_ldata);

        // DataOffset bytes sent this buffer
        fMsg.setShortParameterAt(24,data_off);

        off += send_ldata;


        // byteCount
        fMsg.setContentSize(off - bytes_off);

        fMsg.send(fNBTSession);


        if (send_lparam < lparam  || send_ldata < ldata) {

            // receive interim response

            fMsg.receive(fNBTSession);


            int errorclass = fMsg.getErrorClass();

            if (errorclass != CifsIOException.SUCCESS)
                throw new CifsIOException(errorclass,fMsg.getErrorCode());

            int tot_ldata  = send_ldata;
            int tot_lparam = send_lparam;

            while (tot_ldata < ldata || tot_lparam < lparam) {
                // Now send the next packet



                setupSMBMessageSecondary(fMsg,SMBMessage.SMB_COM_TRANSACTION_SECONDARY);

                /*
                   Command	SMB_COM_TRANSACTION_SECONDARY

                   UCHAR WordCount;	Count of parameter words = 9 (! FID)
                0: USHORT TotalParameterCount;	Total parameter bytes being sent
                2: USHORT TotalDataCount;	Total data bytes being sent
                4: USHORT ParameterCount;	Parameter bytes sent this buffer
                6: USHORT ParameterOffset;	Offset (from header start) to Parameters
                8: USHORT ParameterDisplacement;	Displacement of these Parameter bytes
                10:USHORT DataCount;	Data bytes sent this buffer
                12:USHORT DataOffset;	Offset (from header start) to data
                14:USHORT DataDisplacement;	Displacement of these data bytes
                16:USHORT Fid;	FID for handle based requests, else 0xFFFF.  This field is present only if this is an SMB_COM_TRANSACTION2 request.
                   USHORT ByteCount;	Count of data bytes
                   UCHAR Pad[];	Pad to SHORT or LONG
                   UCHAR Parameters[ParameterCount];	Parameter bytes (# = ParameterCount)
                   UCHAR Pad1[];	Pad to SHORT or LONG
                   UCHAR Data[DataCount];	Data bytes (# = DataCount)
                */

                fMsg.setWordCount(9);

                bytes_off = off = fMsg.getContentOffset();
                free = Math.min(fMsg.getCapacity() - bytes_off,fMaxBufferSize) - 9;


                send_lparam = Math.min(lparam - tot_lparam,free);

                send_ldata  = Math.min(ldata - tot_ldata,free - send_lparam);

                // TotalParameterCount
                fMsg.setShortParameterAt(0,lparam);

                // TotalDataCount
                fMsg.setShortParameterAt(2,ldata);

                // ParameterCount
                fMsg.setShortParameterAt(4,send_lparam);

                // ParameterDisplacement
                fMsg.setShortParameterAt(8,tot_lparam);

                // DataCount
                fMsg.setShortParameterAt(10,send_ldata);

                // DataDisplacement
                fMsg.setShortParameterAt(14,tot_ldata);

                // FID
                fMsg.setShortParameterAt(16,fid);


                /*----------------- set parameters --------------------*/
                // Calculate beginning of Parameters (offset from Header)
                param_off = off = MarshalBuffer.align(off,4);

                // set parameter bytes
                fMsg.setBytesAt(off,param,tot_lparam,send_lparam);

                // ParameterOffset bytes sent this buffer
                fMsg.setShortParameterAt(6,param_off);

                off += send_lparam;

                /*---------------------- set data --------------------*/
                // Calculate beginning of data (offset from Header)
                data_off = off = MarshalBuffer.align(off,4);

                // set data bytes
                if (send_ldata > 0)
                    fMsg.setBytesAt(off,data,tot_ldata,send_ldata);


                // DataOffset bytes sent this buffer
                fMsg.setShortParameterAt(12,data_off);

                off += send_ldata;


                // byteCount
                fMsg.setContentSize(off - bytes_off);

                fMsg.send(fNBTSession);

                tot_lparam += send_lparam;
                tot_ldata  += send_ldata;
            }

        }

    }

    /**
     * Receives SMB_COM_TRANSACTION reply message
     * @param param parameters
     * @param data data buffer
     * @exception IOException  if an I/O error occurs
     */
    protected final void receiveTransaction(MarshalBuffer param, MarshalBuffer data)
                                                        throws IOException {

        if (Debug.debugOn)
            Debug.println(Debug.INFO, "Receive SMB_COM_TRANSACTION");


        fMsg.receive(fNBTSession);

        int errorclass = fMsg.getErrorClass();

        if (errorclass != CifsIOException.SUCCESS)
            throw new CifsIOException(errorclass,fMsg.getErrorCode());

        /*
          UCHAR WordCount;	Count of data bytes; value = 10 + SetupCount
        0:USHORT TotalParameterCount;	Total parameter bytes being sent
        2:USHORT TotalDataCount;	Total data bytes being sent
        4:USHORT Reserved;
        6:USHORT ParameterCount;	Parameter bytes sent this buffer
        8:USHORT ParameterOffset;	Offset (from header start) to Parameters
        10:USHORT ParameterDisplacement;	Displacement of these Parameter bytes
        12:USHORT DataCount;	Data bytes sent this buffer
        14:USHORT DataOffset;	Offset (from header start) to data
        16:USHORT DataDisplacement;	Displacement of these data bytes
        18:UCHAR SetupCount;	Count of setup words
        19:UCHAR Reserved2;	Reserved (pad above to word)
        20:USHORT Setup[SetupWordCount];	Setup words (# = SetupWordCount)
            USHORT ByteCount;	Count of data bytes
            UCHAR Pad[];	Pad to SHORT or LONG
            UCHAR Parameters[ParameterCount];	Parameter bytes (# = ParameterCount)
            UCHAR Pad1[];	Pad to SHORT or LONG
            UCHAR Data[DataCount];	Data bytes (# = DataCount)
        */

        int lparam = 0;
        int ldata  = 0;

        // TotalParameterCount
        int tot_lparam = fMsg.getShortParameterAt(0);
        int tot_ldata  = fMsg.getShortParameterAt(2);

        // alloca buffer
        param.setCapacity(tot_lparam);
        data.setCapacity(tot_ldata);




        while(true) {
            int rcv_lparam = fMsg.getShortParameterAt(6);
            int rcv_ldata  = fMsg.getShortParameterAt(12);

            if (rcv_lparam + lparam > tot_lparam ||
                rcv_ldata  + ldata  > tot_ldata) {
                throw new InternalError("Invalid data");
            }
            if (rcv_lparam > 0) {
                int off_param = fMsg.getShortParameterAt(8);
                int dsp_param = fMsg.getShortParameterAt(10);

                param.setBytesAt(dsp_param,fMsg,off_param,rcv_lparam);
            }
            if (rcv_ldata > 0) {
                int off_data = fMsg.getShortParameterAt(14);
                int dsp_data = fMsg.getShortParameterAt(16);

                data.setBytesAt(dsp_data,fMsg,off_data,rcv_ldata);
            }
            lparam += rcv_lparam;
            ldata  += rcv_ldata;

            // get Total (they can shrink!)
            tot_lparam = fMsg.getShortParameterAt(0);
            tot_ldata  = fMsg.getShortParameterAt(2);

            if (tot_lparam <= lparam && tot_ldata <= ldata)
                break;

            fMsg.receive(fNBTSession);

            errorclass = fMsg.getErrorClass();

            if (errorclass != CifsIOException.SUCCESS)
                throw new CifsIOException(errorclass,fMsg.getErrorCode());

        }

        param.setSize(lparam);
        data.setSize(ldata);
    }

    /**
     * Inititializes SMB message
     * @param msg SMB message
     * @param cmd command
     */
    protected final void setupSMBMessage(SMBMessage msg, byte cmd)  throws IOException {
        msg.setCommand(cmd);
        msg.setUID(fUID);
        msg.setTID(fTID);
        msg.setPID(fPID);
        msg.setMID(nextMID());
        msg.setCanHandleLongNames();
        msg.setExtendedAttributes();

       if (cmd == SMBMessage.SMB_COM_TREE_CONNECT_ANDX ||
           cmd == SMBMessage.SMB_COM_TREE_DISCONNECT   ||
           cmd == SMBMessage.SMB_COM_SESSION_SETUP_ANDX ||
           cmd == SMBMessage.SMB_COM_LOGOFF_ANDX)
           return;

        checkConnection();

    }


    /**
     * Inititializes SMB message for SMB_COM_TRANSACTION_SECONDARY
     * @param msg SMB message
     * @param cmd command
     */
    protected final void setupSMBMessageSecondary(SMBMessage msg, byte cmd) {
        msg.setCommand(cmd);
        msg.setUID(fUID);
        msg.setTID(fTID);
        msg.setPID(fPID);
        msg.setMID(fMID);
        msg.setCanHandleLongNames();
        msg.setExtendedAttributes();

    }

    /**
     * Prompts for login dialog
     * @return false if dialog cancelled
     */
    final boolean promptLogin() {

        if (!CifsSessionManager.getAllowLoginDialog())
            return false;

        if (fLoginDialog == null) {
            fLoginDialog = new LoginDialog(new java.awt.Frame());
        }

        return fLoginDialog.prompt(fShare.getShareName(),fShare.getLogin());
    }

    /**
     * Returns the max number of bytes which can be sent
     * @return number of bytes
     */
    final int howManyBytesCanWeSend() {

        // Message_Capacity - 100 ( > 32 (Header) - 2*15 (Max parameters) )


        return Math.min(fMsg.getCapacity() - 100,fMaxBufferSize - 100);
    }



    /**
     * Enumerates sessions
     * @return Enumeration
     */
    static Enumeration enumerateSessions() {
        return fSessionTable.elements();
    }
    static CifsSession[] getSessions() {
        CifsSession[] sessions;

        synchronized(fSessionTable) {
            sessions = new CifsSession[fSessionTable.size()];
            Enumeration enum = fSessionTable.elements();

            int i = 0;
            while(enum.hasMoreElements() && i < sessions.length)
                sessions[i++] = (CifsSession)enum.nextElement();
        }

        return sessions;

    }
    static CifsSession lookupSession(String sessionname) {
        return (CifsSession)fSessionTable.get(sessionname);
    }
    static void addSession(String sessionname, CifsSession session) {
        fSessionTable.put(sessionname, session);
    }
    static void removeSession(String sessionname) {
        fSessionTable.remove(sessionname);
    }

    abstract int getSortPosition();

    /**
     * Generates next Message ID
     * @return message id
     */
    private synchronized final  int nextMID() {
        fMID++;
        if (fMID == Short.MAX_VALUE)
            fMID = 0;
        return fMID;
    }

    private final void debug() {

        if (!Debug.debugOn || Debug.debugLevel < Debug.INFO)

            return;

        Debug.println(Debug.INFO,"Security mode                     = " + fSecurityMode);
        String am = (isUserLevelSecurity() ? "User" : "Share");
        Debug.println(Debug.INFO,"Access mode                       = " + am);
        Debug.println(Debug.INFO,"Max pending multiplexed requests  = " + fMaxPendingMPRequests);
        Debug.println(Debug.INFO,"Max VCs between client and server = " + fMaxVCs);
        Debug.println(Debug.INFO,"Max transmit buffer size          = " + fMaxBufferSize);
        Debug.println(Debug.INFO,"Max raw      buffer size          = " + fMaxRawSize);
        Debug.println(Debug.INFO,"Session key                       = " + Util.intToHex(fSessionKey));
        Debug.println(Debug.INFO,"Server capabilities               = " + Util.intToHex(fCapabilities));
        Debug.println(Debug.INFO,"System time                       = " + new Date(fSystemTime));
        Debug.println(Debug.INFO,"Time zone                         = " + fTimeZone);
        Debug.println(Debug.INFO,"Encryption key length             = " + fEncryptionKeyLen);
        Debug.println(Debug.INFO,"Encryption key                    = " + Util.bytesToHex(fEncryptionKey));
        Debug.println(Debug.INFO,"UID                               = " + fUID);
        Debug.println(Debug.INFO,"TID                               = " + fTID);
        Debug.println(Debug.INFO,"PID                               = " + fPID);
        Debug.println(Debug.INFO,"Logged as guest                   = " + fLoggedAsGuest);
        Debug.println(Debug.INFO,"Server OS                         = " + fServerOS);
        Debug.println(Debug.INFO,"Server Lanman                     = " + fServerLanMan);
        Debug.println(Debug.INFO,"Server Primary Domain             = " + fServerPrimaryDomain);

        // Dump system properties
        Properties p = System.getProperties();
        Enumeration enum = p.keys();
        Debug.println(Debug.INFO,"---------- System Properties -------------");
        while(enum.hasMoreElements()) {
            String key = (String)enum.nextElement();
            String val = p.getProperty(key);
            Debug.println(Debug.INFO,key + " = " + val);
        }
        Debug.println(Debug.INFO,"------------------------------------------");
    }


}
