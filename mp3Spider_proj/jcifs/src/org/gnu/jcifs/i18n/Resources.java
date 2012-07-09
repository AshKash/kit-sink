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


package org.gnu.jcifs.i18n;



import java.util.*;
import java.text.*;


public class Resources extends ListResourceBundle {

    static final Object[][] fContent = {
        // AuthDialog
         {"DLG_TITLE"       , "CIFS Login" }
        ,{"DLG_ACCOUNT"     , "Account" }
        ,{"DLG_PASSWORD"    , "Password" }
        ,{"DLG_OK"          , "Ok" }
        ,{"DLG_CANCEL"      , "Cancel" }
        ,{"DLG_PROMPT"      , "Login data" }

        // Communication errors
        ,{"CM1"             , "Host unknown: {0}" }
        ,{"CM2"             , "Cannot connect to server: {0}" }
        ,{"CM3"             , "Cannot resolve NetBIOS name {0}" }


        // NetBIOS negativ response errors

        ,{"NB128"           , "Not listining called name" }         // NR_EC_NOT_LISTINING_CALLED_NAME
        ,{"NB129"           , "Not listining calling name" }         // NR_EC_NOT_LISTINING_CALLING_NAME
        ,{"NB130"           , "Called name not present" }            // NR_EC_CALLED_NAME_NOT_PRESENT
        ,{"NB131"           , "Insufficient resources" }             // NR_EC_INSUFFICIENT_RESOURCES
        ,{"NB143"           , "Unspecified NetBIOS response error" } // NR_EC_UNSPECIFIED_ERROR
        ,{"NB9999"          , "NetBIOS negative response {0}" }

        // Own NetBIOS errors
        ,{"NB500"           , "NetBIOS write failure" }
        ,{"NB501"           , "NetBIOS read failure" }
        ,{"NB502"           , "Retarget response not yes implemented" }
        ,{"NB503"           , "Invalid NetBIOS response type: {0}" }
        ,{"NB504"           , "NetBIOS session is closed" }



        // Name
        ,{"MN1"             , "Invalid UNC or URL name syntax: {0}" }
        ,{"WWW1"             , "Cannot read from CifsURLConnection if doInput=false - call setDoInput(true)" }
        ,{"WWW2"             , "Cannot write to a CifsURLConnection if doOutput=false - call setDoOutput(true)" }
        ,{"WWW3"             , "Cannot write output after reading input" }




        /*------------------------ SMB error codes -----------------------*/

        ,{"U:U"             , "Unexpected SMB error code."}
        // DOS errors
        ,{"1:1"             , "Invalid function.  The server did not recognize or could not perform a system call generated by the server"}
        ,{"1:2"             , "File not found"}
        ,{"1:3"             , "Directory invalid"}
        ,{"1:4"             , "Too many open files.  The server has no file handles available."}
        ,{"1:5"             , "Access denied, the client's context does not permit the requested function."}
        ,{"1:6"             , "Invalid file handle.  The file handle specified was not recognized by the server."}
        ,{"1:7"             , "Memory control blocks destroyed."}
        ,{"1:8"             , "Insufficient server memory to perform the requested function."}
        ,{"1:9"             , "Invalid memory block address."}
        ,{"1:10"            , "Invalid environment."}
        ,{"1:11"            , "Invalid format."}
        ,{"1:12"            , "Invalid open mode."}
        ,{"1:13"            , "Invalid data "}
        ,{"1:15"            , "Invalid drive specified."}
        ,{"1:16"            , "A Delete Directory request attempted to remove the server's current directory."}
        ,{"1:17"            , "Not same device"}
        ,{"1:18"            , "A File Search command can find no more files matching the specified criteria."}
        ,{"1:32"            , "The sharing mode specified for an Open conflicts with existing FIDs on the file."}
        ,{"1:33"            , "A Lock request conflicted with an existing lock or specified an invalid mode"}
        ,{"1:50"            , "Operation not supported on this server"}
        ,{"1:80"            , "The file named in a Create Directory, Make New File or Link request already exists."}
        ,{"1:230"           , "Pipe invalid."}
        ,{"1:231"           , "All instances of the requested pipe are busy."}
        ,{"1:232"           , "Pipe close in progress."}
        ,{"1:233"           , "No process on other end of pipe."}
        ,{"1:234"           , "There is more data to be returned."}
        ,{"1:267"           , "Invalid directory name in a path"}
        // SRV errors
        ,{"2:1"             , "Non-specific error code."}
        ,{"2:2"             , "Bad password"}
        ,{"2:4"             , "The client does not have the necessary access rights within the specified context"}
        ,{"2:5"             , "The Tid specified in a command was invalid."}
        ,{"2:6"             , "Invalid network name in tree connect."}
        ,{"2:7"             , "Invalid device - printer request made to non-printer connection or non-printer request made to printer connection."}

        ,{"2:49"            , "Print queue full (files) -- returned by open print file."}
        ,{"2:50"            , "Print queue full -- no space."}
        ,{"2:51"            , "EOF on print queue dump."}
        ,{"2:52"            , "Invalid print file FID."}
        ,{"2:64"            , "The server did not recognize the command received."}
        ,{"2:65"            , "The server encountered an internal error, e.g., system file unavailable."}
        ,{"2:67"            , "The Fid and pathname parameters contained an invalid combination of values."}
        ,{"2:69"            , "The access permissions specified for a file or directory are not a valid combination."}
        ,{"2:71"            , "The attribute mode in the Set File Attribute request is invalid."}
        ,{"2:81"            , "Server is paused.  (reserved for messaging)"}
        ,{"2:82"            , "Not receiving messages.  (reserved for messaging)."}
        ,{"2:83"            , "No room to buffer message.  (reserved for messaging)."}
        ,{"2:87"            , "Too many remote user names.  (reserved for messaging)."}
        ,{"2:88"            , "Operation timed out."}
        ,{"2:89"            , "No resources currently available for request."}
        ,{"2:90"            , "Too many Uids active on this session."}
        ,{"2:91"            , "The Uid is not known as a valid user identifier on this session."}
        ,{"2:250"           , "Temporarily unable to support Raw, use MPX mode."}
        ,{"2:251"           , "Temporarily unable to support Raw, use standard read/write."}
        ,{"2:252"           , "Continue in MPX mode."}
        ,{"2:65535"         , "Function not supported."}

        // HW  errors
        ,{"3:19"            , "Attempt to write on write-protected media"}
        ,{"3:20"            , "Unknown unit."}
        ,{"3:21"            , "Drive not ready."}
        ,{"3:22"            , "Unknown command."}
        ,{"3:23"            , "Data error (CRC)."}
        ,{"3:24"            , "Bad request structure length."}
        ,{"3:25"            , "Seek error."}
        ,{"3:26"            , "Unknown media type."}
        ,{"3:27"            , "Sector not found."}
        ,{"3:28"            , "Printer out of paper."}
        ,{"3:29"            , "Write fault."}
        ,{"3:30"            , "Read fault."}
        ,{"3:31"            , "General failure."}
        ,{"3:32"            , "A open conflicts with an existing open."}
        ,{"3:33"            , "A Lock request conflicted with an existing lock or specified an invalid mode,"}
        ,{"3:34"            , "The wrong disk was found in a drive."}
        ,{"3:35"            , "No FCBs are available to process request."}
        ,{"3:36"            , "A sharing buffer has been exceeded."}

        // CMD error
        ,{"255:0"           , "Command was not in the SMB format"}


        // Protocol error
        ,{"PE1"             , "Server does support requested protocols"}
        ,{"PE2"             , "Only NT LM 0.12 protocol implemented. {0} not supported"}
        ,{"PE3"             , "Invalid server response"}


        // Session
        ,{"SS1"             , "Session {0} already exists"}
        ,{"SS2"             , "Session {0} does not exist"}

        // CifsFile*Stream
        ,{"FS0"             , "Resetting to invalid mark"}
        ,{"FS1"             , "File is closed" }
		,{"FS2"             , "Mode must be r or rw" }



        // RemoteAdmin

        ,{"RA2"             , "RemoteAdmin {0} does not exist"}

        // Disk

        ,{"DK2"             , "Disk {0} does not exist"}
        ,{"DK3"             , "Invalid disk name syntax: {0} "}
        ,{"DK4"             , "File/Directory  {0} already exists"}
        ,{"DK5"             , "File {0} does not exist"}
        ,{"DK6"             , "Directory {0} does not exist"}

        // LM Error codes
        ,{"LM5"             , "User has unsufficient privilege"}
        ,{"LM50"            , "The network request is not supported"}

        ,{"LM65"            , "Network access is denied"}
        ,{"LM86"            , "Invalid password"}
        ,{"LM2103"          , "The server could not be located"}
        ,{"LM2114"          , "The server service on the remote computer is not running"}
        ,{"LM2141"          , "IPC$ is not shared"}
        ,{"LM2150"          , "The specified queue name is invalid"}
        ,{"LM2151"          , "The specified print jon could not be located"}
        ,{"LM2160"          , "The print process is not responding"}
        ,{"LM2161"          , "The spooler is not started on the remote server"}
        ,{"LM2164"          , "Operation cannot be performed on the job"}
        ,{"LM2123"          , "Buffer was too small"}
        ,{"LM2221"          , "The user name was not found"}
        ,{"LM2243"          , "The password cannot be changed"}
        ,{"LM2246"          , "The password ist too short"}


        ,{"LMERROR"         , "Server error ({0})"}


    };

    public Object[][] getContents() {
        return fContent;
    }

}