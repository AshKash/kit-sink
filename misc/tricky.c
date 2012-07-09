#include <stdio.h>

static int f1 (int);
static int f2 (int);
static int f3 (int);

typedef int (*fptype)(int);
const static fptype fptab[] = { f1, f2, f3 };

int main()
{

}


static int f1(int i) {}
static int f2(int i) {}
static int f3(int i) {}
