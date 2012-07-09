package tbf.java_ui.gui;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.border.Border;

public class TBF_GUI implements ActionListener
{

    //first declare all variables	
    JButton send,query,rescan = null;
 
    JPanel jp_pane_bottom = null;
    JPanel jp_pane_left = null;
    JPanel jp_pane = null;

    JTextField jt_moteid = null;
    JTextField jt_dest = null;
    JTextField jt_group = null;
    JTextField jt_tos = null;
    JTextField jt_ttl = null;
    JTextField jt_x_traj = null;
    JTextField jt_y_traj = null;

    JLabel jl_moteid = null;
    JLabel jl_dest = null;
    JLabel jl_group = null;
    JLabel jl_tos = null;
    JLabel jl_ttl = null;
    JLabel jl_x_traj = null;
    JLabel jl_y_traj = null;

    //constructor to initialize everything
    public TBF_GUI()
    {
	
	send = new JButton("Send");
	query = new JButton("Query");
	rescan = new JButton("Rescan");
	jp_pane = new JPanel();
	jp_pane_left = new JPanel();
	jp_pane_bottom = new JPanel();

	jt_moteid = new JTextField(5);
	jt_dest = new JTextField(5);
	jt_group =  new JTextField(10);
	jt_tos =  new JTextField(5);
	jt_ttl =  new JTextField(5);
	jt_x_traj =  new JTextField(20);
	jt_y_traj =  new JTextField(20);
	
	jl_moteid = new JLabel("Moteid (src)");
	jl_dest =  new JLabel("Moteid (dest)");
	jl_group =  new JLabel("Group");
	jl_tos =  new JLabel("TOS");
	jl_ttl =  new JLabel("TTL");
	jl_x_traj =  new JLabel("X Trajectory");
	jl_y_traj =  new JLabel("Y Trajectory");

	send.addActionListener(this);
	query.addActionListener(this);
	rescan.addActionListener(this);

	jp_pane_left.setLayout(new GridLayout(0,2));
	jp_pane_left.add(jl_moteid);
	jp_pane_left.add(jt_moteid);
	jp_pane_left.add(jl_dest);
	jp_pane_left.add(jt_dest);
	jp_pane_left.add(jl_group);
	jp_pane_left.add(jt_group);
	jp_pane_left.add(jl_tos);
	jp_pane_left.add(jt_tos);
	jp_pane_left.add(jl_ttl);
	jp_pane_left.add(jt_ttl);
	jp_pane_left.add(jl_x_traj);
	jp_pane_left.add(jt_x_traj);
	jp_pane_left.add(jl_y_traj);
	jp_pane_left.add(jt_y_traj);
	

	jp_pane_bottom.setLayout(new GridLayout(1,3));
	jp_pane_bottom.add(send);
	jp_pane_bottom.add(rescan);
	jp_pane_bottom.add(query);

	jp_pane.setLayout(new BorderLayout());
	jp_pane.add(jp_pane_left, BorderLayout.WEST);
	jp_pane.add(jp_pane_bottom, BorderLayout.SOUTH);

	   	
    }	


    public Component getComponents()
    {
	return jp_pane;
    }	

    public void actionPerformed(ActionEvent e)
    {
	String action = e.getActionCommand();
	if(action.compareTo("Send")==0)
	    processSend();
	else if(action.compareTo("Rescan")==0)
	    processRescan();
	else 
	    processQuery();
    }

    void processSend(){};

    void processRescan(){};

    void processQuery(){};





}//class TBF_GUI
