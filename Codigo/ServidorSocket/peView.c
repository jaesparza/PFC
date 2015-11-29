#include "peView.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "dataDefinitions.h"
#include "McSemaphoreUtils.h"
#include "McMapUtils.h"

socketMBPacket receivePacket(int socketFD, socketMBPacket * packet) {
	mbHeader header;
	mbTail tail;

	printf("\nComienza la recepción de la cabecera");
	receiveHeader(socketFD, &header);
	printf("\nCabecera recibida");


	printf("\nDirección del esclavo %d", header.slaveAddress);
	fflush(stdout);
	printf("\nOperación a realizar %d", header.op);
	fflush(stdout);

	printf("\nRecepción de tail");
	fflush(stdout);
	receiveTail(socketFD, &tail, header);

	printf("\nComienzo de la reconstrucción del paquete");
	fflush(stdout);
	rebuildPakcet(header, tail, packet);
	return *packet;
}

int receiveHeader(int socketFD, mbHeader * header) {

	int res = recv(socketFD, header, sizeof(*header), 0);
	if ( res == -1) {
		perror("Receive");
		fflush(stdout);
		close(socketFD);
		return -1;
	}
	else
	{
		if (res != sizeof(*header))
		{
			printf("\n Error en la recepción de datos, esperados %d bytes, recibidos %d bytes",sizeof(*header), sizeof(*header) - res );
			fflush(stdout);
			close(socketFD);
			return -1;
		}
		printf("\n############################### RECEPCIÓN OK ##############");
	}

	return 0;
}

int receiveTail(int socketFD, mbTail * tail, mbHeader header) {
	if (header.op == 0x3) // Recepción de una petición de lectura
	{
		if (recv(socketFD, tail, sizeof(*tail), 0) == -1) {
			perror("Receive");
			return -1;
		}

		reverseWord(&(tail->readPacketRequest.numWord));
		reverseWord(&(tail)->readPacketRequest.startWord);
		reverseWord(&(tail->readPacketRequest.CRC));

	}

	if (header.op == 0x10) // Recepción de una petición de escritura
	{

		int res = recv(socketFD, &(tail->writePacketRequest.startWord), sizeof(sWord), 0);

		if (res == -1)
		{
			perror("Receive");
			return -1;
		}
		else
		{
			if (res != sizeof(sWord))
			{
				printf("\nError en la recepción de datos, esperados % bytes, recibidos %d", sizeof(sWord),sizeof(sWord)-res);
				fflush(stdout);
				close(socketFD);
				return -1;
			}

		}


		res = recv(socketFD, &(tail->writePacketRequest.numWord), sizeof(sWord), 0);

		if (res == -1)
		{
			perror("Receive");
			return -1;
		}
		else
		{
			if (res != sizeof(sWord))
			{
				printf("\nError en la recepción de datos, esperados % bytes, recibidos %d", sizeof(sWord),sizeof(sWord)-res);
				fflush(stdout);
				close(socketFD);
				return -1;
			}

		}

		res = recv(socketFD, &(tail->writePacketRequest.byteNum), sizeof(unsigned char), 0);


		if (res == -1)
		{
			perror("Receive");
			return -1;
		}
		else
		{
			if (res != sizeof(unsigned char))
			{
				printf("\nError en al recepción de los datos, esperados %d bytes, recibidos %d", sizeof(unsigned char), sizeof(unsigned char)-res);
				fflush(stdout);
				close(socketFD);
				return -1;
			}
			//reverseWord(&(tail->writePacketRequest.byteNum));
		}


		printf("\nSe va a reservar memoria para %d palabras",tail->writePacketRequest.numWord.Word);
		fflush(stdout);

		printf("\nSe van a escribir a partir de %d",tail->writePacketRequest.startWord.Word);
		fflush(stdout);

		printf("\nCRC recibido: Alto %d Bajo %d", tail->writePacketRequest.CRC.Bytes.ByteHi, tail->writePacketRequest.CRC.Bytes.ByteLo );
		fflush(stdout);





		int i = 0;
		for (i = 0; i < tail->writePacketRequest.numWord.Word; i++) {
			recv(socketFD, &(tail)->writePacketRequest.data[i].Bytes.ByteHi,
					sizeof(unsigned char), 0);
			recv(socketFD, &(tail)->writePacketRequest.data[i].Bytes.ByteLo,
					sizeof(unsigned char), 0);
			//printf("\nRecibido %d", tail->writePacketRequest.data[i]);
		}

	}

	return 0;
}

