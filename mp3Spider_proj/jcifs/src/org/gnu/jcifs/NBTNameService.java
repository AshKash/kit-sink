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
import java.net.*;


/**
 * NB Name Service (WINS)
 */
public final class NBTNameService {

    /* RFC1001/RFC1002

                        1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |         NAME_TRN_ID           | OPCODE  |   NM_FLAGS  | RCODE |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |          QDCOUNT              |           ANCOUNT             |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |          NSCOUNT              |           ARCOUNT             |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

   Field     Description

   NAME_TRN_ID      Transaction ID for Name Service Transaction.
                    Requestor places a unique value for each active
                    transaction.  Responder puts NAME_TRN_ID value
                    from request packet in response packet.

   OPCODE           Packet type code, see table below.

   NM_FLAGS         Flags for operation, see table below.

   RCODE            Result codes of request.  Table of RCODE values
                    for each response packet below.

   QDCOUNT          Unsigned 16 bit integer specifying the number of
                    entries in the question section of a Name

                    Service packet.  Always zero (0) for responses.
                    Must be non-zero for all NetBIOS Name requests.

   ANCOUNT          Unsigned 16 bit integer specifying the number of
                    resource records in the answer section of a Name
                    Service packet.

   NSCOUNT          Unsigned 16 bit integer specifying the number of
                    resource records in the authority section of a
                    Name Service packet.

   ARCOUNT          Unsigned 16 bit integer specifying the number of
                    resource records in the additional records
                    section of a Name Service packet.

   The OPCODE field is defined as:

     0   1   2   3   4
   +---+---+---+---+---+
   | R |    OPCODE     |
   +---+---+---+---+---+



   Symbol     Bit(s)   Description

   OPCODE        1-4   Operation specifier:
                         0 = query
                         5 = registration
                         6 = release
                         7 = WACK
                         8 = refresh

   R               0   RESPONSE flag:
                         if bit == 0 then request packet
                         if bit == 1 then response packet.

   The NM_FLAGS field is defined as:


     0   1   2   3   4   5   6
   +---+---+---+---+---+---+---+
   |AA |TC |RD |RA | 0 | 0 | B |
   +---+---+---+---+---+---+---+

   Symbol     Bit(s)   Description

   B               6   Broadcast Flag.
                         = 1: packet was broadcast or multicast
                         = 0: unicast

   RA              3   Recursion Available Flag.

                       Only valid in responses from a NetBIOS Name
                       Server -- must be zero in all other
                       responses.

                       If one (1) then the NBNS supports recursive
                       query, registration, and release.

                       If zero (0) then the end-node must iterate
                       for query and challenge for registration.

   RD              2   Recursion Desired Flag.

                       May only be set on a request to a NetBIOS
                       Name Server.

                       The NBNS will copy its state into the
                       response packet.

                       If one (1) the NBNS will iterate on the
                       query, registration, or release.

   TC              1   Truncation Flag.

                       Set if this message was truncated because the
                       datagram carrying it would be greater than
                       576 bytes in length.  Use TCP to get the
                       information from the NetBIOS Name Server.

   AA              0   Authoritative Answer flag.

                       Must be zero (0) if R flag of OPCODE is zero
                       (0).

                       If R flag is one (1) then if AA is one (1)
                       then the node responding is an authority for
                       the domain name.

                       End nodes responding to queries always set
                       this bit in responses.

    */

    /** Transaction ID for Name Service Transaction.
        Requestor places a unique value for each active
        transaction.  Responder puts NAME_TRN_ID value
        from request packet in response packet.
    */
    private final static int HDR_NAME_TRN_ID_2 = 0;

    /** Contains OPCODE, NM_FLAGS, RCODE */
    private final static int HDR_FLAGS_2       = 2;

    // Response flag (Bit 15)
    private final static int RESPONSE_PACKET     =  0x8000;

    //----------------- OPCODE (4 bits, Bit 14-11) ----------------

    private final static int OPCODE_MASK         =  0x7800;
    private final static int OPCODE_SHIFT        =  11;

    private final static int OPCODE_QUERY        =  0;
    private final static int OPCODE_REGISTRATION =  5;
    private final static int OPCODE_RELEASE      =  6;
    private final static int OPCODE_WACK         =  7;
    private final static int OPCODE_REFRESH      =  8;

