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
import java.text.*;

/*

 The ***parameters section** of the Transact SMB request contains thefollowing
 (in the order described)
 - The function number: an unsigned short 16 bit integer identifying the
   function being remoted
 - The parameter descriptor string: a null terminated ASCII string
 - The data descriptor string: a null terminated ASCII string.
 - The request parameters, as described by the parameter descriptor  string,
   in the order that the request parameter descriptor characters  appear in
   the parameter descriptor string
 - An optional auxiliary data descriptor string:  a null terminated ASCII  string.
   It will be present if there is an auxiliary data structure  count in the primary
   data struct (an "N" descriptor in the data  descriptor string).RAP requires
   that the length of the return parameters be less than orequal to the length
   of the parameters being sent; this requirement ismade to simply buffer management
   in implementations. This is reasonableas the functions were designed to return
   data in the data section anduse the return parameters for items like data length,
   handles, etc. Ifneed be, this restriction can be circumvented by filling in
   some padbytes into the parameters being sent.

   The Data section for the transaction request is present if the parameter description
   string contains an "s" (SENDBUF) descriptor. If present, itcontains:
 - A primary data struct, as described by the data descriptor string

 - Zero or more instances of the auxiliary data struct, as described by
   the auxiliary data descriptor string. The number of instances is  determined
   by the value of the an auxiliary data structure count  member of the primary
   data struct, indicated by the "N" (AUXCOUNT)  descriptor. The auxiliary data
   is present only if the auxiliary data  descriptor string is non null.

 - Possibly some pad bytes
 - The heap: the data referenced by pointers in the primary and  auxiliary data structs.


 */

/**
 * The Remote Administration Protocol (RAP) provides operations<p>
 * - to get list of share names;<br>
 * - to get user informations;<br>
 * - to get workstation informations;<br>
 * - to get informations about print jobs;<br>
 * - to manage print jobs.<p>
 *
 * @author  Norbert Hranitzky
 * @version 1.0, 20 Nov 1998
 * @since   1.0
 */
final class RemoteAdminImpl extends SessionImpl implements CifsRemoteAdmin {


    private final static short NetShareEnum         = 0;
    private final static short NetServerEnum2       = 104;
    private final static short NetGetServerInfo     = 13;
    private final static short NetWkstaGetInfo      = 63;
    private final static short SamOEMChangePassword = 214;
    private final static short NetUserGetInfo       = 56;
    private final static short DosPrintQEnum        = 69;
    private final static short DosPrintJobEnum      = 76;
    private final static short DosPrintJobGetInfo   = 77;
    private final static short DosPrintJobPause     = 82;
    private final static short DosPrintJobContinue  = 83;
    private final static short DosPrintJobDel       = 81;
    private final static short NetUserPasswordSet   = 115;

	// Changed by Ashwin from 3000 to 15000
    private int                fReturnBufferSize  = 15000;


    private ShareInfoComparator fShareInfoComparator = null;



    RemoteAdminImpl(String sessionname,int prot,Share share,NBTSession nbt, SMBMessage packet) throws IOException {
        super(sessionname,prot,share,nbt,packet);

    }


