/*
 * mp3Spider - Search for mp3 files in SMB shares on a network.
 * Copyright (C) 2000 Ashwin S. Kashyap
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 * Author: Ashwin S Kashyap
 * Email : ashwink@usa.net
 * Web   : http://ashwinS.tripod.com/mp3Spider
 */

package org.gnu.mp3Spider;

import java.lang.*;
import java.util.*;
import java.io.*;

/* jcifs info:
 * Author: Norbert Hranitzky
 * Email : norbert.hranitzky@mchp.siemens.de
 * Web   : http://www.hranitzky.purespace.de
 * Please note that I have made some changes to his code:
 * grep for 3000 and then replace all with 15000
 * this will increase buffer size when listing machines.  */
import org.gnu.jcifs.*;
import org.gnu.jcifs.util.*;

/** This class lists the contents of a share recursively.
 * @version	0.1 16 Nov 2000
 * @serial	The initial release.
 * @since	0.1
 * @author	Ashwin S. Kashyap &lt;ashwink@usa.net&gt; */
class SmbLs {

	/** The Vector that holds all the results. Each element contains
	 * the fully qualified pathname of the requested file. */
	Vector list;
	
	/** Search for these extensions only. */
	String onlyExt;

	/** Constructor
	 * @param onlyExt Will only search for files ending with these
	 * extensions.  */
	SmbLs (String onlyExt) {
		this.onlyExt = onlyExt;
		list = new Vector();
	}

	/** This will add all requested files into the Vector.
	 * @param disk see Cifs documentation
	 * @param path the path from where to search. This will be of the
	 * form //Machine/share. All requested files will be listed under this
	 * share.  */
	void recList(CifsDisk disk, String path) throws IOException{
		CifsFileInfo wantFile[] = null;
		CifsFileInfo dir[] = null;
		CifsFileInfo files[] = null;
		int wantFile_length = 0, dir_length = 0;


/*		try {
			wantFile = disk.listFilesInfo(path + "*." + onlyExt,
					CifsFileInfo.FILE_ATTR_ALL, true);
			if (wantFile == null) wantFile_length = 0;
			else wantFile_length = wantFile.length;
			System.out.println("Files List size: " + wantFile_length);
		} catch (CifsIOException e) {wantFile_length = 0; }
*/

		try {
			dir = disk.listFilesInfo(path + "*",
					CifsFileInfo.FILE_ATTR_ALL, true);
			if (dir == null) dir_length = 0;
			else dir_length = dir.length;
			//System.out.println("Dir size: " + dir_length + " path: " + path);

			// Concatenate the two arrays;
			files = new CifsFileInfo[wantFile_length + dir_length];
			int i =0;
//			for (; i<wantFile_length; i++) files[i] = wantFile[i];
			for (int j = 0; j<dir_length; j++) files[j+i] = dir[j];


		} catch (CifsIOException e) {
			int errcode = e.getSMBErrorCode();
			if (errcode == CifsIOException.SRV_NO_ACCESS
				|| errcode == CifsIOException.DOS_NO_ACCESS
				|| errcode == CifsIOException.HW_NOT_READY
			//	|| errcode == CifsIOException.HW_DATA_ERROR
				|| errcode == CifsIOException.DOS_BAD_FILE
			//	|| errcode == CifsIOException.DOS_BAD_PATH
			 ) 
				return;
			System.err.println("Error trying to list: " 
				+ disk.getShareName() + path 
				+ ": " + e);
			return;
		}

		if (files == null) return;

		// get recursive directory listing
		for(int i = 0; i < files.length; i++) {
			try {
				if(files[i].isDirectory()) {
					if(files[i].getFileName().equals(".")
						 || files[i].getFileName().equals("..")) {
						continue;
					}
					//System.out.println("dir: " + path +
					//	files[i].getFileName() + "\\" );
					recList(disk, path + files[i].getFileName()
						 + "\\");
				} else {
					if (files[i].getPathName().toLowerCase()
							.endsWith(onlyExt)) {
					//	System.out.println("file: " + files[i].getPathName());
						list.add((Object)(disk.getShareName() +
						  files[i].getPathName()));
					}
				}
			} catch (CifsIOException e) {
				System.err.println("Error trying to list: " + e);
				e.printStackTrace();
				return;
			}
		}
	}


	/** This is mainly used as a stand alone tool to list all contents
	 * of a given share.
	 * @param args[0] Machine name
	 * @param args[1] Username to log in to the domain.
	 * @param args[2] Password to use.
	 * @param args[3] Optional ending extension to search for. Default
	 * is mp3. */
	public static void main(String args[]) throws IOException{
		// Change these things for your needs
		CifsRemoteAdmin admin = null;
		CifsShareInfo share[] = null;
		CifsServerInfo server[] = null;

		String mach = null;
		String user = null;
		String pass = null;
		String onlyExt = null;

		try {
			mach = args[0];
			user = args[1];
			pass = args[2];
		} catch (ArrayIndexOutOfBoundsException e) {
			System.err.println("Usage:");
			System.err.println("java SmbLs <Machine> <Username> <Password>" +
					"[<Extension, \"mp3\" default>]");
			System.exit(1);
		}

		if (args.length == 4)
			onlyExt = args[3];
		else onlyExt = "mp3";

		// Open a session to given m/c
		CifsLogin login = new CifsLogin(user, pass);
		try {
			admin = CifsSessionManager.connectRemoteAdmin(
				"SessionName", mach, login);
		} catch (CifsIOException e) {
			System.err.println("Error: " + e);
			return;
		}

		// Get a list of shares
		// Note that there is a bug in Cifs, that will not
		// list any share that is longer than 12 characters. This
		// is very similar to the way smbclient behaves.
		// Please mail me if you know a fix.
		try {
			share = admin.listSharesInfo(true);
		} catch (CifsIOException e) {
			System.err.println("Error: " + e);
			return;
		}

		// Get files in each share
		for (int i =0; i<share.length; i++) {
			Vector list = new Vector();
			CifsDisk disk = null;
			SmbLs mp3List = new SmbLs(onlyExt);

			// Skip shares that end with $
			if (share[i].getUNC().endsWith("$")) continue;
			//System.out.println(share[i].getUNC());
			
			try {

				// Create Disk object
				disk = CifsSessionManager.connectDisk("Session",
					share[i].getUNC(), login);

				// Do a recursive list
				mp3List.recList(disk, "\\");
				disk.disconnect();

			} catch (CifsIOException e) {
				System.err.println("eeeks! " + e);
				if (disk != null) disk.disconnect();
				continue;
			}
			System.out.println("IN: " + share[i].getUNC()
				+ " Found " + mp3List.list.size());

			// Print the files
			for (int j =0; j<mp3List.list.size();j++) {
				System.out.println(mp3List.list.elementAt(j));
			}
		}
	}

}

