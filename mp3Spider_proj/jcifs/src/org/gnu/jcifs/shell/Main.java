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


package org.gnu.jcifs.shell;


import org.gnu.jcifs.*;
import org.gnu.jcifs.util.*;
import java.io.*;
import java.util.*;

/**
 * Test shell
 */
public class Main  {

    private final static String PROGRAM   = "JCIFS ";

    private final static String HELP      = "Type ? for help";

    private final static String DEBUG_FILE = "jcifs.log";

    private static int          fDebug = 0;
    private String              fCurrentSessionName = "$";
    private CifsDisk            fDisk;
    private CifsRemoteAdmin     fAdm;
    private CifsPrinter         fPrinter;
    private Vector              fStartup;

    private String[]            fHistory     = new String[10];
    private int                 fLastHistory  = -1;


    public final static String CRLF = "\r\n";

    private  CifsDiskContext    fCtxt;

    private final static int CMD_MOUNT   = 0;
    private final static int CMD_UNMOUNT = 1;
    private final static int CMD_DIR     = 2;
    private final static int CMD_DEL     = 3;
    private final static int CMD_MKDIR   = 4;
    private final static int CMD_RMDIR   = 5;
    private final static int CMD_PING    = 6;
    private final static int CMD_VINFO   = 7;
    private final static int CMD_DISKS   = 8;
    private final static int CMD_FINFO   = 9;
    private final static int CMD_CD      = 10;
    private final static int CMD_PWD     = 11;
    private final static int CMD_ONADM   = 12;
    private final static int CMD_CLOSE   = 13;
    private final static int CMD_ADMINS  = 14;
    private final static int CMD_UINFO   = 15;
    private final static int CMD_WINFO   = 16;
    private final static int CMD_GETT    = 17;
    private final static int CMD_PUTT     = 18;
    private final static int CMD_PRINTQ  = 19;
    private final static int CMD_DOMAINS = 20;
    private final static int CMD_SINFO   = 21;

    private final static int CMD_SETATTR = 23;
    private final static int CMD_HELP    = 24;
    private final static int CMD_EXIT    = 25;
    private final static int CMD_SHARES  = 26;
    private final static int CMD_REN     = 27;
    private final static int CMD_OPENP   = 28;
    //private final static int CMD_CLOSEP  = 29;
    private final static int CMD_PRINT   = 30;
    private final static int CMD_LOGIN   = 31;
    private final static int CMD_PROMPT  = 32;
    private final static int CMD_SPOOLJOBS  = 33;
    private final static int CMD_CHNGPWD  = 34;
    private final static int CMD_SESSIONS  = 35;
    private final static int CMD_GETB    = 36;
    private final static int CMD_PUTB     = 37;
    private final static int CMD_SETPROP     = 38;
    private final static int CMD_GETPROP     = 39;
    private final static int CMD_LISTPROP    = 40;
    private final static int CMD_DELJOB      = 41;


    private Vector       fCmds  = new Vector();
    private Vector       fXCmds = new Vector();

    private PasswordDialog fPwdDlg = null;

    private DataInputStream  in;

    final static String   NL = System.getProperty("line.separator");

    Vector match = new Vector();

    public static void main(String[] argv) throws IOException {


        System.out.print(PROGRAM + CifsSessionManager.getPackageVersion());
        System.out.println(" ," + CifsSessionManager.getPackageCopyright());
        System.out.println(HELP);

        int n = 0;
        String sf = null;
        String pf = null;

        Properties properties = new Properties();

        while(n < argv.length) {


            if (argv[n].equals("-d")) {
                if (n + 1 < argv.length) {
                  try {
                    fDebug = Integer.parseInt(argv[++n]);
                  } catch(NumberFormatException e) {
                    fDebug = 0;
                  }
                } else {
                    System.err.println("Missing debug level");
                    System.exit(1);
                }
            } else if (argv[n].equals("-s")) {
                if (n + 1 < argv.length)
                    sf = argv[++n];
                else {
                    System.err.println("Missing startup file");
                    System.exit(1);
                }
            } else if (argv[n].equals("-p")) {
                if (n + 1 < argv.length)
                    pf = argv[++n];
                else {
                    System.err.println("Missing property file");
                    System.exit(1);
                }

            } else {
                System.err.println("Options: [-d <level>] [-p <file>] [-s <file>]");
                System.err.println("-d <level>  - Debug level: 1..5");
                System.err.println("-s <file>   - Startup file");
                System.err.println("-p <file>   - Properties file");
                System.exit(1);
            }
            n++;
        }
        Vector startup = new Vector();
        if (sf != null) {
            FileReader fir = new FileReader(sf);
            BufferedReader in = new BufferedReader(fir);
            String s;

            while  ( (s = in.readLine()) != null)
                startup.addElement(s);

            fir.close();
            System.err.println("Startup file  = " + sf);
        }

        if (pf != null) {
            FileInputStream in = new FileInputStream(pf);
            properties.load(in);
            System.err.println("Property file = " + pf);

            Enumeration enum = properties.keys();
            while (enum.hasMoreElements()) {
                String k = (String)enum.nextElement();
                Object v = properties.get(k);
                System.getProperties().put(k,v);
            }
        }

        Main p = new Main();

        if (fDebug > 0) {
            CifsSessionManager.setDebugFile(DEBUG_FILE);
            CifsSessionManager.setDebugLevel(fDebug);

            System.err.println("Debug level=" + fDebug + ",Debug file=" + DEBUG_FILE);
        }

        p.start(startup);
    }

    Main() {

        initCommands();

        fCurrentSessionName = "$";

    }

    void start(Vector startup) throws IOException {

        String cmdline;


        in = new DataInputStream(System.in);



        doStartup(startup );

        CifsSessionManager.setAllowLoginDialog(true);

        while(true) {
            System.out.print(fCurrentSessionName + ":>");
            cmdline = in.readLine();

            if (cmdline == null)
                continue;


            if (cmdline.startsWith("!")) {
                String h = cmdline.trim();
                if (h.length() == 1) {
                    doShowHistory();
                    continue;
                } else
                    cmdline = getHistoryCommand(h);
            } else {
                addHistory(cmdline);
            }

            execCommand(cmdline);

        }
    }


