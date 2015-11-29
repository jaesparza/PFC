/*
 * serial.h
 *
 *  Created on: 06-ago-2009
 *      Author: ja
 */

#ifndef SERIAL_H_
#define SERIAL_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

//! BAUDRATE velocidad en baudios a la que trabaja el bus serie
#define BAUDRATE B9600
//! MODEMDEVICE puerto serie al que se conecta el modem radio
#define MODEMDEVICE "/dev/ttyS0"

//! Apertura y configuración del puerto serie
int openAndConfigure(int * fd, struct termios * oldtio, struct termios * newtio);

int serial_open(char *serial_name, speed_t baud);

//! Cierre del puerto serie
int closePort(int *fd);

// !Lectura del puerto serie
int serialRead (int fd, unsigned char * buff, unsigned char * partialData, int lenght, int timeoutUsec);


#endif /* SERIAL_H_ */
