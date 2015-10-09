#include <linux/module.h>	/* Requerido por todos los módulos */
#include <linux/kernel.h>	/* Definición de KERN_INFO */
MODULE_LICENSE("GPL"); 	/*  Licencia del modulo */

/* Función que se invoca cuando se carga el módulo en el kernel */
int modulo_lin_init(void)
{
	printk(KERN_INFO "Modulo LIN cargado. Hola kernel.\n");

	/* Devolver 0 para indicar una carga correcta del módulo */
	return 0;

	/* 
	   Si retorna un numero negativo, al hacer insmod "modulo.ko" no se carga el modulo y module_init gestiona
	   el error mediante la variable errno y nos muestra el mensaje en concreto. Igualmente si que ejecuta
	   la llamada a printk().
	*/
}

/* Función que se invoca cuando se descarga el módulo del kernel */
void modulo_lin_clean(void)
{
	printk(KERN_INFO "Modulo LIN descargado. Adios kernel.\n");
}

/* Declaración de funciones init y exit */
module_init(modulo_lin_init);
module_exit(modulo_lin_clean);

