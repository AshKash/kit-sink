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
  *  Changes: 1.1 - remove DataInputStream
  */


package org.gnu.jcifs;

import org.gnu.jcifs.util.*;
import java.io.*;
import java.net.*;

/*
 * After the server name has been resolved to an IP address, then a connection
 * to the server needs to be established if one has not already been set up.
 * Connection establishment is done using  the NETBIOS session service,
 * which requires the client to provide a "calling name" and a "called name".
 * The calling name is not significant in CIFS, except that an identical name
 * from the same transport address is assumed to represent the same client;
 * the called name is always "*SMBSERVER      ". Connection establishment
 * results in a "Session Request" packet to port 139 (see section 4.3.2 of RFC 1002).<p>
 *  <b>Backwards compatability</b><p>
 * If a CIFS client wishes to inter-operate with older SMB servers,
 * then if the server rejects the session request, it can retry with a new called name.
 * The choice of the new called name depends on the name resolution mechanism used.
 * If DNS was used, the called name should be constructed from the first component
 * of the server's DNS name, truncated to 15 characters if necessary, and then padded
 * to 16 characters with blank (20 hex) characters. If NETBIOS was used, then the called
 * named is just the NETBIOS name. If these fail, then a NETBIOS "Adapter Status"
 * request may be made to obtain the server's NETBIOS name, and the connection
 * establishment retried with that as the called name.
 */

/**
 * NetBIOS over TCP/IP (Session Service)
 */
final class NBTSession {


    // Standard called name for NetBIOS
    private final static String SMBSERVER_NAME = "*SMBSERVER";

    // Session packet type (byte)
    private final static int HDR_TYPE_1  = 0;

    // Bit 7 is the LENGTH extension
    // Bit 0-6 are reserved and must be 0
    private final static int HDR_FLAGS_1 = 1;

    // Session packet length without header (ushort) and BIG-Endian!!!
    private final static int HDR_LENGTH_2 = 2;

    private final static int HDR_SIZE = 4;


    // Session packet types (Field HDR_TYPE_B)
    private final static byte  SPT_MESSAGE       =  (byte)0;
    private final static byte  SPT_REQUEST       =  (byte)0x81;
    private final static byte  SPT_POS_RESPONSE  =  (byte)0x82;
    private final static byte  SPT_NEG_RESPONSE  =  (byte)0x83;

    // Retarget session response
    private final static byte  SPT_RTG_RESPONSE  =  (byte)0x84;

    private final static byte  SPT_KEEPALIVE     =  (byte)0x85;

    //----------------- Session Request packet ----------------------
    private final static int   RQ_CALLED_NAME_32  = 4;

    //public final static int   RQ_CALLING_NAME_32 = 36;

    //----------------- Session Positive response packet ------------
    // no trailer, LENGTH=0

    //----------------- Session Negative response packet ------------
    private final static int   NR_ERROR_CODE_1 = 8;


    //----------------- Session retargeted response packet ------------
    private final static int   RT_IP_ADDRESS_4 = 8;
    private final static int   RT_PORT_2       = 12;
    private final static int   RT_SIZE         = 6;


    //----------------- Session message packet ------------------------
    private final static int   MS_DATA         = 4;

    //----------------- Session keep alive packet ----------------------
    // has no trailer


    private final static int SSN_SRVC_TCP_PORT      =  139;
    private final static int SSN_RETRY_COUNT        =  4;         // Default
    private final static int SSN_CLOSE_TIMEOUT      =  30;        // Default (sec)
    private final static int SSN_KEEP_ALIVE_TIMEOUT =  60;        // Default (sec)


    // Negative response codes
    /*
    private final static byte  NR_EC_NOT_LISTINING_CALLED_NAME  = (byte)0x80; // 128
    private final static byte  NR_EC_NOT_LISTINING_CALLING_NAME = (byte)0x81; // 129
    private final static byte  NR_EC_CALLED_NAME_NOT_PRESENT    = (byte)0x82; // 130
    private final static byte  NR_EC_INSUFFICIENT_RESOURCES     = (byte)0x83; // 131
    private final static byte  NR_EC_UNSPECIFIED_ERROR          = (byte)0x8F; // 143
    */

    private int              fPort        = SSN_SRVC_TCP_PORT;
    private static String    fLocalHostName;
    private String           fCallingName;
   // private static int       fLocalNameId = 0;
    private InetAddress      fAddress     = null;
    private String           fNetBIOSName = null;

    private Socket           fSocket = null;
    private InputStream  	 fInput  = null;
    private DataOutputStream fOutput = null;

    private byte[]           fSessionHeader = new byte[HDR_SIZE];

