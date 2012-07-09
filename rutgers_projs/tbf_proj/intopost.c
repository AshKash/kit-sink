// Convert infix to postfic notation

/*
  $Log$
*/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mapping.h"
#include "tbf_hdr.h"

//tries to get the best representation for the given number
void best_rep(char **postfix, float val) {
  char cval = val;
  short sval = val;

  // try char first
  if (cval == val) {
    *(*postfix)++ = CHAR;
    *((char *)(*postfix))++ = val;
  } else if (sval == val) {
    *(*postfix)++ = INT;
    *((short *)(*postfix))++ = val;
  } else {
    *(*postfix)++ = FLOAT;
    *((float *)(*postfix))++ = val;
  }
}

// printout the given packet
int bin_print(const char *postfix_str, int k) {
  int lcv;
  for(lcv = 0; lcv < k; lcv ++)
    printf("%02x ", postfix_str[lcv] & 0xff);
  printf("\n");
  fflush(stdout);
  return lcv;
}

// printout the given packet in human readable form
int symb_print(const char *str, int len)
{
  unsigned char c;
  const char *orig = str;
  
  while (len > (str - orig)) {
    switch (c = *str++) {
    case FLOAT:
      printf("%lf ", ((double)*((float*)str)++));
      break;

      // i think the motes have 2byte ints
      // we definitely need to fix this
      // FIXME:
    case INT:
      printf("%lf ",  ((double)*((short*)str)++));
      break;

    case CHAR:
      printf("%lf ", ((double)*((char*)str)++));
      break;

    case T: {
      printf("t ");
      break;
    }
    case MY_PI:
      printf("PI ");
      break;

    default:
	printf("%s ", s_table[c]);
    } // switch
  } // while
  printf("\n");
  return 1;

}

 /**************************************************************\
 |** Mike:  Checks lookup table for alphas.  Returns location **|
 |**        if found else returns -1                          **|
 \**************************************************************/
int lookup(const char* w)
{
  int lcv = 0;

  // change +-/*
  switch (*w) {
  case '+':
    w = "my_add";
    break;
  case '*':
    w = "my_mul";
    break;
  case '/':
    w = "my_div";
    break;
  case '-':
    w = "my_sub";
    break;
  case '^':
    w = "pow";
    break;
  }

  while ((lcv < TABLE_SIZE) && (strcmp(s_table[lcv],w) != 0))
    lcv++;
  if (lcv+1 > TABLE_SIZE) {
    return (-1);
  }
  else {
    return (lcv);
  }
}

// given two infix strings, and a pointer to tbf_hdr,
// encodes the infix in postfix form and dumps it
// into the packet
// returns -1 on failure
inline int dump_traj(const char *inf_x, const char *inf_y,
		     struct tbf_hdr *ptbf_hdr, int data_len)
{
  char *x_traj;
  char *y_traj;
  int err;

  // do x
  x_traj = (char *) (ptbf_hdr + 1);
  err = intopost(inf_x, x_traj);
  if (err < 0) return -1;
  else if (err + sizeof(struct tbf_hdr) > data_len) {
#ifdef TBF_DEBUG
    printf("x String too big\n");
#endif
    return -2;
  }
  ptbf_hdr->x_byte = err;

  // do y
  y_traj = (char *) (x_traj + (int)ptbf_hdr->x_byte);
  err = intopost(inf_y, y_traj);
  if (err < 0) return -1;
  else if (err + ptbf_hdr->x_byte + sizeof(struct tbf_hdr) > data_len) {
#ifdef TBF_DEBUG
    printf("y String too big\n");
#endif
    return -2;
  }
  ptbf_hdr->y_byte = err;

#ifdef TBF_DEBUG
  printf("encodings: \n");
  symb_print(x_traj, ptbf_hdr->x_byte);
  symb_print(y_traj, ptbf_hdr->y_byte);
#endif

  return 1;
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

int intopost(const char *infix, char *postfix)
{
    int st;
    char *orig = postfix; 
    struct stack istack;

    stack_init(&istack);

    /*A*/
    push(&istack, '(');
    while (*infix != '\0') {
      
#ifdef TRACE
      printf("*infix: %c\n", *infix);
#endif

      /*B*/
      if (isspace(*infix)) {
	  ;
      } else if (isdigit(*infix)) {
	/*C*/
	float a;
	int b;
	// read the number as a float
	sscanf(infix, "%f%n", &a, &b);
	best_rep(&postfix, a);
	infix += b-1;

      } else if (isalpha(*infix)) {
	// scans funcs 
	int b, code;
	char w[256];
	sscanf(infix, "%[_a-zA-Z]%n", w, &b);
	// see if it is a t
	if (strcasecmp(w, "t") == 0) {
	  *postfix++ = T;
	} else if (strcasecmp(w, "pi") == 0) {
	  *postfix++ = MY_PI;
	} else {
	  code = lookup(w);
	  if (code == -1) {
	    printf("%s is not valid code\n", w);
	    return -1;
	  }
	  push(&istack, code);
	}
	infix += b-1;

      } else if (*infix == '(') {
	/*D*/
	push(&istack, '(');

      } else if (*infix == ')') {
	/*E*/
	while ((st = pop(&istack)) != '(') {
	  *postfix++ = st;
	}
      } else if (*infix == '*' || *infix == '/' || *infix == '^') {
	char str[] = {*infix};
	int code = lookup(str);
	/*F*/
	while (1) {
	  if ((st = pop(&istack)) == '(' || st == '+' || st == '-') {
	    push(&istack, st);
	    break;
	  }
	  *postfix++ = st;
	}
	if (code != -1) 
	  push(&istack, code);
	else {
	  printf("Unknown symbol %s\n", str);
	  return -1;
	}
	
      } else if (*infix == '+' || *infix == '-') {
	char str[] = {*infix};
	int code = lookup(str);
	/*G*/
	while (1) {
	  if ((st = pop(&istack)) == '(') {
	    push(&istack, st);
	    break;
	  }
	  *postfix++ = st;
	}
	if (code != -1) 
	  push(&istack, code);
	else {
	  printf("Unknown symbol %s\n", str);
	  return -1;
	}
      } else {
	/*H*/
	printf("Unknown input character %c\n", *infix);
	return -1;
      }
      ++infix;
      continue;
    }
    /*I*/
    while ((st = pop(&istack)) != '(') {
      *postfix++ = st;
      
    }

    /*J*/
    *postfix = '\0';
    return (postfix - orig);
}
