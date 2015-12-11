// INCLUDES
#include "include/filesystem.h"
#include "include/constantes_lseek.h"
#include <sys/types.h> 
#include <sys/stat.h> 
#include <sys/fcntl.h> 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

// CONSTANTES
#define DESMONTADO 0
#define MONTADO 1
#define TAMBLOQUE 1024
#define TAMNOMBRE 50

// VARIABLES GLOBALES

// Superbloque
typedef struct{
int numInodos;
int numBloquesDatos;
int comienzoInodos;
int comienzoBloquesDatos;
int bloqueMapaInodos;
int bloqueMapaDatos;
char padding [1000];
}Superbloque;

// Versión
typedef struct{
int version;
int puntero;
int tamano;
int bloque;
}Version;

// Inodo
typedef struct{
char inodo [TAMNOMBRE];
int vActual;
int vUltima;
Version versiones [10];
char padding [806];
}Inodo;

// Superbloque
Superbloque superbloque;

// Mapa de Inodos
char *imap; 	

// Mapa de bloques
char *bmap;				

// Inodos
Inodo *inodos;
int numInodosActivos = 0;

// Descriptor-Inodo. Cuando hay un descriptor asociado a un inodo, es que el fichero del inodo está abierto
typedef struct{
char inodo [TAMNOMBRE];
int descriptor;
}DescInodo;

// Relaciones entre todos los descriptores y los inodos
DescInodo *descsInodos;

int estadoDisc = DESMONTADO;
int descDisc;	

// FUNCIONES LLAMADAS DESDE AQUÍ
int buscarInodo (int descriptor);
int inicializarVersion (int i, int j);
int buscarPosVersion (int numInodo, int version);

/***************************/
/* File system management. */
/***************************/

/*
 * Formats a device.
 * Returns 0 if the operation was correct or -1 in case of error.
 */
int mkFS (int maxNumFiles) {

	char *formato;

	// Si intentamos abrir más de 50 ficheros, devolvemos un error
	if (maxNumFiles>50) {
		printf("Se han intentado abrir más de 50 ficheros\n");
		return -1;
	}

	// Abrimos el disco
	descDisc = open("disk.dat", O_RDWR);	

	// Formateamos el disco a vacío ('0')
	formato = malloc (DEVICE_SIZE);
	memset(formato, '0', DEVICE_SIZE);
	if (lseek (descDisc, 0, SEEK_SET) == -1) {
		printf ("Error al establecer la posición al principio, %s\n", strerror(errno));
		return -1;
	}		
	if (write(descDisc, formato, DEVICE_SIZE) == -1) {
		printf("Error al formatear el disco, %s\n", strerror(errno));
		return -1;
	}

	// Inicializamos el superbloque (bloque 0)
	superbloque.numInodos = maxNumFiles; 
	superbloque.comienzoInodos = 2;
	superbloque.numBloquesDatos = (DEVICE_SIZE/TAMBLOQUE)-(maxNumFiles + superbloque.comienzoInodos);	 	
	superbloque.comienzoBloquesDatos = maxNumFiles + superbloque.comienzoInodos;
	superbloque.bloqueMapaInodos = 1;
	superbloque.bloqueMapaDatos = 1;

	// No debería haber más de 500 bloques de datos
	if (superbloque.numBloquesDatos>500) {
		printf("Se han intentado abrir más de 50 ficheros");
		return -1;
	}

	// Rellenamos imap y bmap a vacío ('1')
	imap = malloc (superbloque.numInodos);
	bmap = malloc (superbloque.numBloquesDatos);
	int i;
	for(i=0; i<superbloque.numInodos; i++){
		imap[i]='1';
	}
	i=0;
	for(i=0; i<superbloque.numBloquesDatos; i++){
		bmap[i]='1';
	}

	// Inicialización del array de relación entre descriptores e inodos
	descsInodos = malloc (superbloque.numInodos*(sizeof(DescInodo)));
	i=0;
	for(i=0; i<superbloque.numInodos; i++){
		descsInodos[i].descriptor = 0;
		memset(descsInodos[i].inodo,'0', TAMNOMBRE);
	}

	// Inicialización del array de inodos
	inodos = malloc (superbloque.numInodos*(sizeof(Inodo)));

	// Nos posicionamos al principio del disco
	if (lseek (descDisc, 0, SEEK_SET) == -1) {
		printf ("Error al establecer la posición al principio, %s\n", strerror(errno));
		return -1;
	}

	// Escribimos el superbloque
	if (write(descDisc, &superbloque, sizeof(Superbloque)) == -1) {
		printf("Error al escribir el superbloque, %s\n", strerror(errno));
		return -1;
	}

	// Escribimos los mapas
	if (write(descDisc, imap, superbloque.numInodos) == -1) {
		printf("Error al escribir el imap, %s\n", strerror(errno));
		return -1;
	} 

	if (write(descDisc, bmap, superbloque.numBloquesDatos) == -1){
		printf("Error al escribir el bmap, %s\n", strerror(errno));
		return -1;
	} 

	// Cerramos el disco
	close(descDisc);
	
	return 0;
}
	
