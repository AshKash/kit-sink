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

/** This program searches SMB shares of all available machines in the given
 * domain recursively, for files ending with .mp3. The results will
 * be written to the given file or to mp3Spider.out if none is specified.
 * The program is based on Norbert Hranitzky's jCifs client (which is
 * quite diferent from jCifs seen at jcifs.samba.org).<BR>
 * KNOWN ISSUES:<BR>
 * Will not enumerate more than 500 machines.<BR>
 * Due to obscure reasons, jCifs will not enumerate shares whose names
 * are longer than 12 characters (This behaviour is also seen in
 * smbclient).<BR>
 * Program creates one thread per machine, this can bog down your
 * machine!
 * JDK1.3 is needed for this to work.
 * @version	0.1 16 Nov 2000
 * @serial	The initial release.
 * @since	0.1
 * @author	Ashwin S. Kashyap &lt;ashwink@usa.net&gt;
 * @see <a href="http://www.hranitzky.purespace.de/jcifs/jcifs.htm">jCifs Client</a>
 * @see <a href="http://jcifs.samba.org>Java Cifs implementation</a>
 */

public class mp3Spider {

	/** Entry point.
	 * @param args[0] Domain controller name.
	 * @param args[1] Domain user name.
	 * @param args[2] Domain password. 
	 * @param args[3] Optional file name. Default is mp3Spider.out */
	public static void main(String args[]) throws IOException,
					InterruptedException {

		String domCtrl = null;
		String domName = null;
		String domUser = null;
		String domPass = null;
		// File to write to
		FileWriter fwrite = null;

		try {
			domCtrl = args[0];
			domName = args[1];
			domUser = args[2];
			domPass = args[3];
		} catch (ArrayIndexOutOfBoundsException e) {
			System.err.println("Usage:");
			System.err.println("java mp3Spider <Domain controller> " +
				"<Domain Name> <Username> <Password> [<Out file>]");
			System.exit(1);
		}

		if (args.length == 5)		
			fwrite = new FileWriter(args[5]);
		else
			fwrite = new FileWriter("mp3Spider.out");

		// Get list of machines on the domain
		CifsLogin login = new CifsLogin(domUser, domPass);
		CifsRemoteAdmin admin = null;
		CifsServerInfo mach[] = null;
		try {
			admin = CifsSessionManager.connectRemoteAdmin(
				"Mach_list_all", domCtrl, login);

			mach = admin.listServersInfo(domName,
				CifsServerInfo.SV_TYPE_ALL);
			admin.disconnect();
		} catch (CifsIOException e) {
			System.err.println("Session failed: " + e);
			e.printStackTrace();
			System.exit(1);
		}

		// Create a thread and start shareVu() for every machine
		ShareVu t[] = new ShareVu[mach.length];
		for (int i=0; i < mach.length; i++) {
			System.out.println("Mach: " + mach[i].getComputerName());
			// Create the thread
			t[i] = new ShareVu(mach[i].getComputerName(),
							login, fwrite);
		}

		// Wait for all threads to finito

		for(int i=0; i < mach.length; i++) {
			try {t[i].t.join();}
			catch (InterruptedException e) {
				System.out.println("Join error " + e);
			}
		}
	}
}