socketMBPacket * buildReadResponse (socketMBPacket incomingPacket, socketMBPacket * responsePacket, sWord * map)
{

	responsePacket->header.op = incomingPacket.header.op;
	responsePacket->header.slaveAddress = incomingPacket.header.slaveAddress;

	if (responsePacket->tail.readPacketResponse.data == NULL)
		printf("\n Error en la reserva de memoria");

	reverseWord(&(incomingPacket).tail.readPacketRequest.numWord);
	reverseWord(&(incomingPacket).tail.readPacketRequest.startWord);

	int startWord = incomingPacket.tail.readPacketRequest.startWord.Word; // Obtención de la palabra de inicio
	int numWords = incomingPacket.tail.readPacketRequest.numWord.Word; // Obtención del número de palabras a leer
	int mapNum = incomingPacket.header.slaveAddress;

	// Se obtienen los datos acediendo al mapeado correspondiente

	getDataArrayFromMap(startWord,numWords,mapNum,responsePacket->tail.readPacketResponse.data);
	//printf("\nError antes de la carga de datos");
	fflush(stdout);

	//printf("\nError tras la carga de datos");
	fflush(stdout);

	//reverseWord(&(incomingPacket).tail.readPacketRequest.numWord);
	responsePacket->tail.readPacketResponse.byteNum = incomingPacket.tail.readPacketRequest.numWord.Word * 2;
	printf("\n ####################### TENGO QUE ENVIAR %d BYTES, %d PALABRAS", responsePacket->tail.readPacketResponse.byteNum,incomingPacket.tail.readPacketRequest.numWord.Word);

	responsePacket->tail.readPacketResponse.CRC.Word = getCRC(*responsePacket,read_response,incomingPacket.tail.readPacketRequest.numWord.Word);

	printf("\nCRC obtenido alto: %d, bajo %d", responsePacket->tail.readPacketResponse.CRC.Bytes.ByteHi,
			responsePacket->tail.readPacketResponse.CRC.Bytes.ByteLo);

	//reverseWord(&(responsePacket->tail.readPacketResponse.CRC));

	//printf("\nError tras invertir las palabras");
	fflush(stdout);

	return responsePacket;
}

void printReadResponse(socketMBPacket packet,int dataLenght)
{


	printf("\n Dirección del esclavo %d", packet.header.slaveAddress);
	printf("\n Operación a realizar %d", packet.header.op);
	printf("\n Numero de bytes a enviar %d", packet.tail.readPacketResponse.byteNum);

	printf("\nDatos contenidos. Formato Byte Alto | Byte bajo #");
	int i=0;
	for(i=0;i<10; i++)
	{
		printf(" %d | %d # ",packet.tail.readPacketResponse.data[i].Bytes.ByteHi,packet.tail.readPacketResponse.data[i].Bytes.ByteLo);
	}

	//datalenght += 1;
	printf("\nCRC contenido en el paquete Alto: %d | Bajo: %d", packet.tail.readPacketResponse.CRC.Bytes.ByteHi, packet.tail.readPacketResponse.CRC.Bytes.ByteLo);
}

