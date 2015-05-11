#ifndef __GYRO_H__
#define	__GYRO_H__

void init_serial(int *serial_fd);
void getData(unsigned char * buf, int fd);
double getAngle(unsigned char *buf);


#endif