    /**
     * Returns the list of shares on the computer
     * @param sort if true the names are sorted
     * @return  list of <code>CifsShareInfo</code> objects
     * @exception  IOException  if an I/O error occurs.
     */
    public synchronized CifsShareInfo[] listSharesInfo(boolean sort) throws IOException {
         if (Debug.debugOn)
            Debug.println(Debug.INFO, "IPC:NetShareEnum");

         MarshalBuffer param = new MarshalBuffer(100);

         int pos = 0;
         pos += param.setShortAt(pos,NetShareEnum);
         // parameter descriptor string
         pos += param.setZTAsciiStringAt(pos,"WrLeh");

         // he data descriptor string for the (returned) data which is "B13BWz"
         pos += param.setZTAsciiStringAt(pos,"B13BWz");

         // corresponding to the "W"
         pos += param.setShortAt(pos,1);

         // buffer size
         pos += param.setShortAt(pos,fReturnBufferSize);

         param.setSize(pos);

         sendTransaction(null,"\\PIPE\\LANMAN",  param,null,0);

         MarshalBuffer data = new MarshalBuffer(1000);

         receiveTransaction(param,data);

         pos = 0;

         int error = param.getShortAt(pos);

         // ignore more data
         if (error != 0  && error != 234)
            throw CifsIOException.getLMException(error);

         pos += 2;

         // converter
         int converter = param.getShortAt(pos) ;
         pos += 2;

         // number of entries
         int counter   = param.getShortAt(pos);

         pos += 2;

         // number max of entries
         int maxcounter   = param.getShortAt(pos);


         /*
            The SHARE_INFO_1 structure is defined as:

            struct SHARE_INFO_1 {
                char                shi1_netname[13]
                char                shi1_pad;
                unsigned short  shi1_type
                char            *shi1_remark;
            }

            where:

            shi1_netname contains a null terminated ASCII string that
                        specifies the share name of the resource.

            shi1_pad aligns the next data strructure element to a word
                    boundary.

            shi1_type contains an integer that specifies the type of the
                    shared resource.


            shi1_remark points to a null terminated ASCII string that contains
                a comment abthe shared resource. The value for shi1_remark is null
                for ADMIN$ and IPC$ share names.  The shi1_remark pointer is a 32
                bit pointer. The higher 16 bits need to be ignored. The converter
                word returned in the parameters section needs to be subtracted
                from the lower 16 bits to calculate an offset into the return
                buffer where this ASCII string resides.

                In case there are multiple SHARE_INFO_1 data structures to return,
                the server may put all these fixed length structures in the return
                buffer, leave some space and then put all the variable length data
                (the actual value of the shi1_remark strings) at the end of the
            buffer.
            */

            if (Debug.debugOn)
                data.debug("NetShareEnum data");

            CifsShareInfo[]  result = new CifsShareInfo[counter];
            CifsShareInfo info;
            pos = 0;
            for(int i=0;i<counter;i++) {

                info = new CifsShareInfo(fShare.getHostName());

                // shi1_netname[13]
                info.fShareName = data.getZTAsciiStringAt(pos,13);
                pos += 13;

                pos += 1;   // pad

                // shi1_type
                info.fShareType = data.getShortAt(pos);

                pos += 2;
                // shi1_remark
                int rptr = (data.getIntAt(pos) & 0xffff);
                pos += 4;

                info.fRemark = data.getZTAsciiStringAt(rptr - converter,255);


                result[i] = info;
            }

        if (sort)
            Util.sort(result,getShareInfoComparator());


        return result;
    }

    /**
     * Returns detailed information about a workstation.
     * @exception  IOException  if an I/O error occurs.
     */
    public CifsWorkstationInfo getWorkstationInfo() throws IOException {
         if (Debug.debugOn)
            Debug.println(Debug.INFO, "IPC:NetWkstaGetInfo");

         MarshalBuffer param = new MarshalBuffer(100);

         int pos = 0;
         pos += param.setShortAt(pos,NetWkstaGetInfo);
         // parameter descriptor string
         pos += param.setZTAsciiStringAt(pos,"WrLh");

         // he data descriptor string for the (returned) data which is "zzzBBzz"
         pos += param.setZTAsciiStringAt(pos,"zzzBBzz");

         // corresponding to the "W"
         pos += param.setShortAt(pos,10);

         // buffer size
         pos += param.setShortAt(pos,fReturnBufferSize);

         param.setSize(pos);

         sendTransaction(null,"\\PIPE\\LANMAN",  param,null,0);

         MarshalBuffer data = new MarshalBuffer(1000);

         param.zero(param.getSize());

         receiveTransaction(param,data);

         pos = 0;

         int error = param.getShortAt(pos);

         // ignore more data
         if (error != 0  && error != 234)
            throw CifsIOException.getLMException(error);

         pos += 2;

         // converter
         int converter = param.getShortAt(pos) ;
         pos += 2;

         /* A 16 bit number representing the total number of available bytes.
            This has meaning only if the return status is NERR_Success or
            ERROR_MORE_DATA. Upon success, this number indicates the number of
            useful bytes available. Upon failure, this indicates how big the
            receive buffer needs to be.
         */
         int bytes   = param.getShortAt(pos);

         pos += 2;


         /*
         struct user_info_11 {
        char                *wki10_computername;
        char                *wki10_username;
        char                *wki10_langroup;
        unsigned char   wki10_ver_major;
        unsigned char       wki10_ver_minor;
        char                *wki10_logon_domain;
        char            *wki10_oth_domains;
        };
          */



        CifsWorkstationInfo info = new CifsWorkstationInfo();

        if (Debug.debugOn)
            data.debug("NetWkstaGetInfo data");

        pos = 0;

        int ptr = data.getIntAt(pos) & 0xffff;

        if (Debug.debugOn)
            Debug.println(Debug.BUFFER,"bytes=" + bytes + " pos=" + pos + ",ptr=" + (ptr-converter));


        pos += 4;
        info.fWorkstationName = data.getZTAsciiStringAt(ptr-converter,bytes);

        ptr = data.getIntAt(pos) & 0xffff;
        pos += 4;
        info.fUserName = data.getZTAsciiStringAt(ptr-converter,bytes);

        // domain to which the workstation belongs.
        ptr = data.getIntAt(pos) & 0xffff;
        pos += 4;
        info.fDomain = data.getZTAsciiStringAt(ptr-converter,bytes);


        // major version number of the networking software
        info.fMajorVersion = data.getByteAt(pos) & 0xff;
        pos ++;
        // minor version number of the networking software
        info.fMinorVersion = data.getByteAt(pos) & 0xff;
        pos ++;

        // the domain for which a user is logged on.
        ptr = data.getIntAt(pos) & 0xffff;
        pos += 4;
        info.fLogonDomain = data.getZTAsciiStringAt(ptr-converter,bytes);

        //all domains in which the computer is enlisted.
        ptr = data.getIntAt(pos) & 0xffff;
        pos += 4;
        info.fAllDomains = data.getZTAsciiStringAt(ptr-converter,bytes);

        return info;

    }

