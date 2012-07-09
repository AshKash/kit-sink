
import java.net.*;
import java.io.*;
import java.util.*;

public class MoteConnection implements Runnable{
	List outputs;
	int[] connections;
  	public void AddOutput(Wired_link link){
		outputs.add(link);
	}		
	InputStream is;
	Wired_link os;
	int id;
  	public int getID(){return id;}
	MoteConnection(int id, InputStream is, Wired_link os, List outputs, int[] connections){
		this.id = id;
		this.is = is;
		this.os = os;
		this.outputs = outputs;
		this.connections = connections;
		Thread t = new Thread(this);
		t.start();	
	}	
	
	public void run(){
		try{
			byte[] data = new byte[554];
			while(1 == 1){
				int cnt = 0;
				while(cnt < 554){
					cnt += is.read(data, cnt, 554-cnt);
					if(cnt == -1){
				  	System.out.println("" + id + " socket closed");
				  	return;
					}
				}
				System.out.println("read data from " + id + " " + cnt);
				synchronized(outputs){
				}
				ListIterator li = outputs.listIterator();
				while(li.hasNext()){
					MoteConnection out = (MoteConnection)li.next();
					System.out.println("checking " + out.getID());
					
					if(isConnected(out)){
						System.out.println("sending data to " + out.getID());
						out.os.send_packet(data);
					}
				}
			}
		}catch(Exception e){
			e.printStackTrace();
		}
	}
				
	public boolean isConnected(MoteConnection other){
		int other_id = other.getID();
		if(1 == connections[other_id]) return true;
		return false;
	}
}
