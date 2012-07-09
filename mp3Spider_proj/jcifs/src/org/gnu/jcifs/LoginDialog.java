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
import java.awt.*;
import java.awt.event.*;
import java.util.ResourceBundle;


final class LoginDialog extends Dialog
    implements ActionListener, FocusListener, KeyListener, WindowListener {



    private Component fFocalComponent;
    private Button    fOkBtn;
    private Button    fCancelBtn;
    private TextField fPasswordTxt;
    //private TextField fHostTxt;
    private TextField fUserTxt;
    private Label     fShareName;


    public LoginDialog(Frame frame) {
        super(frame,  true);

        Dimension mySize = getPreferredSize();
        Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
	    this.setLocation(screenSize.width/2 - mySize.width/2,
			             screenSize.height/2 - mySize.height/2);

        setTitle(CifsRuntimeException.getMessage("DLG_TITLE"));
        setResizable(false);
        GridBagLayout gridbag_layout = new GridBagLayout();
        GridBagConstraints gridbag_constraints = new GridBagConstraints();
        gridbag_constraints.fill = 1;
        gridbag_constraints.insets = new Insets(8, 8, 0, 8);
        setLayout(gridbag_layout);
        setBackground(Color.lightGray);

        /*     0       1
         *  0  Host    HostTxt
         *  1  User    UserTxt
         *  2  Account AccountTxt
         *  3  Password PasswordTxt
         *  4  Panel
         */

        // Host
        fShareName = new Label("login", Label.CENTER);
        add(fShareName, gridbag_layout, gridbag_constraints, 0, 0, 2, 1);



        // User
        Label label = new Label(CifsRuntimeException.getMessage("DLG_ACCOUNT"), 0);
        add(label, gridbag_layout, gridbag_constraints, 0, 1, 1, 1);
        fUserTxt = new TextField(10);
        fUserTxt.addFocusListener(this);
        fUserTxt.addKeyListener(this);
        add(fUserTxt, gridbag_layout, gridbag_constraints, 1, 1, 1, 1);


        // Password
        label = new Label(CifsRuntimeException.getMessage("DLG_PASSWORD"), 0);
        add(label, gridbag_layout, gridbag_constraints, 0, 2, 1, 1);
        fPasswordTxt = new TextField(10);
        fPasswordTxt.setEchoChar('*');
        fPasswordTxt.addFocusListener(this);
        fPasswordTxt.addKeyListener(this);
        add(fPasswordTxt, gridbag_layout, gridbag_constraints, 1, 2, 1, 1);


        // Panel below
        Panel panel = new Panel();
        panel.setLayout(new FlowLayout(1));
        add(panel, gridbag_layout, gridbag_constraints, 0, 3, 2, 2);

        // Ok & Cancel
        Panel panel1 = new Panel();
        panel1.setLayout(new GridLayout(1, 2, 8, 0));
        panel.add(panel1);

        fOkBtn = new Button(CifsRuntimeException.getMessage("DLG_OK"));
        fOkBtn.addActionListener(this);
        fOkBtn.addFocusListener(this);
        fOkBtn.addKeyListener(this);
        panel1.add(fOkBtn);

        fCancelBtn = new Button(CifsRuntimeException.getMessage("DLG_CANCEL"));
        fCancelBtn.addActionListener(this);
        fCancelBtn.addFocusListener(this);
        fCancelBtn.addKeyListener(this);
        panel1.add(fCancelBtn);
        pack();
        addWindowListener(this);
    }


    public boolean prompt(String share, CifsLogin login) {

        fShareName.setText(share);

        fPasswordTxt.setText("");
        fFocalComponent = null;
        String data;


        data = login.getAccount();
        if (data != null) {
            fUserTxt.setText(data);
            fFocalComponent = fPasswordTxt;
        } else {
            fUserTxt.setText("");
            fFocalComponent = fUserTxt;
        }

        show();

        boolean ok = (fFocalComponent != fCancelBtn);

        if (ok) {
            login.setAccount(fUserTxt.getText());

            login.setPassword(fPasswordTxt.getText());
        }
        return ok;
    }




    public void actionPerformed(ActionEvent actionevent) {
        setVisible(false);
    }

    protected void add(Component component, GridBagLayout gridbaglayout, GridBagConstraints gridbagconstraints,
                       int x, int y, int w, int h) {

        gridbagconstraints.gridx      = x;
        gridbagconstraints.gridy      = y;
        gridbagconstraints.gridwidth  = w;
        gridbagconstraints.gridheight = h;
        gridbaglayout.setConstraints(component, gridbagconstraints);
        add(component);
    }


    protected void finalize()  throws Throwable {
        super.finalize();
    }

    public void focusGained(FocusEvent focusevent) {

        fFocalComponent = focusevent.getComponent();
    }

    public void focusLost(FocusEvent focusevent)  {
    }

    public void keyTyped(KeyEvent keyevent) {

    }

    public void keyPressed(KeyEvent keyevent) {
        if(keyevent.getKeyCode() == 10) {
            // Ok
            setVisible(false);
            return;
        }


    }

    public void keyReleased(KeyEvent keyevent)  {
        if(keyevent.getComponent() == fUserTxt) {
            if (fUserTxt.getText().length() > 8) {
                Toolkit.getDefaultToolkit().beep();
                fUserTxt.setText(fUserTxt.getText().substring(0,8));
            }
        }

    }

    public void windowActivated(WindowEvent windowevent)  {
        if(windowevent.getWindow().isVisible())
            fFocalComponent.requestFocus();
    }

    public void windowClosed(WindowEvent windowevent) {
    }

    public void windowClosing(WindowEvent windowevent)  {
        fFocalComponent = fCancelBtn;
        setVisible(false);
    }

    public void windowDeactivated(WindowEvent windowevent)  {
    }

    public void windowDeiconified(WindowEvent windowevent)  {
        if(windowevent.getWindow().isVisible())
            fFocalComponent.requestFocus();
    }

    public void windowIconified(WindowEvent windowevent) {
    }

    public void windowOpened(WindowEvent windowevent) {
        if(windowevent.getWindow().isVisible())
            fFocalComponent.requestFocus();
    }



}