    /**
     * Returns detailed information about a particular user
     * @param user user name
     * @return user inforations
     * @exception  IOException  if an I/O error occurs.
     */

    public CifsUserInfo getUserInfo(String user) throws IOException {
         if (Debug.debugOn)
            Debug.println(Debug.INFO, "IPC:NetUserGetInfo");

         MarshalBuffer param = new MarshalBuffer(100);

         int pos = 0;
         pos += param.setShortAt(pos,NetUserGetInfo);
         // parameter descriptor string
         pos += param.setZTAsciiStringAt(pos,"zWrLh");

         // he data descriptor string for the (returned) data which is "B21BzzzWDDzzDDWWzWzDWb21W"
         pos += param.setZTAsciiStringAt(pos,"B21BzzzWDDzzDDWWzWzDWb21W");

         // user
         pos += param.setZTAsciiStringAt(pos,user);

         // corresponding to the "W"
         pos += param.setShortAt(pos,11);

         // buffer size
         pos += param.setShortAt(pos,fReturnBufferSize);

         param.setSize(pos);

         sendTransaction(null,"\\PIPE\\LANMAN",  param,null,0);

         MarshalBuffer data = new MarshalBuffer(1000);

         receiveTransaction(param,data);

         pos = 0;

         int error = param.getShortAt(pos);

         // ignore more data
         if (error != 0  && error != 234)
            throw CifsIOException.getLMException(error);

         pos += 2;

         // converter
         int converter = param.getShortAt(pos) ;
         pos += 2;

         /* A 16 bit number representing the total number of available bytes.
            This has meaning only if the return status is NERR_Success or
            ERROR_MORE_DATA. Upon success, this number indicates the number of
            useful bytes available. Upon failure, this indicates how big the
            receive buffer needs to be.
         */
         int bytes   = param.getShortAt(pos);

         pos += 2;


         /*
         struct user_info_11 {
        char                usri11_name[21];
        char                usri11_pad;
        char                *usri11_comment;
        char            *usri11_usr_comment;
        char                *usri11_full_name;
        unsigned short      usri11_priv;
        unsigned long       usri11_auth_flags;
        long                usri11_password_age;
        char                *usri11_homedir;
        char            *usri11_parms;
        long                usri11_last_logon;
        long                usri11_last_logoff;
        unsigned short      usri11_bad_pw_count;
        unsigned short      usri11_num_logons;
        char                *usri11_logon_server;
        unsigned short      usri11_country_code;
        char            *usri11_workstations;
        unsigned long       usri11_max_storage;
        unsigned short      usri11_units_per_week;
        unsigned char       *usri11_logon_hours;
        unsigned short      usri11_code_page;

        };
          */

        if (Debug.debugOn)
            data.debug("NetUserGetInfo data");

        CifsUserInfo info = new CifsUserInfo();

        pos = 0;

        // user name for which information is retireved

        info.fUserName = data.getZTAsciiStringAt(pos,21);
        pos += 21;

        // pad
        pos++;

        //comment
        int ptr = data.getIntAt(pos) & 0xffff;
        pos += 4;
        info.fComment = data.getZTAsciiStringAt(ptr-converter,bytes);
        // comment about user
        ptr = data.getIntAt(pos) & 0xffff;
        pos += 4;
        info.fUserComment = data.getZTAsciiStringAt(ptr-converter,bytes);

        // full name  of the user
        ptr = data.getIntAt(pos) & 0xffff;
        pos += 4;
        info.fUserFullName = data.getZTAsciiStringAt(ptr-converter,bytes);

        // level of the privilege assigned to the user
        info.fUserPriv = data.getShortAt(pos);
        pos += 2;

        // account operator privileges.
        info.fOperatorPriv  = data.getIntAt(pos);
        pos += 4;

        // how many seconds have elapsed since the password was last changed.
        info.fPasswordAge = data.getIntAt(pos) & 0xffffffff;
        pos += 4;

        // path name of the user's home directory.
        ptr = data.getIntAt(pos) & 0xffff;
        pos += 4;
        info.fHomeDir = data.getZTAsciiStringAt(ptr-converter,bytes);

        // skip usri11_parms
        pos += 4;

        // last logon
        info.fLastLogon  = data.getIntAt(pos) & 0xffffffff;
        pos += 4;

        // last logon
        info.fLastLogoff  = data.getIntAt(pos) & 0xffffffff;
        pos += 4;

        // bad logons
        info.fBadLogons = data.getShortAt(pos);
        pos += 2;

        // num logons
        info.fNumLogons = data.getShortAt(pos);
        pos += 2;

        // logon server
        ptr = data.getIntAt(pos) & 0xffff;
        pos += 4;
        info.fLogonServer = data.getZTAsciiStringAt(ptr-converter,bytes);

        return info;
    }

