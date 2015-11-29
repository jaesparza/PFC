/*
 * main.c
 *
 *  Created on: 05-ago-2009
 *      Author: ja
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "modbus.h"
#include "serial.h"

#define _DEBUG // Macro de debug
#define _LOOP
#define _WRITE_OP
#define _READ_OP

#define TIMEOUT 200000
#define DEVICE "/dev/ttyS2"

int main()
{

	int fd = 0;

#ifdef _READ_OP
	int partialRead = 0;

	// Declaración de paquetes modbus a utilizar
	sModBus readPacket;
#endif

#ifdef _WRITE_OP
	sModBus writePacket;
#endif

	sModBus packetRX; // Paquete para la recepción y tratamiento de respuestas

	fd = serial_open(DEVICE, 9600); // Inicio de la comunicación serie

	int state = 0; // Estado de inicio
	int retries = 0; // Reintentos en el envío de un paquete

	int nextBlock = 0;

#ifdef _READ_OP
	int readCount = 0;
#endif

	int mbSlave = 100;
	int mbWord = 0;
	int mbNWords = 30;

	int var=0;

	int remoteStations[10]; // Se define el número de estaciones a refrescar
	remoteStations[0] = 100;
	remoteStations[1] = 110;
	remoteStations[2] = 120;
	int stationNumber = 2; // Índice de la última posición ocupada en el array de estaciones

	int getNextStation = 0;
	int n=0;

	int performWriteOperations = 0;


#ifdef _LOOP

	int test = 0;
	while(1)
	{
#endif

		++test;
		printf("\n ###### %d ", test);
		switch (state)
		{


		case 0: // Definición de la operación modbus a realizar
		{

			//performWriteOperations = checkWriteOperations();
			if (performWriteOperations == 0)
			{

				mbSlave = remoteStations[n];

				if (getNextStation == 1)
				{
					nextBlock=0;

					if (n <= stationNumber -1)
					{
						++n;
						mbSlave = remoteStations[n];
					}
					else
					{
						printf("\nComenzamos con la primera estación definida en el array");
						n=0;
						mbSlave = remoteStations[n];
					}


					printf("\n############################################### Tratando la estación número %d", mbSlave);
					getNextStation = 0;
				}

				if (nextBlock >= 7)
				{
					nextBlock =0;
					mbWord = 448;
					mbNWords = 42;
					scanf("%d",&var);
					getNextStation = 1;
				}
				else
				{
					scanf("%d",&var);
					mbWord = nextBlock*64;
					mbNWords = 64;
					printf("\nDefinición de la nueva operación bloque de datos a leer %d; palabra inicio %d", nextBlock, nextBlock*64);
				}


				++nextBlock;
#ifdef _READ_OP
				state = 10; // Estado de transición
#endif
			}
#ifdef _WRITE_OP
			//state = 20;
#endif
			break;

		}

#ifdef _READ_OP
		// -------------------------------------------------------------------------- PETICIÓN MODBUS DE LECTURA DE DATOS
		// Petición modbus de lectura de datos
		case 10:
		{
			printf("\n------------------------------------------------ ESTADO 10");

			readPacket = assembleReadPacket(mbSlave, mbWord, mbNWords, readPacket);
			readPacket = reversePacket(&readPacket);

#ifdef _DEBUG
			printf("\nPaquete modbus de lectura creado correctamente");
#endif
			retries = 0;
			state = 11;
			break;

		}

		// Envía la trama de datos de petición de lectura
		case 11:
		{

			printf("\n------------------------------------------------ ESTADO 11");
			retries++;

#ifdef _DEBUG
			printReadPacket(readPacket);
			sendReadPacket(fd, readPacket);
#endif

			state = 12;

#ifdef _DEBUG
			printf("\nPaquete modbus enviado correctamente (envio número: %d)",
					retries);
#endif

			break;
		}

		// Comprobación de la respuesta del esclavo
		case 12:
		{

			printf("\n------------------------------------------------ ESTADO 12");
			//sleep(1);

#ifdef _DEBUG
			printf("\nComprobación de la respuesta del esclavo\n");
#endif

			partialRead = 0;
			readCount = 0;
			//		unsigned char * buff = malloc(sizeof(unsigned char)
			//				* ((readPacket.NRegistros.Word * 4) + 10));
			//
			//
			//		unsigned char * partialData = malloc(sizeof(unsigned char)
			//				* ((readPacket.NRegistros.Word * 4) + 10));

			unsigned char buff[1000];
			unsigned char partialData[1000];
			int init = 0;
			for(init = 0;init<1000;init++)
			{
				buff[init] = 0;
				partialData[init] = 0;
			}

			int counter = 0;

			printf("\n\\\\\\\\\\\\\\\\Se van a recibir %d palabras", mbNWords);
			int lenght = (mbNWords * 2) + 5;

			if (serialRead(fd,buff,partialData,lenght,TIMEOUT)<0)
			{
				printf("\nTimeout en la recepción de los datos\n");
				state = 11; // Camio al estado de reenvio de la petición
				break;
			}



			// Reconstrucción del paquete
			packetRX.SlaveAddress = buff[0];
			packetRX.Operacion = buff[1];
			packetRX.NBytes = buff[2];

			for (counter = 0; counter < lenght - 3; counter++)
			{
				packetRX.Dato[counter].Word = (unsigned short) buff[counter + 3];
				//printf("\nDato almacenado en el nuevo paquete %d", packetRX.Dato[counter].Word);
			}

			packetRX.CRC.Bytes.ByteLo = buff[mbNWords * 2 + 3];
			packetRX.CRC.Bytes.ByteHi = buff[mbNWords * 2 + 4];

			printReadResponse(packetRX);

			printf("\nSe procede a actualizar los datos del mapeado");
			fflush(stdout);
			updateMap(packetRX,mbWord);
			printf("\nMapeado actualizado");
			fflush(stdout);

			int fullCRC = crc16(buff, lenght - 2);

			int crcLowCalculated = fullCRC / 1000; // Obtención de los dígitos correspondientes
			int crcHighCalculated = fullCRC % 1000;

#ifdef _DEBUG
			printf("\nSegundo CRC calculado High %d, Low %d", crcHighCalculated,
					crcLowCalculated);
#endif

			if ((crcLowCalculated == packetRX.CRC.Bytes.ByteLo)
					&& (crcHighCalculated == packetRX.CRC.Bytes.ByteHi)) // CRC coincide, respuesta correcta
			{
				printf("\nRespuesta correcta");
			}
			else // Respuesta corrupta
			{
				// Es necesario el reenvio de la petición, respuesta recibida corrupta
				state = 10;
				break;
			}

			printf("\n");
			state = 0;
			//		free(buff);
			//		free(partialData);
			break;

		}

#endif

#ifdef _WRITE_OP
		// -------------------------------------------------------------------------- PETICIÓN MODBUS DE ESCRITURA DE DATOS
		case 20: // Petición modbus de escritura de datos
		{
			int i;

			retries = 0;

			int dataOut[10];
			for (i = 0; i < 10; i++)
			{
				dataOut[i] = i;
			}

			writePacket = assembleWritePacket(10, 1000, 2, 4, dataOut, writePacket);
			writePacket = reversePacket(&writePacket);

			state = 21;
			break;
		}

		case 21: // Envío de la petición de escritura de datos
		{
			retries++;
			sendWritePacket(fd, writePacket);
			printf("\nPaquete modbus enviado correctamente (envio número: %d)", retries);
			state = 22;
			break;
		}

		case 22: // Comprobación de la respuesta del esclavo
		{

			unsigned char * buff = malloc(sizeof(unsigned char) * 8);
			unsigned char * partialData = malloc(sizeof (unsigned char) * 8);

			if (serialRead(fd,buff,partialData,8,TIMEOUT) < 0)
			{
				printf("\nTimeout en la recepción de los datos\n");
				state = 21; // Camio al estado de reenvio de la petición
				break;
			}


			//read(fd, buff, 8);
			int i = 0;
			for (i = 0; i < 8; i++)
			{
				printf("\nDatos leidos en el buffer %d", buff[i]);
			}
			printf("\n");

			packetRX.SlaveAddress = buff[0];
			packetRX.Operacion = buff[1];
			packetRX.PalabraInicio.Word = (unsigned short) buff[2] * 256 + (unsigned short) buff[3];
			//		packetRX.PalabraInicio.Bytes.ByteHi = buff[2];
			//		packetRX.PalabraInicio.Bytes.ByteHi = buff[3];
			packetRX.NBytes = (unsigned short) buff[4] * 100 + (unsigned short) buff[5];
			packetRX.CRC.Bytes.ByteLo = (unsigned short) buff[6];
			packetRX.CRC.Bytes.ByteHi = (unsigned short) buff[7];

			printf("\nNbytes: %d", packetRX.NBytes);
			printf("\nPalabra de inicio: %d", packetRX.PalabraInicio.Word);
			printf("\nPalabra de inicio High:%d Low:%d", packetRX.PalabraInicio.Bytes.ByteHi, packetRX.PalabraInicio.Bytes.ByteLo);


			int fullCRC = crc16(buff, 6);
			int crcLowCalculated = fullCRC / 1000;
			int crcHighCalculated = fullCRC % 1000;

			printf("\nCRC contenido en el paquete High %d, Low %d", packetRX.CRC.Bytes.ByteHi, packetRX.CRC.Bytes.ByteLo);
			printf("\nSegundo CRC calculado High %d, Low %d", crcHighCalculated,
					crcLowCalculated);


			if ((crcLowCalculated == packetRX.CRC.Bytes.ByteLo) && (crcHighCalculated == packetRX.CRC.Bytes.ByteHi)) // CRC coincide, respuesta correcta
			{
				printf("\nRespuesta correcta");
			}
			else // CRC no coincide, error en la recepción de los datos
			{
				// Es necesario el reenvio de la petición, respuesta recibida corrupta.
				printf("\nDatos recibidos corruptos, necesario el reenvio de la petición. #####");
				state = 21;
				break;
			}

			state = 0; // actualización de estado
			break;

		}


		default:
		{
			printf("\n #################### Estado erróneo");
			break;
		}

#endif

		}

#ifdef _LOOP
	}
#endif

	return 0;
}
