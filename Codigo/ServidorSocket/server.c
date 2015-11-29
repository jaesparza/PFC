


// ############################################################# Inclusión librerías del sistema
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>


// ############################################################ Inclusión librerías del proyecto
#include "dataDefinitions.h"
#include "McSemaphoreUtils.h"
#include "McMapUtils.h"

#include "peView.h"

#define PORT "200"  // Puerto en el que está disponible el servicio
#define BACKLOG 10     // Número máximo de peticiones en cola

#define WRITE_LOCK 1
#define READ_LOCK 0


//#define MAXDATASIZE 30

void sigchld_handler(int s) {
	while (waitpid(-1, NULL, WNOHANG) > 0)
		;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*) sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

int main(void) {


	int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
	char s[INET6_ADDRSTRLEN];
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;


	struct addrinfo hints, *servinfo, *p;

	struct sigaction sa;
	int yes = 1;

	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol))
				== -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))
				== -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		return 2;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	// creación de un mapeado te pruebas
	sWord map[490];
	int count = count;

	for(count = 0;count<490;count++)
	{
		map[count].Word = count;
	}

	printf("\n\nServidor NEXUS - Versión AVR32: a la espera de conexiones...\n");

	while (1) { // A la espera de conexiones

		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &sin_size);

		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family, get_in_addr(
				(struct sockaddr *) &their_addr), s, sizeof s);
		printf("\n\nServidor NEXUS - Versión AVR32: Conectado a cliente %s\n", s);
		fflush(stdout);

		if (!fork()) { // this is the child process

			unsigned char dataOut[1000];
			sWord update[490];
			socketMBPacket incomingPacket;
			socketMBPacket  responsePacket;
			int size,i;

			int start, mapNum, numWords;

			while(1)
			{



				// 1: recepción de la petición

				receivePacket(new_fd, &incomingPacket);
				printMBPacket(incomingPacket);

				// 2: creación de la respuesta a la petición

				if (incomingPacket.header.op == 0x3)
				{
					printf("\nOperación  de lectura (0x3)");
					fflush(stdout);

					buildReadResponse(incomingPacket,&responsePacket,map);

					printReadResponse(responsePacket,incomingPacket.tail.readPacketRequest.numWord.Word);

					fflush(stdout);

					reverseWord(&(incomingPacket).tail.readPacketRequest.numWord);


					serializeReadResponse(dataOut,&responsePacket,incomingPacket.tail.readPacketRequest.numWord.Word);
					printf("\nPaquete de lectura preparado para enviar");

					fflush(stdout);
				}
				else if(incomingPacket.header.op == 0x10)
				{
					printf("\nOperación de escritura (0x10)");

					fflush(stdout);


					//reverseWord(&(incomingPacket.tail.writePacketRequest.startWord));
					//reverseWord(&(incomingPacket.tail.writePacketRequest.numWord));

					start = incomingPacket.tail.writePacketRequest.startWord.Word;
					mapNum = incomingPacket.header.slaveAddress;
					numWords = incomingPacket.tail.writePacketRequest.numWord.Word;

					printf("\n ########## Start %d #### Direccion %d ##### Numero palabras %d",start,mapNum,numWords);
					fflush(stdout);

					printf("\nReservando memoria para %d", incomingPacket.tail.writePacketRequest.numWord.Word);
					fflush(stdout);


					printf("\nMemoria reservada ok");
					fflush(stdout);



					copysWordArray(update,incomingPacket.tail.writePacketRequest.data,start);


					printf("\nSolicitada la escritura en el mapeado");
					fflush(stdout);
					printf("\nVoy a actualizar el mapeado numero %d, start %d, numero de palabras %d",mapNum,start,numWords);
					fflush(stdout);
					writeOnMap(mapNum,start,numWords,update); // Escritura de los datos de manera directa en el mapeado
					printf("\nEscritura en el mapeado realizada");
					fflush(stdout);

					printsWordArray(numWords,update);

					buildWriteResponse(incomingPacket,&responsePacket); // Construcción del paquete de respuesta



					serializeWriteResponse(dataOut,&responsePacket);

				}
				else
				{
					printf("\nOperación desconocida");
					fflush(stdout);
				}


				// 3: envío de la respuesta

				if(incomingPacket.header.op == 0x3) // Envío de la respuesta a una petición de lectura
				{

					printf("\nOperación 0x3. Words=%d", incomingPacket.tail.readPacketRequest.numWord.Word);
					fflush(stdout);

					printf("\n");

					i=0;
					for(i=0;i<incomingPacket.tail.readPacketRequest.numWord.Word*2+5;i++)
					{
						printf("%d ",dataOut[i]);
					}

					size = send(new_fd,dataOut,incomingPacket.tail.readPacketRequest.numWord.Word*2+5,0);
					if (size == -1)
					{
						printf("Error: Envío de respuesta %d", size);
						close(sockfd);

						return -1;
					}
					else
					{
						if(size != incomingPacket.tail.readPacketRequest.numWord.Word*2+5)
						{

							close(sockfd);
							return -1;
							printf("\n##### ERROR EN EL ENVIO DE DATOS #######");
						}
						printf("\nRespuesta enviada correctamente %d", size);
					}

				}

				else if (incomingPacket.header.op == 0x10) // Envío de la respuesta a una petición de escritura
				{
					printf("\n Enviando respuesta");
					fflush(stdout);
					printf("\nOperación 0x10");
					fflush(stdout);
					size = send(new_fd,dataOut,sizeof(unsigned char)*8,0);
					if (size == -1)
					{

						close(sockfd);


						perror("Envío de respuesta");
						return -1;
					}
					else
					{
						if (size !=sizeof(unsigned char)*8)
						{

							close(sockfd);
							return -1;
						}
						printf("\nRespuesta enviada correctamente");

					}

				}
				else
				{
					printf("\nOperación desconocida");
					fflush(stdout);
				}

			}
		}
	}


	return 0;
}
