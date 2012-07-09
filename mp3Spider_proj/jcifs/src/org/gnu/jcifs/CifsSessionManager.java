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
import java.text.Collator;
import java.awt.Frame;

/**
 * This abtract class implements the factory methods for CIFS services
 * (Disk, Printer, RemoteAdmin).<p>
 * <h3>How to mount a disk?</h3>
 * <pre>
 *   // share_name has UNC- (\\server\share) or URL-Syntax (cifs://server/share)
 *   CifsLogin login = new CifsLogin( user, password);
 *
 *   CifsDisk disk = CifsServiceManager.connectDisk(diskname,"\\server\share",login);
 */
public abstract class CifsSessionManager {

    private final static String  VERSION     = "1.1";
    private final static String  COPYRIGHT   = "(C) 1999, Norbert Hranitzky, GNU GPL";


    /** Sort sessions by name */
    public final static int   SESSION_SORT_NAME = 0;
    /** Sort sessions by type */
    public final static int   SESSION_SORT_TYPE = 1;

    /** Debug none */
    public final static int   DEBUG_NONE     = Debug.NONE;
    /** Debug errors */
    public final static int   DEBUG_ERROR    = Debug.ERROR ;
    /** Debug errors, warnings */
    public final static int   DEBUG_WARNING  = Debug.WARNING;
    /** Debug errors, warnings, infos */
    public final static int   DEBUG_INFO     = Debug.INFO;
    /** Debug errors, warnings, infos, buffers */
    public final static int   DEBUG_BUFFER   = Debug.BUFFER;

    private static CifsLogin  fDefaultLogin  = new CifsLogin();


    private static boolean    fAllowLoginPrompt = false;
    
    private static boolean    fProtHandlerInstalled = false;

    
    // Counter for session name generation
    private static int        fSessionSeqNumber = 0;
    

    private CifsSessionManager() {

    }

    /**
     * Returns the version number of the JCIFS package
     * @return version number
     */
    public static String getPackageVersion() {
        return VERSION;
    }
    /**
     * Returns the copyright text
     * @return copyright text
     */
    public static String getPackageCopyright() {
        return COPYRIGHT;
    }

    /**
     * Sets debug level
     * @param level debug level (DEBUG_*)
     */
    public static void setDebugLevel(int level) {
        Debug.setDebugLevel(level);
    }

    /**
     * Sets debug file. If fname is null, the debug file is closed
     * @param fname debug output file
     */
    public static  void setDebugFile(String fname)  {
        Debug.setDebugFile(fname);
    }

    /**
     * Sets the account name for all connections. If user name is not set, the
     * user name of the process is used.
     * @param account account name
     */
    public static void setAccount(String account) {
        fDefaultLogin.setAccount(account);
    }

    /**
     * Sets the password for all connections
     * @param password password
     */
    public static void setPassword(String password) {
        fDefaultLogin.setPassword(password);
    }


    /**
     * Opens a dialog box and requests login data (account name and password).
     * The login data is used if a connect call has no explicit login data.
     * @param frame frame or null
     * @param false of dialog cancelled
     */
    public static boolean loginDialog(Frame frame) {

        if (frame == null)
            frame = new Frame();

        LoginDialog dlg = new LoginDialog(frame);

        return dlg.prompt(CifsRuntimeException.getMessage("DLG_PROMPT"), fDefaultLogin);

    }

    public static CifsSession lookupSession(String sessionname) {
        return SessionImpl.lookupSession(sessionname);
    }

    /**
     * Enumerates connected sessions (unsorted)
     * @return Enumeration
     */
    public static Enumeration enumerateSessions() {
        return SessionImpl.enumerateSessions();
    }