unsigned char * serializeReadResponse(unsigned char * dataOut, socketMBPacket * responsePacket, int numWords)
{
	int count = 0;



	dataOut[0] = responsePacket->header.slaveAddress;
	dataOut[1] = responsePacket->header.op;
	dataOut[2] = responsePacket->tail.readPacketResponse.byteNum;

	int shift = 3;

	for(count = 0;count< numWords;count++)
	{
		dataOut[shift+count] = responsePacket->tail.readPacketResponse.data[count].Bytes.ByteHi;
		dataOut[shift+1+count] = responsePacket->tail.readPacketResponse.data[count].Bytes.ByteLo;
		shift = shift + 1;
	}

	shift = (numWords * 2) + 3;

	dataOut[shift] = responsePacket->tail.readPacketResponse.CRC.Bytes.ByteHi;
	dataOut[shift+1] = responsePacket->tail.readPacketResponse.CRC.Bytes.ByteLo;

	printf("\nDatos contenidos en el CRC High: %d, CRC Low: %d.",dataOut[shift]  ,dataOut[shift+1]);

	return dataOut;
}

unsigned char * serializeWriteResponse(unsigned char * dataOut, socketMBPacket * responsePacket)
{



	dataOut[0] = responsePacket->header.slaveAddress;
	dataOut[1] = responsePacket->header.op;

	dataOut[2] = responsePacket->tail.writePacketResponse.startWord.Bytes.ByteHi;
	dataOut[3] = responsePacket->tail.writePacketResponse.startWord.Bytes.ByteLo;

	dataOut[4] = responsePacket->tail.writePacketResponse.numWord.Bytes.ByteHi;
	dataOut[5] = responsePacket->tail.writePacketResponse.numWord.Bytes.ByteLo;

	dataOut[6] = responsePacket->tail.writePacketResponse.CRC.Bytes.ByteHi;
	dataOut[7] = responsePacket->tail.writePacketResponse.CRC.Bytes.ByteLo;


	return dataOut;
}


socketMBPacket * buildWriteResponse(socketMBPacket incomingPacket, socketMBPacket * responsePacket) {


	responsePacket->header.slaveAddress = incomingPacket.header.slaveAddress;

	responsePacket->header.op = incomingPacket.header.op;


	responsePacket->tail.writePacketResponse.startWord = incomingPacket.tail.writePacketResponse.startWord;

	reverseWord(&(responsePacket->tail.writePacketResponse.startWord));

	responsePacket->tail.writePacketResponse.numWord = incomingPacket.tail.writePacketResponse.numWord;

	reverseWord(&(responsePacket->tail.writePacketResponse.numWord));

	responsePacket->tail.writePacketResponse.CRC.Word = getCRC(*responsePacket,write_response,0);

	//reverseWord(&(responsePacket->tail.writePacketResponse.CRC));

	printf("\nCRC calculado write Response: %d", responsePacket->tail.writePacketResponse.CRC.Word);

	return responsePacket;
}

int isWriteRequest(socketMBPacket packet)
{
	if ((packet.header.op == 0x10) && (packet.tail.writePacketRequest.byteNum > 0))
	{
		return 1;
	}
	return 0;
}




int isReadRequest(socketMBPacket packet)
{
	if ((packet.header.op == 0x3) && (packet.tail.readPacketRequest.numWord.Word > 0))
	{
		return 1;
	}
	return 0;
}

int isReadResponse(socketMBPacket packet)
{
	if ((packet.header.op == 0x3) && (isReadRequest(packet)	== 0))
	{
		return 1;
	}
	return 0;
}

unsigned short CRC_16 (unsigned short CRC, unsigned short Cod)
{
	unsigned char i;

	CRC = CRC ^ (0x00FF & Cod);
	for (i = 0; i <= 7; i++)
	{
		if ((CRC & 0x0001) == 0x0001)
		{
			CRC = (CRC >> 1) ^ 0xA001;
		}
		else
		{
			CRC = CRC >> 1;
		}
	}

	return CRC;
}

unsigned short getCRC(socketMBPacket mbPacket, int type,int lenght)
{
	sWord crc;
	crc.Word = calculateCRC(mbPacket,type, lenght);
	//reverseWord(&crc);
	return crc.Word;
}

