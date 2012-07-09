package tbf.java_ui.gui;
import java.awt.*;
import javax.swing.*;

public class TBF_GUI_Panel extends JPanel
{
    public void TBF_GUI_Panel()
    {
	setPreferredSize(new Dimension(500,500));
	TBF_GUI_GlassPane gpane = new TBF_GUI_GlassPane();
	add(gpane);
    }
}
