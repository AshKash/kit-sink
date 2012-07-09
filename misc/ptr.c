#include <stdio.h>

int main()
{
	char arr[] = "test string - writable";
	const char * p = arr, *q;

	//p = "test2";
	p[0] = 'w';

	q[0] = 'w';
	q = "test2";
}
