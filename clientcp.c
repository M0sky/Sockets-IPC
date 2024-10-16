/*
 *			C L I E N T C P
 *
 *	This is an example program that demonstrates the use of
 *	stream sockets as an IPC mechanism.  This contains the client,
 *	and is intended to operate in conjunction with the server
 *	program.  Together, these two programs
 *	demonstrate many of the features of sockets, as well as good
 *	conventions for using these features.
 *
 *
 */
 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#define PUERTO 6821
#define TAM_BUFFER 512

int s, end=1;

//FUNCION PARA RECIBIR MENSAJES SOCKET ESPECIFICO

void str_trim_lf (char* arr, int length) {
    int i;
    for (i = 0; i < length; i++) { // trim \n
        if (arr[i] == '\n' || arr[i] == '\r') {
            arr[i] = '\0';
            break;
        }
    }
}

void recv_msg() {
	char mensaje[TAM_BUFFER];
	while (1) {
		int i = recv(s, mensaje, TAM_BUFFER, 0);
        if (i > 0) {
			if (strcmp(mensaje, "ERROR") == 0) {
				puts("Error recibido, saliendo...");
				end = 0;
				break;
			} else if (strcmp(mensaje, "SALIDA") == 0) {
				end = 0;
				break;
			}
            printf("\r%s\n", mensaje);
        } else if (i == 0) {
            break;
        } else { 
			//fprintf(stderr, "Error reading result\n");
        }
	}
}

//FUNCION PARA ENVIAR MENSAJES SOCKET ESPECIFICO

void send_msg(char *fichero) {
	char buf[TAM_BUFFER];	
	int aux = 1,count = 1;

	FILE *ff;
	if ((ff = fopen(fichero, "r")) == NULL) {
		fprintf(stderr, "Fallo al abrir fichero de ordenes\n");
		exit(1);
	}

    while (end) {
        while (fgets (buf, TAM_BUFFER, ff) != NULL) {

			str_trim_lf(buf, TAM_BUFFER);
	
			if (count == 1) {
				send (s, buf, TAM_BUFFER, 0);
				count++;
			} else if (count == 2 && aux == 0) {
				send (s, buf, TAM_BUFFER, 0);
				count++;
			} else {
		    	send (s, buf, TAM_BUFFER, 0);
		    	if (strncmp(buf, "QUIT", 4) == 0) {
					if (shutdown(s, SHUT_WR) == -1) {
						fprintf(stderr, "[CLIENT]: unable to shutdown socket WR\n");
						exit(1);
					}
					sleep(1);            
		    	}
			}
			aux = 0;
		}
	}
	fclose(ff);
}

int main(argc, argv)
int argc;
char *argv[];
{
	int finish = 0;
	char delimitador[] = " ";
	char *nombreFichero = argv[2];
							
   	struct addrinfo hints, *res;
    long timevar;					/* contains time returned by time() */
    struct sockaddr_in myaddr_in;	/* for local socket address */
    struct sockaddr_in servaddr_in;	/* for server socket address */
	int addrlen, i, j, errcode;

	if (argc != 3) {
		fprintf(stderr, "Usage:  %s <remote host>\n", argv[0]);
		exit(1);
	}

	if ((s = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to create socket\n", argv[0]);
		exit(1);
	}
	
	memset ((char *)&myaddr_in, 0, sizeof(struct sockaddr_in));
	memset ((char *)&servaddr_in, 0, sizeof(struct sockaddr_in));

	servaddr_in.sin_family = AF_INET;
	
	memset (&hints, 0, sizeof (hints));
	hints.ai_family = AF_INET;
	 	 
	//IPv6
	errcode = getaddrinfo (argv[1], NULL, &hints, &res); 
	if (errcode != 0){
		fprintf(stderr, "%s: No es posible resolver la IP de %s\n",
		argv[0], argv[1]);
		exit(1);
	} else
		servaddr_in.sin_addr = ((struct sockaddr_in *) res->ai_addr)->sin_addr;
	
	freeaddrinfo(res);

	servaddr_in.sin_port = htons(PUERTO);

	if (connect(s, (const struct sockaddr *)&servaddr_in, sizeof(struct sockaddr_in)) == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to connect to remote\n", argv[0]);
		exit(1);
	}
	
	addrlen = sizeof(struct sockaddr_in);
	if (getsockname(s, (struct sockaddr *)&myaddr_in, &addrlen) == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to read socket address\n", argv[0]);
		exit(1);
	}

	time(&timevar);
	
	printf("\n\tConnected to %s on port %u at %s\n", argv[1], ntohs(myaddr_in.sin_port), (char *) ctime(&timevar));

	//CREAMOS HILO PARA ENVIAR MENSAJES
	pthread_t send_msg_thread;
    if (pthread_create(&send_msg_thread, NULL, (void *) send_msg, (char *)argv[2]) != 0) {
        printf ("Create pthread error!\n");
        exit(1);
    }
	
	//CREAMOS HILO PARA RECIBIR MENSAJES
	pthread_t recv_msg_thread;
    if (pthread_create(&recv_msg_thread, NULL, (void *) recv_msg, NULL) != 0) {
        printf ("Create pthread error!\n");
        exit(1);
    }

	while(end) {
		//HASTA REGISTRAR QUIT
	}	
	
	close(s);	

	time(&timevar);
	printf("\n\tAll done at %s", (char *)ctime(&timevar));
}
