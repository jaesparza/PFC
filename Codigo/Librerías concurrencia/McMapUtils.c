#include "McMapUtils.h"

//! Crear el fichero de un mapeado
/*!
 * Se crea un fichero de tamaño MAP_SIZE lleno de ceros. En caso de que ya exista,
 * se rellena con ceros.
 *
 * \param mapNum Identificador del mapeado solicitado
 * \return 0 si se realiza correctamente
 */
int createMapFile(int mapNum) {
	FILE * fich;
	int i, error =0;
	map r;

	//char name[10];
	char route[50];

	getMapFileName(mapNum, route);

#ifdef _DEBUG
	printf("%s", route);

	char t;
	scanf("%c", &t);
#endif

	if ((fich = fopen(route, "wb")) == NULL) {
		fprintf(stderr, "Error en la apertura del fichero para escritura\n");
		error=1;
	}

	else {
		for (i=0; i<490; i++) {
			r.num[i] = i;
#ifdef _DEBUG
			printf("Dato escrito en el fichero \n");
#endif
		}
		error = fwrite(&r.num, sizeof(short), MAP_WORDS, fich);

#ifdef _DEBUG
		printf("Resultado en el fichero %d\n", error);
#endif
		fclose(fich);
	}

	if (error==1)
		return -1;
	else
		return 0;
}

//! Devolver el puntero a los datos de un mapeado
/*!
 * En función del número de mapeado pedido, se creará el mmap correspondiente
 * \param mapNum Identificador del mapeado solicitado
 * \return Puntero a los datos del mapeado. NULL si hay algún error o si no existe.
 */
short * getMapPointer(int mapNum) {
	int fd;
	short * ptr;

	char route[50];

	getMapFileName(mapNum, route);

	if ((fd = open(route, O_RDWR)) == -1)
		return NULL; // caso de que suceda un error en la apertura del fichero

	// Se crea el mmap con el tamaño del mapeado
	if ((ptr= (short *) mmap(0, MAP_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED)
	{
		close(fd);
		return NULL; // se produce un error en el mapeo de los datos
	}

	// Ya se puede cerrar el fichero
	close(fd);

	return ptr;
}

//! Liberar un mapeado
/*!
 * En función del número de mapeado pedido, se liberará el mmap correspondiente
 * \param mapNum Identificador del mapeado solicitado
 * \return 0 si se consigue liberar correctamente
 */
int releaseMapPointer(short * mapPtr) {

	return munmap(mapPtr, MAP_SIZE);

}

//! Obtiene la ruta correspondiente al fichero del mapeado en el sistema de ficheros
/*!
 * Los cambios se guardan directamente en la cadena pasada como parámetro
 * \param mapNum Identificador del mapeado solicitado
 * \param route String donde se guarda la ruta del mapeado que nos interesa
 */
void getMapFileName(int mapNum, char * path) {

	char name[15];

	sprintf(name, "%d", mapNum); // Conversión a strign del número de mapeado

	// Copio la primera parte de la ruta
	strcpy(path, MAP_PATH);
	// Añado el número del mapeado
	strcat(path, name);
	// Añado la extensión
	strcat(path, MAP_FILE_EXT);

}
