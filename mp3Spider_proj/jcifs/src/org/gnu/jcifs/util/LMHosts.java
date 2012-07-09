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


package org.gnu.jcifs.util;

import java.io.*;
import java.util.*;
import java.net.*;

public  class LMHosts {

    private Hashtable fHostTable = new Hashtable();
    private String    fLMHostFile = null;

    public LMHosts(String urlstring) throws IOException {
        if (urlstring != null)
            load(urlstring);
        fLMHostFile = urlstring;
    }

    public void refresh() throws IOException {
        if (fLMHostFile != null)
            load(fLMHostFile);
    }

    private final void load(String urlstring) throws IOException {

        Reader reader;
        int p = urlstring.indexOf("://");
        if (p > 0) {
            URL url = new URL(urlstring);
            reader  = new InputStreamReader(url.openStream());
        } else {
            reader  = new FileReader(urlstring);
        }

        BufferedReader in = new BufferedReader(reader);

        String line;
        String ip;
        String name;

        while ( (line = in.readLine()) != null) {

            if (line.length() == 0 || line.charAt(0) == '#')
                continue;

            // parse line
            p = line.indexOf(' ');
            if (p <= 0)
                continue;

            ip = line.substring(0,p);

            int q = line.indexOf('#',p+1);

            if (q < 0) {
                name = line.substring(p+1).trim();
            } else {
                name = line.substring(p+1,q).trim();
            }

            if (name.startsWith("\"")) {
                if (name.endsWith("\"")) {
                    name = name.substring(1,name.length() - 1);

                } else
                    continue;
            }

            fHostTable.put(name.toUpperCase(),ip);

        }

        try {
            reader.close();
        } catch(IOException e) {
        }
    }

    public String lookup(String netbios) {
        return (String)fHostTable.get(netbios.toUpperCase());
    }
}