    private void execCommand(String cmdline) {

        Cmd    cmd;
        String op;

        if (cmdline == null)
                return;

        String[] argv = getParams(cmdline);

        if (argv.length == 0)
            return;

        op = argv[0].trim();

        if (op.endsWith(":")) {
            // change session
            if (op.charAt(0) == '$') {
                fCurrentSessionName = "$";

                return;
            }
            // get session name
            op = op.substring(0,op.length()-1);

            CifsSession session = CifsSessionManager.lookupSession(op);
            if (session != null) {
                fCurrentSessionName = op;
                System.out.println(session.toString());
            } else
                System.out.println("Session '" + op + "' does not exist");

            return;
        }



            lookupCmd(op,match);

            if (match.size() == 0) {
                System.out.println("Command '" + op + "' not defined");
                return;
            }
            if (match.size() > 1) {
                System.out.println("Ambiguous command:" + op);
                for(int i=0;i<match.size();i++) {
                    cmd = (Cmd)match.elementAt(i);
                    System.out.println(cmd.fName);
                }
                return;
            }

            cmd = (Cmd)match.elementAt(0);

            try {

                switch(cmd.fType) {
                    case CMD_SETPROP:
                        doSetProperty(cmdline,cmd);
                        break;
                    case CMD_GETPROP:
                        doGetProperty(argv,cmd);
                        break;
                    case CMD_LISTPROP:
                        doListProperties(argv,cmd);
                        break;
                    case CMD_HELP:
                        doHelp(argv,cmd);
                        break;
                    case CMD_EXIT:
                        doExit();
                        break;
                    case CMD_MOUNT:
                        doMount(argv,cmd);
                        break;
                    case CMD_UNMOUNT:
                        doUnmount(argv,cmd);
                        break;
                    case CMD_DIR:
                        doDir(argv,cmd);
                        break;
                    case CMD_DEL:
                        doDel(argv,cmd);
                        break;
                    case CMD_REN:
                        doRen(argv,cmd);
                        break;
                    case CMD_MKDIR:
                        doMkDir(argv,cmd);
                        break;
                    case CMD_RMDIR:
                        doRmDir(argv,cmd);
                        break;
                    case CMD_PING:
                        doEcho(argv,cmd);
                        break;
                    case CMD_VINFO:
                        doVolInfo(argv,cmd);
                        break;
                    case CMD_DISKS:
                        doDisks(argv,cmd);
                        break;
                    case CMD_FINFO:
                        doGetFileInfo(argv,cmd);
                        break;
                    case CMD_CD:
                        doCd(argv,cmd);
                        break;
                    case CMD_PWD:
                        doPwd(argv,cmd);
                        break;
                    case CMD_ONADM:
                        doAdmOn(argv,cmd);
                        break;
                    case CMD_CLOSE:
                        doClose(argv,cmd);
                        break;
                    case CMD_ADMINS:
                        doAdmins(argv,cmd);
                        break;
                    case CMD_SHARES:
                        doShares(argv,cmd);
                        break;

                    case CMD_WINFO:
                        doWInfo(argv,cmd);
                        break;
                    case CMD_UINFO:
                        doUserInfo(argv,cmd);
                        break;
                    case CMD_GETT:
                        doGetAsText(argv,cmd);
                        break;
                    case CMD_PUTT:
                        doPutAsText(argv,cmd);
                        break;
                    case CMD_GETB:
                        doGetAsBin(argv,cmd);
                        break;
                    case CMD_PUTB:
                        doPutAsBin(argv,cmd);
                        break;
                    case CMD_PRINTQ:
                        doPrintQ(argv,cmd);
                        break;
                    case CMD_DOMAINS:
                        doDomains(argv,cmd);
                        break;
                    case CMD_SINFO:
                        doSInfo(argv,cmd);
                        break;

                    case CMD_SETATTR:
                        doSetAttr(argv,cmd);
                        break;
                    case CMD_OPENP:
                        doOpenPrinter(argv,cmd);
                        break;

                    case CMD_PRINT:
                        doPrint(argv,cmd);
                        break;
                    case CMD_LOGIN:
                        doLogin(argv);
                        break;
                    case CMD_PROMPT:
                        doPrompt(argv,cmd);
                        break;
                    case CMD_SPOOLJOBS:
                        doSpoolJobs(argv,cmd);
                        break;
                    case CMD_CHNGPWD:
                        doChangePwd(argv,cmd);
                        break;
                    case CMD_SESSIONS:
                        doEnumSessions(argv,cmd);
                        break;
                    case CMD_DELJOB:
                        doDelPrintJob(argv,cmd);
                        break;

                }

            } catch(CifsIOException e) {
                System.out.println("[" + cmd.fName + "]:" + e.getMessage());
                if (e.isConnectionLost())
                    System.out.println("!!!! Connection lost !!!");

                if (fDebug >= Debug.INFO) e.printStackTrace();
            } catch(Exception e) {
                System.out.println( e.getMessage());
                if (fDebug >= Debug.INFO) e.printStackTrace();
            }

        }

    /*
     * exit
     */
    private void doExit() {
          disconnect();
          Debug.close();
          System.exit(0);
    }

    /*
     * help [cmd]
     */
    private void doHelp(String[] argv, Cmd cmd) {
        Cmd hcmd;

        if (argv.length == 1) {
            for(int i=0;i<fCmds.size();i++) {
                if ( (i+1) % 20 == 0)
                    more();
                hcmd = (Cmd)fCmds.elementAt(i);
                System.out.println(hcmd.getShort());
            }
            for(int i=0;i<fXCmds.size();i++) {
                hcmd = (Cmd)fXCmds.elementAt(i);
                 System.out.println(hcmd.getShort());
            }
            System.out.println();
            System.out.println("Type ? cmd for more informations");
        } else {
            Vector match = new Vector();
            lookupCmd(argv[1],match);
            for(int i=0;i<match.size();i++) {
                hcmd = (Cmd)match.elementAt(i);
                System.out.println(hcmd.getLong());
            }

        }

    }