    /**
     * @exception  IOException  if an I/O error occurs.
     */
    public synchronized CifsPrinterQueueInfo[] listPrinterQueues() throws IOException {
         if (Debug.debugOn)
            Debug.println(Debug.INFO, "IPC:DosPrintQEnum");

         MarshalBuffer param = new MarshalBuffer(100);

         int pos = 0;
         pos += param.setShortAt(pos,DosPrintQEnum);
         // parameter descriptor string
         pos += param.setZTAsciiStringAt(pos,"WrLeh");

         // he data descriptor string for the (returned) data which
         pos += param.setZTAsciiStringAt(pos,"zWWWWzzzzWWzzl");

         // corresponding to the "W": Info level 3
         pos += param.setShortAt(pos,3);

         // buffer size
         pos += param.setShortAt(pos,fReturnBufferSize);

         param.setSize(pos);

         sendTransaction(null,"\\PIPE\\LANMAN",  param,null,0);

         MarshalBuffer data = new MarshalBuffer(1000);

         receiveTransaction(param,data);

         pos = 0;

         int error = param.getShortAt(pos);

         // ignore more data
         if (error != 0  && error != 234)
            throw CifsIOException.getLMException(error);

         pos += 2;

         // converter
         int converter = param.getShortAt(pos) ;
         pos += 2;

         // number of entries
         int counter   = param.getShortAt(pos);
         pos += 2;

         // number max of entries
         int maxcounter   = param.getShortAt(pos);


         /*
            struct	PRQINFO_3 {
	            char			*pszName;
	            unsigned short	Priority;
	            unsigned short 	Starttime;
	            unsigned short	UntilTime;
	            unsigned short	Pad1;
	            char			*pszSepFile;
	            char			*pszPrProc;
	            char			*pszParms;
	            char			*pszComment;
	            unsigned short	Status;
	            AUXCOUNT		cJobs;
	            char			*pszPrinters;
	            char			*pszDriverName
	            void			*pDriverData;
            }

         */

         if (Debug.debugOn)
            data.debug("DosPrintQEnum data");


        CifsPrinterQueueInfo[] infoList = new CifsPrinterQueueInfo[counter];
        CifsPrinterQueueInfo info;
        pos = 0;
        for(int i=0;i<counter;i++) {

                info = new CifsPrinterQueueInfo();

                // pszName
                int rptr = (data.getIntAt(pos) & 0xffff);
                pos += 4;
                info.fName = data.getZTAsciiStringAt(rptr - converter,255);

                info.fPriority = data.getShortAt(pos);
                pos += 2;

                info.fStartTime = data.getShortAt(pos);
                pos += 2;

                info.fUntilTime = data.getShortAt(pos);
                pos += 2;

                pos += 2; // pad
                pos += 4; // pszSepFile
                pos += 4; // pszPrProc


                rptr = (data.getIntAt(pos) & 0xffff);
                pos += 4;
                info.fParams = data.getZTAsciiStringAt(rptr - converter,255);

                rptr = (data.getIntAt(pos) & 0xffff);
                pos += 4;
                info.fComment = data.getZTAsciiStringAt(rptr - converter,255);

                info.fStatus = data.getShortAt(pos);
                pos += 2;

                info.fJobs = data.getShortAt(pos);
                pos += 2;

                rptr = (data.getIntAt(pos) & 0xffff);

                pos += 4;
                if (rptr != 0)
                    info.fPrintDestination = data.getZTAsciiStringAt(rptr - converter,255);
                else
                     info.fPrintDestination = "";

                rptr = (data.getIntAt(pos) & 0xffff);
                pos += 4;
                if (rptr != 0)
                    info.fDriver = data.getZTAsciiStringAt(rptr - converter,255);
                else
                    info.fDriver = "";

                pos += 4; // DriverData;
                infoList[i] = info;
            }
        return infoList;
    }