    // NM_FLAGS
    private final static int NM_FLAGS_MASK       =  0x07F0;
    private final static int NM_FLAGS_SHIFT      =  4;

    private final static int BROADCAST           =  0x01;
    private final static int RECURSIVE_AVAIL     =  0x08;
    private final static int RECURSIVE_DESIRED   =  0x10;
    private final static int TRUNCATION          =  0x20;
    private final static int AUTHORITATIVE_ANSWER=  0x40;

    // RCODE
    private final static int RCODE_MASK          =  0x000F;

    // Format error
    private final static int RCODE_FMT_ERR       =  0x1;
    // Server error
    private final static int RCODE_SVR_ERR       =  0x2;
    // Name error
    private final static int RCODE_NAM_ERR       =  0x3;
    // Unsupported request error
    private final static int RCODE_IMP_ERR       =  0x4;
    // Refused error
    private final static int RCODE_RFS_ERR       =  0x5;

    /**   */
    private final static int HDR_QDCOUNT_2       = 4;
    private final static int HDR_ANCOUNT_2       = 6;
    private final static int HDR_NSCOUNT_2       = 8;
    private final static int HDR_ARCOUNT_2       = 10;


    private final static int HDR_SIZE            = 12;

    private final static int QUESTION_NAME_OFF   = 12;
    /**
    QUESTION SECTION

                        1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                                                               |
   /                         QUESTION_NAME                         /
   /                                                               /
   |                                                               |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |         QUESTION_TYPE         |        QUESTION_CLASS         |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

   Field            Description

   QUESTION_NAME    The compressed name representation of the
                    NetBIOS name for the request.

   QUESTION_TYPE    The type of request.  The values for this field
                    are specified for each request.

   QUESTION_CLASS   The class of the request.  The values for this
                    field are specified for each request.

   QUESTION_TYPE is defined as:

   Symbol      Value   Description:

   NB         0x0020   NetBIOS general Name Service Resource Record
   NBSTAT     0x0021   NetBIOS NODE STATUS Resource Record (See NODE
                       STATUS REQUEST)

   QUESTION_CLASS is defined as:


   Symbol      Value   Description:

   IN         0x0001   Internet class
   */

   private final static int QUESTION_TYPE_NB       = 0x0020;
   private final static int QUESTION_TYPE_NBSTAT   = 0x0021;

   private final static int QUESTION_CLASS_IN      = 0x0001;

   private final static int NAME_SERVICE_UDP_PORT  = 137;

   private DatagramSocket   fSocket;

   private byte[]           fData;
   private int              fTRID = 0;


   private final static int UCAST_REQ_RETRY_TIMEOUT   = 5000;  // msec
   private int              fTimeout = UCAST_REQ_RETRY_TIMEOUT;

   public NBTNameService() throws IOException {

        fSocket = new DatagramSocket();
        fData   = new byte[4096];

   }

    public InetAddress lookup(InetAddress winsaddr,String netbiosname) {
    	
    	
        String addr = null;

        try {
            addr = queryName(winsaddr,netbiosname);
            
            if (addr != null)

            	return InetAddress.getByName(addr);

        } catch(Exception e) {

        }

        return null;

    }