    /***********************************************************
     *               Disk operations
     **********************************************************/

    /*
     * gett rfile lfile
     */
    void doGetAsText(String[] argv, Cmd cmd)  throws IOException{
        if (argv.length < 3) {
            syntaxError(cmd);
            return;
        }


        fDisk = getDisk(fCurrentSessionName);

        FileWriter fw = new FileWriter(argv[2]);
        BufferedWriter out = new BufferedWriter(fw,2*4096);


        long bt = System.currentTimeMillis();
        fDisk.getFile(getPathName(argv[1]),out);

        out.flush();
        fw.close();
        long et = System.currentTimeMillis();
        System.out.println("get " + argv[1] + " ->" + argv[2] + " done (" + (et-bt) + " msec)");

    }
    /*
     * getb rfile lfile
     */
    void doGetAsBin(String[] argv, Cmd cmd)  throws IOException{
        if (argv.length < 3) {
            syntaxError(cmd);
            return;
        }


        fDisk = getDisk(fCurrentSessionName);

        FileOutputStream fw = new FileOutputStream(argv[2]);
        BufferedOutputStream out = new BufferedOutputStream(fw,2*4096);


        long bt = System.currentTimeMillis();
        fDisk.getFile(getPathName(argv[1]),out);

        out.flush();
        fw.close();
        long et = System.currentTimeMillis();
        System.out.println("get " + argv[1] + " ->" + argv[2] + " done (" + (et-bt) + " msec)");

    }

    /*
     * put lfile rfile
     */
    void doPutAsText(String[] argv, Cmd cmd)  throws IOException{
        if (argv.length < 3) {
             syntaxError(cmd);
            return;
        }


        fDisk = getDisk(fCurrentSessionName);

        FileReader in = new FileReader(argv[1]);

        long bt = System.currentTimeMillis();
        fDisk.putFile(getPathName(argv[2]),in);

        in.close();
        long et = System.currentTimeMillis();
        System.out.println("put " + argv[1] + " ->" + argv[2] + " done (" + (et-bt) + " msec)");

    }

    /*
     * put lfile rfile
     */
    void doPutAsBin(String[] argv, Cmd cmd)  throws IOException{
        if (argv.length < 3) {
             syntaxError(cmd);
            return;
        }


        fDisk = getDisk(fCurrentSessionName);

        FileInputStream in = new FileInputStream(argv[1]);

        long bt = System.currentTimeMillis();
        fDisk.putFile(getPathName(argv[2]),in);

        in.close();
        long et = System.currentTimeMillis();
        System.out.println("put " + argv[1] + " ->" + argv[2] + " done (" + (et-bt) + " msec)");

    }

    void doPwd(String[] argv,Cmd cmd)  throws IOException{

        fDisk = getDisk(fCurrentSessionName);

        fCtxt = (CifsDiskContext)fDisk.getProperty("cd");

        System.out.println(fCtxt.getCurrentDir());
    }

    /*
     * cd dir
     */
    void doCd(String[] argv,Cmd cmd)  throws IOException{
        if (argv.length == 1) {
            doPwd(null,cmd);
            return;
        }

        fDisk = getDisk(fCurrentSessionName);

        fCtxt = (CifsDiskContext)fDisk.getProperty("cd");

        fCtxt.changeCurrentDir(argv[1]);

    }


    /*
     * info file
     */
    void doGetFileInfo(String[] argv,Cmd cmd) throws IOException {
        if (argv.length != 2) {
            syntaxError(cmd);
            return;
        }

        fDisk = getDisk(fCurrentSessionName);

        CifsFileInfo info = fDisk.getFileInfo(getPathName(argv[1]));

        System.out.println(info);

    }
    /*
     * mktmp file
     */
     /* does not work
    void doMkTmp(String[] argv)  throws IOException{
        if (argv.length != 2) {
            System.out.println("Syntax: mktmp file");
            return;
        }

       fDisk = getDisk(fCurrentSessionName);

       System.out.println("File=" + fDisk.createTemporaryFile(argv[1]));

    }
    */
    /*
     * mkdir dir
     */
    void doMkDir(String[] argv,Cmd cmd)  throws IOException{
        if (argv.length != 2) {
            syntaxError(cmd);
            return;
        }

        fDisk = getDisk(fCurrentSessionName);

        fDisk.mkdir(getPathName(argv[1]));

    }

