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



void init_serial(int *serial_fd, struct termios *saveterm)
{

  struct termios new;
  int fd=open("/dev/ttyUSB0",O_RDWR|O_NOCTTY|O_NONBLOCK);
  if (fd == -1)
  {
    perror("init_serial: Unable to open /dev/ttyUSB0 ");
    exit(0);
  }
  if(!isatty(fd)) {printf("fd is not from a serial port");}

  tcgetattr(fd,saveterm); // save current port settings
  bzero(&new,sizeof(new));

  new.c_cflag &= ~PARENB;
  new.c_cflag &= ~CSTOPB;
  new.c_cflag &= ~CSIZE; /* Mask the character size bits */
  new.c_cflag=CLOCAL|CREAD|B115200|CS8;
  new.c_iflag=0;
  new.c_oflag=0;
  new.c_lflag=0; // set input mode (non-canonical, no echo,...)
  new.c_cc[VTIME]=0; // inter-character timer unused
  new.c_cc[VMIN]=1; // blocking read until 1 char received
  tcflush(fd, TCIFLUSH);

  tcsetattr(fd,TCSANOW,&new);
  *serial_fd = fd;
	sleep(1);
}



void getData(unsigned char * buf, int fd){

	unsigned char c;

	read(fd, &c, 1);
	if(c == 0xff){
		buf[0]=c;
		read(fd, buf+1, 7);
	}

}



void getData2(unsigned char * buf, int fd){

	unsigned char c;

	do read(fd, &c, 1);
	while(c != 0xff);
	if(c == 0xff){
		buf[0]=c;
		read(fd, buf+1, 7);
	}

}



double getAngle(unsigned char *buf){

	short int rate;
	short int angle;
	short check_sum;

	//Verify packet heading information
	if(buf[0] != 0xFF || buf[1] != 0xFF)
	{
		return 0;
	}

	//Assemble data
	rate = (buf[2] & 0xFF) | ((buf[3] << 8) & 0xFF00);
	angle = (buf[4] & 0xFF) | ((buf[5] << 8) & 0XFF00);

	//Verify checksum
	check_sum = 0xFFFF + rate + angle;
	if(((unsigned char)(check_sum & 0xFF) != buf[6]) || ((unsigned char)((check_sum>>8) & 0xFF) != buf[7]))
	{
		return 0;
	}

	//Scale and store data
	//double gRate = rate / 100.0;
	double gAngle = angle / 100.0;
	return gAngle;

}



int main(){

  struct termios old;
  int fd;
	double angle;
  unsigned char buf[8] = {0};
	init_serial(&fd, &old);

	while(1){
		getData2(buf,fd);
		angle = getAngle(buf);
		if (angle != 0) printf("%f\n",angle);
	}		

  close(fd);
  return 0;
}
