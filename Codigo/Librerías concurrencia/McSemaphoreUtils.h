#ifndef MCSEMAPHOREUTILS_H_
#define MCSEMAPHOREUTILS_H_

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

extern void exit();
extern void perror();

#define KEY_PATH "/home/avr32/blockKeys/"
#define KEY_EXT ".key"

//! Define de DEBUG
//#define _DEBUG

//! Bloqueo de lectura/escritura
#define WRITE_LOCK 1

//! Bloqueo de lectura
#define READ_LOCK 0

//! Número de semáforos por conjunto de semáforos
#define SEMSET 128

//! Tamaño de la cola de espera para la obtención de un semáforo de lectura
#define QUEUESIZE 20

typedef union semun {
	int val;
	struct semid_ds *buf;
	ushort * array;
} argument;

//! Indica el conjunto de semáforos que contiene los semáforos de un mapeado concreto.
/*! Función que asocia una estación a una posición en el depósito de llaves que permite la obtención de los semáforos
 *
 * \param mapNum número de mapeado, correspondiente a una estación
 * \return Entero que indica la posición en el array de llaves de la key correspondiente al mapeado
 * */
int locateMap(int mapNum);

/*! Función que localiza el semáforo de lectura de un mapeado concreto en el array de semáforos
 *
 * \param mapNum número de mapeado, correspondiente a una estación
 * \return Posición del semáforo dentro del conjunto de semáforos
 * */
int semPositionR(int mapNum);

//! Localiza semaforo RW
/*! Función que localiza el semáforo de lectura escritura de un mapeado concreto en el array de semáforos
 *
 * \param mapNum número de mapeado, correspondiente a una estación
 * \return Posición del semáforo dentro del conjunto de semáforos
 * */
int semPositionRW(int mapNum);

//! Función de ordenación de un array de enteros utilizando el método de la burbuja
/*!
 * \param array Puntero al primer elemento contenido en el array de enteros
 * \param elements Número de elementos contenidos en el array
 * \return 0 al terminar la ordenación
 * */
int sortArray(int* array, int elements);

//! Crear un semáforo para un mapeado
/*!
 * Crear el semáforo de lectura/escritura necesario para controlar el mapeado de identificador mapNum
 *
 * \param mapNum Identificador del mapeado para el que hay que crear el semáforo
 * \return 0 si se realiza correctamente, -1 si hay algún errro
 */
int createSemaphore(int mapNum);

//! Eliminación de un conjunto de semáforos cuando ya no sean necesarios
/*
 *  \param mapNum Número correspondie//! Crear un semáforo para un mapeado
 */
int freeSemaphore(int mapNum);


//! Bloquear un semáforo de un mapeado
/*!
 * Bloquear el semáforo de lectura/escritura necesario para controlar el mapeado de identificador mapNum
 *
 * \param lockMode Modo de bloqueo de los mapeados: READ_LOCK para bloqueos de lectura o WRITE_LOCK para bloqueos de lectura/escritura
 * \param mapNum Identificador del mapeado para el que hay que bloquear el semáforo
 * \return 0 si se realiza correctamente, -1 si hay algún errro
 */
int lockSemaphore(int lockMode, int mapNum);

//! Liberar un semáforo de un mapeado
/*!
 * Liberar el semáforo de lectura/escritura necesario para controlar el mapeado de identificador mapNum
 *
 * \param lockMode Modo de bloqueo de los mapeados: READ_LOCK para bloqueos de lectura o WRITE_LOCK para bloqueos de lectura/escritura
 * \param mapNum Identificador del mapeado para el que hay que liberar el semáforo
 * \return 0 si se realiza correctamente, -1 si hay algún error
 */
int unlockSemaphore(int lockMode, int mapNum);

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
int getLock(int lockMode, int mapCount, int * mapSet);

//! Liberar una serie de mapeados para lectura/escritura
/*!
 * \param lockMode Modo de bloqueo de los mapeados: READ_LOCK para bloqueos de lectura o WRITE_LOCK para bloqueos de lectura/escritura
 * \param mapCount Número de mapeados a liberar
 * \param mapSet Puntero al primer elemento del array de mapeados con los que vamos a trabajar
 * \return En caso de error se devolverá un número negativo
 */
int releaseLock(int lockMode, int mapCount, int * mapSet);

//! Función para la obtención de las llaves
/*! Obtiene la llave asociada al conjunto de semáforos que contiene los semáforos correspondientes
 *! a un mapeado concreto pasado como parámentro
 *
 * \param mapNum número de mapeado del que queremos conocer la llave para acceder a los semáforos correspondientes
 * \return key_t Devuelve una key en caso de que se haya obtenido adecuadamente o número negativo en caso de error
 * */
key_t getKey(int mapNum);

//! Obtiene el valor del semáforo especificado
/*!
 * \param mapNum número de mapeado
 * \return valor del semáforo. En caso de error se devolverá un número negativo
 * */
int getSemValue(int lockMode, int mapNum);


//! Crea el conjunto de llaves necesario para la identificación de los semáforos
/*! La función crea los ficheros de extensión .key, necesarios para la identificación
 *! de los distintos conjuntos de semáforos que se utilizan en la implementación.
 * \return 0 en caso de ejecución satisfactoria. En caso de error se devolverá un número negativo
 * */
int createKeys();

#endif /*MCSEMAPHOREUTILS_H_*/