    /**
     * List sessions
     * @param sortby sorted by (SESSION_SORT_NAME or SESSION_SORT_TYPE)
     * @return CifsSession array
     */
    public static CifsSession[] getSessions(int sortby) {
        CifsSession[] sessions  = SessionImpl.getSessions();

        Util.sort(sessions,new SessionComparator(sortby));
        return sessions;
    }
    /**
     * Connect to the given share. The default user name and password will be used
     * @param sessionname a symbolic name of this session. Names with the prefix $ 
     *                    are reserved for internal use.
     * @param sharename: <code>\\host\share</code> or <code>cifs://host/share</code>
     * @return disk
     */
    public static CifsDisk connectDisk(String sessionname,String sharename)
                                    throws IOException {


		return connectDisk(sessionname,sharename,(CifsLogin)fDefaultLogin.clone());

    }
    /**
     * Connect to the given share
     * @param sessionname a symbolic name of the session
     * @param sharename: <code>\\host\share</code> or <code>cifs://host/share</code>
     * @param login   authentication data
     * @return disk
     */
    public static CifsDisk connectDisk(String sessionname,String sharename, CifsLogin login)
                                    throws IOException {

        // check if disk already open

        CifsSession session = lookupSession(sessionname);

        if (session != null)
            throw new CifsIOException("SS1",sessionname);

        if (login == null)
            login = fDefaultLogin;

        Share      share = new Share(sharename,Share.DISK,login);
        NBTSession nbt   = new NBTSession();
        SMBMessage smb   = SessionImpl.allocateSMBMessage();


        int protocol;
        DiskImpl disk = null;

        try {
            protocol      = SessionImpl.negotiate(nbt,share.getHostName(),smb);
            disk = new DiskImpl(sessionname,protocol,share,nbt,smb);
            disk.connect();

        } catch(IOException e) {
            nbt.doHangup();
            nbt = null;
            smb = null;

            throw e;
        }


        return disk;
    }
    /**
     * Lookup disk with the given session name
     * @param sessionname disk session name
     * @return CifsDisk or null if no disk
     */
    public static CifsDisk lookupConnectedDisk(String sessionname) {

        CifsSession session =  SessionImpl.lookupSession(sessionname);
        if (session instanceof CifsDisk)
            return (CifsDisk)session;
        return null;
    }

    /**
     * Enumerates disk sessions
     * @return Enumeration of disk sessions (objects of type <code>CifsDisk</code>
     */
    public static Enumeration enumerateDiskSessions() {
        Enumeration enum  = SessionImpl.enumerateSessions();
        Vector      disks = new Vector();
        while(enum.hasMoreElements()) {
            CifsSession session = (CifsSession)enum.nextElement();
            if (session instanceof CifsDisk)
                disks.addElement(session);
        }
        return disks.elements();
    }



    /**
     * Disconnects the given session
     * @param sessionname the name of  session
     * @exception IOException  IO failure
     */
    public static void disconnectSession(String sessionname) throws IOException {
        CifsSession session = lookupSession(sessionname);

        if (session == null)
            throw new CifsIOException("SS2",sessionname);

        session.disconnect();
    }


    /**
     * Connect to Remote Admin Protocol
     * @param admname local alias name for this connection
     * @param host host name
     * @param login   authentication data
     * @return disk
     */
    public static CifsRemoteAdmin connectRemoteAdmin(String admname,String host) throws IOException {
        return connectRemoteAdmin(admname,host,(CifsLogin)fDefaultLogin.clone());

    }
    /**
     * Connect to Remote Admin Protocol
     * @param admname local alias name for this connection
     * @param host host name
     * @param login   authentication data
     * @return disk
     */
    public static CifsRemoteAdmin connectRemoteAdmin(String sessionname,String host, CifsLogin login) throws IOException {

        // check admname connection already open

        CifsSession session = lookupSession(sessionname);

        if (session != null)
            throw new CifsIOException("SS1",sessionname);


        if (login == null)
            login = fDefaultLogin;

        Share share = new Share(login);
        share.set(Share.IPC,host,"IPC$");

        NBTSession nbt = new NBTSession();
        SMBMessage smb = SessionImpl.allocateSMBMessage();


        int protocol;
        RemoteAdminImpl admin;


        try {
            protocol = SessionImpl.negotiate(nbt,share.getHostName(),smb);
            admin    = new RemoteAdminImpl(sessionname,protocol,share,nbt,smb);
            admin.connect();

        } catch(IOException e) {
            nbt.doHangup();
            nbt = null;
            smb = null;

            throw e;
        }


        return admin;

    }

    /**
     * Lookup remote admin session
     * @param sessionname remote admin session name
     * @return CifsRemoteAdmin or null if no session
     */
    public static CifsRemoteAdmin lookupRemoteAdminSession(String sessionname) {

        CifsSession session =  SessionImpl.lookupSession(sessionname);
        if (session instanceof CifsRemoteAdmin)
            return (CifsRemoteAdmin)session;
        return null;
    }

    /**
     * Enumerates remote admin sessions
     * @return Enumeration of remote admin sessions (objects of type <code>CifsRemoteAdmin</code>
     */
    public static Enumeration enumerateRemoteAdminSessions() {
        Enumeration enum   = SessionImpl.enumerateSessions();
        Vector      admins = new Vector();
        while(enum.hasMoreElements()) {
            CifsSession session = (CifsSession)enum.nextElement();

            if (session instanceof CifsRemoteAdmin)
                admins.addElement(session);
        }
        return admins.elements();
    }



