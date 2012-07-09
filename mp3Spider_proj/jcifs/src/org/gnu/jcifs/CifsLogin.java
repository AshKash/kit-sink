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
  *  Changes:
  *     1999.08.20 : - LM Authentication was wrong, adapt code from Samba
  *     1999.08.28 : - make class cloneable
  */


package org.gnu.jcifs;

import org.gnu.jcifs.util.*;
import java.io.*;



/**
 * CifsLogon holds user authentication data<p>
 *
 * @version 1.1, 22 July 1999
 * @author Norbert Hranitzky
 * @since  1.0
 *  
 */

public class CifsLogin implements Cloneable {


    private String fAccount  = null;
    private String fPassword = null;


    /**
     * Creates a new object. The user name is the name in the
     * system property <code>user.name</code>
     */
    public CifsLogin() {
        setAccount(System.getProperty("user.name"));
    }

    /**
     * Creates a new object. The user name is the name in the
     * system property <code>user.name</code>
     * @param password user password
     */
    public CifsLogin(String password) {
        setAccount(System.getProperty("user.name"));
        setPassword(password);
    }

    /**
     * Creates a new object
     * @param account user name
     * @param password user password
     */
    public CifsLogin(String account, String password) {
        setAccount(account);
        setPassword(password);
    }

    /**
     * Sets the account name
     * @param account user name
     */
    public void setAccount(String account) {
        fAccount = account;
    }

    /**
     * Sets the password
     * @param password password
     */

    public void setPassword(String password) {
        fPassword = password;
    }

    /**
     * Returns the account name
     * @return account name
     */

    public String getAccount() {
        return fAccount;
    }


    /**
     * Returns the password
     * @return password
     */
    String getPassword() {
        return fPassword;
    }

    /**
     * Compares if the two object are the same (same account and password)
     * @param obj the object to test
     * @return true of obj is the same <code>CifsLogin</code> object
     */
    public boolean equals(Object obj) {

        if ((obj != null) && (obj instanceof CifsLogin)) {

            CifsLogin anobj = (CifsLogin)obj;

            if (!anobj.fAccount.equals(fAccount))
                return false;

            if (anobj.fPassword == null && fPassword == null)
                return true;

            if (anobj.fPassword != null || fPassword != null)
                return false;
            return (anobj.fPassword.equals(fPassword));

        }
        return false;
    }

    /**
     * Clone this object
     */
    public synchronized Object clone() {

	    CifsLogin newlogin = new CifsLogin();

	    if (fAccount != null)
	        newlogin.fAccount  = new String(fAccount);
	    if (fPassword != null)
	        newlogin.fPassword = new String(fPassword);

	    return newlogin;
    }

    public String toString() {
        return "[Login] Account=" + getAccount();
    }

    /*====================================================================
     *                    Helper methods for authentication
     *===================================================================*/

    private final static byte[] NONE_PASSWORD = new byte[0];
    private final static byte[] NULL_PASSWORD = {(byte)0};

    // "KGS!@#$%"
    private final static byte[] S8 = { (byte)0x4b, (byte)0x47, (byte)0x53, (byte)0x21,
                                       (byte)0x40, (byte)0x23, (byte)0x24, (byte)0x25};


    /**
     * Makes NT authentication response data
     * @param password password
     * @param c8 challange
     * @return byte[24] authentication data
     */
    static byte[] getNTAuthData(String password,byte[] c8 ) {

        if (password == null)
            return NONE_PASSWORD;

        byte[] s21 = getNTSessionKey(password);

        return getAuthData(s21,c8);

    }

