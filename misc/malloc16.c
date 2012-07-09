#include <stdio.h>
#include <stddef.h>

void free16(void *p)
{
	unsigned char *n = ((unsigned char*)p + *((unsigned char*)p - 1));
	free(n);
}

void* malloc16(size_t s)
{
	unsigned char *p = (unsigned char*)malloc(s + 16);
	unsigned char *newp = (unsigned char*)(((int)p  + 16 ) & 0xfffffff0);

	if(!p) return p;

	*(newp - 1) = newp - p;

	return newp;
}

int main()
{

	char* p = (char*)malloc16(100);

	strcpy(p, "This string can be 100 bytes long!");
	printf("hello: %s\n", p);

	free16(p);

	return 0;

}
