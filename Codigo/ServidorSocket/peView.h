
#define NDatosMax			30

#define read_request	0
#define read_response	1
#define write_request	2
#define write_response	3


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

typedef struct
{
	unsigned char   			slaveAddress;
	unsigned char   			op;
} mbHeader;

typedef union
{

	struct
	{
		sWord startWord;
		sWord numWord;
		sWord CRC;

	} readPacketRequest;

	struct
	{
		unsigned char byteNum;
		sWord data[490];
		sWord CRC;
	} readPacketResponse;

	struct
	{
		sWord startWord;
		sWord numWord;
		unsigned char byteNum;
		sWord data[490];
		sWord CRC;
	} writePacketRequest;

	struct
	{
		sWord startWord;
		sWord numWord;
		sWord CRC;
	} writePacketResponse;

} mbTail;

typedef struct
{
	mbHeader header;
	mbTail tail;
} socketMBPacket;

void printMBPacket(socketMBPacket packet);

int receiveTail(int socketFD,mbTail * tail, mbHeader header);

socketMBPacket receivePacket(int socketFD, socketMBPacket * packet);

int receiveHeader(int socketFD, mbHeader  * header);

void reverseWord(sWord * a);

socketMBPacket * buildWriteResponse(socketMBPacket query, socketMBPacket * response);

void printMBResponse(socketMBPacket packet);

int isReadRequest(socketMBPacket packet);

int isWriteRequest(socketMBPacket packet);

int isReadResponse(socketMBPacket packet);

int isWriteResponse(socketMBPacket packet);

unsigned short calculateCRC(socketMBPacket mbPacket,int type,int leght);

unsigned short getCRC(socketMBPacket mbPacket, int type,int lenght);

socketMBPacket * buildReadResponse (socketMBPacket incomingPacket, socketMBPacket * responsePacket, sWord * map);

unsigned char * serializeReadResponse(unsigned char * dataOut, socketMBPacket * responsePacket, int numWords);

unsigned char * serializeWriteResponse(unsigned char * dataOut, socketMBPacket * responsePacket);

sWord * getDataArrayFromMap(int startWord, int numWords, int mapNum, sWord * recoveredData);

socketMBPacket * getDataFromMap(socketMBPacket * incomingPacket, socketMBPacket * responsePacket, sWord * map);

void printsWordArray(int numWords, sWord * data);

sWord * copysWordArray(sWord * dest, sWord * source, int lenght );

int writeOnMap(int mapNum, int start, int lenght, sWord * update);

void printReadResponse(socketMBPacket packet,int dataLenght);

socketMBPacket rebuildPakcet(mbHeader header, mbTail tail,	socketMBPacket * packet);

