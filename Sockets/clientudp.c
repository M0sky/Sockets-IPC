/*
 *			C L I E N T U D P
 *
 *	This is an example program that demonstrates the use of
 *	sockets as an IPC mechanism.  This contains the client,
 *	and is intended to operate in conjunction with the server
 *	program.  Together, these two programs
 *	demonstrate many of the features of sockets, as well as good
 *	conventions for using these features.
 *
 *
 *	This program will request the internet address of a target
 *	host by name from the serving host.  The serving host
 *	will return the requested internet address as a response,
 *	and will return an address of all ones if it does not recognize
 *	the host name.
 *
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

extern int errno;

#define ADDRNOTFOUND	0xffffffff	/* value returned for unknown host */
#define RETRIES	5		/* number of times to retry before givin up */
#define BUFFERSIZE	512	/* maximum size of packets to be received */
#define PUERTO 6821
#define TIMEOUT 6
#define MAXHOST 512


int s, end=1, n_retry=RETRIES;
int	addrlen;

char hostname[MAXHOST];

struct sockaddr_in servaddr_in;		/* for server socket address 		*/
struct sockaddr_in myaddr_in;		/* for local socket address 		*/
struct in_addr reqaddr;				/* for returned internet address 	*/

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
	char mensaje[BUFFERSIZE];
	while (1) {
		int i = recvfrom(s, (char *)mensaje, BUFFERSIZE-1,  0, (struct sockaddr *) &servaddr_in, &addrlen); 
			
		if (i > 0) {
			mensaje[i] = '\0';

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

void send_msg(char *fichero) {
	char buf[BUFFERSIZE];	
	int aux = 1,count = 1, end1 = 1;

	FILE *ff;
	if ((ff = fopen(fichero, "r")) == NULL) {
		fprintf(stderr, "Fallo al abrir fichero de ordenes\n");
		exit(1);
	}

		while (end1 && n_retry > 0 && fgets (buf, BUFFERSIZE, ff) != NULL) {
			
			sleep(1);

			str_trim_lf(buf, strlen(buf)+1);
			strcat(buf, "    ");
			//PARA ENVIAR LOS MENSAJES
			if (count == 1) {
				aux = 0;
				if (sendto (s, buf, strlen(buf), 0, (struct sockaddr *)&servaddr_in, sizeof(struct sockaddr_in)) == -1) {
					fprintf(stderr, "ClientUDP: unable to send request\n");
					fclose(ff);
					exit(1);
				}
				count++;
			} else if (count == 2 && aux == 0) {
				if (sendto (s, buf, strlen(buf), 0, (struct sockaddr *)&servaddr_in, sizeof(struct sockaddr_in)) == -1) {
					fprintf(stderr, "ClientUDP: unable to send request\n");
					fclose(ff);
					exit(1);
				}
				count++;
			} else {
				if (sendto (s, buf, strlen(buf), 0, (struct sockaddr *)&servaddr_in, sizeof(struct sockaddr_in)) == -1) {
			   		fprintf(stderr, "ClientUDP: unable to send request\n");
					fclose(ff);
			   		exit(1);
			   	}
				if (strncmp(buf, "QUIT", 4) == 0) {
					fclose(ff);
					end1 = 0;            
				}
			}		
		}
}

/*
 *			H A N D L E R
 *
 *	This routine is the signal handler for the alarm signal.
 */
void handler()
{
 printf("Alarma recibida \n");
}

/*
 *			M A I N
 *
 *	This routine is the client which requests service from the remote
 *	"example server".  It will send a message to the remote nameserver
 *	requesting the internet address corresponding to a given hostname.
 *	The server will look up the name, and return its internet address.
 *	The returned address will be written to stdout.
 *
 *	The name of the system to which the requests will be sent is given
 *	as the first parameter to the command.  The second parameter should
 *	be the the name of the target host for which the internet address
 *	is sought.
 */
int main(argc, argv)
int argc;
char *argv[];
{
	int i, errcode;
	int retry = RETRIES;				/* holds the retry count 			*/
    long timevar;                       /* contains time returned by time() */
    
    struct sigaction vec;
   	struct addrinfo hints, *res;

	if (argc != 3) {
		fprintf(stderr, "Usage:  %s <nameserver> <target>\n", argv[0]);
		exit(1);
	}
	
		/* Create the socket. */
	s = socket (AF_INET, SOCK_DGRAM, 0);
	if (s == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to create socket\n", argv[0]);
		exit(1);
	}
	
    /* clear out address structures */
	memset ((char *)&myaddr_in, 0, sizeof(struct sockaddr_in));
	memset ((char *)&servaddr_in, 0, sizeof(struct sockaddr_in));
	
		/* Bind socket to some local address so that the
		 * server can send the reply back.  A port number
		 * of zero will be used so that the system will
		 * assign any available port number.  An address
		 * of INADDR_ANY will be used so we do not have to
		 * look up the internet address of the local host.
		 */

	myaddr_in.sin_family = AF_INET;
	myaddr_in.sin_port = 0;
	myaddr_in.sin_addr.s_addr = INADDR_ANY;

	if (bind(s, (const struct sockaddr *) &myaddr_in, sizeof(struct sockaddr_in)) == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to bind socket\n", argv[0]);
		exit(1);
	}
	
	addrlen = sizeof(struct sockaddr_in);
    if (getsockname(s, (struct sockaddr *)&myaddr_in, &addrlen) == -1) {
            perror(argv[0]);
            fprintf(stderr, "%s: unable to read socket address\n", argv[0]);
            exit(1);
    }

            /* Print out a startup message for the user. */
    time(&timevar);
            /* The port number must be converted first to host byte
             * order before printing.  On most hosts, this is not
             * necessary, but the ntohs() call is included here so
             * that this program could easily be ported to a host
             * that does require it.
             */
    printf("Connected to %s on port %u at %s", argv[1], ntohs(myaddr_in.sin_port), (char *) ctime(&timevar));

	//PREPARAMOS ADDRESS STRUCTURE DEL SERVIDOR

	/* Set up the server address. */
	servaddr_in.sin_family = AF_INET;

   	memset (&hints, 0, sizeof (hints));
   	hints.ai_family = AF_INET;

 	/* esta función es la recomendada para la compatibilidad con IPv6 gethostbyname queda obsoleta*/
    errcode = getaddrinfo (argv[1], NULL, &hints, &res); 
    if (errcode != 0){
			/* Name was not found.  Return a
			 * special value signifying the error. */
		fprintf(stderr, "%s: No es posible resolver la IP de %s\n",
				argv[0], argv[1]);
		exit(1);
    } else {
			/* Copy address of host */
		servaddr_in.sin_addr = ((struct sockaddr_in *) res->ai_addr)->sin_addr;
	}
   	freeaddrinfo(res);
    /* puerto del servidor en orden de red*/
	servaddr_in.sin_port = htons(PUERTO);

	//REGISTRAMOS SIGALRM EVITAR BLOQUE recvfrom
    vec.sa_handler = (void *) handler;
    vec.sa_flags = 0;
    if ( sigaction(SIGALRM, &vec, (struct sigaction *) 0) == -1) {
            perror(" sigaction(SIGALRM)");
            fprintf(stderr,"%s: unable to register the SIGALRM signal\n", argv[0]);
            exit(1);
   	}

	//ENVIAMOS ACK PARA QUE EL SERVIDOR CREE UN SOCKET

	char ackCliente[] = "ACK";
	char ackServer[BUFFERSIZE];

	if (sendto (s, ackCliente, strlen(ackCliente), 0, (struct sockaddr *)&servaddr_in, sizeof(struct sockaddr_in)) == -1) {
		fprintf(stderr, "ClientUDP: fallo al enviar ACK\n");
		exit(1);
	}
	
	if (recvfrom(s, ackServer, BUFFERSIZE-1,  0, (struct sockaddr *) &servaddr_in, &addrlen) == -1) {
		fprintf(stderr, "ClientUDP: fallo al recibir ACK\n");
		exit(1);
	}

	if (strncmp(ackCliente, ackServer, 3) == 0) {
		puts("[CLIENTE]:ACK RECIBIDO");

	
		//CREAMOS HILO PARA ENVIAR MENSAJES
		pthread_t send_msg_thread;
		if (pthread_create(&send_msg_thread, NULL, (void *) send_msg, (char *)argv[2]) != 0) {
		    printf ("Create pthread error!\n");
		    exit(1);
		}

		//CREAMOS HILO PARA LEER MENSAJES
		pthread_t recv_msg_thread;
		if (pthread_create(&recv_msg_thread, NULL, (void *) recv_msg, NULL) != 0) {
		    printf ("Create pthread error!\n");
		    exit(1);
		}

		while(end){

		}

		if (n_retry == 0) {
			printf("Unable to get response from");
			printf(" %s after %d attempts.\n", argv[1], RETRIES);
	   	}
	} else {
		puts("Fallo al establecer comunicacion");
	}
	
	close(s);
	exit(0);
}