/*
 * estadoDiscs a file system from the device deviceName.
 * Returns 0 if the operation was correct or -1 in case of error.
 */
int mountFS() {

	// Si el disco ya esta montado
	if (estadoDisc == MONTADO) {
		printf("El disco ya esta montado\n");
		return -1;
	}
	// Si no lo está
	else if (estadoDisc == DESMONTADO) {

		int i;
		int j;
		int *aux;
		int numVersiones;

		// Montamos el disco
		descDisc = open("disk.dat", O_RDWR);
		estadoDisc=MONTADO;	

		// Nos posicionamos en el bloque de mapas
		if (lseek (descDisc, TAMBLOQUE*superbloque.bloqueMapaInodos, SEEK_SET) == -1) {
			printf ("Error al posicionar con lseek\n");
			return -1;
		}

		// Obtenemos todos los mapas y los guardamos en memoria (imap y bmap)
		if (read(descDisc, imap, superbloque.numInodos) ==-1) {
			printf("Error al leer del disco\n");
			return -1;
		}

		if (read(descDisc, bmap, superbloque.numBloquesDatos)==-1) {
			printf("Error al leer del disco\n");
			return -1;
		}

		// Nos posicionamos en el bloque de inodos
		if (lseek(descDisc, TAMBLOQUE*superbloque.comienzoInodos, SEEK_SET) == -1) {
			printf("Error al posicionar con lseek\n");
			return -1;
		}

		// Obtenemos todos los inodos activos y los guardamos en memoria (inodos)
		for (i = 0; i<numInodosActivos; i++){

			// Guardamos el nombre del inodo
			if (read(descDisc, inodos[i].inodo, TAMNOMBRE)==-1)  {
				printf("Error al leer del disco\n");
				return -1;
			}

			// Guardamos la versión actual
			aux = malloc(sizeof(int));
			if (read(descDisc, (int *) aux, sizeof(int))==-1)  {
				printf("Error al leer del disco\n");
				return -1;
			}
			inodos[i].vActual = *aux;
			free(aux);

			// Guardamos la última versión
			aux = malloc(sizeof(int));
			if (read(descDisc, (int *) aux, sizeof(int))==-1)  {
				printf("Error al leer del disco\n");
				return -1;
			}
			inodos[i].vUltima = *aux;
			free(aux);

			// Recorremos todas las versiones
			numVersiones = sizeof(inodos[i].versiones)/sizeof(Version);	
			for (j=0; j < (numVersiones); j++){

				// Guardamos la versión
				aux = malloc(sizeof(int));
				if (read(descDisc, (int *) aux, sizeof(int))==-1)  {
					printf("Error al leer del disco\n");
					return -1;
				}
				inodos[i].versiones[j].version = *aux;
				free (aux);
				
				// Guardamos el puntero
				aux = malloc(sizeof(int));
				if (read(descDisc, (int *) aux, sizeof(int))==-1)  {
					printf("Error al leer del disco\n");
					return -1;
				}
				inodos[i].versiones[j].puntero = *aux;
				free (aux);

				// Guardamos el tamano
				aux = malloc(sizeof(int));
				if (read(descDisc, (int *) aux, sizeof(int))==-1)  {
					printf("Error al leer del disco\n");
					return -1;
				}
				inodos[i].versiones[j].tamano = *aux;
				free (aux);

				// Guardamos el bloque al que apunta
				aux = malloc(sizeof(int));
				if (read(descDisc, (int *) aux, sizeof(int))==-1)  {
					printf("Error al leer del disco\n");
					return -1;
				}
				inodos[i].versiones[j].bloque = *aux;
				free (aux);
			}

			// Nos posicionamos al final de cada bloque de inodos
			if (lseek(descDisc, TAMBLOQUE*(i+superbloque.comienzoInodos), SEEK_SET)==-1) {
				printf("Error al posicionar con lseek\n");;
				return -1;
			}
		}
	}
	return 0;
}