    /*
     * rmkdir dir
     */
    void doRmDir(String[] argv,Cmd cmd)  throws IOException{
        if (argv.length != 2) {
            syntaxError(cmd);
            return;
        }

        fDisk = getDisk(fCurrentSessionName);

        fDisk.rmdir(getPathName(argv[1]));


    }
    /*
     * del path
     */
    void doDel(String[] argv,Cmd cmd)  throws IOException{
        if (argv.length != 2) {
            syntaxError(cmd);
            return;
        }
        fDisk = getDisk(fCurrentSessionName);

        fDisk.deleteFile(getPathName(argv[1]));


    }
    /*
     * setattr file attr
     */
    void doSetAttr(String[] argv, Cmd cmd)  throws IOException{
        if (argv.length < 3) {
            syntaxError(cmd);
            return;
        }
        fDisk = getDisk(fCurrentSessionName);

        int[] attr = new int[2];
        int   type = 1;
        String p = argv[2];

        for(int i=0;i<p.length();i++) {
            switch(p.charAt(i)) {
                case '+':
                    type = 1;
                    break;
                case '-':
                    type = 0;
                    break;
                case 'r':
                    attr[type] |= CifsFile.FILE_ATTR_READ_ONLY;
                    break;
                case 'h':
                    attr[type] |= CifsFile.FILE_ATTR_HIDDEN_FILE;
                    break;
                case 's':
                    attr[type] |= CifsFile.FILE_ATTR_SYSTEM_FILE;
                    break;
                case 'a':
                    attr[type] |= CifsFile.FILE_ATTR_ARCHIVE;
                    break;
                default:
                    throw new IOException("Invalid file attr:" + p);
            }
        }

        if (attr[0]==0 && attr[1]==0)
            return;


        fDisk.setFileAttribute(argv[1],attr[1],true);
        fDisk.setFileAttribute(argv[1],attr[0],false);


    }
    /*
     * ls path
     */
    void doLS(String[] argv,Cmd cmd)  throws IOException{
        String path = "*.*";
        int attr = CifsFile.FILE_ATTR_ALL;

        if (argv.length > 1) {
            path = argv[1];
        }



        fDisk = getDisk(fCurrentSessionName);


        String[] result = fDisk.listFilesName(getPathName(path),attr,true);

        for(int i=0;i<result.length;i++) {
            if ((i + 1) % 20 == 0)
                more();
             System.err.println(result[i]);
        }
        System.out.println("#Files=" + result.length);

    }
    /*
     * dir path [attr]
     */
    void doDir(String[] argv, Cmd cmd)  throws IOException{
        String path = "*.*";
        int attr = CifsFile.FILE_ATTR_ALL;

        if (argv.length > 1) {
            path = argv[1];
        }
        if (argv.length > 2) {
            attr = getAttr(argv[2]);
        }


        fDisk = getDisk(fCurrentSessionName);


        CifsFileInfo[] result = fDisk.listFilesInfo(getPathName(path),attr,true);

        long totalsize = 0;
        for(int i=0;i<result.length;i++) {

            totalsize += result[i].length();
            System.out.println(result[i]);
        }
        System.out.println("Files=" + result.length + ", Space=" + totalsize +  " bytes");
    }

    /*
     * ren path path
     */
    void doRen(String[] argv, Cmd cmd)  throws IOException{
        if (argv.length != 3) {
            syntaxError(cmd);
            return;
        }

        fDisk = getDisk(fCurrentSessionName);

        fDisk.renameFile(getPathName(argv[1]),getPathName(argv[2]),CifsFile.FILE_ATTR_ANY);

    }



    /*
     * mount diskname share [-u user] [-p password]
     */
    void doMount(String[] argv, Cmd cmd)  throws IOException{
        if (argv.length < 3) {
            syntaxError(cmd);
            return;
        }
        boolean login_spec = false;

        CifsLogin login  = new CifsLogin();
        for(int i=3;i<argv.length;i++) {
            if ("-u".equals(argv[i])) {
                if (i+1 < argv.length) {
                    login_spec = true;
                    login.setAccount(argv[++i]);
                } else {
                    syntaxError(cmd);
                    return;
                }
            } else if ("-p".equals(argv[i])) {
                if (i+1 < argv.length) {
                    login_spec = true;
                    login.setPassword(argv[++i]);
                } else {
                    syntaxError(cmd);
                    return;
                }
            } else {
                    syntaxError(cmd);
                    return;
            }
        }


        if (login_spec)
            fDisk = CifsSessionManager.connectDisk(argv[1],argv[2],login);
        else
            fDisk = CifsSessionManager.connectDisk(argv[1],argv[2]);

        CifsDiskContext ctxt = new CifsDiskContext(fDisk);
        fDisk.setProperty("cd",ctxt);


        showServer(fDisk);
    }

    /*
     * unmount diskname
     */
    void doUnmount(String[] argv, Cmd cmd)  throws IOException {

       if (argv.length < 2) {
            syntaxError(cmd);
            return;
       }

       fDisk = getDisk(argv[1]);

       fDisk.disconnect();

    }

    /*
     * Info
     */
    void doVolInfo(String[] argv, Cmd cmd)  throws IOException{

        fDisk = getDisk(fCurrentSessionName);




        CifsFileSystemInfo info = fDisk.getFileSystemInfo();

        System.out.println("Volume Label          = " + info.getVolumeLabel());
        if (info.isSizeInfoValid()) {
                System.out.println("TotalAllocUnits       = " + info.getTotalAllocUnits());
                System.out.println("FreeAllocUnits        = " + info.getFreeAllocUnits());
                System.out.println("SectorsPerUnit        = " + info.getSectorsPerUnit());
                System.out.println("BytesPerSector        = " + info.getBytesPerSector());

        }
        if (info.isDeviceInfoValid() ) {
                System.out.println("DeviceType            = " + info.getDeviceType());
                System.out.println("DeviceCharacteristics = " + info.getDeviceCharacteristics());
        }
        System.out.println("FileSystemName        = " + info.getFileSystemName());
        System.out.println("FileSystemAttr        = " + Util.intToHex(info.getFileSystemAttr()));


    }

    /*
     * Info
     */
    void doDisks(String[] argv,Cmd cmd) {

       Enumeration disks = CifsSessionManager.enumerateDiskSessions();

       while(disks.hasMoreElements()) {
            CifsDisk disk = (CifsDisk)disks.nextElement();
            System.out.println(disk.toString());
       }

    }

    /***********************************************************
     *                   Printer operations
     **********************************************************/

    /*
     * openp name share [-u user] [-p password]
     */
    void doOpenPrinter(String[] argv, Cmd cmd)  throws IOException{
        if (argv.length < 3) {
            syntaxError(cmd);
            return;
        }
        boolean login_spec = false;

        CifsLogin login  = new CifsLogin();
        for(int i=3;i<argv.length;i++) {
            if ("-u".equals(argv[i])) {
                if (i+1 < argv.length) {
                    login_spec = true;
                    login.setAccount(argv[++i]);
                } else {
                    syntaxError(cmd);
                    return;
                }
            } else if ("-p".equals(argv[i])) {
                if (i+1 < argv.length) {
                     login_spec = true;
                    login.setPassword(argv[++i]);
                } else {
                    syntaxError(cmd);
                    return;
                }
            } else {
                    syntaxError(cmd);
                    return;
            }
        }


        if (login_spec)
            fPrinter = CifsSessionManager.connectPrinter(argv[1],argv[2],login);
        else
             fPrinter = CifsSessionManager.connectPrinter(argv[1],argv[2]);


        showServer(fPrinter);
    }


