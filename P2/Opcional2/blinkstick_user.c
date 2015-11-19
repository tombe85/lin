#include <linux/errno.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#define BLINKSTICK "/dev/usb/blinkstick0"
#define NUM_VUELTAS 7


unsigned char *sample_colors[]={"1:0x110000\n","0:0x110000\n", "1:0x001100\n", "2:0x000011\n", "3:0x110011\n"};
unsigned char *sample_off[]={"1:0x000000\n","0:0x000000\n", "1:0x000000\n", "2:0x000000\n", "3:0x000000\n"};


int pintaLeds(char *cadena) {
	
	int fd = open(BLINKSTICK, O_WRONLY,S_IWRITE);	
	if(fd == -1) {
		fprintf(stderr, "Error open /dev/usb/blinkstick0\n");
		exit(1);
	}
	
	write(fd, cadena, sizeof(char)*strlen(cadena));
	close(fd);

}



int main(void){
	
	int i=0;
	int combinacion[NUM_VUELTAS];
	int suma=0;
	int sumajugador=0;
	int secuencia=0;
	int salir = 0;
	int aleat;
	int dificultad = 0;
	
	srand(time(NULL));
	
	while(salir == 0 && dificultad < 900000) {
	
		for(i=0; i<NUM_VUELTAS;i++) {
			aleat = rand() % (4 - 1 +1) + 1;
			combinacion[i] = aleat;
			suma += aleat;
			//printf("%d\n", combinacion[i]);
			pintaLeds(sample_colors[aleat]);
			usleep(1000000 - dificultad);
			pintaLeds(sample_off[aleat]);
		}
		
		printf("Introduce la secuencia correcta: ");
		scanf("%d", &secuencia);
		printf("\n");
		//printf("%d\n", suma);
		
		if(secuencia >= 1000000 && secuencia <= 9999999) {
			sumajugador += secuencia / 1000000;
			sumajugador += (secuencia % 1000000) / 100000;
			sumajugador += ((secuencia % 1000000) % 100000) / 10000;
			sumajugador += (((secuencia % 1000000) % 100000) % 10000) / 1000;
			sumajugador += ((((secuencia % 1000000) % 100000) % 10000) % 1000) / 100;
			sumajugador += (((((secuencia % 1000000) % 100000) % 10000) % 1000) % 100) / 10;
			sumajugador += ((((((secuencia % 1000000) % 100000) % 10000) % 1000) % 100) % 10) / 1;
		}
		
		//printf("%d\n", sumajugador);
		
		
		if(suma == sumajugador)
			printf("Has acertado\n");
		else {
			salir = 1;
			printf("Has fallado\n");
		}
			
		printf("Secuncia correcta: ");
		
		for(i=0; i<NUM_VUELTAS; i++) {
			printf("%d", combinacion[i]);
		}
		
		printf("\n");
		
		printf("Secuencia introducida: %d\n", secuencia);
		
		dificultad += 100000;
	
	}
	
    return 0;
}