/*
 * UnestadoDisc file system.
 * Returns 0 if the operation was correct or -1 in case of error.
 */
int umountFS() {

	// Si el disco ya está desmontado
	if (estadoDisc == DESMONTADO) {
		printf("El descDisc ya esta desmontado\n");
		return -1;
	}

	// Si el disco ya está montado
	else if (estadoDisc==MONTADO) {

		int i;
		int j;
		int *aux;
		int numVersiones;	
		char *vacio = malloc (TAMBLOQUE*(superbloque.numInodos));
		for(i=0; i<TAMBLOQUE*(superbloque.numInodos); i++){
			vacio[i]='0';
		}

		// Nos posicionamos al principio del mapa de inodos.
		if (lseek (descDisc, superbloque.bloqueMapaInodos, SEEK_SET) == -1) {
			printf ("Error al posicionar con lseek\n");
			return -1;
		}

		// Copiamos al disco los mapas
		if (write (descDisc, imap, superbloque.numInodos)==-1)  {
			printf("Error al escribir en el disco\n");
			return -1;
		}

		if (write(descDisc, bmap, superbloque.numBloquesDatos)==-1)  {
			printf("Error al escribir en el disco\n");
			return -1;
		}

		// Nos posicionamos al principio de los inodos
		if (lseek(descDisc, TAMNOMBRE*superbloque.comienzoInodos, SEEK_SET) == -1) {
			printf("Error al posicionar con lseek\n");
			return -1;
		}
	
		// Formateamos el disco por si acaso hay algún inodo copiado de otro montaje anterior
		if (write(descDisc, vacio, TAMBLOQUE*(superbloque.numInodos))==-1)  {
			printf("Error al escribir en el disco\n");
			return -1;
		}

		// Nos posicionamos al principio de los inodos
		if (lseek(descDisc, TAMBLOQUE*superbloque.comienzoInodos, SEEK_SET) == -1) {
			printf("Error al posicionar con lseek\n");
			return -1;
		}

		// Recorremos todo el array de inodos sobre los inodos activos
		i = 0;
		for (i = 0; i<numInodosActivos; i++){

			// Copiamos a disco el nombre del inodo
			if (write(descDisc, inodos[i].inodo, TAMNOMBRE)==-1)  {
				printf("Error al escribir en el disco\n");
				return -1;
			}

			// Copiamos a disco la versión actual
			aux = &inodos[i].vActual;
			if (write(descDisc, (int *) aux, sizeof(int))==-1)  {
				printf("Error al escribir en el disco\n");
				return -1;
			}

			// Copiamos a disco la última versión
			aux = &inodos[i].vUltima;
			if (write(descDisc, (int *) aux, sizeof(int))==-1)  {
				printf("Error al escribir en el disco\n");
				return -1;
			}

			// Recorremos todas las versiones de este inodo
			numVersiones = sizeof(inodos[i].versiones)/sizeof(Version);	
			for (j=0; j < (numVersiones); j++){

				// Copiamos a disco la versión
				aux = &inodos[i].versiones[j].version;
				if (write (descDisc, (int *) aux, sizeof(int))==-1)  {
					printf("Error al escribir en el disco\n");
					return -1;
				}
				
				// Copiamos a disco el puntero
				aux = &inodos[i].versiones[j].puntero;
				if (write(descDisc, (int *) aux, sizeof(int))==-1)  {
					printf("Error al escribir en el disco\n");
					return -1;
				}

				// Copiamos a disco el tamaño
				aux = &inodos[i].versiones[j].tamano;
				if (write(descDisc, (int *) aux, sizeof(int))==-1)  {
					printf("Error al escribir en el disco\n");
					return -1;
				}

				// Copiamos a disco el bloque
				aux = &inodos[i].versiones[j].bloque;
				if (write(descDisc, (int *) aux, sizeof(int))==-1)  {
					printf("Error al escribir en el disco\n");
					return -1;
				}
			}
			
			// Nos posicionamos en el siguiente bloque de inodos
			if (lseek(descDisc, TAMBLOQUE*(i+superbloque.comienzoInodos), SEEK_SET) == -1) {
				printf("Error al escribir en el disco\n");;
				return -1;
			}			
		}
		// Cerramos el disco y lo desmontamos
		close(descDisc);
		estadoDisc=DESMONTADO;
	}	
	return 0;
}

/*******************/
/* File read/write */
/*******************/