    /*
     * print  file
     */
    void doPrint(String[] argv, Cmd cmd)  throws IOException {

       if (argv.length < 2) {
            syntaxError(cmd);
            return;
       }

       fPrinter = getPrinter(fCurrentSessionName);

       FileReader fr = new FileReader(argv[1]);
       BufferedReader in = new BufferedReader(fr,4096);


       CifsPrintOutputStream pout = fPrinter.openPrintFile(argv[1],CifsPrinter.TEXT_MODE);

       PrintWriter out = new PrintWriter(pout);

       String line;
       while( (line = in.readLine()) != null)
            out.print(line + CRLF);


       System.out.println("File " + argv[1] + " printed ");

       out.close();
       fr.close();


    }

    /***********************************************************
     *                   Remote Admin operations
     **********************************************************/
    /*
     * spooljobs queue
     */
    void doSpoolJobs(String[] argv, Cmd cmd)  throws IOException {

       if (argv.length < 2) {
            syntaxError(cmd);
            return;
       }

       fAdm = getAdm(fCurrentSessionName);

       CifsPrintJobInfo[] result = fAdm.listPrintJobs(argv[1]);

       for(int i=0;i<result.length;i++) {
            System.out.println("JobId=" + result[i].getJobId());
       }
    }


    void doDelPrintJob(String[] argv, Cmd cmd)  throws IOException {
        if (argv.length < 2) {
            syntaxError(cmd);
            return;
        }

        fAdm = getAdm(fCurrentSessionName);
        int jobid;

        try {
            jobid = Integer.parseInt(argv[1]);
        } catch(NumberFormatException e) {
            System.out.println("Invalid jobid");
            return;
        }
        fAdm.deletePrintJob(jobid);
    }


    /*
     * admin sessionname host [-u user] [-p password]
     */
    void doAdmOn(String[] argv, Cmd cmd)  throws IOException{
        if (argv.length < 3) {
            syntaxError(cmd);
            return;
        }
        boolean login_spec = false;
        CifsLogin login  = new CifsLogin();
        for(int i=3;i<argv.length;i++) {
            if ("-u".equals(argv[i])) {
                if (i+1 < argv.length) {
                    login_spec = true;
                    login.setAccount(argv[++i]);
                } else {
                    syntaxError(cmd);
                    return;
                }
            } else if ("-p".equals(argv[i])) {
                if (i+1 < argv.length) {
                    login_spec = true;
                    login.setPassword(argv[++i]);
                }else {
                    syntaxError(cmd);
                    return;
                }
            } else {
                    syntaxError(cmd);
                    return;
            }
        }


        if (login_spec)
            fAdm = CifsSessionManager.connectRemoteAdmin(argv[1],argv[2],login);
        else
            fAdm = CifsSessionManager.connectRemoteAdmin(argv[1],argv[2]);

        showServer(fAdm);
    }



    /*
     * shares [sessioname]
     */
    void doShares(String[] argv, Cmd cmd)  throws IOException {


       if (argv.length == 1)
          fAdm = getAdm(fCurrentSessionName);
       else
          fAdm = getAdm(argv[1]);

       CifsShareInfo[]  result = fAdm.listSharesInfo(true);

        for(int i=0;i<result.length;i++) {

            System.out.println(result[i].toString());
        }
    }

    /*
     * winfo [sessionname]
     */
    void doWInfo(String[] argv, Cmd cmd)  throws IOException {

      if (argv.length == 1)
          fAdm = getAdm(fCurrentSessionName);
       else
          fAdm = getAdm(argv[1]);


       fAdm = getAdm(argv[1]);

       CifsWorkstationInfo info = fAdm.getWorkstationInfo();


       System.out.println("Computer     =" + info.getWorkstationName());
       System.out.println("User         =" + info.getUserName());
       System.out.println("Domain       =" + info.getDomain());
       System.out.println("Version      =" + info.getMajorVersion() + "." + info.getMinorVersion());
       System.out.println("Logon Domain =" + info.getLogonDomain());
       System.out.println("All Domains  =" + info.getAllDomains());
    }

    /*
     * printq [sessionname]
     */
    void doPrintQ(String[] argv,Cmd cmd)  throws IOException {

       if (argv.length == 1)
          fAdm = getAdm(fCurrentSessionName);
       else
          fAdm = getAdm(argv[1]);

       CifsPrinterQueueInfo[] info = fAdm.listPrinterQueues();

        for(int i=0;i<info.length;i++) {
            System.out.println("Name         = " + info[i].getPrinterName());
            System.out.println("Comment      = " + info[i].getComment());
            System.out.println("Driver       = " + info[i].getDriver());
            System.out.println("Priority     = " + info[i].getPriority());
            System.out.println("Status       = " + info[i].getStatus());
            System.out.println("Starttime    = " + info[i].getStartTime());
            System.out.println("Untiltime    = " + info[i].getUntilTime());
            System.out.println("Jobs         = " + info[i].getJobsNumber());
            System.out.println("Destinations = " + info[i].getPrintDestinations());
            System.out.println("Params       = " + info[i].getParams());
        }
    }

