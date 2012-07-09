import java.io.*;

public class Reliable_link implements Wired_link{
	OutputStream os;
	public Reliable_link(OutputStream os){
		this.os = os;
	}
	public void send_packet(byte[] data){
		try{
			os.write(data);
		}catch(Exception e){
		    	e.printStackTrace();
		}
	}
}