    public static CifsPrinter connectPrinter(String sessionname,String printer) throws IOException {
        return connectPrinter(sessionname,printer,(CifsLogin)fDefaultLogin.clone());
    }

    public static CifsPrinter connectPrinter(String sessionname,String printer, CifsLogin login) throws IOException {
        Share share = new Share(printer,Share.PRINTER,login);

        CifsSession session = lookupSession(sessionname);

        if (session != null)
            throw new CifsIOException("RA2",sessionname);

        NBTSession nbt = new NBTSession();
        SMBMessage smb = SessionImpl.allocateSMBMessage();


        int protocol;
        PrinterImpl p = null;

        try {
            protocol = SessionImpl.negotiate(nbt,share.getHostName(),smb);
            p = new PrinterImpl(sessionname,protocol,share,nbt,smb);
            p.connect();

        } catch(IOException e) {
            nbt.doHangup();
            nbt = null;
            smb = null;

            throw e;
        }


        return p;

    }

    /**
     * Lookup printer session
     * @param sessionname printer session name
     * @return CifsRemoteAdmin or null if no session
     */
    public static CifsPrinter lookupPrinterSession(String sessionname) {

        CifsSession session =  SessionImpl.lookupSession(sessionname);
        if (session instanceof CifsPrinter)
            return (CifsPrinter)session;
        return null;
    }

    /**
     * Enumerates printer sessions
     * @return Enumeration of remote admin sessions (objects of type <code>CifsPrinter</code>
     */
    public static Enumeration enumeratePrinterSessions() {
        Enumeration enum   = SessionImpl.enumerateSessions();
        Vector      admins = new Vector();
        while(enum.hasMoreElements()) {
            CifsSession session = (CifsSession)enum.nextElement();

            if (session instanceof CifsPrinter)
                admins.addElement(session);
        }
        return admins.elements();
    }

    public static void setAllowLoginDialog(boolean on) {
        fAllowLoginPrompt = on;
    }

    public static boolean getAllowLoginDialog() {
        return fAllowLoginPrompt;
    }

    /**
     * Clears the name resolver cache and reload LMHOST file
     */
    public static void clearNameResolverCache() {
        NBTNameResolver.clearCache();
    }
    
    /**
     * Creates an unique session name. The generated name has
     * the prefix:<code>$usn_</code>
     * @return unique session name
     * @since 1.1
     */
    public static synchronized String createUSN() {
    	
    	return "$usn_" + (fSessionSeqNumber++);
    }

	/**
     * Install protocol handler package. This method can be called more than one time.
     * If the protocol handler package cannot be installed, you must install the
     * protocol stream handler factory explicitly:<p>
     * <code>URL.setURLStreamHandlerFactory(new CifsURLStreamHandlerFactory ());</code>
     *
     * @return true if the protocol handler package is installed
     * @since 1.1
     */
    public static boolean installCifsProtocolHandler() {
    	
    	if (fProtHandlerInstalled)
    		return true;
    		
    	try {
    		 String pkgs = System.getProperty("java.protocol.handler.pkgs");
    		 if (pkgs != null)
    		 	pkgs += "| org.gnu.jcifs.www";
    		 else
    		 	pkgs = "org.gnu.jcifs.www";

			 System.getProperties().put("java.protocol.handler.pkgs",pkgs);
			 fProtHandlerInstalled = true;
			 
    	} catch(Exception e) {
    		
    	}
    	return fProtHandlerInstalled;
    }

	/**
	 * Returns true if the protocol handler "cifs" was installed
	 * @return true protocol handler installed
	 * @since 1.1
	 */
	public boolean isProtocolHandlerInstalled() {
		return 	fProtHandlerInstalled;
		
	}
	static {
		installCifsProtocolHandler();
		
	}
}
// Modified by Ashwin, full qualified Comparator
class SessionComparator implements org.gnu.jcifs.util.Comparator {

     Collator fCollator = Collator.getInstance();

     int fSort ;

     public SessionComparator(int sort) {
        fSort = sort;

     }
     public int compare(Object o1, Object o2) {
        SessionImpl fo1 = (SessionImpl)o1;
        SessionImpl fo2 = (SessionImpl)o2;

        switch(fSort) {
            case CifsSessionManager.SESSION_SORT_NAME:
                return fCollator.compare(fo1.getSessionName(),fo2.getSessionName());

            case CifsSessionManager.SESSION_SORT_TYPE:
                int p1 = fo1.getSortPosition();
                int p2 = fo2.getSortPosition();

                if (p1 == p2)
                    return fCollator.compare(fo1.getSessionName(),fo2.getSessionName());

                return (p1 < p2) ? -1: +1;
        }


        throw new InternalError("SessionComparator");
    }
}