    /*
     * uinfo  user
     */
    void doUserInfo(String[] argv, Cmd cmd)  throws IOException {

       if (argv.length < 2) {
            syntaxError(cmd);
            return;
       }

       fAdm = getAdm(fCurrentSessionName);

       CifsUserInfo info = fAdm.getUserInfo(argv[1]);


       String s;
       System.out.println("User         =" + info.getUserName());
       System.out.println("Full User    =" + info.getFullUserName());
       System.out.println("Comment      =" + info.getComment());
       System.out.println("User Comment =" + info.getUserComment());
       System.out.println("Home dir     =" + info.getHomeDir());



       switch(info.getUserPrivilege()){
            case CifsUserInfo.USER_PRIV_GUEST:
                s = "GUEST";
                break;
            case CifsUserInfo.USER_PRIV_USER:
                s = "USER ";
                break;
            case CifsUserInfo.USER_PRIV_ADMIN:
                s = "ADMIN";
                break;
            default:
                s = "?????";
                break;
       }
       System.out.println("User priv    =" + s);
       switch(info.getOperatorPrivileges()){
            case CifsUserInfo.AF_OP_PRINT:
                s = "PRINT";
                break;
            case CifsUserInfo.AF_OP_COMM:
                s = "COMM";
                break;
            case CifsUserInfo.AF_OP_SERVER:
                s = "SERVER";
                break;
            case CifsUserInfo.AF_OP_ACCOUNTS:
                s = "ACCOUNTS";
                break;
            default:
                s = "?????";
                break;
       }
       System.out.println("Operator priv=" + s);
       System.out.println("Last logon   =" + info.getLastLogon());
       System.out.println("Last logoff  =" + info.getLastLogoff());
       System.out.println("Password age =" + info.getPasswordAge());
       System.out.println("Bad logons   =" + info.getBadLogons());
       System.out.println("Logons       =" + info.getLogons());
       System.out.println("Logon server =" + info.getLogonServer());

    }


    /*
     * passwd  [user olwd new]
     */
    void doChangePwd(String[] argv, Cmd cmd)  throws IOException {

       fAdm = getAdm(fCurrentSessionName);

       PasswordDialog.PasswordData data = new PasswordDialog.PasswordData ();

       if (argv.length > 1)
            data.user = argv[1];

       if (argv.length > 2)
            data.oldpwd = argv[2];

       if (argv.length > 3)
            data.newpwd = argv[3];
       else {

            if (fPwdDlg == null)
                fPwdDlg = new PasswordDialog(new java.awt.Frame());



            if (fPwdDlg.prompt(fAdm.getNetBIOSName(),data) == false)
                return;


       }

       fAdm.changePassword(data.user,data.oldpwd,data.newpwd);

    }
    /*
     * sinfo [sessionname]
     */
    void doSInfo(String[] argv, Cmd cmd)  throws IOException {

       if (argv.length == 1)
          fAdm = getAdm(fCurrentSessionName);
       else
          fAdm = getAdm(argv[1]);



       CifsServerInfo info;

       info = fAdm.getServerInfo();

       System.out.println("Computer =" + info.getComputerName());
       System.out.println("Type     =" + Util.intToHex(info.getType()));
       System.out.println("Version  =" + info.getMajorVersion() + "." + info.getMinorVersion());
       System.out.println("Comment  =" + info.getComment());

    }
    /*
     * domains [domain]
     */
    void doDomains(String[] argv, Cmd cmd)  throws IOException {


       fAdm = getAdm(fCurrentSessionName);

       CifsServerInfo[] info;
       if (argv.length == 1)
            info = fAdm.listServersInfo(null,CifsServerInfo.SV_TYPE_DOMAIN_ENUM);
       else
            info = fAdm.listServersInfo(argv[1],CifsServerInfo.SV_TYPE_ALL);

       for (int i=0;i<info.length;i++) {
            if (i % 20 == 0)
                more();

            System.out.println("Computer =" + info[i].getComputerName());
            System.out.println("Type     =" + Util.intToHex(info[i].getType()));
            System.out.println("Version  =" + info[i].getMajorVersion() + "." + info[i].getMinorVersion());
            System.out.println("Comment  =" + info[i].getComment());
            System.out.println("------------------------");
       }
       System.out.println("Number of computer=" + info.length);
    }






    private void doStartup(Vector cmds) {

        for(int i=0;i<cmds.size();i++) {
            String c = (String)cmds.elementAt(i);
           // System.out.println("Startup:" + c);
            execCommand(c);
        }


    }




    private int getAttr(String s) {
        if (s.equalsIgnoreCase("ALL"))
            return CifsFile.FILE_ATTR_ALL;

        int a = 0;

        s = s.toUpperCase();

        if (s.indexOf('R') >= 0)
            a |= CifsFile.FILE_ATTR_READ_ONLY;
        if (s.indexOf('H') >= 0)
            a |= CifsFile.FILE_ATTR_HIDDEN_FILE;
        if (s.indexOf('S') >= 0)
            a |= CifsFile.FILE_ATTR_SYSTEM_FILE;
        if (s.indexOf('V') >= 0)
            a |= CifsFile.FILE_ATTR_VOLUME;
        if (s.indexOf('D') >= 0)
            a |= CifsFile.FILE_ATTR_DIRECTORY;
        if (s.indexOf('A') >= 0)
            a |= CifsFile.FILE_ATTR_ARCHIVE;
        return a;
    }

    String getPathName(String file) {
        fCtxt = (CifsDiskContext)fDisk.getProperty("cd");

        return fCtxt.getPathName(file);
    }

    /***********************************************************
     *                   Session operations
     **********************************************************/
     /*
     * echo text
     */
    void doEcho(String[] argv, Cmd cmd) throws IOException {
        if (argv.length < 2) {
            syntaxError(cmd);
            return;
        }

        CifsSession session = CifsSessionManager.lookupSession(fCurrentSessionName);

        if (session == null) {
            System.out.println("No Session " + fCurrentSessionName );
            return;
        }



        System.out.println(session.echo(argv[1]));
    }

    void doGetProperty(String[] argv, Cmd cmd) {

        if (argv.length < 2) {
            syntaxError(cmd);
            return;
        }
        String key = argv[1].trim();
        String val = System.getProperty(key);
        System.out.println(key + "=" + val);
    }

     /*
     * setprop name=value
     */
    void doSetProperty(String argv, Cmd cmd) {

        int p = argv.indexOf(' ');

        if (p < 0) {
            syntaxError(cmd);
            return;
        }

        argv = argv.substring(p+1).trim();

        p = argv.indexOf('=');
        if (p < 0) {
            syntaxError(cmd);
            return;
        }
        String k = argv.substring(0,p).trim();
        String v = argv.substring(p+1).trim();

        System.getProperties().put(k,v);

    }

