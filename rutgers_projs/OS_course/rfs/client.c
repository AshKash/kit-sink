#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>

#include "rfs.h"
extern int errno;


int main (int argc, char **argv)
{
    int v_rfd;
    int v_wfd;
    int len = 0;
    char buff[1024];
    //char name_buff[MAX_FILES][MAX_PATH];
    
    printf("c: %d, r: %d, w: %d, rl: %d, rd: %d\n",
	   O_CREAT, O_RDONLY, O_WRONLY, O_REPLICATE, OP_REDIRECT);
    /* Init the library */
    rfs_init(argc, argv);

    printf("File Read test\n");
    do {
      /* Open remote file */
      if ((v_rfd = my_fopen("README", O_RDONLY)) < 0) {
	perror("File open");
	exit(1);
      }

      /* Read its contents */
      if ((len = my_fread(v_rfd, buff, sizeof(buff))) < 0) {
	perror("File read");
	my_fclose(v_rfd);
	exit(1);
      }

      buff[len] = '\0';
      printf("***READ %d bytes:\n %s\n", len, buff);

      my_fclose(v_rfd);

      printf("Press enter to repeat\n");
    } while(getchar() == '\n');
    getchar();

    /* open new file for writing */
    printf("File write test\n");
    do {
      v_wfd = my_fopen("write_file", O_CREAT | O_WRONLY);

      /* write to file */
      strncpy(buff, "This is a file created by RFS client\nwriten for OS\n",
	      sizeof(buff));
      len = my_fwrite(v_wfd, buff, strlen(buff));
	  
      /* close */
      my_fclose(v_wfd);

      printf("Press enter to repeat\n");
    } while (getchar() == '\n');

    return(0);
}
    