/*
 * Creates or opens a file.
 * Returns file descriptor or -1 in case of error.
 */
int openFS(char *fileName) {

	int descriptor = 0;

	// Si el disco está desmontado
	if(estadoDisc==DESMONTADO) {
		printf ("El disco no está montado\n");
		return -1;
	}

	// Si el disco está montado
	else if (estadoDisc==MONTADO) {

		// Verificamos si el fichero ya está abierto. En caso de que lo esté devolvemos su descriptor asociado
		int i;
		for (i=0; i<numInodosActivos; i++){
			if (strcmp (descsInodos[i].inodo, fileName) == 0) {
				descriptor = descsInodos[i].descriptor;
				return descriptor;
			}
		}

		// Verificamos si existe el inodo
		i=0;
		int existe = 0;
		for (i=0; i<numInodosActivos; i++){
			if (strcmp (inodos[i].inodo, fileName) == 0) {
				existe = 1;
				break;
			}
		}
		
		// En caso de que exista, 
		// encontramos la primera posición libre del array de descriptores y le asignamos un inodo y un descriptor
		// (Abrimos el fichero)
		if (existe == 1){
			int j;
			for (j=0; j<numInodosActivos; j++){
				if (descsInodos[j].descriptor == 0) break;	
			}
			descsInodos[j].descriptor = j+3;
			strcpy (descsInodos[j].inodo, fileName);
			descriptor = descsInodos[i].descriptor;
			return descriptor;
		}

		// Si no existe el inodo, debemos crear el fichero
		else if (existe == 0) {

			// Si hay tantos inodos activos como el número total de inodos, no podemos crear más
			if (numInodosActivos + 1 > superbloque.numInodos) {
				printf ("No se permite la creación de un fichero adicional\n");
				return -1;
			}
		
			// Inicializamos el inodo con los datos del fichero y las versiones actuales y últimas a 1
			strcpy (inodos[numInodosActivos].inodo, fileName);
			inodos[numInodosActivos].vActual = 1;
			inodos[numInodosActivos].vUltima = 1;

			// Inicializamos todo el array de versiones del nuevo inodo
			i=0;
			int numeroVersiones = sizeof(inodos[numInodosActivos].versiones)/sizeof(Version);
			for (i=0; i<numeroVersiones; i++){
				inicializarVersion (numInodosActivos, i);		
			}

			// Activamos al primera versión, que no ocupará bloque alguno 						
			inodos[numInodosActivos].versiones[0].version = 1;

			// Aumentamos el número de inodos activos
			numInodosActivos++;

			// Marcamos el inodo como ocupado
			imap[numInodosActivos-1] = '0';

			// Encontramos la primera posición libre del array de descriptores y le asignamos un inodo y un descriptor
			// (Abrimos el fichero)
			int j;
			for (j=0; j<superbloque.numInodos; j++){
				if (descsInodos[j].descriptor == 0) break;	
			}
			descsInodos[j].descriptor = j+3;
			strcpy (descsInodos[numInodosActivos-1].inodo, fileName);
			descriptor = descsInodos[j].descriptor;
			return descriptor;
			}
		}
		else {
			printf("Error con la variable booleana: existe\n");
			return -1;
		}
	return -1;
}

/*
 * Closes a file.
 * Returns 0 if the operation was correct or -1 in case of error.
 */
int closeFS(int fileDescriptor) {

	// Si el disco está desmontado
	if(estadoDisc==DESMONTADO) {
		printf ("El disco no está montado\n");
		return -1;
	}

	// Si el disco está montado
	else if (estadoDisc==MONTADO) {	

		// i = Descriptor del fichero que se quiere cerrar
		int i;
		for (i=0; i < numInodosActivos; i++){
			if (descsInodos[i].descriptor == fileDescriptor) break;	
		}
		if (i == numInodosActivos) {
			printf("Estas cerrando un fichero que no está abierto\n");
			return -1;
		}
		// Borramos el descriptor y el nombre (cerramos el fichero)
		descsInodos[i].descriptor = 0;
		memset(descsInodos[i].inodo,'0', TAMNOMBRE);
		return 0;
	}
	return -1;
}

/*
 * Reads a number of bytes from a file and stores them in a buffer.
 * Returns the number of bytes read or -1 in case of error.
 */