   private String queryName(InetAddress winsaddr, String netbiosname) throws IOException {


        byte[] encodedname   = buildSecondLevelEncodedName(netbiosname);
        
        

        int trid = nextTRID();

        clearHeader();

        int pos = HDR_NAME_TRN_ID_2;
        setShortAt(pos,fData,trid);

        int opcode;
        int rcode;
        int nmflags = RECURSIVE_DESIRED ;
        int flags = (nmflags << NM_FLAGS_SHIFT);

        pos = HDR_FLAGS_2;
        setShortAt(pos,fData,(flags & 0xffff));

        pos = HDR_QDCOUNT_2;
        setShortAt(pos,fData,1);

        pos = QUESTION_NAME_OFF;
        for(int i=0;i<encodedname.length;i++)
            fData[pos++] = encodedname[i];

        setShortAt(pos,fData,QUESTION_TYPE_NB);
        pos += 2;
        setShortAt(pos,fData,QUESTION_CLASS_IN);
        pos += 2;

        DatagramPacket packet = new DatagramPacket(fData,pos, winsaddr,NAME_SERVICE_UDP_PORT);


        if (Debug.debugOn && Debug.debugLevel >= Debug.BUFFER) {
                Debug.println(Debug.BUFFER,"NBNS name query request:");
                Debug.println(Debug.BUFFER,fData,0, pos);
        }


        fSocket.send(packet);

        StringBuffer rrname = new StringBuffer();

        DatagramPacket rcvpacket = new DatagramPacket(fData,fData.length);

        while(true)  {

            fSocket.setSoTimeout(fTimeout);

            fSocket.receive(rcvpacket);

            if (trid != getShortAt(0,fData))
                continue;

            if (Debug.debugOn && Debug.debugLevel >= Debug.BUFFER) {
                Debug.println(Debug.BUFFER,"NBNS response:");
                Debug.println(Debug.BUFFER,fData,0, rcvpacket.getLength());
            }

            flags = getShortAt(2,fData);
            rcode = flags & RCODE_MASK;

            if ((flags & RESPONSE_PACKET) == 0)
                break; // not a response

            opcode  = ((flags & OPCODE_MASK) >> OPCODE_SHIFT) & 0xf;
            nmflags = ((flags & NM_FLAGS_MASK) >> NM_FLAGS_SHIFT) & 0x7f;

            if (opcode == OPCODE_WACK) {
                /*
                    WAIT FOR ACKNOWLEDGEMENT (WACK) RESPONSE

                                        1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3
                    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |         NAME_TRN_ID           |1|  0x7  |1|0|0|0|0 0|0|  0x0  |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |          0x0000               |           0x0001              |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |          0x0000               |           0x0000              |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |                                                               |
                    /                            RR_NAME                            /
                    /                                                               /
                    |                                                               |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |          NULL (0x0020)        |         IN (0x0001)           |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |                              TTL                              |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |           0x0002              | OPCODE  |   NM_FLAGS  |  0x0  |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+



                The NAME_TRN_ID of the WACK RESPONSE packet is the same
                NAME_TRN_ID of the request that the NBNS is telling the requestor
                to wait longer to complete.  The RR_NAME is the name from the
                request, if any.  If no name is available from the request then
                it is a null name, single byte of zero.

                The TTL field of the ResourceRecord is the new time to wait, in
                seconds, for the request to complete.  The RDATA field contains
                the OPCODE and NM_FLAGS of the request.

                A TTL value of 0 means that the NBNS can not estimate the time it
                may take to complete a response.
                */

                // read RR_NAME

                pos = HDR_SIZE;
                pos = parseSecondLevelEncodedName(fData,pos,rrname);

                // skip
                pos += 4;
                int ttl = getIntAt(pos,fData);
                if (ttl == 0)
                    fTimeout = UCAST_REQ_RETRY_TIMEOUT;
                else
                    fTimeout = ttl;

                continue;
            }

            if (rcode == 0) {

                // check if redirect

                if ((nmflags & AUTHORITATIVE_ANSWER) != 0) {
                    /*
                 POSITIVE NAME QUERY RESPONSE

                                        1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3
                    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |         NAME_TRN_ID           |1|  0x0  |1|T|1|?|0 0|0|  0x0  |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |          0x0000               |           0x0001              |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |          0x0000               |           0x0000              |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |                                                               |
                    /                            RR_NAME                            /
                    /                                                               /
                    |                                                               |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |           NB (0x0020)         |         IN (0x0001)           |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |                              TTL                              |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |           RDLENGTH            |                               |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               |
                    |                                                               |
                    /                       ADDR_ENTRY ARRAY                        /
                    /                                                               /
                    |                                                               |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

                    The ADDR_ENTRY ARRAY a sequence of zero or more ADDR_ENTRY
                    records.  Each ADDR_ENTRY record represents an owner of a name.
                    For group names there may be multiple entries.  However, the list
                    may be incomplete due to packet size limitations.  Bit 22, "T",
                    will be set to indicate truncated data.

                    Each ADDR_ENTRY has the following format:

                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |          NB_FLAGS             |          NB_ADDRESS           |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |   NB_ADDRESS (continued)      |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

                */

                    // read RR_NAME

                    pos = HDR_SIZE;

                    pos = parseSecondLevelEncodedName(fData,pos,rrname);

                    //  skip

                    pos += 8;

                    int rdlength = getShortAt(pos,fData);
                    pos += 2;

                    if (rdlength >= 6) {
                        pos += 2; // skip NB_FLAGS


                        return Util.getIPAddress(fData,pos);
                    }
                } else {
                    /*
                        REDIRECT NAME QUERY RESPONSE

                                        1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3
                    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |         NAME_TRN_ID           |1|  0x0  |0|0|1|0|0 0|0|  0x0  |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |          0x0000               |           0x0000              |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |          0x0001               |           0x0001              |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |                                                               |
                    /                            RR_NAME                            /
                    /                                                               /
                    |                                                               |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |           NS (0x0002)         |         IN (0x0001)           |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |                              TTL                              |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |           RDLENGTH            |                               |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               +
                    |                                                               |
                    /                            NSD_NAME                           /
                    /                                                               /
                    |                                                               |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |                                                               |
                    /                            RR_NAME                            /
                    /                                                               /
                    |                                                               |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |           A (0x0001)          |         IN (0x0001)           |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |                              TTL                              |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |             0x0004            |           NSD_IP_ADDR         |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                    |     NSD_IP_ADDR, continued    |
                    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

                    An end node responding to a NAME QUERY REQUEST always responds
                    with the AA and RA bits set for both the NEGATIVE and POSITIVE
                    NAME QUERY RESPONSE packets.  An end node never sends a REDIRECT
                    NAME QUERY RESPONSE packet.

                    When the requestor receives the REDIRECT NAME QUERY RESPONSE it
                    must reiterate the NAME QUERY REQUEST to the NBNS specified by
                    the NSD_IP_ADDR field of the A type RESOURCE RECORD in the
                    ADDITIONAL section of the response packet.  This is an optional
                    packet for the NBNS.

                    The NSD_NAME and the RR_NAME in the ADDITIONAL section of the
                    response packet are the same name.  Space can be optimized if
                    label string pointers are used in the RR_NAME which point to the
                    labels in the NSD_NAME.

                    The RR_NAME in the AUTHORITY section is the name of the domain
                    the NBNS called by NSD_NAME has authority over.
                */
                    Debug.println(Debug.ERROR,"NB redirect not implemented");
                    return null;
                }

            } else  {
                // NEGATIVE NAME QUERY RESPONSE
                if (Debug.debugOn && Debug.debugLevel >= Debug.ERROR)
                    Debug.println(Debug.ERROR,"WINS error:" + rcode);
                return null;
            }
            break;

        }

        return null;
    }


