#ifndef MCMAPUTILS_H_
#define MCMAPUTILS_H_

#include "dataDefinitions.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>

#include <string.h>

//#define _DEBUG

#define MAP_WORDS 490
#define MAP_PATH "/home/avr32/memoryMaps/"
#define MAP_FILE_EXT ".bin"

//! Número de palabras del mapeado
#define MAP_WORDS 490

//! Tamaño del mapeado
#define MAP_SIZE MAP_WORDS * sizeof(short)

//! Devolver el puntero a los datos de un mapeado
/*!
 * En función del número de mapeado pedido, se creará el mmap correspondiente
 * \param mapNum Identificador del mapeado solicitado
 * \return Puntero a los datos del mapeado. NULL si hay algún error o si no existe.
 */
short * getMapPointer(int mapNum);

//! Liberar un mapeado
/*!
 * En función del número de mapeado pedido, se liberará el mmap correspondiente
 * \param mapNum Identificador del mapeado solicitado
 * \return 0 si se consigue liberar correctamente
 */
int releaseMapPointer(short * mapPtr);

//! Crear el fichero de un mapeado
/*!
 * Se crea un fichero de tamaño MAP_SIZE lleno de ceros. En caso de que ya exista,
 * se rellena con ceros.
 *
 * \param mapNum Identificador del mapeado solicitado
 * \return 0 si se realiza correctamente
 */
int createMapFile(int mapNum);

//! Obtiene la ruta correspondiente al fichero del mapeado en el sistema de ficheros
/*!
 * Los cambios se guardan directamente en la cadena pasada como parámetro
 * \param mapNum Identificador del mapeado solicitado
 * \param route String donde se guarda la ruta del mapeado que nos interesa
 */
void getMapFileName(int mapNum, char * route);

#endif /*MCMAPUTILS_H_*/