    private int              fTimeout ;
    private boolean          fTcpNoDelay;

    private NBTSessionListener fListener = null;

    public NBTSession() {

        // Get properties
        String p = System.getProperty("org.gnu.jcifs.socket.timeout");

        if (p != null) {
            try {
                fTimeout = Integer.parseInt(p);
            } catch(NumberFormatException e) {
                fTimeout = SSN_CLOSE_TIMEOUT;
                Debug.println(Debug.ERROR,"Invalid org.gnu.jcifs.socket.timeout property");
            }
        } else
             fTimeout = SSN_CLOSE_TIMEOUT;

        // time in millisec
        fTimeout *= 1000;

        p = System.getProperty("org.gnu.jcifs.socket.tcpnodelay","true");

        if (p != null)
            fTcpNoDelay = p.equalsIgnoreCase("true");
        else
            fTcpNoDelay = true;

        if (Debug.debugOn & Debug.debugLevel >= Debug.INFO) {
           Debug.println(Debug.INFO,"NetBIOS: Timeout=" + fTimeout);
           Debug.println(Debug.INFO,"NetBIOS: Tcpnodelay=" + fTcpNoDelay);
        }

        // calling name
        fCallingName = System.getProperty("org.gnu.jcifs.netbios.callingname");
        if (fCallingName == null) {

            int i = fLocalHostName.indexOf('.');
            if (i > 0)
                fCallingName = fLocalHostName.substring(0,i);
            else
                fCallingName = fLocalHostName;
        }
    }

    public void addSessionListener(NBTSessionListener listener) {
        fListener = listener;
    }

    public String getNetBIOSName() {
        return fNetBIOSName;
    }

    public InetAddress getInetAddress() {
        return fAddress;
    }

    public synchronized void doCall(String netbiosName)  throws IOException  {

        int retry_count = 0;
        int port = SSN_SRVC_TCP_PORT;


        fNetBIOSName = netbiosName.toUpperCase();

        fAddress = NBTNameResolver.resolve(fNetBIOSName);

        doConnect(fAddress,port);


        byte[] packet = makeRequestPacket(fNetBIOSName);

        if (Debug.debugOn & Debug.debugLevel >= Debug.INFO) {
           Debug.println(Debug.INFO,"NetBIOS: doCall");
           Debug.println(Debug.INFO,"Called  name=" + fNetBIOSName);
           Debug.println(Debug.INFO,"Calling name=" + fCallingName);
           Debug.println(Debug.INFO,"Called  addr=" + fAddress.toString());
        }


        try {
            fOutput.write(packet);
            fOutput.flush();
        } catch(IOException e) {
            doHangup();
            throw new CifsIOException("NB500").setDetail(e);
        }


        // read header

        try {
            int count = read(packet,0,HDR_SIZE);

            if (count < HDR_SIZE) {
                doHangup();
                throw new CifsIOException("NB501");
            }

            byte type = packet[HDR_TYPE_1];

            switch(type) {
                case SPT_POS_RESPONSE:
                /*
                    POSITIVE SESSION RESPONSE PACKET

                                        1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3
                    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |      TYPE     |     FLAGS     |            LENGTH             |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                */

                    break;
                case SPT_NEG_RESPONSE:
                /*
                    NEGATIVE SESSION RESPONSE PACKET

                                        1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3
                    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |      TYPE     |     FLAGS     |            LENGTH             |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |   ERROR_CODE  |
                    +-+-+-+-+-+-+-+-+
                 */
                    int rc = fInput.read();
                    Debug.println(Debug.ERROR,"NetBIOS:Negative response:" + rc);

                    doHangup();
                    throw  CifsIOException.getNBException( rc & 0xff );

                case SPT_RTG_RESPONSE:
                 /*
                    SESSION RETARGET RESPONSE PACKET

                        1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3
                    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |      TYPE     |     FLAGS     |            LENGTH             |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |                      RETARGET_IP_ADDRESS                      |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |           PORT                |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                  */
                    count = read(packet,0,RT_SIZE);
                    doHangup();
                    throw new CifsIOException("NB502");

                default:
                    doHangup();
                    throw new CifsRuntimeException("NB503", new Integer(type));

            }

        } catch(IOException e) {
            doHangup();
            throw new CifsIOException("NB501").setDetail(e);
        }


    }

    public void doSend(NBTOutput data)  throws IOException  {

        if (!isAlive())
            throw new CifsIOException("NB504");

        int size = data.getSize() & 0xffff;

         // set packet header
        fSessionHeader[HDR_TYPE_1]  = SPT_MESSAGE;
        fSessionHeader[HDR_FLAGS_1] = 0;
        setShortAt(HDR_LENGTH_2,fSessionHeader,(short)size);

        try {
            fOutput.write(fSessionHeader);
            data.writeTo(fOutput,size);
            fOutput.flush();
        } catch(IOException e) {
            doHangup(true);
            throw new CifsIOException("NB500").setDetail(e).setConnectionLost();

        }



    }