    /**
     * Lists print jobs in the specified printer queue
     * @param queuename printer queue name
     * @return CifsPrintJobInfo
     * @exception  IOException  if an I/O error occurs.
     */
    public synchronized CifsPrintJobInfo getPrintJobInfo(int jobId) throws IOException {
         if (Debug.debugOn)
            Debug.println(Debug.INFO, "IPC:DosPrintJobGetInfo");

         MarshalBuffer param = new MarshalBuffer(100);

         int pos = 0;
         pos += param.setShortAt(pos,DosPrintJobGetInfo);
         // parameter descriptor string
         pos += param.setZTAsciiStringAt(pos,"WWrLh");

         // he data descriptor string for the (returned) data which (Level 2)
         pos += param.setZTAsciiStringAt(pos,"WWzWWDDzz");

         // print job id
         pos += param.setShortAt(pos,jobId);
         // Level = 2
         pos += param.setShortAt(pos,2);
         // buffer size
         pos += param.setShortAt(pos,fReturnBufferSize);

         param.setSize(pos);

         sendTransaction(null,"\\PIPE\\LANMAN",  param,null,0);

         MarshalBuffer data = new MarshalBuffer(100);

         receiveTransaction(param,data);

         pos = 0;

         int error = param.getShortAt(pos);

         // ignore more data
         if (error != 0  && error != 234)
            throw CifsIOException.getLMException(error);

         pos += 2;

         // converter
         int converter = param.getShortAt(pos) ;
         pos += 2;

         // number of total bytes
         int bytes   = param.getShortAt(pos);
         pos += 2;




         /*
            struct PRJINFO_2 {
	            unsigned short		JobId;
	            unsigned short		Priority;
	            char  			*pszUserName;
	            unsigned short		Position;
	            unsigned short		Status;
	            unsigned long		Submitted;
	            unsigned long		Size;
	            char  			*pszComment;
	            char  			*pszDocument;
            }


         */

        if (Debug.debugOn)
            data.debug("DosPrintJobGetInfo data");


        CifsPrintJobInfo result = new CifsPrintJobInfo();


        result.fJobId = data.getShortAt(pos);
        pos += 2;

        result.fPriority = data.getShortAt(pos);
        pos += 2;

        // pszName
        int rptr = (data.getIntAt(pos) & 0xffff);
        pos += 4;
        result.fUserName = data.getZTAsciiStringAt(rptr - converter,255);

        result.fPosition = data.getShortAt(pos);
        pos += 2;

        result.fStatus = data.getShortAt(pos);
        pos += 2;

        result.fSubmitted = data.getIntAt(pos) & 0xffffffff;
        pos += 4;

        result.fSize = data.getIntAt(pos) & 0xffffffff;
        pos += 4;


        rptr = (data.getIntAt(pos) & 0xffff);
        pos += 4;
        result.fComment = data.getZTAsciiStringAt(rptr - converter,255);

        rptr = (data.getIntAt(pos) & 0xffff);
        pos += 4;
        result.fDocument = data.getZTAsciiStringAt(rptr - converter,255);


        return result;
    }


