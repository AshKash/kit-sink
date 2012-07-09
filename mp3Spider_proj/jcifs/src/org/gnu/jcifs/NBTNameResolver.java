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
import java.util.*;


/**
 * NetBIOS Name Resolver
 *
 * @author  Norbert Hranitzky
 * @version 1.1, 1999-07-21
 * @since   1.0
 * changes: 1.1 Wrong index use in resolve loop
 *              Wrong flag set in resolve string evaluation
 */
final class NBTNameResolver {

    private static LMHosts fLMHosts = null;

    private static Hashtable fCache = new Hashtable();
    private static NBTNameService   fWins     = null;
    private static InetAddress      fWinsAddr = null;

    private final static int RESOLVE_CACHE      = 0;
    private final static int RESOLVE_PROPERTIES = 1;
    private final static int RESOLVE_WINS       = 2;
    private final static int RESOLVE_LMHOSTS    = 3;
    private final static int RESOLVE_DNS        = 4;

    private static int[]   fResolveOrder = null;
    private static boolean fUseCache = true;

    private final static String DEFAULT_RESOLVE_ORDER = "dns,wins,props,lmhosts";
    private final static String[] KEYSTABLE = {"Cache", "Properties", "WINS", "LMHOSTS", "DNS" };


    private NBTNameResolver() { }


    static void clearCache() {
        fCache.clear();
        fLMHosts = null;
    }

    /**
     * Resolve NetBIOS name: <p>
     * - 1. check cache
     * - 2. System property
     * - 3. WINS
     * - 4. LMHosts
     * - 5. DNS
     */
    static InetAddress resolve(String netbiosname) throws IOException {
        InetAddress addr = null;
        String netbiosup = netbiosname.toUpperCase();


        int[] resolveOrder = getResolveOrder();
        int index = RESOLVE_CACHE;


        addr = resolveInCache(netbiosup);

		
        for(int i=0;i<resolveOrder.length && addr == null;i++) {
            index = resolveOrder[i];
           
            switch(index) {

                case RESOLVE_PROPERTIES:

                    addr = resolveInProperties(netbiosup);
                    break;
                case RESOLVE_WINS:

                    addr = resolveInWins(netbiosup);
                    break;
                case RESOLVE_LMHOSTS:

                    addr = resolveInLMHosts(netbiosup);
                    break;
                case RESOLVE_DNS:

                    addr = resolveInDNS(netbiosname);
                    break;
                default:
                    break;
            }
        }
        if (addr != null) {

            if (Debug.debugOn && Debug.debugLevel >= Debug.INFO) {

                Debug.println(Debug.INFO,"NetBIOS name found in " + KEYSTABLE[index] + ": " + netbiosname + "=" + addr.toString());
            }
            if (index != RESOLVE_CACHE && fUseCache)
                fCache.put(netbiosup,addr);

            return addr;
        }

        throw new CifsIOException("CM3",netbiosname);
    }


    private static InetAddress resolveInWins(String netbios) {

        if (fWins == null) {
            String wins = System.getProperty("org.gnu.jcifs.resolve.wins.server");
            if (wins == null)
                return null;

            if (Debug.debugOn && Debug.debugLevel >= Debug.INFO)
                Debug.println(Debug.INFO,"Check name in WINS");

            try {
                fWinsAddr = InetAddress.getByName(wins);
                fWins     = new NBTNameService();
            } catch(Exception e) {
                return null;
            }
        }
        return fWins.lookup(fWinsAddr,netbios);

    }
    private static InetAddress resolveInCache(String netbios) {

        if (!fUseCache)
            return null;

        if (Debug.debugOn && Debug.debugLevel >= Debug.INFO)
                Debug.println(Debug.INFO,"Check name in Cache");

        return (InetAddress)fCache.get(netbios);

    }

    private static InetAddress resolveInProperties(String netbios) {
        // 2. Check system property
        String name = System.getProperty("org.gnu.jcifs.resolve.name." + netbios);

        if (name != null) {
            if (Debug.debugOn && Debug.debugLevel >= Debug.INFO)
                Debug.println(Debug.INFO,"Check name in Properties");

            try {
                return InetAddress.getByName(name);

            } catch(Exception e) {
            }
        }
        return null;
    }

    private static InetAddress resolveInDNS(String netbios) {

        if (Debug.debugOn && Debug.debugLevel >= Debug.INFO)
                Debug.println(Debug.INFO,"Check name in DNS");

        try {
            return InetAddress.getByName(netbios);

        } catch(Exception e) {
        }

        return null;
    }

    private static InetAddress resolveInLMHosts(String netbios) {

        if (fLMHosts == null) {
            String url = null;
            try {
                if (Debug.debugOn && Debug.debugLevel >= Debug.INFO)
                    Debug.println(Debug.INFO,"Check name in LMHOSTS");

                url = System.getProperty("org.gnu.jcifs.resolve.lmhosts");
                fLMHosts = new LMHosts(url);
            } catch(IOException e) {
                Debug.println(Debug.WARNING,"LMHOSTS file cannot be read:" + url);
                return null;
            }
        }

        String ip = fLMHosts.lookup(netbios);
        if (ip != null) {

            try {
                return InetAddress.getByName(ip);

            } catch(Exception e) {
            }
        }
        return null;
    }

    private static int[] getResolveOrder() {
        if (fResolveOrder != null)
            return fResolveOrder;

        String use =  System.getProperty("org.gnu.jcifs.resolve.cache","true");
        fUseCache = use.equalsIgnoreCase("true");

        String order = System.getProperty("org.gnu.jcifs.resolve.order",DEFAULT_RESOLVE_ORDER);

        if (order == null)
            order = DEFAULT_RESOLVE_ORDER;

        StringTokenizer token = new StringTokenizer(order,",");

        boolean cacheSpec   = false;
        boolean propsSpec   = false;
        boolean lmhostsSpec = false;
        boolean dnsSpec     = false;
        boolean winsSpec    = false;

        fResolveOrder = new int[4];

        int i = 0;
        while(token.hasMoreTokens()) {
            String key = token.nextToken();

            if (key.equalsIgnoreCase("props")) {
	            if (!propsSpec) fResolveOrder[i++] = RESOLVE_PROPERTIES;
	            propsSpec = true;
            } else if (key.equalsIgnoreCase("lmhosts")) {
	            if (!lmhostsSpec) fResolveOrder[i++] = RESOLVE_LMHOSTS;
	            lmhostsSpec = true;
	        } else if (key.equalsIgnoreCase("dns")) {
	            if (!dnsSpec) fResolveOrder[i++] = RESOLVE_DNS;
	            dnsSpec = true;
	        } else if (key.equalsIgnoreCase("wins")) {
	            if (!winsSpec) fResolveOrder[i++] = RESOLVE_WINS;
	            winsSpec = true;
	        }
	    }
	    return fResolveOrder;
	}

}