package tbf.java_ui;

public class driver{

  
 
    public static void main(String args[]){

	intopost parser = new intopost("5*3/2");
       
	System.out.println((parser.getPostfix()).getBytes());
    }
    
   
}
