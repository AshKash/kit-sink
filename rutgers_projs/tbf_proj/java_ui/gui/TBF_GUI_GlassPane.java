package tbf.java_ui.gui;
import java.awt.*;
import javax.swing.*;

public class TBF_GUI_GlassPane extends JPanel
{
    public TBF_GUI_GlassPane()
    {
	setOpaque(false);
    }
    
    public void paintComponent(Graphics g)
    {
	ImageIcon icon = new ImageIcon("mote.gif");
	g.drawImage(icon.getImage(), 100,100, this);
    }
}
