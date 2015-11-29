/*
 * serial.c
 *
 *  Created on: 06-ago-2009
 *      Author: ja
 */


#include "serial.h"


int openAndConfigure(int * fd, struct termios * oldtio, struct termios * newtio)
{

	*fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY);
	if (*fd < 0)
	{
		perror(MODEMDEVICE);
		exit(-1);
	}

	tcgetattr(*fd, oldtio); // Guardado de la configuración actual del terminal

	bzero(newtio, sizeof(*newtio));
	newtio->c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
	newtio->c_iflag = IGNPAR;
	newtio->c_oflag = ~OPOST;



	// Configuración modo de entrada (non-canonical, no echo,...)
	//newtio->c_lflag = 0;
	newtio->c_cc[VTIME] = 10; // Tiempo entre caracteres
	newtio->c_cc[VMIN] = 5; /* blocking read until 5 chars received */

	return 0;
}

int serial_open(char *serial_name, speed_t baud)
{
	struct termios newtermios;
	int fd;

	fd = open(serial_name,O_RDWR | O_NOCTTY);

	newtermios.c_cflag= CBAUD | CS8 | CLOCAL | CREAD;
	newtermios.c_iflag=IGNPAR;
	newtermios.c_oflag=0;
	newtermios.c_lflag=0;
	//newtermios.c_cc[VMIN]=1;
	//newtermios.c_cc[VTIME]=0;

	cfsetospeed(&newtermios,baud);
	cfsetispeed(&newtermios,baud);

	if (tcflush(fd,TCIFLUSH)==-1) return -1;
	if (tcflush(fd,TCOFLUSH)==-1) return -1;
	if (tcsetattr(fd,TCSANOW,&newtermios)==-1) return -1;

	return fd;
}

int serialRead (int fd, unsigned char * buff, unsigned char * partialData, int lenght, int timeoutUsec)
{

	int cont = 1;
	int i = 0;
	int partialRead = 0;
	int readCount = 0;

	fd_set fds;
	struct timeval timeout;
	int ret = 1;

	while (cont == 1)
	{ // Recepción de los datos enviados por el maestro

		//-- Set the fds variable to wait for the serial descriptor
		FD_ZERO(&fds);
		FD_SET (fd, &fds);
		//-- Set the timeout in usec.
		timeout.tv_sec = 0;
		timeout.tv_usec = timeoutUsec;
		//-- Wait for the data
		ret=select (FD_SETSIZE,&fds, NULL, NULL,&timeout);



		partialRead = read(fd, buff, sizeof(unsigned char));

		//#ifdef _DEBUG
		//			printf("\nValor readCount %d", readCount);
		//			printf("\nValor de partialRead %d", partialRead);
		//#endif

		for (i = 0; i < partialRead; i++)
		{
			partialData[readCount + i] = buff[i];
		}

		readCount = readCount + partialRead;

//		if (ret != 1)
//			printf("\n Timeout!");
		if ((readCount >= lenght)| (ret != 1))
			cont = 0;
	}



	if (ret == 1)
	{
		// Copia de las lecturas parciales al buffer de lectura
		for (i = 0; i < lenght; i++)
		{
			buff[i] = partialData[i];
		}
		return lenght;
	}
	else
	{
		printf("\n ################################ TIMEOUT ##################################");
		return -1;
	}
}


int serial_read(int serial_fd, unsigned char *data, int size, int timeout_usec)
{
	fd_set fds;
	struct timeval timeout;
	int count=0;
	int ret;
	int n;

	//-- Wait for the data. A block of size bytes is expected to arrive
	//-- within the timeout_usec time. This block can be received as
	//-- smaller blocks.
	do {
		//-- Set the fds variable to wait for the serial descriptor
		FD_ZERO(&fds);
		FD_SET (serial_fd, &fds);

		//-- Set the timeout in usec.
		timeout.tv_sec = 0;
		timeout.tv_usec = timeout_usec;

		//-- Wait for the data
		ret=select (FD_SETSIZE,&fds, NULL, NULL,&timeout);

		//-- If there are data waiting: read it
		if (ret==1) {

			//-- Read the data (n bytes)
			n=read (serial_fd, &data[count], sizeof(unsigned char));

			//-- The number of bytes receives is increased in n
			count+=n;

			//-- The last byte is always a 0 (for printing the string data)
			data[count]=0;
		}

		printf("\n Datos leidos %s", data);

		int tmp=0;
		for(tmp=0;tmp<size;tmp++)
		{
			printf("\nDatos %d", data[tmp]);
		}
		//-- Repeat the loop until a data block of size bytes is received or
		//-- a timeout occurs
	} while (count<size && ret==1);

	//-- Return the number of bytes reads. 0 If a timeout has occurred.
	return count;
}




int closePort(int * fd)
{
	close(*fd);
	return 0;
}