    private synchronized final  int nextTRID() {
        fTRID++;
        if (fTRID == Short.MAX_VALUE)
            fTRID = 0;
        return fTRID;
    }

    private void clearHeader() {
        for(int i=0;i<HDR_SIZE;i++)
            fData[i] = 0;
    }


    private  static int getShortAt( int pos,byte[] buffer) {


        return  (((buffer[pos]   & 0xff)  << 8 )      +
                 ((buffer[pos+1] & 0xff)) ) & 0xffff;

    }

    private  static int getIntAt( int pos,byte[] buffer) {


        return (((buffer[pos]   & 0xff) << 24) +
                ((buffer[pos+1] & 0xff) << 16) +
                ((buffer[pos+2] & 0xff) << 8) +
                ((buffer[pos+3] & 0xff) ));

    }
    private  void setShortAt(int pos, byte[] buffer,int val) {


        // use big-endian encoding
        buffer[pos]    = (byte)((val >>  8) & 0xff);
        buffer[pos+1]  = (byte)(val & 0xff);



    }

    /**
     * Build first level representation of the name (RFC1001, Chapter 14.1:<p>
       The 16 byte NetBIOS name is mapped into a 32 byte wide field using a
       reversible, half-ASCII, biased encoding.  Each half-octet of the
       NetBIOS name is encoded into one byte of the 32 byte field.  The
       first half octet is encoded into the first byte, the second half-
       octet into the second byte, etc.<p>

       Each 4-bit, half-octet of the NetBIOS name is treated as an 8-bit,
       right-adjusted, zero-filled binary number.  This number is added to
       value of the ASCII character 'A' (hexidecimal 41).  The resulting 8-
       bit number is stored in the appropriate byte.  The following diagram
       demonstrates this procedure:
        <pre>

                         0 1 2 3 4 5 6 7
                        +-+-+-+-+-+-+-+-+
                        |a b c d|w x y z|          ORIGINAL BYTE
                        +-+-+-+-+-+-+-+-+
                            |       |
                   +--------+       +--------+
                   |                         |     SPLIT THE NIBBLES
                   v                         v
            0 1 2 3 4 5 6 7           0 1 2 3 4 5 6 7
           +-+-+-+-+-+-+-+-+         +-+-+-+-+-+-+-+-+
           |0 0 0 0 a b c d|         |0 0 0 0 w x y z|
           +-+-+-+-+-+-+-+-+         +-+-+-+-+-+-+-+-+
                   |                         |
                   +                         +     ADD 'A'
                   |                         |
            0 1 2 3 4 5 6 7           0 1 2 3 4 5 6 7
           +-+-+-+-+-+-+-+-+         +-+-+-+-+-+-+-+-+
           |0 1 0 0 0 0 0 1|         |0 1 0 0 0 0 0 1|
           +-+-+-+-+-+-+-+-+         +-+-+-+-+-+-+-+-+

    </pre>
   This encoding results in a NetBIOS name being represented as a
   sequence of 32 ASCII, upper-case characters from the set
   {A,B,C...N,O,P}.<p>

   The NetBIOS scope identifier is a valid domain name (without a
   leading dot).<p>

   An ASCII dot (2E hexidecimal) and the scope identifier are appended
   to the encoded form of the NetBIOS name, the result forming a valid
   domain name.<p>


   For example, the NetBIOS name "The NetBIOS name" in the NetBIOS scope
   "SCOPE.ID.COM" would be represented at level one by the ASCII
   character string:

        FEGHGFCAEOGFHEECEJEPFDCAHEGBGNGF.SCOPE.ID.COM
     */
    public static String buildFirstLevelEncodedName(String name) {


        StringBuffer buf = new StringBuffer(name.toUpperCase());

        // cut string if longer as 15
        if (buf.length() > 15)
            buf.setLength(15);

        // append blank up 16 bytes

        int blanks = 16 - buf.length();
        for(int i=0;i<blanks;i++)
            buf.append(' ');


        StringBuffer name32 = new StringBuffer(32);

        for(int i=0;i<16;i++) {
            byte b = (byte)(buf.charAt(i) & 0xff);

            char c1 = (char)((b >> 4 & 0x0F)  + 'A') ;
            char c2 = (char)((b  & 0x0F)  + 'A') ;

            name32.append(c1) ;
            name32.append(c2) ;
        }


        return name32.toString();
    }