    public int doReceive(NBTInput data) throws IOException {
        byte type ;


        try {
            do {
                int count = read(fSessionHeader,0,HDR_SIZE);

                if (count < HDR_SIZE) {
                    doHangup(true);
                    throw new CifsIOException("NB501");
                }

                type = fSessionHeader[0];

            } while(type == SPT_KEEPALIVE);

            if (type != SPT_MESSAGE) {
                doHangup(true);
                throw new CifsIOException("NB503",new Integer(type)).setConnectionLost();

            }

            int data_size = getShortAt(HDR_LENGTH_2,fSessionHeader);

            return data.readFrom(fInput,data_size);

        } catch(IOException e) {
            doHangup(true);
            throw new CifsIOException("NB501").setDetail(e).setConnectionLost();

        }

    }

    public void doHangup() {
        doHangup(false);

    }
    private void doHangup(boolean notify) {
        if (fSocket != null) {
            try {

                fInput.close();
                fOutput.close();
                fSocket.close();
            } catch(IOException e) {

            }
        }
        fSocket = null;
        fInput  = null;
        fOutput = null;

        if (notify)
            notifyListener();
    }


    /**
        SESSION REQUEST PACKET:

                            1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3
        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |      TYPE     |     FLAGS     |            LENGTH             |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |                                                               |
        /                          CALLED NAME                          /
        |                                                               |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |                                                               |
        /                          CALLING NAME                         /
        |                                                               |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    */

    private byte[] makeRequestPacket(String callname) {
         // build called and calling names
        byte[] calledname     = NBTNameService.buildSecondLevelEncodedName(callname); // NetBios.SMBSERVER_NAME);
        byte[] callingname    = NBTNameService.buildSecondLevelEncodedName(fCallingName);

        int packetsize = HDR_SIZE + calledname.length + callingname.length;
        byte[] packet = new byte[packetsize];

        // set packet header
        packet[HDR_TYPE_1]  = SPT_REQUEST;
        packet[HDR_FLAGS_1] = 0;
        setShortAt(HDR_LENGTH_2,packet,(short)(packetsize - HDR_SIZE));

        int pos = RQ_CALLED_NAME_32;

        for(int i=0;i<calledname.length;i++)
            packet[pos++] = calledname[i];

        for(int i=0;i<callingname.length;i++)
            packet[pos++] = callingname[i];

        return packet;

    }

    private void doConnect(InetAddress addr, int port) throws IOException {
        try {
            synchronized(this){
                if (isAlive())
                    return;

                fSocket = new Socket(addr,port);

                fSocket.setSoTimeout(fTimeout);

                fSocket.setTcpNoDelay(fTcpNoDelay);

                fInput  = fSocket.getInputStream();
                fOutput = new DataOutputStream(fSocket.getOutputStream());

            }

        } catch(UnknownHostException e) {
            throw new CifsIOException("CM1",addr.getHostName()).setDetail(e);
        } catch(IOException e) {
           throw new CifsIOException("CM2",addr.getHostName()).setDetail(e);
        }

    }
    boolean isAlive() {
		
		// code changed: fSocket.getInputStream().available(); does not 
		// raise exception if the connection is closed
		return (fSocket != null);
        
    }

    private int read(byte[] buffer, int off, int len) throws IOException {


        if (len <= 0)
            return 0;

        int count = 0;

        while(len > 0) {

		    int result = fInput.read(buffer, count, len);

		    if(result <= 0)
                throw new EOFException();

		    count += result;
		    len   -= result;
	    }
        return count;

    }

    private void notifyListener() {
        if (fListener != null)
            fListener.connectionLost();
    }

    private static  int getShortAt( int pos , byte[] buffer) {

        if (pos + 2 > buffer.length )
            throw new ArrayIndexOutOfBoundsException();

        return  ((buffer[pos+1]   & 0xff)        +
                ((buffer[pos]    & 0xff)  <<  8) ) & 0xffff;

    }
    private static  void setShortAt(int pos, byte[] buffer,short val) {


        // use big-endian encoding
        buffer[pos]    = (byte)((val >>  8) & 0xff);
        buffer[pos+1]  = (byte)(val & 0xff);



    }


    static {
        try {
         fLocalHostName = InetAddress.getLocalHost().getHostName();

        } catch(Exception e) {

            System.err.println("FATAL: Cannot get local host name!");
        }
    }
}