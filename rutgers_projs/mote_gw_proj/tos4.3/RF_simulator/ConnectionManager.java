import java.io.*;
import java.util.*;
import java.net.*;

public class ConnectionManager implements Runnable{
	List outputs = new LinkedList();
	int[][] connectivity = new int[512][512];
	static String port;

	void create_connection_list(){
		for(int i = 0; i < 255; i ++){
			for(int j = i + 1; j < 255; j ++){
			   add_conn(i, j);
			}
		}
	}
	void add_conn(int a, int b){
		if(a > connectivity.length) return;
		if(b > connectivity.length) return;
		connectivity[a][b] = 1;
		connectivity[b][a] = 1;
		System.out.println("added connection between " + a + " and " + b);
	}
	void remove_conn(int a, int b){
		if(a > connectivity.length) return;
		if(b > connectivity.length) return;
		connectivity[a][b] = 0;
		connectivity[b][a] = 0;
	}
	public static void main(String[] args){
	   port = args[0];
	   new ConnectionManager().run();
	}

	public void run(){
	 create_connection_list();	
	 try{
		ServerSocket s = new ServerSocket(Integer.parseInt(port));
		while(1 == 1){
			try{
				Socket sock = s.accept();
				byte[] d = new byte[4];
				InputStream is = sock.getInputStream();
				is.read(d);
				Wired_link wl = new Reliable_link(sock.getOutputStream());
				int id = (int)d[0];
				System.out.println("got new connection, mote " + id);
				synchronized(outputs){
					MoteConnection m = new MoteConnection(id, is, wl, outputs, connectivity[id]);
					outputs.add(0, m);
				}
				
			}catch(Exception f){
				f.printStackTrace();
			}
		}		
 	 }catch(Exception e){
		e.printStackTrace();
	 }
	}
}