    /**
     * Lists print jobs in the specified printer queue
     * @param queuename printer queue name
     * @return CifsPrintJobInfo array
     * @exception  IOException  if an I/O error occurs.
     */
    public synchronized CifsPrintJobInfo[] listPrintJobs(String queuename) throws IOException {
         if (Debug.debugOn)
            Debug.println(Debug.INFO, "IPC:DosPrintJobEnum");

         MarshalBuffer param = new MarshalBuffer(100);

         int pos = 0;
         pos += param.setShortAt(pos,DosPrintJobEnum);
         // parameter descriptor string
         pos += param.setZTAsciiStringAt(pos,"zWrLeh");

         // he data descriptor string for the (returned) data which (Level 2)
         pos += param.setZTAsciiStringAt(pos,"WWzWWDDzz");

         // queue name
         pos += param.setZTAsciiStringAt(pos,queuename);

         // Level = 2
         pos += param.setShortAt(pos,2);

         // buffer size
         pos += param.setShortAt(pos,fReturnBufferSize);

         param.setSize(pos);

         sendTransaction(null,"\\PIPE\\LANMAN",  param,null,0);

         MarshalBuffer data = new MarshalBuffer(1000);

         receiveTransaction(param,data);

         pos = 0;

         int error = param.getShortAt(pos);

         // ignore more data
         if (error != 0  && error != 234)
            throw CifsIOException.getLMException(error);

         pos += 2;

         // converter
         int converter = param.getShortAt(pos) ;
         pos += 2;

         // number of entries
         int counter   = param.getShortAt(pos);
         pos += 2;




         /*
            struct PRJINFO_2 {
	            unsigned short		JobId;
	            unsigned short		Priority;
	            char  			*pszUserName;
	            unsigned short		Position;
	            unsigned short		Status;
	            unsigned long		Submitted;
	            unsigned long		Size;
	            char  			*pszComment;
	            char  			*pszDocument;
            }


         */

        if (Debug.debugOn)
            data.debug("DosPrintJobEnum data");


        CifsPrintJobInfo[] result = new CifsPrintJobInfo[counter];
        CifsPrintJobInfo info;
        pos = 0;
        for(int i=0;i<counter;i++) {

                info = new CifsPrintJobInfo();

                info.fJobId = data.getShortAt(pos);
                pos += 2;

                info.fPriority = data.getShortAt(pos);
                pos += 2;

                // pszName
                int rptr = (data.getIntAt(pos) & 0xffff);
                pos += 4;
                info.fUserName = data.getZTAsciiStringAt(rptr - converter,255);

                info.fPosition = data.getShortAt(pos);
                pos += 2;

                info.fStatus = data.getShortAt(pos);
                pos += 2;

                info.fSubmitted = data.getIntAt(pos) & 0xffffffff;
                pos += 4;

                info.fSize = data.getIntAt(pos) & 0xffffffff;
                pos += 4;


                rptr = (data.getIntAt(pos) & 0xffff);
                pos += 4;
                info.fComment = data.getZTAsciiStringAt(rptr - converter,255);

                rptr = (data.getIntAt(pos) & 0xffff);
                pos += 4;
                info.fDocument = data.getZTAsciiStringAt(rptr - converter,255);


                result[i] = info;
            }
        return result;
    }


    /**
     * Pauses a print job in a printer queue
     * @param jobId print job id
     * @exception  IOException  if an I/O error occurs.
     */
    public synchronized void pausePrintJob(int jobId) throws IOException {
         if (Debug.debugOn)
            Debug.println(Debug.INFO, "IPC:DosPrintJobPause");

         doPrintJobAction(DosPrintJobPause,jobId);
    }

    /**
     * Resumes a paused print job
     * @param jobId print job id
     * @exception  IOException  if an I/O error occurs.
     */
    public synchronized void continuePrintJob(int jobId) throws IOException {
         if (Debug.debugOn)
            Debug.println(Debug.INFO, "IPC:DosPrintJobContinue");

         doPrintJobAction(DosPrintJobContinue,jobId);
    }

    /**
     * Deletes a print job
     * @param jobId print job id
     * @exception  IOException  if an I/O error occurs.
     */
    public synchronized void deletePrintJob(int jobId) throws IOException {
         if (Debug.debugOn)
            Debug.println(Debug.INFO, "IPC:DosPrintJobDel");

         doPrintJobAction(DosPrintJobDel,jobId);
    }
    /**
     * Executes a print jon action (pause, continue, del)
     * @param jobId print job id
     * @exception  IOException  if an I/O error occurs.
     */
    private synchronized void doPrintJobAction(short action,int jobId) throws IOException {

         MarshalBuffer param = new MarshalBuffer(100);

         int pos = 0;
         pos += param.setShortAt(pos,action);
         // parameter descriptor string
         pos += param.setZTAsciiStringAt(pos,"W");

         // the data descriptor string for the (returned) data which = null string
         pos += param.setZTAsciiStringAt(pos,"");

         // print job id
         pos += param.setShortAt(pos,jobId);

         param.setSize(pos);

         sendTransaction(null,"\\PIPE\\LANMAN",  param,null,0);

         MarshalBuffer data = new MarshalBuffer(10);

         receiveTransaction(param,data);

         pos = 0;

         int error = param.getShortAt(pos);

         // ignore more data
         if (error != 0  && error != 234)
            throw CifsIOException.getLMException(error);


    }

