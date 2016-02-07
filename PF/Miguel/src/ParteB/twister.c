#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

int fd_fifo_reader;
int parentpid;

void exitClient(int sign){
	int i, res;
	int fd_fifo = 0;
	int wbytes;

	switch(sign){
		case SIGINT:
			if(getpid() == parentpid){
				printf("\nSaliendo de Twister...\n");
				close(fd_fifo_reader);
				wait(NULL);
				fd_fifo=open("/proc/fifomulti/control",O_WRONLY);
				if (fd_fifo<0) {
					perror("/proc/fifomulti/control");
					exit(1);
				}
				wbytes=write(fd_fifo,"delete fifotwister",sizeof("create fifotwister"));
				if(wbytes < 0 || wbytes == 0){
					perror("Can't delete fifo");
					exit(1);
				}
				close(fd_fifo);
				exit(1);
			}
		break;
		default:
		break;
	}
}

static void fifo_send (const char* path_fifo) {
	int message;
	int fd_fifo=0;
	int bytes=0,wbytes=0;

	srand(time(NULL));

	fd_fifo=open(path_fifo,O_WRONLY);

	if (fd_fifo<0) {
		perror(path_fifo);
		exit(1);
	}

	/* Bucle de envío de datos a través del FIFO
	- Leer de la entrada estandar hasta fin de fichero
	*/
	while(1){
		message = (rand() % 4);
		wbytes=write(fd_fifo,&message,sizeof(int));

		if (wbytes > 0 && wbytes!=sizeof(int)) {
			fprintf(stderr,"Can't write the whole register\n");
			exit(1);
		}else if (wbytes < 0){
			perror("Error when writing to the FIFO\n");
			exit(1);
		}else if(wbytes == 0){
			close(fd_fifo);
			exit(0);
		}
	}
}

static void fifo_receive (const char* path_fifo) {
	int orderpart, ordercolor;
	int fd_fifo=0;
	int bytes=1,wbytes=0, readbool=1;
	char aux;
	char body[4][21] = {"el pie izquierdo\0", "el pie derecho\0", "la mano izquierda\0", "la mano derecha\0"};
	char color[4][21] = {"en el color azul\0", "en el color rojo\0", "en el color verde\0", "en el color amarillo\0"};

	fd_fifo=open(path_fifo,O_RDONLY);

	if (fd_fifo<0) {
		perror(path_fifo);
		exit(1);
	}
	fd_fifo_reader = fd_fifo;
	printf("Pulsa enter para recibir indicaciones nuevas o Ctrl + c para salir...\n");
	
	while(readbool){
		if((bytes=read(fd_fifo,&orderpart,sizeof(int)))==sizeof(int)){
			if((bytes=read(fd_fifo,&ordercolor,sizeof(int)))==sizeof(int)){
				aux = getchar();
				printf("\n -> Pon %s %s\n", body[orderpart], color[ordercolor]);
			}else{
				readbool = 0;
			}
		}else{
			readbool = 0;
		}
	}

	if (bytes > 0){
		fprintf(stderr,"Can't read the whole register\n");
		exit(1);
	}else if (bytes < 0) {
		fprintf(stderr,"Error when reading from the FIFO\n");
		exit(1);
	}

	close(fd_fifo);
}

int main (void){
	int pid;
	int fd_fifo = 0;
	int wbytes;
	
	fd_fifo=open("/proc/fifomulti/control",O_WRONLY);
	if (fd_fifo<0) {
		perror("/proc/fifomulti/control");
		exit(1);
	}
	wbytes=write(fd_fifo,"create fifotwister",sizeof("create fifotwister"));
	if(wbytes < 0 || wbytes == 0){
		perror("Can't create fifo");
		exit(1);
	}
	close(fd_fifo);
	//Capturar señal Ctrl + c y eliminar la fifo

	// Crear hijo
	pid = fork();
	if(pid == 0){
		fifo_send("/proc/fifomulti/fifotwister");
	}else{
		parentpid=getpid();
		signal(SIGINT, exitClient);
		fifo_receive("/proc/fifomulti/fifotwister");
	}
}
