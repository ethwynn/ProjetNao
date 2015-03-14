/*minicom -b 115200 -D /dev/ttyUSB0 -H -w
 *To test using minicom
 */


#include <stdio.h>   /* Standard input/output definitions */
#include <stdlib.h>
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

void init_serial(char* device, int *serial_fd, struct termios *saveterm)
{

struct termios new;
int fd=open(device,O_RDWR|O_NOCTTY|O_NONBLOCK);
if (fd == -1)
{
  perror("init_serial: Unable to open /dev/ttyUSB0 ");
  exit(0);
}
tcgetattr(fd,saveterm); // save current port settings
bzero(&new,sizeof(new));
new.c_cflag=CLOCAL|CREAD|B115200|CS8;
new.c_iflag=0;
new.c_oflag=0;
new.c_lflag=0; // set input mode (non-canonical, no echo,...)
new.c_cc[VTIME]=0; // inter-character timer unused
new.c_cc[VMIN]=1; // blocking read until 1 char received
tcflush(fd, TCIFLUSH);
tcsetattr(fd,TCSANOW,&new);
*serial_fd = fd;
sleep(2);
}

int main(){

  struct termios old;
  char* device = "/dev/ttyUSB0";
  int fd;
  init_serial(device, &fd, &old);
  char buf[8] = {0};
  int i;
  while(1){
	  read(fd, buf, 8);

	  for(i=0;i<8;i++){
	    printf("%x ",buf[i]); //Testing 
	  }
	  printf("\n");
	  sleep(1);
  }

  return 0;
}