unsigned short calculateCRC(socketMBPacket mbPacket,int type,int lenght)
{
	unsigned short tempCRC;

	int i=0;

	tempCRC = 0xFFFF;

	tempCRC = CRC_16(tempCRC,mbPacket.header.slaveAddress);
	tempCRC = CRC_16(tempCRC,mbPacket.header.op);

	switch(type)
	{
	case read_request:
	{
		tempCRC = CRC_16(tempCRC,mbPacket.tail.readPacketRequest.startWord.Bytes.ByteHi);
		tempCRC = CRC_16(tempCRC,mbPacket.tail.readPacketRequest.startWord.Bytes.ByteLo);
		tempCRC = CRC_16(tempCRC,mbPacket.tail.readPacketRequest.numWord.Bytes.ByteHi);
		tempCRC = CRC_16(tempCRC,mbPacket.tail.readPacketRequest.numWord.Bytes.ByteLo);
		break;
	}

	case read_response:
	{
		printf("\nCalculando el crc para la respuesta de lectura");
		tempCRC = CRC_16(tempCRC,mbPacket.tail.readPacketResponse.byteNum);


		for(i=0;i<lenght;i++)
		{
			tempCRC = CRC_16(tempCRC,mbPacket.tail.readPacketResponse.data[i].Bytes.ByteHi);
			tempCRC = CRC_16(tempCRC,mbPacket.tail.readPacketResponse.data[i].Bytes.ByteLo);
		}
		break;
	}

	case write_request:
	{

		break;
	}

	case write_response:
	{
		tempCRC = CRC_16(tempCRC,mbPacket.tail.writePacketResponse.startWord.Bytes.ByteHi);
		tempCRC = CRC_16(tempCRC,mbPacket.tail.writePacketResponse.startWord.Bytes.ByteLo);
		tempCRC = CRC_16(tempCRC,mbPacket.tail.writePacketResponse.numWord.Bytes.ByteHi);
		tempCRC = CRC_16(tempCRC,mbPacket.tail.writePacketResponse.numWord.Bytes.ByteLo);
		break;
	}

	}

	return tempCRC;
}


socketMBPacket rebuildPakcet(mbHeader header, mbTail tail,	socketMBPacket * packet) {
	packet->header = header;
	packet->tail = tail;

	return *packet;
}

void reverseWord(sWord * a) {
	unsigned char temp = a->Bytes.ByteHi;
	a->Bytes.ByteHi = a->Bytes.ByteLo;
	a->Bytes.ByteLo = temp;
}

void printMBPacket(socketMBPacket packet) {
	printf("\nDirección esclavo %d", packet.header.slaveAddress);
	printf("\nOpearación solicitada %d", packet.header.op);

	if (packet.header.op == 0x03) {
		printf("\nPalabra de inicio byte alto %d",
				packet.tail.readPacketRequest.startWord.Bytes.ByteHi);
		printf("\nPalabra de inicio byte bajo %d",
				packet.tail.readPacketRequest.startWord.Bytes.ByteLo);
		printf("\nNúmero de palabras byte alto %d",
				packet.tail.readPacketRequest.numWord.Bytes.ByteHi);
		printf("\nNúmero de palabras byte bajo %d",
				packet.tail.readPacketRequest.numWord.Bytes.ByteLo);
		printf("\nCRC byte alto %d",
				packet.tail.readPacketRequest.CRC.Bytes.ByteHi);
		printf("\nCRC byte bajo %d",
				packet.tail.readPacketRequest.CRC.Bytes.ByteLo);
		printf("\n");

	}

	if (isWriteRequest(packet)) {
		printf("\nPalabra de inicio byte alto %d",
				packet.tail.writePacketRequest.startWord.Bytes.ByteHi);
		printf("\nPalabra de inicio byte bajo %d",
				packet.tail.writePacketRequest.startWord.Bytes.ByteLo);
		printf("\nNúmero de palabras byte alto %d",
				packet.tail.writePacketRequest.numWord.Bytes.ByteHi);
		printf("\nNúmero de palabras byte bajo %d",
				packet.tail.writePacketRequest.numWord.Bytes.ByteLo);

		printf("\nDatos recibidos en el paquete de escritura");
		int i = 0;
		for (i = 0; i < packet.tail.writePacketRequest.numWord.Word; i++) {
			printf("\n Dato #%d byte alto: %d", i,
					packet.tail.writePacketRequest.data[i].Bytes.ByteHi);
			printf("\n Dato #%d byte bajo %d", i,
					packet.tail.writePacketRequest.data[i].Bytes.ByteLo);
		}


		printf("\nCRC byte alto %d",
				packet.tail.writePacketRequest.CRC.Bytes.ByteHi);
		printf("\nCRC byte bajo %d ",
				packet.tail.writePacketRequest.CRC.Bytes.ByteLo);

		printf("\n");
	}

}

