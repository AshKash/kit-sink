package tbf.java_ui;

import net.tinyos.util.*;
import net.tinyos.packet.*;

import java.io.*;
import java.util.*;


public class clTBF implements PacketListenerIF{

    AMPacket pkt=null;
    TBF_hdr tbf_h=null;
    
    byte[] x_traj = null;
    byte[] y_traj = null;

    SerialForwarderStub sf = null;

    public void clTBF(String host, String port){
	sf = new SerialForwarderStub(host,Integer.parseInt(port));
	try{
	    sf.Open();
	    sf.registerPacketListener(this);
	}catch(Exception e){
	    e.printStackTrace();
	}
	
	pkt = new AMPacket();
	tbf_h = new TBF_hdr();
    }

    public void packetReceived(byte[] data){
	//put data processing code here.
    }

    public void setAMPacket(short moteid, byte group, byte type){
	pkt.packetField_AM_dest = moteid;
	pkt.packetField_AM_group = group;
	pkt.packetField_AM_type = type;
    }

    public int set_x_Traj(String infix){
	intopost parser = new intopost(infix);
	String postfix = parser.getPostfix();
	x_traj = postfix.getBytes();
	tbf_h.packetField_x_byte = (short)x_traj.length;
	return 1;
    }

    public int set_y_Traj(String infix){
	intopost parser = new intopost(infix);
	String postfix = parser.getPostfix();
	y_traj = postfix.getBytes();
	tbf_h.packetField_y_byte = (short)y_traj.length;
	return 1;
    }

    public int sendPacket() throws IllegalArgumentException, IOException    {
	//Construct the tbf packet
	int total_traj = tbf_h.packetField_x_byte + tbf_h.packetField_y_byte;
	int tbf_packetLength = total_traj + tbf_h.packetLength();
	
	byte[] tbf_packet = new byte[tbf_packetLength];
	System.arraycopy(tbf_h.toByteArray(), 0, tbf_packet, 0, tbf_h.packetLength());
	System.arraycopy(x_traj, 0, tbf_packet, tbf_h.packetLength(), tbf_h.packetLength() + tbf_h.packetField_x_byte);
	System.arraycopy(y_traj, 0, tbf_packet, tbf_h.packetLength()+tbf_h.packetField_x_byte, tbf_packetLength);
	//append to AMpacket
	pkt.setData(tbf_packet);

	sf.Write(pkt.toByteArray());
	
	return 1;
    }

}//end clTBF
