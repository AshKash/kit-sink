/*
	INTOPOST.JAVA
	Ported to java from C
*/
package tbf.java_ui;

import java.util.*;

public class intopost{

String FLOAT= "01";//     = 1;
String INT= "02";//       = 2;
String CHAR="03";//	= 3;
String SIN="04";//        = 4;
String COS="05";//        = 5;
String TAN="06";//        = 6;
String ASIN="07";//       = 7;
String ACOS="08";//       = 8;
String ATAN="09";//       = 9;
String SINH="0a";//       = 10;
String COSH="0b";//       = 11;
String TANH="0c";//       = 12;
String ASINH="0d";//      = 13;
String ACOSH="0e";//	= 14;
String ATANH="0f";//	= 15;
String LOG10="10";//	= 16;
String LOG="11";//	= 17;
String EXP="12";//	= 18;
String SQRT="13";//	= 19;
String CEIL="14";//	= 20;
String FLOOR="15";//	= 21;
String MY_PI="16";//	= 22;
String ADD="81";//	= 129;
String SUB="82";//	= 130;
String MUL="83";//	= 131;
String DIV="84";//	= 132;
String POW="85";//	= 133;
String T	="ff";//        = 255;
int TABLE_SIZE= 255;


  String s_table[] = {"NULL","NULL","NULL","NULL","sin","cos","tan","asin","acos","atan","sinh","cosh","tanh","asinh","acosh","atanh","log10","log","exp","sqrt","ceil","floor","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","my_add","my_sub","my_mul","my_div","pow","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL","NULL",};


    StringBuffer infix = null;
    StringBuffer postfix = null;
    String finished = null;
    Stack stk = null;
    int i_len;

    /*
     * Constructor
     */
    public intopost(String inf){
	infix = new StringBuffer(inf);
	postfix = new StringBuffer();
	i_len = inf.length();
	stk = new Stack();
	process();
    };

    public String getPostfix(){ 
	if(process()!=-1)
	    return finished;
	else
	    return null;
    }

    void best_rep(int pptr, float val) {
	char cval = (char)val;
	short sval = (short)val;

	System.out.println("got " + val);
  
	// try char first
	if (cval == val) {
	    System.out.println("putting as char\n");
	    postfix.append(CHAR);
	    postfix.append((char)val);
	} else if (sval == val) {
	    System.out.println("putting as short\n");
	    postfix.append(INT);
	    postfix.append((int)val);
	} else {
	    System.out.println("putting as float\n");
	    postfix.append(FLOAT);
	    postfix.append((float)val);
	}
    }

/*--------------------------------------------------------------------------

FUNCTION: intopost - converts infix to postfix

A.	Push a ( on the stack. This sentinel allows us to detect when we
	have flushed out the stack on completion in step 1.

--- Perform steps B through H for each character in the infix string ---

B.	Ignore white space.

C.	Pass alphabetic characters through to postfix list.

D.	Push any ( on the stack. These sentinels allows us to detect when
	have flushed out the stack when handling ) and operators.

E.	Have a ) so pop off the stack and put into postfix list until a (
	is popped. Discard the (.

F	Have a * or /. Pop off any operators of equal or higher precedence
	and put them into postfix list. If a ( or lower precedence operator
	(such as + or -) is popped, put it back and stop looking.
	Push new * or /.

G.	Have a + or -. Pop off any operators of equal or higher precedence
	(that includes all of them) and put them into postfix list. If a (
	is popped, put it back and stop looking. Push new + or -.

H.	Report unknown character on input.

--------

I.	Have processed all input characters. New flush stack until we find
	the matchint ( put there in step A.

J.	Terminate the postfix list.

--------------------------------------------------------------------------*/
    int parseFloat( StringBuffer sb, int start, int end, float f){
	StringBuffer ret = new StringBuffer();
	int x;

	for(x=start;x<end;x++){
	    if(Character.isDigit(sb.charAt(x))||sb.charAt(x)=='.'){
		ret.append(sb.charAt(x));
	    }
	    else
		break;
	}

	f=Float.parseFloat(ret.toString());
	return x;
    }

    int parseString( StringBuffer sb, int start, int end, String s){
	StringBuffer ret = new StringBuffer();
	int x;

	for(x=start;x<end;x++){
	    if(Character.isLetter(sb.charAt(x))||
	       (!Character.isLetterOrDigit(sb.charAt(x))&&Character.isWhitespace(sb.charAt(x)))){
		ret.append(sb.charAt(x));
	    }
	    else
		break;
	}

	s=ret.toString();
	return x;
    }