    /**
     * Makes LM authentication response data
     * @param password password
     * @param c8 challange
     * @return byte[24] authentication data
     */
    static byte[] getLMAuthData(String password,byte[] c8 ) {

        if (password == null)
            return NONE_PASSWORD;
			
		byte[] p14 = new byte[15];
		byte[] p21 = new byte[21];
		byte[] p24 = new byte[24];


		password = password.toUpperCase();
        for(int i=0;i<password.length() && i<14;i++)
            p14[i] = (byte)(password.charAt(i) & 0xff);
            
        E_P16(p14,p21);
        
        SMBOWFencrypt(p21,c8,p24);

		return p24;
    }

    /**
     * Encrypts password
     * @param s21 Session key
     * @param c8 challange
     * @return byte[24]
     */
    static byte[] getAuthData(byte[] s21,byte[] c8 ) {

        byte[] key7 = new byte[7];
        byte[] key8 = new byte[8];
        byte[] e8   = new byte[8];
        byte[] rn   = new byte[24];

        for(int i=0;i<3;i++) {
            System.arraycopy(s21,7*i,key7,0,7);
            DES.makeSMBKey(key7,key8);

            DES des = new DES(key8);

            des.encrypt(c8,e8);

            System.arraycopy(e8,0,rn,8*i,8);
        }
        return rn;


    }
    /**
     * S16 = MD4(U(PN))
     * S21 = concat(S16,zeros(5))
     */
    static byte[] getNTSessionKey(String password) {

        byte[] pn = getPasswordBytesUnicode(password);

        MD4 md4 = new MD4();

        md4.update(pn);

        byte[] dig = md4.digest();

        byte[] s21 = new byte[21];

        System.arraycopy(dig,0,s21,0,dig.length);

        return s21;

    }

    
	public static void E_P16(byte[] p14, byte[] p16) {

		
        byte[] key7 = new byte[7];
        byte[] key8 = new byte[8];
        byte[] e8   = new byte[8];

        for(int i=0;i<2;i++) {
            System.arraycopy(p14,7*i,key7,0,7);
            DES.makeSMBKey(key7,key8);

            DES des = new DES(key8);

            des.encrypt(S8,e8);

            System.arraycopy(e8,0,p16,8*i,8);
        }

    }
    
	private static void E_P24(byte[] p21, byte[] c8, byte[] p24) {

		
        byte[] key7 = new byte[7];
        byte[] key8 = new byte[8];
        byte[] e8   = new byte[8];

        for(int i=0;i<3;i++) {
            System.arraycopy(p21,7*i,key7,0,7);
            DES.makeSMBKey(key7,key8);

            DES des = new DES(key8);

            des.encrypt(c8,e8);

            System.arraycopy(e8,0,p24,8*i,8);
        }

	}
	/* One Way Transformation. 
    */
	
	private static void SMBOWFencrypt(byte[] passwd16, byte[] c8, byte[] p24) {
				
		byte [] p21 = new byte[21];
				
		System.arraycopy(passwd16,0,p21,0,16);
				
		E_P24(p21,c8,p24);
			
	}


    /* One Way Transformation. The spec says that the password must be
     * padded with blanks, Samba does it with 0 ???
     * OWF = Ex(P14, S8)
     * return byte[16];
     */
    
    static byte[] getLMOWF(String password) {

        byte[] p14 = new byte[14];

        password = password.toUpperCase();
        for(int i=0;i<password.length();i++)
            p14[i] = (byte)(password.charAt(i) & 0xff);



        byte[] s16  = new byte[16];
        byte[] key7 = new byte[7];
        byte[] key8 = new byte[8];
        byte[] e8   = new byte[8];

        for(int i=0;i<2;i++) {
            System.arraycopy(p14,7*i,key7,0,7);
            DES.makeSMBKey(key7,key8);

            DES des = new DES(key8);

            des.encrypt(S8,e8);

            System.arraycopy(e8,0,s16,8*i,8);
        }


        return s16;

    }
    

