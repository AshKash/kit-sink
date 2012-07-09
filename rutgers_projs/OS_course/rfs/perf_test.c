#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>

#include "rfs.h"
extern int errno;


int main (int argc, char **argv)
{
    int v_rfd;
    int v_wfd;
    int len = 0;
    int i, rem;
    int LOOP = 20;
    struct timeval t_start, t_end;
    int w_time, r_time;
    char * fname = "big_file";
    char buff[1024 * 20];
    
    /* Init the library */
    rfs_init(argc, argv);


    /* prepare a buffer */
    i = 0;
    rem = sizeof(buff);
    while (rem) {
      char str[256];
      sprintf(str, "This is the %s file, line %d\n", fname, i++);
      len = strlen(str);
      rem -= len;
      if (rem > 0) strncpy(buff, str, rem);
      else rem = 0;
    }

    /* open new file for writing */
    printf("File write test\n");
    i = LOOP;
    gettimeofday(&t_start, NULL);
    do {
      v_wfd = my_fopen(fname, O_CREAT | O_WRONLY);

      /* write to file */
      len = my_fwrite(v_wfd, buff, sizeof(buff));
	  
      /* close */
      my_fclose(v_wfd);

    } while (--i);
    gettimeofday(&t_end, NULL);
    w_time = (int) (t_end.tv_sec - t_start.tv_sec);


    /* Read test */
    printf("File Read test\n");
    sleep(1);
    i = LOOP;
    gettimeofday(&t_start, NULL);
    do {
      /* Open remote file */
      if ((v_rfd = my_fopen(fname, O_RDONLY)) < 0) {
	perror("File open");
	exit(1);
      }

      /* Read its contents */
      if ((len = my_fread(v_rfd, buff, sizeof(buff))) < 0) {
	perror("File read");
	my_fclose(v_rfd);
	exit(1);
      }

      my_fclose(v_rfd);

    } while(--i);
    gettimeofday(&t_end, NULL);
    r_time = (int) (t_end.tv_sec - t_start.tv_sec);

    /* Report */
    printf("******* Time ****** taken to do %d writes: %d\n", 
	   LOOP, w_time);

    printf("***** Time ****** taken to do %d reads %d\n",
	   LOOP, r_time);




    return(0);

}