int readFS(int fileDescriptor, void *buffer, int numBytes, int *currentVersion) {

	int leido = 0;

	// Si el disco está desmontado
	if(estadoDisc==DESMONTADO) {
		printf ("El disco no está montado\n");
		return -1;
	}

	// Si el disco está montado
	else if (estadoDisc==MONTADO) {	
		
		// Obtenemos la versión actual del inodo pasado por parámetro
		int numInodo = buscarInodo (fileDescriptor);
		if (numInodo == -1) return -1;		
		int vActual = inodos[numInodo].vActual;

		// Obtenemos el bloque de la versión actual (el que se va a leer)
		int posVersion = buscarPosVersion (numInodo,vActual);
		if (posVersion == -1) return -1;
		int bloqueALeer = inodos[numInodo].versiones[posVersion].bloque;

		int tamano = inodos[numInodo].versiones[posVersion].tamano;
		if (numBytes > tamano) {
			printf ("No se pueden leer más datos de los que hay escritos\n");
			return -1;
		}

		// Nos posicionamos en el inicio del bloque a leer
		if (lseek (descDisc, (bloqueALeer + superbloque.comienzoBloquesDatos)*TAMBLOQUE, SEEK_SET) == -1) {
			printf ("Fallo al posicionarse \n");
			return -1;
		}

		// Leemos el bloque de datos y lo guardamos en el buffer dado
		buffer = malloc (numBytes);
		leido = read(descDisc, buffer, numBytes);
		if (leido==-1) {
			printf("Fallo al leer\n");
			return -1;
		}

		*currentVersion = vActual;	
		return leido;	
	}
	return -1;
}

/*
 * Reads number of bytes from a buffer and writes them in a file.
 * Update parameter newVersion with the number of the version created for this
 * write. Exceeding the number of versions allowed is considered an error.
 * Returns the number of bytes written or -1 in case of error.
 */
int writeFS(int fileDescriptor, void *buffer, int numBytes, int *newVersion) {

	int bytesEscritos;

	// Si el disco está desmontado
	if(estadoDisc==DESMONTADO) {
		printf ("El disco no está montado\n");
		return -1;
	}

	// Si el disco está montado
	else if (estadoDisc==MONTADO) {
		
		// Obtenemos el inodo requerido
		int numInodo = buscarInodo (fileDescriptor);
		if (numInodo == -1) return -1;

		// Verificamos si tenemos versiones disponibles. Obtenemos la versión nueva
		int versionNueva;
		int numeroVersiones = sizeof(inodos[numInodo].versiones)/sizeof(Version);
		for (versionNueva = 0; versionNueva < numeroVersiones; versionNueva++){
			if (inodos[numInodo].versiones[versionNueva].version == 0) break;
		}
		if (versionNueva == numeroVersiones) {
			printf("No hay versiones disponibles\n");
			return -1;
		}

		// Verificamos si tenemos bloques de datos libres. Obtenemos el bloque a escribir
		int bloqueAEscribir;
		for (bloqueAEscribir=0; bloqueAEscribir<superbloque.numBloquesDatos; bloqueAEscribir++){
			if (bmap[bloqueAEscribir] == '1') break;
		}
		if (bloqueAEscribir == superbloque.numBloquesDatos) {
			printf ("No hay bloques de datos suficientes\n");
			return -1;
		}

		// Obtenemos la versión y la posición antiguas
		int vActual = inodos[numInodo].vActual;
		int posVersion = buscarPosVersion(numInodo,vActual);
		if (posVersion == -1) return -1;
		
		// Sacamos el numero de bytes que habia escritos en la version antigua
		int bytesOcupados = inodos[numInodo].versiones[posVersion].tamano;
		// Obtenemos el bloque a leer.
		int bloqueALeer = inodos[numInodo].versiones[posVersion].bloque;

		// No se puede haber más bytes que un bloque en una versión
		if (numBytes + bytesOcupados > TAMBLOQUE) {
			printf("Se supera el tamaño máximo del fichero\n");
			return -1;
		}		

		// Nos posicionamos en el bloque a leer
		if (lseek (descDisc, (bloqueALeer + superbloque.comienzoBloquesDatos)*TAMBLOQUE, SEEK_SET) == -1) {
			printf ("Fallo al posicionarse \n");
			return -1;
		}

		// Leemos el bloque de datos
		char *buf = malloc (TAMBLOQUE);
		if (read(descDisc, buf, TAMBLOQUE)==-1) {
			printf("Fallo al leer\n");
			return -1;
		}

		// Escribimos en el bufer lo nuevo que queremos escribir a continuacion de lo que estaba escrito 
		char *nuevosDatos = (char *) buffer;
		for (bytesEscritos=0; bytesEscritos < numBytes; bytesEscritos++){
			buf[bytesOcupados + bytesEscritos] = nuevosDatos[bytesEscritos];
		}
	
		// Nos posicionamos en el bloque de datos correspondiente a la nueva version 
		if (lseek (descDisc, TAMBLOQUE*(superbloque.comienzoBloquesDatos + bloqueAEscribir), SEEK_SET) == -1) {
			printf ("Fallo al posicionarse \n");
			return -1;
		}

		// Escribimos en el bloque de datos nuevo
		if (write(descDisc, buf, (bytesOcupados + bytesEscritos))==-1)  {
			printf("Error al leer de descDisc\n");
			return -1;
		}
		
		// Actualizamos la versión a la última versión + 1
		inodos[numInodo].versiones[versionNueva].version = inodos[numInodo].vUltima + 1;
		// Actualizamos el tamano a todo lo que se ha escrito
		inodos[numInodo].versiones[versionNueva].tamano = numBytes + bytesOcupados;
		// Actualizamos el puntero colocándolo al final
		inodos[numInodo].versiones[versionNueva].puntero = inodos[numInodo].versiones[versionNueva].tamano;
		// Actualizamos el bloque al nuevo bloque a escribir
		inodos[numInodo].versiones[versionNueva].bloque = bloqueAEscribir;
		// Actualizamos la versión última y la actual a la nueva versión
		inodos[numInodo].vUltima = inodos[numInodo].versiones[versionNueva].version;
		inodos[numInodo].vActual = inodos[numInodo].versiones[versionNueva].version;

		// Actualizamos el valor de la versión pasado por parámetro
		*newVersion = inodos[numInodo].vActual;

		//Marcamos el bloque como ocupado 
		bmap[bloqueAEscribir] = '0';

		return bytesEscritos;	
	}
	return -1;
}