    void doListProperties(String[] argv, Cmd cmd){

        Properties props = System.getProperties();
        Enumeration enum = props.keys();

        while (enum.hasMoreElements()) {
            String key = (String)enum.nextElement();
            if (key.startsWith("org.gnu.jcifs.")) {
                String val = props.getProperty(key);
                System.out.println(key + "=" + val);
            }
        }
    }

    /*
     * login
     */
    void doLogin(String[] argv) {


        CifsSessionManager.loginDialog(null);

    }

    /**
     * close [sessionname]
     */
    void doClose(String[] argv, Cmd cmd)  throws IOException {

        CifsSession session;
        String name;
        if (argv.length == 1)
            name = fCurrentSessionName;
        else
            name = argv[1];

        session = CifsSessionManager.lookupSession(name);

        if (session == null) {
            System.out.println("Session " + name + " does not exist");
            return;
       }

       session.disconnect();

    }

    /*
     * sessions [name|type]
     */
    void doEnumSessions(String[] argv, Cmd cmd) {

       int sortBy ;
       if (argv.length == 1)
            sortBy = CifsSessionManager.SESSION_SORT_NAME;
       else {
            if (argv[1].equalsIgnoreCase("name"))
                sortBy = CifsSessionManager.SESSION_SORT_NAME;
            else if (argv[1].equalsIgnoreCase("type"))
                sortBy = CifsSessionManager.SESSION_SORT_TYPE;
            else {
                syntaxError(cmd);
                return;
            }
       }
       CifsSession[] sessions = CifsSessionManager.getSessions(sortBy);

       for(int i=0;i<sessions.length;i++)

            System.out.println(sessions[i].toString());


    }

    /*
     * Info
     */
    void doAdmins(String[] argv,Cmd cmd) {

       Enumeration enum = CifsSessionManager.enumerateRemoteAdminSessions();

       while(enum.hasMoreElements()) {
            CifsRemoteAdmin adm = (CifsRemoteAdmin)enum.nextElement();
            System.out.println(adm.toString());
       }

    }

    void disconnect() {

       Enumeration enum = CifsSessionManager.enumerateSessions();

       while(enum.hasMoreElements()) {
            CifsSession session = (CifsSession)enum.nextElement();

            try {
                session.disconnect();
            } catch(Exception e) {
            }
       }


    }

    private void showServer(CifsSession service) {
        String os = service.getServerOS();
        String lm = service.getServerLanMan();
        String pd = service.getServerPrimaryDomain();
        System.out.println(service.toString());
        System.out.println("[OS=" + os + ",LanMan=" + lm + ",PrimaryDomain=" + pd + "]");
    }
    /*
     * prompt on|off
     */
    void doPrompt(String[] argv, Cmd cmd)  throws IOException{
         if (argv.length < 2) {
            syntaxError(cmd);
            return;
        }

        if (argv[1].equalsIgnoreCase("on"))
            CifsSessionManager.setAllowLoginDialog(true);
        else
            CifsSessionManager.setAllowLoginDialog(false);



    }

    private CifsDisk getDisk(String sessioname) throws IOException {
         CifsDisk d = CifsSessionManager.lookupConnectedDisk(sessioname);

         if (d == null)
            throw new IOException("Disk session '" + sessioname + "' does not exist");

         return d;
    }

    private CifsPrinter getPrinter(String name) throws IOException {
         CifsPrinter d = CifsSessionManager.lookupPrinterSession(name);

         if (d == null)
            throw new IOException("Printer session'" + name + "' does not exist");

         return d;
    }

    private CifsRemoteAdmin getAdm(String sessionname) throws IOException {
         CifsRemoteAdmin d = CifsSessionManager.lookupRemoteAdminSession(sessionname);

         if (d == null)
            throw new IOException("RemoteAdmin session '" + sessionname + "' unknown");

         return d;

    }

   /***********************************************************
     *                   Command helpers
     **********************************************************/

    void  doShowHistory() {


        for(int i=0;i<fHistory.length;i++) {
            String s = fHistory[i];

            if (s == null)
                return;
            if (i == fLastHistory)
                System.out.println(i + ":" + fHistory[i] + "[!!]");
            else
                System.out.println(i + ":" + fHistory[i]);

        }

    }



    String getHistoryCommand(String s) {
        if (s.equals("!!"))
            return fHistory[fLastHistory];
        s = s.substring(1);
        int p ;

        try {
            p = Integer.parseInt(s);
        } catch(NumberFormatException e) {
            return null;
        }

        if (p < 0  || p > fHistory.length)
            return null;
        return fHistory[p];

    }

    void addHistory(String s) {
        fLastHistory++;
        fLastHistory %= fHistory.length;
        fHistory[fLastHistory] = s;

    }

    void more() {
        System.out.print("(More...)");
        try {
            in.readLine();
        } catch(Exception e) {
        }
        System.out.println();
    }


