#include <linux/errno.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#define CONTROL "/proc/listmulti/control"
#define MAX_SIZE_READ 50


/* Variables globales */
char lista[11];


int executeInst(char *inst, char *source, char *ret) {
		
	int fd = open(source, O_RDWR, S_IRUSR | S_IWUSR);
	char *cadena = (char *)malloc(sizeof(char)*100);	
	
	if(fd == -1) {
		printf("Error\n");
		strcat(cadena, "Error open /proc/listmulti/");
		strcat(cadena, source);
		printf(cadena);
		printf("\n");
		exit(1);
	}

	if(strcmp(inst, "listar") != 0) {
		write(fd, inst, sizeof(char)*strlen(inst));
	}
	else {
		read(fd, (char *)cadena, sizeof(int)*MAX_SIZE_READ);
		strcpy(ret, cadena);
	}
	
	free(cadena);
		
	close(fd);
}



int main(void) {
	
	int salir = 0;
	char * cadena;
	char comando[50] = "";
	
	strcat(lista, "/proc/listmulti/");
	
	printf("Introduzca alias (minusculas y max 10 caracteres): ");
	scanf("%s", cadena);
	strcat(comando, "create ");
	strcat(comando, cadena);
	executeInst(comando, CONTROL, NULL);
	printf("\n");
	
	strcat(lista, cadena);
	
	
	while(!salir) {
	
		printf("> ");
		scanf(" %[^\n]", comando);

		if(!strcmp(comando, "salir"))
			salir = 1;
		else if(!strcmp(comando, "listar")) {
			executeInst(comando, lista, cadena);
			printf("%s\n",cadena);
		}
		else {
			executeInst(comando, lista, NULL);
		}
	
	}
	
	
	
    return 0;
}