    /**
     * Lists all computers of the specified type or types that are visible
     * in the specified domain. It may also enumerate domains.
     * @param domain the name of the workgroup in which to enumerate computers
     *               of the specified type or types. If domain is null, servers
     *               are enumerated for the current domain of the computer
     * @param types  the type or types of computer to enumerate. Computer that
     *               match at least one of the specified types are returned (SV_*)
     * @exception  IOException  if an I/O error occurs.
     */
    public CifsServerInfo[] listServersInfo(String domain, int types) throws IOException {
         if (Debug.debugOn)
            Debug.println(Debug.INFO, "listServersInfo");

         MarshalBuffer param = new MarshalBuffer(100);
         MarshalBuffer data  = new MarshalBuffer(1000);

         doNetServerEnum2(domain,types,1,param,data);

         int pos = 0;

         pos += 2;

         // converter
         int converter = param.getShortAt(pos) ;
         pos += 2;

        // number of entries
         int counter   = param.getShortAt(pos);
         pos += 2;

         int maxcounter   = param.getShortAt(pos);
         pos += 2;


        if (maxcounter > counter)
            if (Debug.debugOn)
                Debug.println(Debug.WARNING,"The buffer for NetServerEnum2 was too small");

        /*
        struct SERVER_INFO_1 {
	        char			sv1_name[16];
	        char			sv1_version_major;
	        char			sv1_version_minor;
	        unsigned long	sv1_type;
	        char  		*sv1_comment_or_master_browser;
        };
        */

        CifsServerInfo[] infolist = new CifsServerInfo[counter];

        pos = 0;
        for(int i=0;i<counter;i++) {


            CifsServerInfo info = new CifsServerInfo();

            info.fComputerName = data.getZTAsciiStringAt(pos,16);
            pos += 16;

            info.fMajorVersion = data.getByteAt(pos) & 0xff;
            pos += 1;

            info.fMinorVersion = data.getByteAt(pos) & 0xff;
            pos += 1;

            info.fType = data.getIntAt(pos);
            pos += 4;

            int rptr = (data.getIntAt(pos) & 0xffff);
            pos += 4;

            if (rptr != 0)
                info.fComment = data.getZTAsciiStringAt(rptr - converter,255);

            infolist[i] = info;
        }

        return infolist;

    }

    /**
     * Lists all computers of the specified type or types that are visible
     * in the specified domain. It may also enumerate domains.
     * @param domain the name of the workgroup in which to enumerate computers
     *               of the specified type or types. If domain is null, servers
     *               are enumerated for the current domain of the computer
     * @param types  the type or types of computer to enumerate. Computer that
     *               match at least one of the specified types are returned (SV_*)
     * @return  <code>java.lang.String</code> (sorted)
     * @exception  IOException  if an I/O error occurs.
     */
    public synchronized String[] listServersNames(String domain, int types) throws IOException {
         if (Debug.debugOn)
            Debug.println(Debug.INFO, "listServersNames");

         MarshalBuffer param = new MarshalBuffer(100);
         MarshalBuffer data  = new MarshalBuffer(1000);

         // Level 0
         doNetServerEnum2(domain,types,0,param,data);

         int pos = 0;

         pos += 2;

         // converter
         int converter = param.getShortAt(pos) ;
         pos += 2;

        // number of entries
         int counter   = param.getShortAt(pos);
         pos += 2;

         int maxcounter   = param.getShortAt(pos);
         pos += 2;

         if (maxcounter > counter)
            if (Debug.debugOn)
                Debug.println(Debug.WARNING,"The buffer for NetServerEnum2 was too small");



        /*
        struct SERVER_INFO_0 {
	        char		sv0_name[16];
        };
        */


        String[] names = new String[counter];


        for(int i=0;i<counter;i++)
            names[i] = data.getZTAsciiStringAt(pos,i*16);

        Util.sortStrings(names);

        return names;

    }
    private void doNetServerEnum2(String domain, int types, int level,
                                  MarshalBuffer param, MarshalBuffer data) throws IOException {



         int pos = 0;
         pos += param.setShortAt(pos,NetServerEnum2);
         // parameter descriptor string
         pos += param.setZTAsciiStringAt(pos,"WrLehDz");

         // he data descriptor string for the (returned) data
         switch(level) {
            case 0:
                pos += param.setZTAsciiStringAt(pos,"B16");
                break;
            case 1:
                pos += param.setZTAsciiStringAt(pos,"B16BBDz");
                 break;
            default:
                Debug.println(Debug.ERROR,"Invalid NetServerEnum2 level");
                throw new InternalError("doNetServerEnum2");
         }


         // corresponding to the "W": Level 1
         pos += param.setShortAt(pos,level);

         // buffer size
         pos += param.setShortAt(pos,fReturnBufferSize);

         if (domain == null)
            types |= CifsServerInfo.SV_TYPE_DOMAIN_ENUM;

         // select types
         pos += param.setIntAt(pos,types);


         // domain
         if (domain != null)
            pos += param.setZTAsciiStringAt(pos,domain);
         else
            pos += param.setByteAt(pos,(byte)0);

         param.setSize(pos);


         sendTransaction(null,"\\PIPE\\LANMAN",  param,null,0);
         receiveTransaction(param,data);


         int error = param.getShortAt(0);

         // ignore more data
         if (error != 0  && error != 234)
            throw CifsIOException.getLMException(error);
    }

