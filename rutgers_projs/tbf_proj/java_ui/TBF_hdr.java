package tbf.java_ui;

import java.util.*;
import net.tinyos.packet.*;

public class TBF_hdr extends TOSPacket{

    public TBF_hdr(){
	super();
    }

    public short packetField_src;
    public short packetField_dest;
    public byte packetField_ttl;
    
    public byte packetField_tos;
    public boolean packetField_proc;
    public boolean packetField_fwd;

    public short packetField_policy; //will take the least significant 4 bits
    public short packetField_x_byte; //need to convert to little endian before
    public short packetField_y_byte; //transmitting.


    public int packetLength() {return 8;}

    /* Initializes structure with incoming packet
     * Empty for now: no useful purpose for header
     *
     * @param packet Incoming packet to parse
     */ 
    public void initialize(byte[] packet) throws IllegalArgumentException {
     //ADD PARSING CODE HERE
    }
    
    /* Gets the data section of the header. This really has no purpose as the data section
     * will have variable length. Put in to appease the compiler
     *
     * @return null
     */
    public byte[] getDataSection() {return null;}

    /* Converts fields into a byte array
     *
     * @return byte[] representing this hdr structure
     */
    public byte[] toByteArray() {
	byte[] packet = new byte[packetLength()];
	int offset = super.headerLength();
	
	ArrayPackerLE.putShort(packet, offset + 0, packetField_src);
	ArrayPackerLE.putShort(packet, offset + 2, packetField_dest);
	ArrayPackerLE.putByte(packet, offset + 4, packetField_ttl); 
	ArrayPackerLE.putByte(packet, offset + 5, packetField_tos);

	BitSet pkt = fromByteArray(packet);

	if(packetField_proc)
	    pkt.set(48);
	else
	    pkt.clear(48);

	if(packetField_fwd)
	    pkt.set(49);
	else 
	    pkt.clear (49);
	
	

       	//Get the last 4 bits of policy, x_byte and y_byte
	bitCopy(4, pkt, fromByte(new Short(packetField_policy).byteValue()), true, 50);
	bitCopy(4, pkt, fromByte(new Short(packetField_x_byte).byteValue()), true, 54);
	bitCopy(4, pkt, fromByte(new Short(packetField_y_byte).byteValue()), true, 58);

	packet = toByteArray(pkt);
	
	return packet;
    }

    /* Copies least significant n bits into packet from a BitSet at 
     * offset.
     *
     * @param n number of bits to copy
     * @param pkt the BitSet to copy to
     * @param bits the source BitSet
     * @param convert whether to convert (little to big, big to little endian)
     * @param offset offset of pkt where to start copying
     */
    public void bitCopy(int n, BitSet pkt, BitSet bits, boolean convert, int offset){
	
	if(convert){
	    for(int x=bits.length();x>bits.length()-n;x--){
		if(bits.get(x-1))
		    pkt.set(offset);
		else pkt.clear(offset);
		
		offset++;
	    }
	}else{
	    for(int x=bits.length()-n;x<bits.length();x++){
		if(bits.get(x))
		    pkt.set(offset);
		else pkt.clear(offset);
		
		offset++;
	    }
	}
    } 
    /* Returns a bitset containing the values in byte.
     * The byte-ordering of byte must be big-endian which means 
     * the most significant bit is in element 0.
     *
     * @param b the byte to convert
     * @return the BitSet representing the byte
     */
    public BitSet fromByte(byte b) {
        BitSet bits = new BitSet();
        for (int i=0; i<8; i++) {
            if ((b&(1<<(i%8))) > 0) {
                bits.set(i);
            }
        }
        return bits;
    }

    /* Returns a bitset containing the values in bytes.
     * The byte-ordering of bytes must be big-endian which means 
     * the most significant bit is in element 0.
     *
     * @param bytes the byte array to convert
     * @return the BitSet representing the byte array
     */
    public BitSet fromByteArray(byte[] bytes) {
        BitSet bits = new BitSet();
        for (int i=0; i<bytes.length*8; i++) {
            if ((bytes[bytes.length-i/8-1]&(1<<(i%8))) > 0) {
                bits.set(i);
            }
        }
        return bits;
    }
    
    /* Returns a byte array of at least length 1.
     * The most significant bit in the result is guaranteed not to be a 1
     * (since BitSet does not support sign extension).
     * The byte-ordering of the result is big-endian which means the most 
     * significant bit is in element 0. The bit at index 0 of the bit set 
     * is assumed to be the least significant bit.
     *
     * @param bits BitSet to convert to byte array
     * @return the byte array representing the bitset
     */
    public byte[] toByteArray(BitSet bits) {
        byte[] bytes = new byte[bits.length()/8+1];
        for (int i=0; i<bits.length(); i++) {
            if (bits.get(i)) {
                bytes[bytes.length-i/8-1] |= 1<<(i%8);
            }
        }
        return bytes;
    }

}