/*
 * Repositions the pointer of a file. Assume that the count begins from
 * byte 0. A greater offset than the current size is considered an error.
 * Returns new position or -1 in case of error.
 */
int lseekFS (int fileDescriptor, long offset, int whence) {

	// Si el disco no está montado
	if(estadoDisc==DESMONTADO) {
		printf ("El disco no está montado\n");
		return -1;
	}

	// Si el disco está montado
	else if (estadoDisc==MONTADO) {

		// Obtenemos el inodo, la versión actual de ese inodo y el máximo direccionamiento
		int numInodo = buscarInodo (fileDescriptor);
		int vActual = inodos[numInodo].vActual;
		int posVersion = buscarPosVersion (numInodo, vActual);
		int punteroMax = inodos[numInodo].versiones[posVersion].tamano;

		// Si whence es SF_SEEK_SET
		if (whence == SF_SEEK_SET){
			// Verificamos que no se supere el tamano del bloque
			if (offset > punteroMax) {
				printf("No se puede colocar el puntero en una posicion superior a la longitud total del fichero\n");
				return -1;
			}
			else {
				// Actualizamos el puntero de la versión actual en el inodo que se pide
				if (numInodo == -1) return -1;				
				if (posVersion == -1) return -1;				
				inodos[numInodo].versiones[posVersion].puntero = (int) offset;
				return inodos[numInodo].versiones[posVersion].puntero;
			}
		}

		// Si whence es SF_SEEK_END
		else if (whence == SF_SEEK_END){
			// Actualizamos el puntero de la versión actual en el inodo que se pide
			if (numInodo == -1) return -1;
			if (posVersion == -1) return -1;
			inodos[numInodo].versiones[posVersion].puntero = punteroMax;
			return inodos[numInodo].versiones[posVersion].puntero;
		}
		else {
			printf("Parámetro whence no reconocido\n");
			return -1;
		}
	}
	return -1;
}

/**********************/
/* Version management */
/**********************/

/*
 * Changes current version of the file to the one specified as a parameter.
 * Changing to a non-existent version is considered an error.
 * Returns 0 if the operation was correct or -1 in case of error.
 */