    /**
     * Returns information about the current  server
     * @return server informations
     * @exception  IOException  if an I/O error occurs.
     */
    public synchronized CifsServerInfo getServerInfo() throws IOException {
         if (Debug.debugOn)
            Debug.println(Debug.INFO, "getServerInfo");

         MarshalBuffer param = new MarshalBuffer(100);
         MarshalBuffer data  = new MarshalBuffer(1000);


         int pos = 0;
         pos += param.setShortAt(pos,NetGetServerInfo);
         // parameter descriptor string
         pos += param.setZTAsciiStringAt(pos,"WrLh");

         // data descriptor
         pos += param.setZTAsciiStringAt(pos,"B16BBDz");


         // corresponding to the "W": Level 1
         pos += param.setShortAt(pos,1);

         // buffer size
         pos += param.setShortAt(pos,fReturnBufferSize);

         param.setSize(pos);


         sendTransaction(null,"\\PIPE\\LANMAN",  param,null,0);
         receiveTransaction(param,data);

         pos = 0;
         int error = param.getShortAt(pos);

         // ignore more data
         if (error != 0  && error != 234)
            throw CifsIOException.getLMException(error);

         pos += 2;

         // converter
         int converter = param.getShortAt(pos) ;
         pos += 2;

        // number of entries
         int counter   = param.getShortAt(pos);
         pos += 2;

         int bytes   = param.getShortAt(pos);
         pos += 2;


        /*
        struct SERVER_INFO_1 {
	        char			sv1_name[16];
	        char			sv1_version_major;
	        char			sv1_version_minor;
	        unsigned long	sv1_type;
	        char  		*sv1_comment_or_master_browser;
        };
        */

        CifsServerInfo info = new CifsServerInfo();

        pos = 0;

        info.fComputerName = data.getZTAsciiStringAt(pos,16);
        pos += 16;

        info.fMajorVersion = data.getByteAt(pos) & 0xff;
        pos += 1;

        info.fMinorVersion = data.getByteAt(pos) & 0xff;
        pos += 1;

        info.fType = data.getIntAt(pos);
        pos += 4;

        int rptr = (data.getIntAt(pos) & 0xffff);
        pos += 4;

        if (rptr != 0)
        info.fComment = data.getZTAsciiStringAt(rptr - converter,255);

        return info;

    }
    /*
     * Changes password on the server
     * @param user user name
     * @param oldpwd old password
     * @param newpwd new password
     * @exception  IOException  if an I/O error occurs.
     */
    public synchronized void changePassword(String user, String oldpwd, String newpwd) throws IOException {

        if (Debug.debugOn)
            Debug.println(Debug.INFO, "SamOEMChangePassword");

         MarshalBuffer param = new MarshalBuffer(100);

         int pos = 0;
         pos += param.setShortAt(pos,SamOEMChangePassword);
         // parameter descriptor string
         pos += param.setZTAsciiStringAt(pos,"zsT");

         // the data descriptor string for the (returned) data which = null string
         pos += param.setZTAsciiStringAt(pos,"B516B16");

         // user
         pos += param.setZTAsciiStringAt(pos,user);

         // data size
         pos += param.setShortAt(pos,532);

         param.setSize(pos);

         byte[] data = CifsLogin.getChangePasswordData(oldpwd,newpwd);




         sendTransaction(null,"\\PIPE\\LANMAN",  param,data,data.length);

         MarshalBuffer rdata = new MarshalBuffer(100);

         receiveTransaction(param,rdata);

         pos = 0;

         int error = param.getShortAt(pos);

         // ignore more data
         if (error != 0  && error != 234)
            throw CifsIOException.getLMException(error);


    }



    public String toString() {

        return "Session:" + fSessionName + ", Type=Admin, Host=" + fShare.getHostName();

    }


    int getSortPosition() {
        return 2;
    }

    private ShareInfoComparator getShareInfoComparator() {

        if (fShareInfoComparator == null)
            fShareInfoComparator = new ShareInfoComparator();

        return fShareInfoComparator;
    }
}
// Modified by Ashwin, full qualified Comparator
class ShareInfoComparator implements org.gnu.jcifs.util.Comparator {

     Collator fCollator = Collator.getInstance();

     public int compare(Object o1, Object o2) {
        CifsShareInfo fo1 = (CifsShareInfo)o1;
        CifsShareInfo fo2 = (CifsShareInfo)o2;
        return fCollator.compare(fo1.fShareName,fo2.fShareName);


     }

}
