#include "McSemaphoreUtils.h"


//! Crear un semáforo para un mapeado
/*!
 * Crear el semáforo de lectura/escritura necesario para controlar el mapeado de identificador mapNum
 * 
 * \param mapNum Identificador del mapeado para el que hay que crear el semáforo
 * \return 0 si se realiza correctamente, -1 si hay algún errro
 */
int createSemaphore(int mapNum) {
	int id;

	// TODO: Implemetar la creación del semáforo/s

	argument parameter;

	parameter.val = 1;

	//int map = locateMap(mapNum);

	id = semget(getKey(mapNum), SEMSET, 0666 | IPC_CREAT);

#ifdef _DEBUG
	printf("El id del semaforo es %d", id);
#endif

	if (id < 0) {
		fprintf(stderr, "No se pudo obtener el semaforo\n");
		return (-1);
	}

	// Inicialización del semáforo con el valor 1 (recurso disponible por defecto)
	int i=0;
	// número del semáforo a modificar contenido en el segundo argumento
	// opción de set nuevo valor en macro SETVAL
	for (i=0; i<(SEMSET/2); i++) {
		if (semctl(id, i, SETVAL, parameter) < 0) {
			fprintf( stderr, "No se pudo fijar el valor del semaforo.\n");
		}

#ifdef _DEBUG

		fflush(stdout); // Terminar la impresión de los datos contenidos en el buffer de pantalla
		else {
			fflush(stdout);
			fprintf(stderr, "Semaforo %d iniciado a 1.\n", getKey(mapNum));
		}
#endif

	}

	parameter.val = QUEUESIZE;

	for (i=SEMSET/2; i<(SEMSET); i++) {
		if (semctl(id, i, SETVAL, parameter) < 0) {
			fprintf( stderr, "No se pudo fijar el valor del semaforo.\n");
			return -1;
		}

#ifdef _DEBUG
		else {
			fflush(stdout);
			fprintf(stderr, "Semaforo %d iniciado a 1.\n", getKey(mapNum));
		}
#endif

	}

	for (i=0; i<SEMSET; i++) {
		if (semctl(id, i, GETVAL, parameter) < 0) {
			fflush(stdout);
			fprintf( stderr, "No se pudo obtener el valor del semaforo.\n");
			return -1;
		}

#ifdef _DEBUG		
		else {
			fprintf(stderr, "Semaforo %d con valor %d.\n", getKey(mapNum), argument.val);
			fflush(stdout);
		}
#endif

	}

	return 0;
}

//! Bloquear un semáforo de un mapeado
/*!
 * Bloquear el semáforo de lectura/escritura necesario para controlar el mapeado de identificador mapNum
 * 
 * \param lockMode Modo de bloqueo de los mapeados: READ_LOCK para bloqueos de lectura o WRITE_LOCK para bloqueos de lectura/escritura
 * \param mapNum Identificador del mapeado para el que hay que bloquear el semáforo
 * \return 0 si se realiza correctamente, -1 si hay algún errro
 */
int lockSemaphore(int lockMode, int mapNum) {
	int id; // id del semáforo

	struct sembuf operations[1];
	int retval;

	// Dependiendo de que número de estación hay que buscar la clave en una
	// posición u otra del array

	// Obtención del id del semáforo haciendo uso de la clave externa
	id = semget(getKey(mapNum), SEMSET, 0666);
	if (id < 0)
	/* Semaphore does not exist. */
	{
		fprintf(stderr, "Semaforo no encontrado.\n");
		return -1;
	}

	operations[0].sem_op = -1; // se decrementa el valor del semáforo
	operations[0].sem_flg = 0;

	// Bloqueo de lectura/escritura
	if (lockMode == WRITE_LOCK) {
		operations[0].sem_num = semPositionRW(mapNum);
	}

	// Bloqueo de lectura
	else {
		// creación del buffer que contiene las operaciones a realizar
		operations[0].sem_num = semPositionR(mapNum); // posición del mapeado a leer
	}

	retval = semop(id, operations, 1); // se realiza la operación de bloqueo

	if (retval < 0) // control de errores 
	{
		fprintf( stderr, "Error: mapeado NO bloqueado.\n");
		return -1;
	}

	return 0;
}

//! Liberar un semáforo de un mapeado
/*!
 * Liberar el semáforo de lectura/escritura necesario para controlar el mapeado de identificador mapNum
 * 
 * \param lockMode Modo de bloqueo de los mapeados: READ_LOCK para bloqueos de lectura o WRITE_LOCK para bloqueos de lectura/escritura
 * \param mapNum Identificador del mapeado para el que hay que liberar el semáforo
 * \return 0 si se realiza correctamente, -1 si hay algún error
 */