int switchFS(char *fileName, int version) {

	// Si el disco no está montado
	if(estadoDisc==DESMONTADO) {
		printf ("El disco no está montado\n");
		return -1;
	}

	// Si el disco está montado
	else if (estadoDisc==MONTADO) {

		// Verificamos si el fichero ha sido previamente abierto
		int i;
		for (i = 0; i<numInodosActivos; i++){
			if (strcmp (descsInodos[i].inodo, fileName) == 0) {
				printf("El fichero está abierto\n");
				return -1;
			}
		}

		// Si el fichero está cerrado se puede cambiar la versión
		if (i == numInodosActivos) {


			// Obtenemos el inodo y la versión dentro de ese inodo
			int numInodo;
			for (numInodo=0; numInodo<numInodosActivos; numInodo++){
				if (strcmp (inodos[numInodo].inodo, fileName) == 0) break;
			}
			if (numInodo == numInodosActivos) {
				printf ("No se encuentra el inodo requerido\n");
				return -1;
			}

			int posVersion = buscarPosVersion (numInodo, version);
			if (posVersion == -1) return -1;

			// Cambiamos la versión actual de ese inodo
			inodos[numInodo].vActual = inodos[numInodo].versiones[posVersion].version;

			return 0;
		}
	}	
	return -1;
}

/*
 * Deletes specified version from a file's list of versions. Updates current
 * version if necessary. Trying to delete a non-existent version is considered
 * an error. Returns 0 if the operation was correct or -1 in case of error.
 */
int deleteFS(char *filename, int version) {

	// Si el disco no está montado
	if(estadoDisc==DESMONTADO) {
		printf ("El disco no está montado\n");
		return -1;
	}

	// Si el disco está montado
	else if (estadoDisc==MONTADO)  {

		// Verificamos si el fichero ha sido previamente abierto
		int i;
		for (i = 0; i<numInodosActivos; i++){
			if (strcmp (descsInodos[i].inodo, filename) == 0) {
				printf("El fichero está abierto\n");
				return -1;
			}
		}

		// Si el fichero está cerrado se puede eliminar la version
		if (i == numInodosActivos){

			// Obtenemos el inodo y la versión dentro de ese inodo
			int numInodo;
			for (numInodo=0; numInodo<numInodosActivos; numInodo++){
				if (strcmp (inodos[numInodo].inodo, filename) == 0) break;
			}
			if (numInodo == numInodosActivos) {
				printf ("No se encuentra el inodo requerido\n");
				return -1;
			}

			int versiones;
			int numVersiones = sizeof(inodos[numInodo].versiones)/sizeof(Version);
			int j=0;
			while (j<numVersiones){
				if (inodos[numInodo].versiones[j].version != 0) versiones++;
				j++;
			}

			int posVersion = buscarPosVersion (numInodo, version);
			if (posVersion == -1) return -1;
			
			// Si solo hay una versión del fichero
			if(versiones == 0) {

				// Obtenemos el bloque a borrar
				int bloqueABorrar = inodos[numInodo].versiones[posVersion].bloque;

				// Nos posicionamos sobre el bloque
				if (lseek (descDisc, (bloqueABorrar+superbloque.comienzoBloquesDatos)*TAMBLOQUE, SEEK_SET) == -1) {
					printf ("Error al establecer la posición al principio \n");
					return -1;
				}

				// Vaciamos el bloque
				char *vacio = malloc (TAMBLOQUE);
				int i;
				for(i=0; i<TAMBLOQUE; i++){
					vacio[i]='0';
				}	
				if (write(descDisc, vacio, TAMBLOQUE)==-1)  {
					printf("Error al leer de disco - %s\n", strerror(errno));
					return -1;
				}
				free (vacio);

				// Marcamos el bloque como libre
				bmap[bloqueABorrar] = '1';
				
				// Borramos el inodo y la versión
				memset(inodos[numInodo].inodo, '0', TAMNOMBRE);
				inodos[numInodo].vActual = 0;
				inodos[numInodo].vUltima = 0;
				inicializarVersion (numInodo, posVersion);

				// Ordenamos el array de inodos y sus arrays de versiones
				int v = numInodo+1;
				int t = 0;
				int numVersiones = sizeof(inodos[v].versiones)/sizeof(Version);
				while (v < numInodosActivos){
					strcpy (inodos[v-1].inodo, inodos[v].inodo);
					inodos[v-1].vActual = inodos[v].vActual;
					inodos[v-1].vUltima = inodos[v].vUltima;
					while (t < numVersiones){
						inodos[v-1].versiones[t].version = inodos[v].versiones[t].version;
						inodos[v-1].versiones[t].tamano = inodos[v].versiones[t].tamano;
						inodos[v-1].versiones[t].puntero = inodos[v].versiones[t].puntero;
						inodos[v-1].versiones[t].bloque = inodos[v].versiones[t].bloque;
						t++;
					}
					v++;
				}

				// Borramos el último inodo de los anteriores para que no esté duplicado
				t = 0;
				memset(inodos[v].inodo, '0', TAMNOMBRE);
				inodos[v].vActual = 0;
				inodos[v].vUltima = 0;
				while (t < numVersiones){
					inicializarVersion (v, t);
					t++;
				}	

				// Disminuimos el número de inodos activos y lo marcamos como libre
				numInodosActivos--;
				imap[numInodosActivos] = '1';
				return 0;
			}
			// Si hay más de una versión
			else {

				// Obtenemos el numero de versiones
				int numVersiones = sizeof(inodos[numInodo].versiones)/sizeof(Version);

				// Obtenemos el bloque a borrar
				int bloqueABorrar = inodos[numInodo].versiones[numInodo].bloque;

				// Nos posicionamos en el bloque a borrar y lo borramos
				if (lseek (descDisc, (bloqueABorrar+superbloque.comienzoBloquesDatos)*TAMBLOQUE, SEEK_SET) == -1) {
					printf ("Error al establecer la posición al principio \n");
					return -1;
				}
				char *vacio = malloc (TAMBLOQUE);
				int i;
				for(i=0; i<TAMBLOQUE; i++){
					vacio[i]='0';
				}
				if (write(descDisc, vacio, TAMBLOQUE)==-1)  {
					printf("Error al leer de descDisc - %s\n", strerror(errno));
					return -1;
				}
				free (vacio);

				// Actualizamos el bmap
				bmap[bloqueABorrar] = '1';

				// Borramos la versión actual
				inicializarVersion (numInodo, posVersion);

				// Actualizamos todas las versiones
				int t = posVersion+1;
				while (t < numVersiones) {
					inodos[numInodo].versiones[t-1].version = inodos[numInodo].versiones[t].version;

					inodos[numInodo].versiones[t-1].tamano = inodos[numInodo].versiones[t].tamano;
					inodos[numInodo].versiones[t-1].puntero = inodos[numInodo].versiones[t].puntero;
					inodos[numInodo].versiones[t-1].bloque = inodos[numInodo].versiones[t].bloque;
					t++;
				}
				inicializarVersion (numInodo, t-1);

				// Si se trata de la versión última o de la versión actual, se actualizan las mismas 
				int ultimaVersion;
				int anteriorVersion = 0;
				int mayorVersion = 0;

				for (ultimaVersion=0; ultimaVersion<numVersiones; ultimaVersion++){
					if (inodos[numInodo].versiones[ultimaVersion].version > anteriorVersion) {
						mayorVersion = inodos[numInodo].versiones[ultimaVersion].version;
					}
					anteriorVersion = inodos[numInodo].versiones[ultimaVersion].version;
				}					
				inodos[numInodo].vUltima = mayorVersion;

				if (inodos[numInodo].vActual == version) {
					inodos[numInodo].vActual = inodos[numInodo].vUltima;
				}

				// Devolvemos la nueva versión vigente del fichero
				return inodos[numInodo].vActual;
			}
		}
	}
	return -1;	
}