    int process()
    {

	String st;
	int ptr_pf_orig = 0;
	int ptr_infix = 0;
	int ptr_postfix = 0;

	/*A*/
	stk.push("(");
	while (ptr_infix != i_len) {
	    

	    /*B*/
	    if (Character.isSpaceChar(infix.charAt(ptr_infix))) {
		;
	    } else if (Character.isDigit(infix.charAt(ptr_infix))) {
		/*C*/
		float a = 0;
		int b = 0;
		// read the number as a float
		b=parseFloat(infix, ptr_infix, i_len, a);
		best_rep(ptr_postfix, a);
		ptr_infix = b;
 System.out.println("Saw a number "+a);		
	    } else if (Character.isLetter(infix.charAt(ptr_infix))) {
		// scans funcs 
		int b=0;
		String w = null;
		b = parseString(infix,ptr_infix,i_len,w);
 System.out.println("Saw a word "+w);
		System.out.println("Got "+w+", consumed "+ b);
		// see if it is a t
		if (w.compareToIgnoreCase("t")==0) {
		    postfix.append(T);
		} else if (w.compareToIgnoreCase("pi")==0 ) {
		    postfix.append(MY_PI);
		} else {
		    String code = lookup(w);
		    if (code.compareTo("-1")==0) {
			System.out.println(w+" is not valid code\n");
			return -1;
		    }
		    stk.push(code);
		}
		ptr_infix = b;
		
	    } else if (infix.charAt(ptr_infix) == '(') {
		/*D*/
		stk.push("(");
		ptr_infix++;
		
	    } else if (infix.charAt(ptr_infix) == ')') {
		/*E*/
		while ((st = (String)stk.pop()).compareTo("(")==0) {
		    postfix.append(st);		
		}
		ptr_infix++;
	    } else if (infix.charAt(ptr_infix) == '*' || infix.charAt(ptr_infix) == '/' || 
		       infix.charAt(ptr_infix) == '^') {
		char[] tmp = {infix.charAt(ptr_infix)};
		String str = new String(tmp);
 System.out.println("Saw this op: "+str);	
		String code = lookup(str);
		/*F*/
		while (true) {
		    if ((st = (String)stk.pop()).compareTo("(") == 0 || st.compareTo("+") == 0
			|| st.compareTo("-") == 0) {
			stk.push(st);
			break;
		    }
		    postfix.append(st);
		}
		if (code.compareTo("-1")!=0) 
		    stk.push(code);
		else {
		    System.out.println("Unknown symbol " + str);
		    return -1;
		}
		ptr_infix++;
	    } else if (infix.charAt(ptr_infix) == '+' || infix.charAt(ptr_infix) == '-') {
		char tmp[] = {infix.charAt(ptr_infix)};
		String str = new String(tmp);
		String code = lookup(str);
 System.out.println("Saw this op: "+str);
		/*G*/
		while (true) {
		    if ((st = (String)stk.pop()).compareTo("(") == 0) {
			stk.push(st);
			break;
		    }
		    postfix.append(st);
		}
		if (code.compareTo("-1") != 0) 
		    stk.push(code);
		else {
		    System.out.println("Unknown symbol" +  str);
		    return -1;
		}
		ptr_infix++;
	    } else {
		/*H*/
		System.out.println("Unknown input character "+ infix.charAt(ptr_infix));
	    }
	   
	    continue;
	}
	/*I*/
	while ((st =(String)stk.pop()).compareTo("(") != 0) {
	    postfix.append(st);
	    
	}
	
	/*J*/
       
        finished = postfix.toString();
	return 1;
    }

 /**************************************************************\
 |** Mike:  Checks lookup table for alphas.  Returns location **|
 |**        if found else returns -1                          **|
 \**************************************************************/
String lookup(String w)
{
  int lcv = 0;

  // change +-/*
  if(w.compareTo("+")==0)
    w = "my_add";
  else if(w.compareTo("*")==0)
    w = "my_mul";
  else if (w.compareTo("/")==0)
    w = "my_div";
  else if (w.compareTo("-")==0)
    w = "my_sub";
  else if (w.compareTo("^")==0)
    w = "pow";
 
  System.out.println("Got "+w+", Lookup says: ");

  while ((lcv < TABLE_SIZE) && ( s_table[lcv].compareTo(w)!= 0))
    lcv++;
  
  if (lcv+1 > TABLE_SIZE) {
    System.out.println("-1\n");
    return ("-1");
  }
  else {
    System.out.println(Integer.toHexString(lcv));
    if(lcv < 16)
	return ("0"+Integer.toHexString(lcv));
    return Integer.toHexString(lcv);
  }
}

double my_add(double a, double b) {return a+b;}
double my_mul(double a, double b) {return a*b;}
double my_div(double a, double b) {return a/b;}
double my_sub(double a, double b) {return a-b;}

/*
// returns a success or failure
int rpncalc(const char *str, int len, double t,
	    double *result) {
  unsigned char c;
  char *orig = str;


  System.out.println("rpncalc got len: %d\n", len);
  while (len > (str - orig)) {
    switch (c = *str++) {
    case FLOAT:
      push_dbl((double)*((float*)str)++);
      break;

      // i think the motes have 2byte ints
      // we definitely need to fix this
      // FIXME:
    case INT:
      push_dbl((double)*((short*)str)++);
      break;

    case CHAR:
      push_dbl((double)*((char*)str)++);
      break;

    case T: {
      double my_t = t;
      System.out.println("Got t, treating it as %lf\n", my_t);
      push_dbl(my_t);
      break;
    }
    case MY_PI:
      push_dbl(M_PI);
      break;

    default:
      if (c > START_ARG2_FUNCS) {
	// has 2 args
	double a;
	double b;
	double (*func)(double a, double b);

	a = pop_dbl();
	b = pop_dbl();
	// call the func
	func = f_table[c];	
	if (func != NULL) {
	  System.out.println("%s(%lf, %lf)\n", s_table[c], b, a);
	  push_dbl(func(b, a));
	} 
      } else {
	double (*func)(double a);
	double a = pop_dbl();

	// has only one arg
	// call the function
	func = f_table[c];
	if (func != NULL) {
	  System.out.println("%s(%lf)\n", s_table[c], a);
	  push_dbl(func(a));
	}
      }
      break;
    } // switch
  } // while
  *result = pop_dbl();
  if (stack_dbl_err() < 0) {
    clear_dbl();
    return (-1);
  }
  return 1;

  } // rpncalc*/
}