    /**
     *   +----------------------+
     *   |                      |
     *   |   new password       + 512
     *   |   new password len   + 516
     *   |   encrypted hash     |
     *   +----------------------+
     */
    static byte[] getChangePasswordData(String oldpwd, String newpwd) {

        byte[] data = new byte[532];


        // calculate OWF of the old password
        byte[] oldpwdhash = CifsLogin.getLMOWF(oldpwd);

        //String p = Util.bytesToHex(oldpwdhash);
        //System.err.println(p);

        // setup new password structure
        byte[] plainnewpwd = Util.getStringBytes(newpwd);

        System.arraycopy(plainnewpwd,0,data,512-plainnewpwd.length,plainnewpwd.length);
        MarshalBuffer.setIntAt(512,data,plainnewpwd.length);

        // encrypt new password structure
        CifsLogin.SamOEMHash(data,oldpwdhash,1);


        // calculate OWF of the new password
        byte[] newpwdhash = CifsLogin.getLMOWF(newpwd);


        // now encrypt new password hash with the old password hash
        byte[] sig = CifsLogin.getOldPWHash(newpwdhash,oldpwdhash);
        System.arraycopy(sig,0,data,516,16);

        return data;
    }

    private static byte[] getOldPWHash(byte[] p14key,byte[] in16) {


        byte[] s16  = new byte[16];
        byte[] key7 = new byte[7];
        byte[] key8 = new byte[8];
        byte[] e8   = new byte[8];
        byte[] s8   = new byte[8];

        for(int i=0;i<2;i++) {
            System.arraycopy(p14key,7*i,key7,0,7);
            DES.makeSMBKey(key7,key8);

            DES des = new DES(key8);
            System.arraycopy(in16,8*i,s8,0,8);

            des.encrypt(s8,e8);

            System.arraycopy(e8,0,s16,8*i,8);
        }


        return s16;

    }

    /**
     * Returns the password as a byte array. Each Unicode character
     * is represented by the 2 bytes in Intel byte-order (little-endian)
     * @param password password
     * @return byte array
     */
    static byte[] getPasswordBytesUnicode(String password) {

        if (password == null)
            return NONE_PASSWORD;

        int len = password.length();

        if (len > 128)
            len = 128;

        byte[] p = new byte[2*len];

        for(int i=0;i<len;i++) {
            int c = password.charAt(i) & 0xffff;

            p[2*i]   = (byte)(c  & 0xff);
            p[2*i+1] = (byte)((c >> 8) & 0xff);
        }
        return p;
    }

    /**
     * Returns the password as byte array. The byte array
     * contains the Ascii characters and null terminated
     * @param password password
     * @return byte array
     */

    static byte[] getPasswordBytesAscii(String password) {

        if (password == null)
            return NULL_PASSWORD;

        return Util.getZTStringBytes(password.toUpperCase());
    }

    /**
     * Code from samba
     */

    static void SamOEMHash(byte[] data,  byte[] key, int val) {

        byte s_box[] = new byte[256];
        int index_i = 0;
        int index_j = 0;
        int j = 0;
        int ind;

        for (ind = 0; ind < 256; ind++)
            s_box[ind] = (byte)(ind & 0xff);


        for( ind = 0; ind < 256; ind++) {

            byte tc;

            j = ( j + (s_box[ind] + key[ind % 16])) & 0xff;
            tc         = s_box[ind];
            s_box[ind] = s_box[j];
            s_box[j]   = tc;
        }

        for( ind = 0; ind < (val != 0 ? 516 : 16); ind++)  {
            int tc;
            int t;

            index_i = (index_i + 1) & 0xff;
            index_j = ( index_j + s_box[index_i]) & 0xff;



            tc             = s_box[index_i] & 0xff;
            s_box[index_i] = s_box[index_j];
            s_box[index_j] = (byte)tc;

            t         = ((s_box[index_i] + s_box[index_j]) & 0xff);
            //System.err.println("index_i=" +index_i + ",index_j=" + index_j +",t=" +t);
            data[ind] = (byte)((data[ind] ^ s_box[t]) & 0xff);
        }
    }

}