// Devuelve el inodo asociado a un descriptor
int buscarInodo (int descriptor) {

	// i = Posición en el array de Descriptores-Inodos
	int i;
	for (i = 0; i<superbloque.numInodos; i++){
		if (descsInodos[i].descriptor == descriptor) break;
	}
	if (i == superbloque.numInodos) {
		printf("Se ha intentado buscar un fichero que no esta abierto\n");
		return -1;
	}
	
	// j = Posición en el array de Inodos
	int j;
	for (j = 0; j<superbloque.numInodos; j++){
		if (strcmp (inodos[j].inodo, descsInodos[i].inodo) == 0) break;
	}
	if (j == superbloque.numInodos) {
		printf("No se ha encontrado el inodo correspondiente con el fichero\n");
		return -1;
	}
	return j;
}

// Elimina del inodo i la versión j 
int inicializarVersion (int i, int j){
	inodos[i].versiones[j].version = 0;
	inodos[i].versiones[j].puntero = 0;
	inodos[i].versiones[j].tamano = 0;
	inodos[i].versiones[j].bloque = 501;
	return 0;
}

// Busca la posición en el array de versiones de la version dada
int buscarPosVersion (int numInodo, int version) {
	int i;
	for (i=0; i<superbloque.numInodos; i++){
		if (inodos[numInodo].versiones[i].version == version) break;
	}
	if (i == superbloque.numInodos) {
		printf("No se ha encontrado la posición de la versión dada\n");
		return -1;
	}
	return i;
}