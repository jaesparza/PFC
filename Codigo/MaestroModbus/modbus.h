/*
 * modbus.h
 *
 *  Created on: 05-ago-2009
 *      Author: ja
 */

#ifndef MODBUS_H_
#define MODBUS_H_


#include "serial.h"

// Definiciones del tama�o de la trama RPIP
#define NDatosMax			128

//*******************************************************************************
//*          DEFINICIONES DE LA COMUNICACI�N MODBUS								*
//*******************************************************************************

// Códigos de operación modbus
#define OP_LECTURA_REGISTROS	0x03	// Lectura n registros
#define OP_ESCRIBE_REGISTROS	0x10	// Escritura n registros
#define ERROR_LECTURA_REGISTROS	0x83	// Error Lectura n registros
#define ERROR_ESCRIBE_REGISTROS	0x90	// Error Escritura n registros

typedef union
{
	unsigned short			Word;
	struct
	{
		unsigned char		ByteLo;
		unsigned char		ByteHi;
	} Bytes;

	struct
	{
		unsigned char		bit0	: 1;
		unsigned char		bit1	: 1;
		unsigned char		bit2	: 1;
		unsigned char		bit3	: 1;
		unsigned char		bit4	: 1;
		unsigned char		bit5	: 1;
		unsigned char		bit6	: 1;
		unsigned char		bit7	: 1;
		unsigned char		bit8	: 1;
		unsigned char		bit9	: 1;
		unsigned char		bit10	: 1;
		unsigned char		bit11	: 1;
		unsigned char		bit12	: 1;
		unsigned char		bit13	: 1;
		unsigned char		bit14	: 1;
		unsigned char		bit15	: 1;
	} Bits;
} sWord;

typedef union
{
	unsigned long			DWord;
	struct
	{
		sWord				WordLo;
		sWord				WordHi;
	} Words;
} sDWord;

typedef enum {REPOSO, ENVIO_DATOS, ESPERA_ACK, ACK_OK} sEstadoTX;
typedef enum {SLV_ADD, OPERACION, ERROR, PAL_INICIO_HI, PAL_INICIO_LO, NREG_HI, NREG_LO, NBYTES, DATOS_HI, DATOS_LO, CRC_LO, CRC_HI} sEstadoRX;
typedef enum {ESPERANDO, RECEPCION} sEstadoCOM;

typedef struct
{
	unsigned char   			SlaveAddress;
	unsigned char   			Operacion;
	unsigned char				ErrorCode;
	sWord						PalabraInicio;
	sWord		 				NRegistros;
	unsigned char				NBytes;
	sWord     					Dato[NDatosMax];
	unsigned char				Idata;
	sWord     					CRC;
} sModBus;


sWord reverseWord(sWord inverted);

//extern unsigned char 			TikTimeoutCom;		// Temporizador de timeout de las comunicaciones x 100 ms


unsigned short CRC_16 (unsigned short CRC, unsigned short Cod);
unsigned short CalculaCRC (sModBus ModBus);

sModBus  reversePacket(sModBus * inverted);

sModBus assembleReadPacket (unsigned char slave , unsigned short startWord , unsigned char registerNum,sModBus readPacket);

char * serializeReadPacket(sModBus readPacket,char * serializedPacket);

void printReadResponse(sModBus readResponse);

void printReadPacket(sModBus readResponse);

int sendWritePacket(int fd, sModBus sendIt);

int sendReadPacket(int fd, sModBus sendIt);

sModBus assembleWritePacket(unsigned char slave, unsigned short startWord,unsigned short registerNum, unsigned char NBytes, int * data ,sModBus writePacket);

unsigned int crc16(unsigned char* modbusframe,int Length);

int updateMap(sModBus packetRX,int startingWord);

#endif /* MODBUS_H_ */



