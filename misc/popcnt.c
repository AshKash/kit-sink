#include <stdio.h>
#include <stdlib.h>

/* Counts number of set ibt by using compiler builtin
 * Newer processors have this instruction. */
int main(int argc, char* argv[])
{
    int x=atoi(argv[1]);
    printf("%d\n", __builtin_popcount (x));
}
