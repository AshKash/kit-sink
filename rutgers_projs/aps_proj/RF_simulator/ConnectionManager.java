import java.io.*;
import java.util.*;
import java.net.*;

public class ConnectionManager implements Runnable{
	List outputs = new LinkedList();
        int[][] connectivity = new int[512][512];
  
	void create_connection_list(){
	      add_conn(1, 2);
	      add_conn(1, 3);
	      add_conn(1, 4);

	      add_conn(2, 3);
	      add_conn(2, 4);
	      add_conn(2, 5);

	      add_conn(3, 6);
	      add_conn(3, 4);

	      add_conn(4, 7);
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
	   new ConnectionManager().run();
	}

	public void run(){
	 create_connection_list();	
	 try{
		ServerSocket s = new ServerSocket(9876);
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
