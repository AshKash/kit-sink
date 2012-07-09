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
import java.awt.*;
import java.awt.event.*;
import java.util.*;


public class PasswordDialog extends java.awt.Dialog implements ActionListener {

    public static class PasswordData {

        String user;
        String oldpwd;
        String newpwd;
    }

    private java.awt.Label fLblUser;
    private java.awt.TextField fTxtUser;
    private java.awt.Label fLblOldPwd;
    private java.awt.TextField fTxtOldPwd;
    private java.awt.Label fLblNewPwd1;
    private java.awt.TextField fTxtNewPwd1;
    private java.awt.Label fLblNewPwd2;
    private java.awt.TextField fTxtNewPwd2;
    private java.awt.Button fBtnCancel;
    private java.awt.Button fBtnOk;
    private java.awt.Label fLblServer;

    private boolean fCancelled = false;

    /** Initializes the Form */
    public PasswordDialog(java.awt.Frame parent) {
        super (parent, true);
        setTitle("Change password");
        setResizable(false);
        initComponents ();
        pack ();
        Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
        Dimension mySize = getPreferredSize();
	    this.setLocation(screenSize.width/2 - mySize.width/2,
			         screenSize.height/2 - mySize.height/2);
    }


    void initComponents () {
        addWindowListener (new java.awt.event.WindowAdapter () {
            public void windowClosing (java.awt.event.WindowEvent evt) {
                closeDialog (evt);
            }
        }
        );

        Insets insets = new Insets(8, 8, 0, 8);
        setLayout (new java.awt.GridBagLayout ());
        java.awt.GridBagConstraints gridBagConstraints2;

        fLblUser = new java.awt.Label ();
        fLblUser.setText ("User");
        gridBagConstraints2 = new java.awt.GridBagConstraints ();
        gridBagConstraints2.gridx = 0;
        gridBagConstraints2.gridy = 1;
        gridBagConstraints2.insets = insets;
        gridBagConstraints2.anchor = java.awt.GridBagConstraints.WEST;
        add (fLblUser, gridBagConstraints2);

        fTxtUser = new java.awt.TextField ();
        fTxtUser.setColumns (12);

        gridBagConstraints2 = new java.awt.GridBagConstraints ();
        gridBagConstraints2.gridx = 1;
        gridBagConstraints2.gridy = 1;
        gridBagConstraints2.insets = insets;
        gridBagConstraints2.anchor = java.awt.GridBagConstraints.WEST;
        add (fTxtUser, gridBagConstraints2);

        fLblOldPwd = new java.awt.Label ();
        fLblOldPwd.setText ("Old password");

        gridBagConstraints2 = new java.awt.GridBagConstraints ();
        gridBagConstraints2.gridx = 0;
        gridBagConstraints2.insets = insets;
        gridBagConstraints2.anchor = java.awt.GridBagConstraints.WEST;
        add (fLblOldPwd, gridBagConstraints2);

        fTxtOldPwd = new java.awt.TextField ();
        fTxtOldPwd.setText ("");
        fTxtOldPwd.setColumns (12);
        fTxtOldPwd.setEchoChar('*');

        gridBagConstraints2 = new java.awt.GridBagConstraints ();
        gridBagConstraints2.gridx = 1;
        gridBagConstraints2.gridy = 2;
        gridBagConstraints2.insets = insets;
        gridBagConstraints2.anchor = java.awt.GridBagConstraints.WEST;
        add (fTxtOldPwd, gridBagConstraints2);

        fLblNewPwd1 = new java.awt.Label ();
        fLblNewPwd1.setText ("New password");
        fLblNewPwd1.setName ("fLblNewPwd");
        gridBagConstraints2 = new java.awt.GridBagConstraints ();
        gridBagConstraints2.gridx = 0;
        gridBagConstraints2.insets = insets;
        add (fLblNewPwd1, gridBagConstraints2);

        fTxtNewPwd1 = new java.awt.TextField ();
        fTxtNewPwd1.setText ("");
        fTxtNewPwd1.setColumns (12);
        fTxtNewPwd1.setEchoChar('*');


        gridBagConstraints2 = new java.awt.GridBagConstraints ();
        gridBagConstraints2.gridx = 1;
        gridBagConstraints2.gridy = 3;
        gridBagConstraints2.insets = insets;
        gridBagConstraints2.anchor = java.awt.GridBagConstraints.WEST;
        add (fTxtNewPwd1, gridBagConstraints2);

        fLblNewPwd2 = new java.awt.Label ();
        fLblNewPwd2.setText ("New password");
        fLblNewPwd2.setName ("fLblNewPwdR");
        gridBagConstraints2 = new java.awt.GridBagConstraints ();
        gridBagConstraints2.gridx = 0;
        gridBagConstraints2.insets = insets;
        add (fLblNewPwd2, gridBagConstraints2);

        fTxtNewPwd2 = new java.awt.TextField ();
        fTxtNewPwd2.setText ("");
        fTxtNewPwd2.setColumns (12);

        fTxtNewPwd2.setEchoChar('*');

        gridBagConstraints2 = new java.awt.GridBagConstraints ();
        gridBagConstraints2.gridx = 1;
        gridBagConstraints2.gridy = 4;
        gridBagConstraints2.insets = insets;
        gridBagConstraints2.anchor = java.awt.GridBagConstraints.WEST;
        add (fTxtNewPwd2, gridBagConstraints2);

        fBtnCancel = new java.awt.Button ();
        fBtnCancel.setLabel ("Cancel");
        fBtnCancel.setActionCommand ("cancel");
        fBtnCancel.addActionListener (this);

        gridBagConstraints2 = new java.awt.GridBagConstraints ();
        gridBagConstraints2.gridx = 0;
        gridBagConstraints2.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints2.insets = new Insets(8, 8, 8, 8);
        add (fBtnCancel, gridBagConstraints2);

        fBtnOk = new java.awt.Button ();
        fBtnOk.setLabel ("Ok");
        fBtnOk.setActionCommand ("ok");
        fBtnOk.addActionListener (this);

        gridBagConstraints2 = new java.awt.GridBagConstraints ();
        gridBagConstraints2.gridx = 1;
        gridBagConstraints2.gridy = 5;
        gridBagConstraints2.fill = java.awt.GridBagConstraints.HORIZONTAL;
        gridBagConstraints2.insets = new Insets(8, 8, 8, 8);
        add (fBtnOk, gridBagConstraints2);

        fLblServer = new java.awt.Label ();
        fLblServer.setText ("                    ");

        gridBagConstraints2 = new java.awt.GridBagConstraints ();
        gridBagConstraints2.gridx = 0;
        gridBagConstraints2.gridy = 0;
        gridBagConstraints2.gridwidth = 2;
        add (fLblServer, gridBagConstraints2);

    }