int unlockSemaphore(int lockMode, int mapNum) {
	int id; // id del semáforo

	struct sembuf operations[1];
	int retval;

	// Obtención del id del semáforo haciendo uso de la clave externa
	id = semget(getKey(mapNum), SEMSET, 0666);

	if (id < 0) {
		fprintf(stderr, "Semaforo no encontrado.\n");
		return -1;
	}

	operations[0].sem_op = 1; // se decrementa el valor del semáforo
	operations[0].sem_flg = 0;

	// Bloqueo de lectura/escritura
	if (lockMode == WRITE_LOCK) {

		operations[0].sem_num = semPositionRW(mapNum);

	}

	// Bloqueo de lectura
	else {
		// creación del buffer que contiene las operaciones a realizar
		operations[0].sem_num = semPositionR(mapNum); // posición del mapeado a leer

	}

	retval = semop(id, operations, 1); // se realiza la operación de bloqueo

	if (retval < 0) {
		fprintf( stderr, "Error: mapeado NO desbloqueado.\n");
	}

	// TODO: devolver el error correspondiente
	return retval;
}

//! Bloquear una serie de mapeados para lectura/escritura
/*!
 * Obtener los semaforos de lectura/escritura de los mapeados pedidos. Para evitar
 * posibles deadlock, se harán las peticiones en orden creciente del número de mapeado.
 * 
 * \param lockMode Modo de bloqueo de los mapeados: READ_LOCK para bloqueos de lectura o WRITE_LOCK para bloqueos de lectura/escritura
 * \param mapCount Número de mapeados a bloquear
 * \param mapSet puntero al primer elemento del array que contiene los mapeados a bloquear
 * \return En caso de error se devolverá un número negativo
 */
int getLock(int lockMode, int mapCount, int * mapSet) {

	// La función recibe como parámetro un array ya ordenado con las estaciones
	// para las que hace falta un bloqueo

	int i; // Contador
	int error=0;

	sortArray(mapSet, mapCount); // Ordenación del array en orden creciente

	int j=0;

	// se recorre el array que contiene los mapeados a bloquear y se procedea ello

	for (i=0; i<mapCount; i++) {

		if (lockMode == WRITE_LOCK) // en el caso de que estemos solicitando un bloqueo de escritura
		{
			error = lockSemaphore(lockMode, mapSet[i]);
			if (error<0)
				break;
			// Una vez obtenido el semáforo de escritura intentamos adquirir todos los puestos disponibles
			// para el semáforo de lectura


			for (j=0; j<QUEUESIZE; j++) {

				error = lockSemaphore(READ_LOCK,mapSet[i]);
				if (error <0)
					break;
			}

			if (error <0)
				break;

		} else if (lockMode == READ_LOCK) // en el caso de que estemos solicitando un bloqueo de lectura
		{
			lockSemaphore(lockMode, mapSet[i]);
		}

		/* en el caso de que se produzca algún error se liberan todos los mapeados bloqueados hasta el momento
		 en el que ha sucedido */
		if (error <0) {
			releaseLock(lockMode, i, mapSet);
			return -1;
		}

	}

	return 0;
}

//! Liberar una serie de mapeados para lectura/escritura
/*!
 * \param lockMode Modo de bloqueo de los mapeados: READ_LOCK para bloqueos de lectura o WRITE_LOCK para bloqueos de lectura/escritura
 * \param mapCount Número de mapeados a liberar
 * \param mapSet Puntero al primer elemento del array de mapeados con los que vamos a trabajar
 * \return En caso de error se devolverá un número negativo
 */
int releaseLock(int lockMode, int mapCount, int * mapSet) {

	// La función recibe como parámetro un array ya ordenado con las estaciones
	// para las que hace falta un bloqueo

	int i, j; // Contador
	int retval=0;
	//int errorIndex = -1; // Índice de error

	sortArray(mapSet, mapCount); // Ordenación del array en orden creciente

	for (i=0; i<mapCount; i++) {

		if (lockMode == WRITE_LOCK) {
			retval = unlockSemaphore(lockMode, mapSet[i]);

			for (j=0; j<QUEUESIZE; j++) {
				retval = unlockSemaphore(READ_LOCK,mapSet[i]);
			}

			//releaseLock(lockMode,i,mapSet); // En caso de que se produzca un error se liberan los mapeados bloqueados
		} else if (lockMode == READ_LOCK) // en el caso de que estemos solicitando un bloqueo de lectura
		{
			retval = unlockSemaphore(lockMode, mapSet[i]);
			//releaseLock(lockMode,i,mapSet); // En caso de que se produzca un error se liberan los mapeados bloqueados
		}
	}

	return retval;
}

//! Eliminación de un conjunto de semáforos cuando ya no sean necesarios
/*
 *  \param mapNum Número correspondiente a un semáforo contenido en el conjunto de semáforos
 *  \return En caso de error devuelve un número negativo
 */