    String[] getParams(String cmdline) {


        StringTokenizer tok = new StringTokenizer(cmdline," ");

        String[] params = new String[tok.countTokens()];

        int i = 0;
        while(tok.hasMoreTokens()) {

            params[i++] = tok.nextToken().trim();
        }

        return params;
    }
    private void initCommands() {

        fCmds.addElement(new Cmd("?"        , CMD_HELP,       "[<cmd>]", "Show help message"));
        fCmds.addElement(new Cmd("exit"     , CMD_EXIT,       "", "Terminate program"));

        fCmds.addElement(new Cmd("mount"    , CMD_MOUNT,      "<session> <share> [-u user] [-p password]","Mounts the disk"));
        fCmds.addElement(new Cmd("unmount"  , CMD_UNMOUNT,    "<session>","Unmounts the disk"));
        fCmds.addElement(new Cmd("disks" , CMD_DISKS,    "","Enumerates connected disk"));
        fCmds.addElement(new Cmd("echo"     , CMD_PING,       "<text>","Ping current server"));
        fCmds.addElement(new Cmd("dir"      , CMD_DIR,        "<file>",   "Lists files in directory"));
        fCmds.addElement(new Cmd("cd"       , CMD_CD,          "<dir>", "Change directory"));
        fCmds.addElement(new Cmd("pwd"      , CMD_PWD,       "", "Show current directory"));
        fCmds.addElement(new Cmd("del"      , CMD_DEL,        "<file>", "Deletes file" ));
        fCmds.addElement(new Cmd("ren"      , CMD_REN,        "<file> <file>", "Rename file" ));
        fCmds.addElement(new Cmd("mkdir"    , CMD_MKDIR,      "<dir>","Makes directory"));
        fCmds.addElement(new Cmd("rmdir"    , CMD_RMDIR,      "<dir>","Removes directory"));
        fCmds.addElement(new Cmd("gett"      , CMD_GETT,       "<rfile> <lfile>", "Gets file - text mode"));
        fCmds.addElement(new Cmd("getb"      , CMD_GETB,       "<rfile> <lfile>", "Gets file - binary mode"));
        fCmds.addElement(new Cmd("putt"      , CMD_PUTT,       "<lfile> <rfile>","Puts file - text mode"));
        fCmds.addElement(new Cmd("putb"      , CMD_PUTB,       "<lfile> <rfile>","Puts file - binary mode"));
        fCmds.addElement(new Cmd("getattr"  , CMD_FINFO,       "<file>", "Shows file attributes"));
        fCmds.addElement(new Cmd("setattr"  , CMD_SETATTR,       "<file> +|-dahrs","Sets file attributes"));
        fCmds.addElement(new Cmd("vinfo"    , CMD_VINFO,       "", "Lists volume informations"));
        fCmds.addElement(new Cmd("connprt"    , CMD_OPENP,       "<session> <share> [-u user] [-p password]", "Connect printer session"));

        fCmds.addElement(new Cmd("print"    , CMD_PRINT,      "<file>", "Print file"));


        fCmds.addElement(new Cmd("connadm"    , CMD_ONADM,       "<session> <host> [-u user] [-p password]", "Connect admin session"));

        fCmds.addElement(new Cmd("admins"   , CMD_ADMINS,  "", "Enumerates admin connections"));
        fCmds.addElement(new Cmd("uinfo"    , CMD_UINFO,       "<user>","Shows informations about user"));
        fCmds.addElement(new Cmd("winfo"    , CMD_WINFO,       "[<session>]","Show workstation informations"));
        fCmds.addElement(new Cmd("printq"   , CMD_PRINTQ,       "[<session>]","Lists print queues"));
        fCmds.addElement(new Cmd("domains"  , CMD_DOMAINS,      "[<domain>]"," Lists computers"));
        fCmds.addElement(new Cmd("sinfo"    , CMD_SINFO,       "<session>","Shows informations about the server"));
        fCmds.addElement(new Cmd("shares"   , CMD_SHARES,       "[<session>]","Lists shares on the server"));
        fCmds.addElement(new Cmd("jobs"     , CMD_SPOOLJOBS,    "<queue> ", "List spool jobs"));
        fCmds.addElement(new Cmd("deljob"     , CMD_DELJOB,    "<jobid> ", "Delete spool job"));
        fCmds.addElement(new Cmd("passwd"   , CMD_CHNGPWD,      "<user> <oldpwd> <newpwd>","Change password on the server"));
        fCmds.addElement(new Cmd("close "   , CMD_CLOSE,        "[<session>]", "Closes session"));
        fCmds.addElement(new Cmd("sessions "   , CMD_SESSIONS, "[name|type]", "Enumerates sessions"));

        fCmds.addElement(new Cmd("login"     , CMD_LOGIN,       "","Login dialog"));
        fCmds.addElement(new Cmd("prompt"    , CMD_PROMPT,     "on|off","Enables/Disables login prompt"));
        fCmds.addElement(new Cmd("setprop"     , CMD_SETPROP,       "key=val","Set system property"));
        fCmds.addElement(new Cmd("getprop"     , CMD_GETPROP,       "key","Get system property"));
        fCmds.addElement(new Cmd("props"     , CMD_LISTPROP,       "","List system properties"));

        fXCmds.addElement(new Cmd("session:" , 0,       "","Changes current session"));
        fXCmds.addElement(new Cmd("!"        , 0,       "","Shows command history"));
        fXCmds.addElement(new Cmd("!!"       , 0,       "","Executes last command"));
        fXCmds.addElement(new Cmd("!<n>"     , 0,       "","Execute  command in the history with index n"));


    }

    void lookupCmd(String s, Vector match) {
        match.removeAllElements();

        s = s.toLowerCase();

        for(int i=0;i<fCmds.size();i++) {
            Cmd cmd = (Cmd)fCmds.elementAt(i);
            if (s.equals(cmd.fName)) {
                match.addElement(cmd);
                return ;
            }
            if (cmd.fName.startsWith(s)) {
                match.addElement(cmd);
            }
        }

    }

    void syntaxError(Cmd cmd) {
        if (cmd != null)
            System.out.println("Syntax error:" + cmd.fSyntax);
        else
            System.out.println("Syntax error");
    }
}

class Cmd {

    String fName;
    int    fType;
    String fSyntax;
    String fHelp;

    Cmd(String name, int type, String syntax, String help) {
        fName = name;
        fType = type;
        fSyntax = syntax;
        fHelp = help;
    }

    public String getShort() {
        StringBuffer buf = new StringBuffer(100);

        buf.append(fName);
        buf.append(" ");
        buf.append(fSyntax);

        return buf.toString();
    }

    public String getLong() {
        StringBuffer buf = new StringBuffer(100);

        buf.append(fName);
        buf.append(" ");
        buf.append(fSyntax);
        buf.append(Main.NL);
        for(int i=0;i<25;i++) buf.append(' ');
        buf.append("- ");
        buf.append(fHelp);

        return buf.toString();
    }
}