void printMBResponse(socketMBPacket packet)
{
	printf("\n## Paquete respuesta de escritura");
	printf("\nPalabra de inicio byte alto %d",
			packet.tail.writePacketResponse.startWord.Bytes.ByteHi);
	printf("\nPalabra de inicio byte bajo %d",
			packet.tail.writePacketResponse.startWord.Bytes.ByteLo);

	printf("\nNúmero de palabras byte alto %d",
			packet.tail.writePacketResponse.numWord.Bytes.ByteHi);
	printf("\nNúmero de palabras byte bajo %d",
			packet.tail.writePacketResponse.numWord.Bytes.ByteLo);


	printf("\nCRC word %d", packet.tail.writePacketResponse.CRC.Word);

	printf("\n");
}

socketMBPacket * getDataFromMap(socketMBPacket * incomingPacket, socketMBPacket * responsePacket, sWord * map)
{
	int count = 0;

	int shift = incomingPacket->tail.readPacketRequest.startWord.Word;

	for(count=0;count< (incomingPacket->tail.readPacketRequest.numWord.Word);count++)
	{
		responsePacket->tail.readPacketResponse.data[count].Word = map[count+shift].Word;
	}
	return responsePacket;
}

// puntero a short por poner
sWord * getDataArrayFromMap(int startWord, int numWords, int mapNum, sWord * recoveredData)
{

	short * data;





	if ((data = getMapPointer(mapNum)) == NULL)
	{
		perror("Obtención de puntero a mapeado");
	}

	printf("\nPuntero para el mapeado número %d obtenido", mapNum);
	printf("\nTengo que leer %d palabras, a partir de la número %d", numWords, startWord);

	int shift = startWord;

	int i=0;

	for(i=0;i<numWords;i++)
	{
		recoveredData[i].Word = data[shift+i];
		reverseWord(&recoveredData[i]);
	}


	releaseMapPointer(data);

	return recoveredData;
}


void printsWordArray(int numWords, sWord * data)
{
	int j=0;

	for(j=0;j<numWords;j++)
	{
		printf("\n Dato %d, byteHi: %d, byteLo %d", j, data[j].Bytes.ByteHi, data[j].Bytes.ByteLo);
	}

}

sWord * copysWordArray(sWord * dest, sWord * source, int lenght )
{
	int i=0;

	for(i=0;i<lenght;i++)
	{
		dest->Word = source->Word;
	}

	return dest;
}

int writeOnMap(int mapNum, int start, int lenght, sWord * update)
{

	int i=0;

	short * data;
	if ((data = getMapPointer(mapNum)) == NULL)
	{
		perror("Obtención de puntero a mapeado");
		return -1;
	}
	printf("\nPuntero a mapeado obtenido");
	fflush(stdout);


	printf("\nBloqueo obtenido");
	fflush(stdout);


	// realizar las operaciones necesarias

	for(i=0;i<lenght;i++)
	{
		data[start+i] = update[i].Word;
	}
	printf("\nCambios realizados");
	fflush(stdout);

	releaseMapPointer(data);


	return 0;
}
