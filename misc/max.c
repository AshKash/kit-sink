#include <stdio.h>

int main (int argc, char *argv[])
{
	int arr[] = {1,4,2,4,-2,1,-10,99,2,2,1,3,4,-1,-4,3,7,-5};
	unsigned int i, len = sizeof arr / sizeof arr[0];
	int s, e, smax;
	int sum=0, maxp=0;

	s = -1;
	for (i=0; i<len; i++) {
		sum += arr[i];
		if (sum < 0) {
			sum = 0;
			s = -1;
		} else { 
			if (s < 0) s = i;
			if (sum > maxp) {
				maxp = sum;
				e = i;
				smax = s;
			}
		}
	}

	printf("sum: %d, a=%d, b=%d\n", maxp, s, e);
}