    /**
     * Build second level representation of the name. RFC1002 says:<p>
     * For ease of description, the first two paragraphs from page 31,
     * the section titled "Domain name representation and compression",
     * of RFC 883 are replicated here:<p>
     *
     *  Domain names messages are expressed in terms of a sequence
     *  of labels.  Each label is represented as a one octet length
     *  field followed by that number of octets.  Since every domain
     *  name ends with the null label of the root, a compressed
     *  domain name is terminated by a length byte of zero.  The
     *  high order two bits of the length field must be zero, and
     *  the remaining six bits of the length field limit the label
     *   to 63 octets or less.<p>
     *
     *   To simplify implementations, the total length of label
     *   octets and label length octets that make up a domain name is
     *   restricted to 255 octets or less.
     *
     * @param plainname name (not encoded)
     *  |len| name |00|
     */
    public static byte[] buildSecondLevelEncodedName(String plainname) {
        String name = buildFirstLevelEncodedName(plainname);
        
        
        int size   = name.length();
        byte[] buf = new byte[size+1+1];
        int pos    = 0;
        buf[pos++] = (byte)size;

        for(int i=0;i<size;i++)
            buf[pos++] = (byte)name.charAt(i);

        buf[pos] = 0;

        return buf;
    }

    public static int parseSecondLevelEncodedName(byte[] buf, int off, StringBuffer name) {

        name.setLength(0);
        int pos = off;
        int len = buf[pos++] & 0xff;


        while(len > 0) {

            if (name.length() == 0) {
                int end = pos + len;
                // first part
                while(pos < end) {
                    int c1 = (buf[pos++] & 0xff) - 'A';
                    int c2 = (buf[pos++] & 0xff) - 'A';
                    char c = (char) ((c1 << 4) | c2);

                    name.append(c);
                }
            } else {
                name.append('.');
                for(int i=0;i<len;i++)
                    name.append((char)buf[pos++]);
            }

            len = buf[pos++] & 0xff;
        }
        return pos;
    }

}