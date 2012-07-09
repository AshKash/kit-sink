#include <stdio.h>

struct num{
        int i;
        int flag;
}n[256];

int main(int argc, char **argv)
{
        int k,nn,l;
        int count = 0, i,j;
        int reg, tmp;

        if (argc != 3)
                exit(1);
        nn = atoi(argv[1]);
        k = atoi(argv[2]);
        l = nn - k + 1;
	printf ("%d, %d, %d\n", nn, k, l);

        for (i=0; i < nn; i++) {
                n[i].i = i;
		n[i].flag = 0;
	}

	i=count=0;
	reg=n[k].i;
        for (j=0; j < nn; j++)
                printf("%d ", n[j].i);
        printf("%d\n", reg);
        for (; i<nn; i++) {

                n[count].i = reg;
                reg = n[count+l].i;
		n[count].flag= 1;
                count = (count + l)%nn;
		for (j=0; j < nn; j++)
			printf("%d ", n[j].i);
		printf(" \t(%d, %d, %d)\n", i, count, reg);


        }
}

