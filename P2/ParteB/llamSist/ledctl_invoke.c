#include <linux/errno.h>
#include <sys/syscall.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __i386__
#define __NR_LEDCTL	353
#else
#define __NR_LEDCTL	316
#endif

long int ledctl(unsigned int mask){
  return (int) syscall(__NR_LEDCTL,mask,mask);
}

int main(int argc, char *argv[]){
    if(argc != 2){
	printf("Uso: ledctl_invoke 0x[0..7]\n");
	return -1;
    }
    int num = strtol(argv[1], NULL, 16);
    if(ledctl(num) < 0){
	perror("Ledctl error: ");
	return -1;
    }
    return 0;
}
