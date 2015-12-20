#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "fwall.h"

#define MY_MACIG 'G'
#define READ_IOCTL _IOR(MY_MACIG, 0, int)
#define WRITE_IOCTL _IOW(MY_MACIG, 1, int)


int main(int argc, char **argv){
        char buf[200];
        int fd = -1;
        mystruct ob;
	if (argc == 2) {
		ob.r1=atoi(argv[1]);
		ob.r2=0;
		ob.r3=0;
	} else if (argc == 3) {
		ob.r1=atoi(argv[1]);
		ob.r2 =atoi(argv[2]);
		ob.r3=0;
	} else {
		ob.r3=atoi(argv[3]);
		ob.r1=atoi(argv[1]);
		ob.r2 =atoi(argv[2]);
	}

        if ((fd = open("/dev/cdev_example", O_RDWR)) < 0) {
                perror("open");
                return -1;
        }
        if(ioctl(fd, WRITE_IOCTL,&ob) < 0)
                perror("first ioctl");
        if(ioctl(fd, READ_IOCTL, buf) < 0)
                perror("second ioctl");

        return 0;

}