    public void actionPerformed(ActionEvent ev) {
        String name = ev.getActionCommand();
        if (name.equals("ok"))
            doOkAction(ev);
        else if (name.equals("cancel"))
            doCancelAction(ev);

    }

    private void doCancelAction (java.awt.event.ActionEvent evt) {
        fCancelled = true;
        hide();

    }



    private void doOkAction (java.awt.event.ActionEvent evt) {

        String val = fTxtUser.getText();

        if (val == null || val.length() == 0) {
            fTxtUser.requestFocus();
            return;
        }

        val = fTxtOldPwd.getText();
        if (val == null || val.length() == 0) {
            fTxtOldPwd.requestFocus();
            return;
        }

        String np1 = fTxtNewPwd1.getText();
        if (np1 == null || np1.length() == 0) {
            fTxtNewPwd1.requestFocus();
            return;
        }
        String np2 = fTxtNewPwd2.getText();
        if (np2 == null || np2.length() == 0) {
            fTxtNewPwd2.requestFocus();
            return;
        }
        if (!np1.equals(np2)) {
            fTxtNewPwd2.requestFocus();
            return;

        }

        hide();

    }

    /** Closes the dialog */
    void closeDialog(java.awt.event.WindowEvent evt) {
        fCancelled = true;

        setVisible (false);
        dispose ();
    }

    public boolean prompt(String server,PasswordData data) {

        fCancelled = false;
        fLblServer.setText (server);

        if (data.user != null) {
            fTxtUser.setText(data.user);
            fTxtOldPwd.requestFocus();
        } else
            fTxtUser.requestFocus();

        show();

        if (!fCancelled) {
            data.user   = fTxtUser.getText();
            data.oldpwd = fTxtOldPwd.getText();
            data.newpwd = fTxtNewPwd1.getText();
        }


        return !fCancelled;
    }


}
