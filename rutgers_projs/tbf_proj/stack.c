// implements a simple stack of ints and doubles
// the int stack is used for the infix to postfix conversion
// the double stack is used for the postfix evaluation

/*
  $Log$
*/

#include "tbf_hdr.h"

// init the stack
void stack_init(struct stack *p)
{
  p->err = 0;
  p->ptr = 0;
}

// init the stack_dbl
void stack_dbl_init(struct stack_dbl *p)
{
  p->err = 0;
  p->ptr = 0;
}

// check to see if there was an error
int stack_dbl_err(struct stack_dbl *p) {
  if (p->err)
    return -1;
  return 1;
}

// error stuff
int stack_err(struct stack *p) {
  if (p->err)
    return -1;
  return 1;
}

// clear the stack
void clear_dbl(struct stack_dbl *p) {
  p->err = 0;
  p->ptr = 0;
}

void clear(struct stack *p) {
  p->err = 0;
  p->ptr = 0;
}

void push(struct stack *p, int value)
{

#ifdef STACK_TRACE
    printf("pushing: %c\n", value);
#endif

    if (p->ptr == TBF_STACK_SIZE) {
#ifdef STACK_TRACE
	printf("Stack is full\n");
#endif
	p->err = 2;
    }
    else
	p->stack[p->ptr++] = value;
}

void push_dbl(struct stack_dbl *p, double value)
{

#ifdef STACK_TRACE
    printf("pushing: %lf\n", value);
#endif

    if (p->ptr == TBF_STACK_SIZE) {
#ifdef STACK_TRACE
	printf("Stack is full\n");
#endif
	p->err = 2;
    }
    else
	p->stack[p->ptr++] = value;
}

  
int pop(struct stack *p)
{
    if (p->ptr == 0) {
#ifdef STACK_TRACE
	printf("Stack is empty\n");
#endif
	p->err = 1;
	return 0;
    }
#ifdef STACK_TRACE
    printf("poping: %c\n", p->stack[p->ptr - 1]);
#endif

    return p->stack[--p->ptr];
}

double pop_dbl(struct stack_dbl *p)
{
    if (p->ptr == 0) {
#ifdef STACK_TRACE
	printf("Stack is empty\n");
#endif
	p->err = 1;
	return 0;
    }
#ifdef STACK_TRACE
    printf("poping: %lf\n", p->stack[p->ptr - 1]);
#endif

    return p->stack[--p->ptr];
}

