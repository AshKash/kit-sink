package tbf.java_ui.gui;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.border.Border;


public class TBF_MAIN implements ActionListener
{
   
    public TBF_MAIN(){
  
    }

    public static void main(String[] args)
    {
	try
	{
	    UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
	}catch(Exception e){}

	//create top-level container and add contents to it
	JFrame frame = new JFrame("TBF Viewer");

	//creates components for frame
	TBF_GUI app = new TBF_GUI();
	Component contents = app.getComponents();
	
	//adds components to frame
	JPanel jp = new JPanel();
	TBF_GUI_GlassPane gp = new TBF_GUI_GlassPane();
	frame.setGlassPane(gp);
	//gp.setOpaque(false);
	//jp.add(gp);
	frame.getGlassPane().setVisible(true);
	jp.setPreferredSize(new Dimension(1000,1000));
	JScrollPane scroller =  new JScrollPane(jp);
	//scroller.setViewport(vp);
	frame.getContentPane().add(scroller,BorderLayout.CENTER);
	scroller.setPreferredSize(new Dimension(800, 400));
	frame.getContentPane().add(contents, BorderLayout.SOUTH);


	frame.addWindowListener(new WindowAdapter()
	{
		public void windowClosing(WindowEvent e)
		{
			System.exit(0);
		}
	});

	frame.pack();
	frame.setVisible(true);

	        
			
    }//main()

    public void actionPerformed(ActionEvent e){}


}//APS_MAIN	
