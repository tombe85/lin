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

void ledctl(unsigned int mask){
    if((int) syscall(__NR_LEDCTL,mask,mask) < 0){
        perror("Ledctl error: ");
        exit(-1);
    }
}

void winSec(){
    ledctl(0x7);
    usleep(500000);
    ledctl(0x0);
    usleep(500000);
    ledctl(0x7);
    usleep(500000);
    ledctl(0x0);
    usleep(500000);
    ledctl(0x7);
    usleep(500000);
    ledctl(0x0);
}

void loseSec(){
    ledctl(0x1);
    usleep(500000);
    ledctl(0x2);
    usleep(500000);
    ledctl(0x4);
    usleep(500000);
    ledctl(0x1);
    usleep(500000);
    ledctl(0x2);
    usleep(500000);
    ledctl(0x4);
    usleep(500000);
    ledctl(0x0);
}

int main(int argc, char *argv[]){
    if(argc != 4){
    	printf("Uso: leds_user.c num_ini num_fin numero\n");
    	return -1;
    }
    srand(time(NULL));
    int ini = atoi(argv[1]), fin = atoi(argv[2]), num = atoi(argv[3]);
    int aleat = rand() % (fin - ini +1) + ini;
    
    printf("Numero aleatorio: %i\n", aleat);
    if(aleat == num){
        printf("Has ganado!\n");
        winSec();
    }else{
        printf("Has perdido...\n");
        loseSec();
    }
    return 0;
}
