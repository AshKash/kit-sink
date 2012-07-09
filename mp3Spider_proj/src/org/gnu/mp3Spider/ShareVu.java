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
import java.io.*;
import java.util.*;

/* jcifs info: 
 * Author: Norbert Hranitzky
 * Email : norbert.hranitzky@mchp.siemens.de
 * Web   : http://www.hranitzky.purespace.de
 * Please note that I have made some changes to his code:
 * grep for 3000 and then replace all with 15000
 * this will increase buffer size when listing machines.  */
import org.gnu.jcifs.*;
import org.gnu.jcifs.util.*;
import java.lang.*;

/** This class is the actual thing that lists the contents of an SMB share
 * recursively.
 * @version	0.1 16 Nov 2000
 * @serial	The initial release.
 * @since	0.1
 * @author	Ashwin S. Kashyap &lt;ashwink@usa.net&gt; */
class ShareVu implements Runnable {
	
	/** Thread that does all the dirty work */
	Thread t;
	/** The machine name to which we are connected */
	String mach;
	/** Stream representing the file to write the output */
	FileWriter fwrite;
	/** CifsLogin object that holds login info */
	CifsLogin login;

	/** Constructor
	 * @param mach Machine name to connect.
	 * @param login Object representing the login info
	 * @param fwrite File to write results */
	ShareVu(String mach, CifsLogin login, FileWriter fwrite) {
		// Create a new thread
		this.mach = mach;
		this.fwrite = fwrite;
		this.login = login;
		t = new Thread(this, mach);
		t.start();
	}

	/** Entry point for the thread */ 
	public void run() {
		CifsRemoteAdmin admin = null;
		CifsShareInfo share[] = null;

		try {
			// Connect
			admin = CifsSessionManager.connectRemoteAdmin(
				"mp3Spider_" + mach, mach, login);

			// Get a list of shares on the machine
			share = admin.listSharesInfo(true);
		} catch (CifsIOException e) {
			System.err.println("Prob on "+mach+": "+e);
			//e.printStackTrace();
			return;
		} catch (IOException io) {
			System.err.println("Fatal IO: " + io);
			return;
		}

		if (share == null) {
			return;
		}
		
		// For each share call recList()
		for (int i =0; i < share.length; i++) {
			SmbLs mp3List = null;;
			CifsDisk disk = null;

			// Skip shares that end with $
			if (share[i].getUNC().endsWith("$")) continue;

			// Open a session to the share
			try {
				mp3List = new SmbLs(".mp3");

				// Create Disk object
				disk = CifsSessionManager.connectDisk(
					this.t.getName(),
					share[i].getUNC(), login);
				if (disk == null) {
					continue;
				}

				// Do a recursive list
				mp3List.recList(disk, "\\");
				disk.disconnect();

			} catch (CifsIOException e) {
				System.err.println("eeeks! for " +
					share[i].getUNC() + ": " + e);
				if (disk != null) disk.disconnect();
						
			} catch (IOException io) {
				System.err.println("Serious: " + io);
				if (disk != null) disk.disconnect();
			}

			// Write result to file (must be atomic)
			synchronized(fwrite) {
				for (int j = 0; j<mp3List.list.size(); j++) {
					
					//System.out.println("url: " +
					//	mp3List.list.elementAt(j));

					try{
						fwrite.write((String)mp3List.list.elementAt(j) + "\n");
					} catch (IOException e) {
						System.err.println("File write: " + e);
						System.exit(1);
					}
				}

				try {
					fwrite.flush();
				} catch (IOException e) { 
					System.err.println("File flush: " + e);
				}
			}
		}
	}
}
