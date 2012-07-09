#include <stdio.h>
#include <stdlib.h>

#define SWAP(a,b) ((a==b) || (a^=b,b^=a,a^=b))
static int iter=0;

void my_qsort(int a[], int begin, int end)
{
  if (end > begin) {
    int pivot = a[rand() * (end - begin) / RAND_MAX + begin];
    int l = begin + 1;
    int r = end;
    while (l<r) {
      iter++;
      if (a[l] <= pivot)
	l++;
      else {
	r--;
	SWAP(a[l], a[r]);
      }
    }
    l--;
    SWAP(a[begin], a[l]);
    my_qsort(a, begin, l);
    my_qsort(a, r, end);
  }
}

int main()
{
  int a[1000000];
  int len = sizeof(a) / sizeof(a[0]);
  int i;
  
  for (i=0; i<len; i++) {
    a[i] = rand();
    /*printf("%d\n", a[i]);*/
  }
  
  printf("UNSORTED\n\n");
  
  my_qsort(a, 0, len);
  
  for (i=0; i<len; i++) {
    /*printf("%d\n", a[i]);*/
  }
  printf("DONE: %d\n\n", iter);

}