int freeSemaphore(int mapNum) {

	int id=0;

	id = semget(getKey(mapNum), SEMSET, 0666);

#ifdef _DEBUG
	printf("ID del semaforo a eliminar %d", id);
	printf("\nKey obtenida: %d", getKey(mapNum));
#endif

	if (id < 0) {
		fprintf(stderr, "No se pudo obtener el semaforo\n");
		return id;
	}

	id = semctl(id, 0, IPC_RMID);

	if (id < 0) {
		fprintf(stderr, "No se pudo eliminar el semaforo\n");
		return id;
	}

	return 0;
}

//! Indica el conjunto de semáforos que contiene los semáforos de un mapeado concreto.
/*! Función que asocia una estación a una posición en el depósito de llaves que permite la obtención de los semáforos
 * 
 * \param mapNum número de mapeado, correspondiente a una estación
 * \return Entero que indica la posición en el array de llaves de la key correspondiente al mapeado 
 * */
int locateMap(int mapNum) {
	 
	if (mapNum < 256)
	 	return (mapNum / (SEMSET / 2));
	else
	  	return 9;
}

//! Localiza semaforo RW
/*! Función que localiza el semáforo de lectura escritura de un mapeado concreto en el array de semáforos
 * 
 * \param mapNum número de mapeado, correspondiente a una estación 
 * \return Posición del semáforo dentro del conjunto de semáforos
 * */
int semPositionRW(int mapNum) {
	return (mapNum % (SEMSET / 2)); // Localización del semáforo de lectura/escritura
}

/*! Función que localiza el semáforo de lectura de un mapeado concreto en el array de semáforos
 * 
 * \param mapNum número de mapeado, correspondiente a una estación 
 * \return Posición del semáforo dentro del conjunto de semáforos
 * */
int semPositionR(int mapNum) {
	return (mapNum % (SEMSET / 2)) + (SEMSET / 2); // Localización del semáforo de lectura
}

//! Función de ordenación de un array de enteros utilizando el método de la burbuja
/*! 
 * \param array Puntero al primer elemento contenido en el array de enteros 
 * \param elements Número de elementos contenidos en el array
 * \return 0 al terminar la ordenación
 * */
int sortArray(int* array, int elements) {
	int i=0;
	int j;
	int aux_elem;
	int movements;

	movements = 0;

	for (i = 0; i < elements - 1; i++) {
		for (j = 1; j < elements; j++) {
			if (array[j] < array[j-1]) { // si el elemento anterior es mayor, hacemos el cambio
				aux_elem = array[j];
				array[j] = array[j-1];
				array[j-1] = aux_elem;
				movements++;
			}
		}
	}

	return 0; // ordenación completada
}

//! Función para la obtención de las llaves
/*! Obtiene la llave asociada al conjunto de semáforos que contiene los semáforos correspondientes
 *! a un mapeado concreto pasado como parámentro
 * 
 * \param mapNum número de mapeado del que queremos conocer la llave para acceder a los semáforos correspondientes
 * \return key_t Devuelve una key en caso de que se haya obtenido adecuadamente o número negativo en caso de error
 * */
key_t getKey(int mapNum) {

	key_t semkey;
	
	char tmp[6];
	
	sprintf(tmp, "%d", locateMap(mapNum));
	strcpy(tmp,".key");
	
	if ((semkey = ftok(tmp,locateMap(mapNum)) == (key_t) -1))
		return -1;

	return semkey;

}

//! Obtiene el valor del semáforo especificado
/*! 
 * \param mapNum número de mapeado
 * \return valor del semáforo. En caso de error se devolverá un número negativo
 * */
int getSemValue(int lockMode, int mapNum) {

	argument parameter;

	int value=0;
	int semnum = 0;
	int semid;
	
	if (lockMode == WRITE_LOCK) {
		semnum = semPositionRW(mapNum);

	} else if (lockMode == READ_LOCK) {

		semnum = semPositionR(mapNum);
	}


	if ((semid = semget(getKey(mapNum), SEMSET, 0666)) < 0)
		return -1;
	
	if ((value = semctl(semid, semnum, GETVAL, parameter)) <0 )
		return -1;

	return value;
}

//! Crea el conjunto de llaves necesario para la identificación de los semáforos
/*! La función crea los ficheros de extensión .key, necesarios para la identificación
 *! de los distintos conjuntos de semáforos que se utilizan en la implementación.
 * \return 0 en caso de ejecución satisfactoria. En caso de error se devolverá un número negativo
 * */
int createKeys()
{
	FILE * fich;
	
	int i, error =0;
	
	char tmp[6];

	char file_name[100];
	
	for (i=0;i<4;i++)
	{
		
		strcpy(file_name,KEY_PATH);
		
		sprintf(tmp, "%d", i+1);
		
		strcat(file_name,tmp);
		
		strcat(file_name,KEY_EXT);
		
		if ((fich = fopen(file_name, "wb")) == NULL)
		{
			fprintf(stderr, "Error en la creacion de las llaves\n");
			error=-1;
		}
	}
	
	return error;
}
