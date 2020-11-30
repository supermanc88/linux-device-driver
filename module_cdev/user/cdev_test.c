#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


#define BUFF_SIZE 4096
#define CLEAR_DATA 0
#define DEV_NAME "/root/cdev"



int main(void)
{
    int fd;
    char buf[BUFF_SIZE] = {0};

    fd = open(DEV_NAME, O_RDWR);
    if (fd < 0) {
        printf("open cdev failed\n");
        return -1;
    }

    memcpy(buf, "this data is writed by user!", strlen("this data is writed by user!"));

    if (write(fd, buf, strlen(buf)) < 0) {
        printf("write cdev failed : %s\n", strerror(errno));
        return -1;
    }

    if (lseek(fd, 0, SEEK_SET) < 0) {
        printf("lseek cdev failed : %s\n", strerror(errno));
        return -1;
    }

    memset(buf, 0, BUFF_SIZE);

    if (read(fd, buf, BUFF_SIZE) < 0) {
        printf("read cdev failed : %s\n", strerror(errno));
        return -1;
    }

    printf("read data : %s\n", buf);

    if (ioctl(fd, CLEAR_DATA, NULL) < 0) {
        printf("ioctl cdev failed : %s\n", strerror(errno));
        return -1;
    }

    if (lseek(fd, 0, SEEK_SET) < 0) {
        printf("lseek cdev failed : %s\n", strerror(errno));
        return -1;
    }

    memset(buf, 0, BUFF_SIZE);

    if (read(fd, buf, BUFF_SIZE) < 0) {
        printf("read cdev failed : %s\n", strerror(errno));
        return -1;
    }

    printf("read data : %s\n", buf);


    close(fd);

    return 0;
}
