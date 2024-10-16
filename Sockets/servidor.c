/*
 *          		S E R V I D O R
 *
 *	This is an example program that demonstrates the use of
 *	sockets TCP and UDP as an IPC mechanism.  
 *
 */

#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/ipc.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

#define PUERTO 6821
#define ADDRNOTFOUND	0xffffffff	/* return address for unfound host */
#define BUFFERSIZE	512	/* maximum size of packets to be received */
#define TAM_BUFFER 512
#define MAXHOST 128

char volcado[512] = "";
char nombreHost[20] = "";

FILE *fp;

void serverTCP(int s, struct sockaddr_in peeraddr_in);
void serverUDP(int s, char * buffer, struct sockaddr_in clientaddr_in);
void errout(char *);		/* declare error out routine */

//CREADAS POR NOSOTROS
char **split (char *string, const char sep);

pthread_mutex_t mutex;
pthread_mutex_t mutexUDP;
pthread_mutex_t mutexFP;

typedef struct nodoCliente{
	int		sockNum;
	struct 	nodoCliente *sig;
	char	nick[10];
	char	user[30];
	char 	canal[MAXHOST][200];
}Cliente;

typedef struct nodoClienteUDP{
	int		sockNum;
	struct 	nodoClienteUDP *sig;
	char	nick[10];
	char	user[30];
	char 	canal[MAXHOST][200];
	struct sockaddr_in clientaddr_in;
	int		addrlen;
}ClienteUDP;

Cliente *raiz;
ClienteUDP *raizUDP;

Cliente *newNode (int sockfd) {
	Cliente *nuevo;
	if ((nuevo = (Cliente *)malloc(sizeof(Cliente))) == NULL) {
		fprintf(stderr, "Error: Fallo al crear nodo");
	}
	nuevo->sockNum = sockfd;
	strcpy(nuevo->nick, " ");
	strcpy(nuevo->user, " ");
	for (int i=0; i<MAXHOST; i++) {
		strcpy(nuevo->canal[i], " ");
	}
	nuevo->sig = NULL;
	return nuevo;
}

ClienteUDP *newNodeUDP (int sockfd, struct sockaddr_in * clientaddr_in, int addrlen) {
	ClienteUDP *nuevo;
	if ((nuevo = (ClienteUDP *)malloc(sizeof(ClienteUDP))) == NULL) {
		fprintf(stderr, "Error: Fallo al crear nodo");
	}

	nuevo->sockNum = sockfd;
	strcpy(nuevo->nick, " ");
	strcpy(nuevo->user, " ");
	for (int i=0; i<MAXHOST; i++) {
		strcpy(nuevo->canal[i], " ");
	}
	nuevo->clientaddr_in = *clientaddr_in;
	nuevo->addrlen = addrlen;
	nuevo->sig = NULL;        
	return nuevo;
}



//VARIABLES GLOBALES

int serverSockTCP = 0, clientSockUDP = 0, clientSockTCP = 0;
int addrlen;

//FUNCIONES DE LISTAS ENLZADAS

//LISTAS TCP

int crearVacia(Cliente **root) {
	
	*root = NULL;
	return 0;

}

int estaVacia(Cliente *root) {

	return (root == NULL);

}

int insertarNodoFinal(Cliente **root, Cliente *nuevo) {

	Cliente *ultimo;
	
	ultimo = *root;	
	
   	if (!estaVacia(*root))
    	while (ultimo->sig != NULL)
      		ultimo = ultimo->sig;

  	// El ultimo nodo está apuntado por ultimo
    if (estaVacia(*root)) {
    	*root = nuevo;
        ultimo = nuevo;
  	} else {
        ultimo->sig = nuevo;
        ultimo = nuevo;  // Siempre dejamos apuntado el último nodo
	}

	return 0;
}


int eliminarNodo(Cliente **root, Cliente *pos) {

	Cliente *ant;	
	if (estaVacia(*root))
		return -1;
	else if (pos == NULL)
		return -2;
	else if (pos == *root) {
		*root = (*root)->sig;
		free (pos);
		return 0;
	} else {
		ant = *root;
		while (ant != NULL && ant->sig != pos)
			ant = ant->sig;
		if (ant != NULL) {
			ant->sig = pos->sig;
			free (pos);
			return 0;	
		} else
			return -3;
	} 
}

int mostrarListaEnlazada(Cliente **root) {
	Cliente *aImprimir;
	int res = 0,i = 0;
	
	printf("\n\n");
	printf("%-14s%-10s\n","  Socket", "Valor");
	printf("%-14s%-10s\n","  ==========", "======");


	aImprimir = *root;
	while (aImprimir != NULL) {
		printf("\t%3d)\t%s\n",aImprimir->sockNum, aImprimir->nick);
		aImprimir = aImprimir->sig;
	}
	return res;
}

//LISTAS UDP

int crearVaciaUDP(ClienteUDP **root) {
	
	*root = NULL;
	return 0;

}

int estaVaciaUDP(ClienteUDP *root) {

	return (root == NULL);

}

int insertarNodoFinalUDP(ClienteUDP **root, ClienteUDP *nuevo) {

	ClienteUDP *ultimo;
	
	ultimo = *root;	
	
   	if (!estaVaciaUDP(*root))
    	while (ultimo->sig != NULL)
      		ultimo = ultimo->sig;

  	// El ultimo nodo está apuntado por ultimo
    if (estaVaciaUDP(*root)) {
    	*root = nuevo;
        ultimo = nuevo;
  	} else {
        ultimo->sig = nuevo;
        ultimo = nuevo;  // Siempre dejamos apuntado el último nodo
	}

	return 0;
}


int eliminarNodoUDP(ClienteUDP **root, ClienteUDP *pos) {

	ClienteUDP *ant;	
	if (estaVaciaUDP(*root))
		return -1;
	else if (pos == NULL)
		return -2;
	else if (pos == *root) {
		*root = (*root)->sig;
		free (pos);
		return 0;
	} else {
		ant = *root;
		while (ant != NULL && ant->sig != pos)
			ant = ant->sig;
		if (ant != NULL) {
			ant->sig = pos->sig;
			free (pos);
			return 0;	
		} else
			return -3;
	} 
}

int mostrarListaEnlazadaUDP(ClienteUDP **root) {
	ClienteUDP *aImprimir;
	int res = 0,i = 0;
	
	printf("\n\n");
	printf("%-14s%-10s\n","  Socket", "Valor");
	printf("%-14s%-10s\n","  ==========", "======");


	aImprimir = *root;
	while (aImprimir != NULL) {
		printf("\t%3d)\t%s\n",aImprimir->sockNum, aImprimir->nick);
		aImprimir = aImprimir->sig;
	}
	return res;
}


/*
 *				S E R V E R T C P
 *
 *	This is the actual server routine that the daemon forks to
 *	handle each individual connection.  Its purpose is to receive
 *	the request packets from the remote client, process them,
 *	and return the results to the client.  It will also write some
 *	logging information to stdout.
 *
 */


void client_handler(void *p_client, struct sockaddr_in clientaddr_in) {
    int leave_flag = 0;
    int salida = 1;

	char buf[TAM_BUFFER];
	char hostname[MAXHOST];

	int status, salir = 1;
    struct hostent *hp;	
    long timevar;				
	
    char recv_buffer[TAM_BUFFER] = {};
    char send_buffer[TAM_BUFFER] = {};
    Cliente *np = (Cliente *)p_client;

	struct linger linger;
	 
	status = getnameinfo((struct sockaddr *)&clientaddr_in,sizeof(clientaddr_in),
                           hostname,MAXHOST,NULL,0,0);
    if(status){
		/* inet_ntop para interoperatividad con IPv6 */
    	if (inet_ntop(AF_INET, &(clientaddr_in.sin_addr), hostname, MAXHOST) == NULL)
           	perror(" inet_ntop \n");
    }
    /* Log a startup message. */
    time (&timevar);
	printf("\n\E[0;31m[+]\E[00m\E[1;32mStartup from %s port %u at %s\E[00m", hostname, ntohs(clientaddr_in.sin_port), (char *) ctime(&timevar));
	fflush(stdout);
	
	pthread_mutex_lock (&mutexFP);
	sprintf(volcado, " %s | %s | %s | %u | %s\n", nombreHost , hostname, "TCP", ntohs(clientaddr_in.sin_port), (char *) ctime(&timevar));
	fputs(volcado, fp);
	pthread_mutex_unlock (&mutexFP);

	linger.l_onoff  =1;
	linger.l_linger =1;
	if (setsockopt(np->sockNum, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger)) == -1) {
		errout(hostname);
	}	


    // NAMING NICK
	pthread_mutex_lock (&mutex);
    if (recv(np->sockNum, buf, TAM_BUFFER, 0) <= 0) {
        perror("Error lectura nick\n");
		time (&timevar);
		printf("\n\E[0;31m[+]\E[00m\E[1;32mCompleted %s port %u, at %s\n\E[00m", hostname, ntohs(clientaddr_in.sin_port),(char *) ctime(&timevar));
	fflush(stdout);  	    
		leave_flag = 1;
    } else if ((strncmp(buf, "NICK", 4)) != 0) {
		char msg[TAM_BUFFER] = "ERROR";
		if (send(np->sockNum, msg, TAM_BUFFER, 0) != TAM_BUFFER) errout(hostname);
		printf("\n\E[0;31m[+]\E[00m\E[1;32mCompleted %s port %u, at %s\n\E[00m", hostname, ntohs(clientaddr_in.sin_port),(char *) ctime(&timevar));
		fflush(stdout);		
		leave_flag = 1; 
	} else {
		char **bufSplit = split(buf, '\n');		
		char **tokenSplit = split(bufSplit[0], ' ');
		int condicion = 1;

		pthread_mutex_lock (&mutexFP);
		sprintf(volcado, " %s| %s\n", tokenSplit[0], (char *) ctime(&timevar));
		fputs(volcado, fp);
		pthread_mutex_unlock (&mutexFP);

		Cliente *tmp = raiz;
		while (tmp != NULL) {
			if (strcmp(tokenSplit[1], tmp->nick) == 0) {
				condicion = 0;
				char msgErr[TAM_BUFFER];
				sprintf(msgErr, "\E[0;31m[+]\E[00m\E[1;33m433 ERR_NICKNAMEINUSE\E[00m");
				if (send(np->sockNum, msgErr, TAM_BUFFER, 0) != TAM_BUFFER) errout(hostname);
				char msg[TAM_BUFFER] = "ERROR";
				if (send(np->sockNum, msg, TAM_BUFFER, 0) != TAM_BUFFER) errout(hostname);
				printf("\n\E[0;31m[+]\E[00m\E[1;32mCompleted %s port %u, at %s\n\E[00m", hostname, ntohs(clientaddr_in.sin_port),(char *) ctime(&timevar));
				fflush(stdout);
				leave_flag = 1;

				break;
			}
			tmp = tmp->sig;
		}

		if (condicion) {
			strncpy(np->nick, tokenSplit[1], 9);
    	}
	}
	pthread_mutex_unlock (&mutex);


	// NAMING USER
	pthread_mutex_lock (&mutex);
	while(salida) {
		if (recv(np->sockNum, buf, TAM_BUFFER, 0) <= 0) {
		    perror("Error lectura user\n");
			time (&timevar);
		} else if ((strncmp(buf, "USER", 4)) != 0) {
			char msg[TAM_BUFFER] = "ERROR";
			if (send(np->sockNum, msg, TAM_BUFFER, 0) != TAM_BUFFER) errout(hostname);
			printf("\n\E[0;31m[+]\E[00m\E[1;32mCompleted %s port %u, at %s\n\E[00m", hostname, ntohs(clientaddr_in.sin_port),(char *) ctime(&timevar));
			fflush(stdout);
			salida = 0;		
			leave_flag = 1; 
		} else {
			char **bufSplit = split(buf, '\n');		
			char **tokenSplit = split(bufSplit[0], ' ');
			int condicion = 1;

			pthread_mutex_lock (&mutexFP);
			sprintf(volcado, " %s| %s\n", tokenSplit[0], (char *) ctime(&timevar));
			fputs(volcado, fp);
			pthread_mutex_unlock (&mutexFP);

			Cliente *tmp = raiz;
			while (tmp != NULL) {
				if (strcmp(tokenSplit[1], tmp->user) == 0) {
					condicion = 0;
					char msgErr[TAM_BUFFER];
					sprintf(msgErr, "\E[0;31m[+]\E[00m\E[1;33m462 ERR_ALREADYREGISTRED\E[00m");
					if (send(np->sockNum, msgErr, TAM_BUFFER, 0) != TAM_BUFFER) errout(hostname);

					break;
				}
				tmp = tmp->sig;
			}
			if (condicion) {
				salida = 0;
				strncpy(np->user, tokenSplit[1], 9);
			}
		}
	}
	pthread_mutex_unlock (&mutex);

    	// Conversacion
	   	while (1) {
		    if (leave_flag) {
		        break;
		    }
		    int receive = recv(np->sockNum, buf, TAM_BUFFER, 0);

	
			char **bufSplit = split(buf, '\n');		
			char **tokenSplit = split(bufSplit[0], ' ');
			char msg[TAM_BUFFER];

			pthread_mutex_lock (&mutexFP);
			sprintf(volcado, " %s| %s\n", tokenSplit[0], (char *) ctime(&timevar));
			fputs(volcado, fp);
			pthread_mutex_unlock (&mutexFP);


			pthread_mutex_lock (&mutex);	    
	
			//QUIT
		    if (strncmp(buf, "QUIT", 4) == 0 || strlen(buf) == 0) {
				
		        printf("%s(%d) Se ha desconectado...\n", np->nick, np->sockNum);
				if (send(np->sockNum, "SALIDA", TAM_BUFFER, 0) != TAM_BUFFER) errout(hostname);	
		        leave_flag = 1;
				
			//PRIVMSG
			} else if (strncmp(buf, "PRIVMSG", 7) == 0) {

				char *mensaje = bufSplit[0] + strlen(tokenSplit[0]) + strlen(tokenSplit[1]) + 2;

				//DIFERENCIAMOS SI ES UN CANAL O UN NICK
				char auxiliar[200];
				strncpy(auxiliar, tokenSplit[1], 200);

				//ES UN CANAL
				if (auxiliar[0] == '#') {
					Cliente *tmp = raiz;					
					
					//ENVIAMOS A TODOS EL MENSAJE
					while (tmp != NULL) {
					
		  	 			if (np->sockNum != tmp->sockNum) {
							for (int x=0; x<MAXHOST; x++) {
								if (strncmp(tmp->canal[x], tokenSplit[1], 200) == 0) {
		        					sprintf(msg, "[%s] %s : %s\n", np->canal, np->nick, mensaje);
									if (send(tmp->sockNum, msg, TAM_BUFFER, 0) != TAM_BUFFER) errout(hostname);		
								}
							}
		    			}
		    			tmp = tmp->sig;
	   			 	}	

				//ES UN NICK
				} else {
					Cliente *tmp = raiz;
					int ex = 1;

					//OBTENEMOS SOCKET RECEPTOR
					while(tmp != NULL && ex) {

						if (strcmp(tmp->nick, tokenSplit[1]) == 0) {

							sprintf(msg, "%s : %s\n", np->nick, mensaje);
							if (send(tmp->sockNum, msg, TAM_BUFFER, 0) != TAM_BUFFER) errout(hostname);
							ex = 0;
						
						}
						tmp = tmp->sig;
					}
				
					if (ex) {
						sprintf(msg, "\E[0;31m[+]\E[00m\E[1;33m401 ERR_NOSUCHNICK\E[00m");
						if (send(np->sockNum, msg, TAM_BUFFER, 0) != TAM_BUFFER) errout(hostname);
					}		
				}

			//JOIN
			} else if (strncmp(buf, "JOIN", 4) == 0) {
				Cliente *tmp = raiz;
				int aux = 1;

				//DIFERENCIAMOS SI ES UN CANAL O UN NICK
				char auxiliar[200];
				strncpy(auxiliar, tokenSplit[1], 200);

				//ES UN CANAL
				if (auxiliar[0] == '#') {
				
					for (int i=0; i<MAXHOST && aux; i++) {
						if (strcmp(np->canal[i], " ") == 0) {
							strcpy(np->canal[i], tokenSplit[1]);
							sprintf(msg, "%s se ha unido al canal %s", np->nick, np->canal[i]);	
							aux = 0;
						}
					}						

					//ENVIAMOS MENSAJE DE ENTRADA A TODOS LOS DEL CANAL MENOS AL BIENVENIDO
					while (tmp != NULL && aux == 0) {
						
						
		  	 			if (np->sockNum != tmp->sockNum) {
							for (int x=0; x<MAXHOST; x++) {
								if (strcmp(tmp->canal[x], tokenSplit[1]) == 0) {
		        					send(tmp->sockNum, msg, TAM_BUFFER, 0);
								}
							}
		    			}

		    			tmp = tmp->sig;
	   			 	}
				} else {
					sprintf(msg, "\E[0;31m[+]\E[00m\E[1;33mNEW ERR_SINTAXIS\E[00m");
					if (send(np->sockNum, msg, TAM_BUFFER, 0) != TAM_BUFFER) errout(hostname);
				}				
			//PART
			} else if (strncmp(buf, "PART", 4) == 0) {
				Cliente *tmp = raiz;
				int aux = 1;

				//DIFERENCIAMOS SI ES UN CANAL O UN NICK
				char auxiliar[200];
				strncpy(auxiliar, tokenSplit[1], 200);

				//ES UN CANAL
				if (auxiliar[0] == '#') {				

					for (int i=0; i<MAXHOST && aux; i++) {
						if (strcmp(np->canal[i], tokenSplit[1]) == 0) {				
							strcpy(np->canal[i], " ");				
							sprintf(msg, "%s ha salido del canal %s", np->nick, tokenSplit[1]);
							aux = 0;
						}
					}			


					//ENVIAMOS MENSAJE DE SALIDA A TODOS LOS DEL CANAL MENOS AL BIENVENIDO
					while (tmp != NULL && aux == 0) {
						
		  	 			if (np->sockNum != tmp->sockNum) {
							for (int x=0; x<MAXHOST; x++) {
								if (strncmp(tmp->canal[x], tokenSplit[1], 200) == 0) {
		        					send(tmp->sockNum, msg, TAM_BUFFER, 0);
								}
							}
		    			}

		    			tmp = tmp->sig;
	   			 	}
				} else {
					sprintf(msg, "\E[0;31m[+]\E[00m\E[1;33m403 ERR_NOSUCHCHANNEL\E[00m");
					if (send(np->sockNum, msg, TAM_BUFFER, 0) != TAM_BUFFER) errout(hostname);
				}		
			}	

			pthread_mutex_unlock (&mutex);
			sleep(1);

		}	/*Fin del while*/

    // Remove Node
	pthread_mutex_lock (&mutex);
    eliminarNodo(&raiz, np);	
	pthread_mutex_unlock (&mutex);

	printf("\n\E[0;31m[+]\E[00m\E[1;32mCompleted %s port %u, at %s\n\E[00m", hostname, ntohs(clientaddr_in.sin_port),(char *) ctime(&timevar));
	fflush(stdout);

}

/*
 *				S E R V E R U D P
 *
 *	This is the actual server routine that the daemon forks to
 *	handle each individual connection.  Its purpose is to receive
 *	the request packets from the remote client, process them,
 *	and return the results to the client.  It will also write some
 *	logging information to stdout.
 *
 */

//COMO MULTIPLEXAMOS LAS DIRECCIONES IP PARA QUE ESCUCHE EN EL MISMO PAR IP-PROTOCOLO, PORQUE NECESITAMOS UN PROTOCOLO DE NIVEL SUPERIOR

void clientUDP_handler (void *p_client, struct sockaddr_in clientaddr_in){
	int leave_flag = 0;
    int salida = 1, cc, i;

	char buf[BUFFERSIZE];
	char hostname[MAXHOST];

	int status, salir = 1;
    struct hostent *hp;	
    long timevar;				
	
    char recv_buffer[BUFFERSIZE] = {};
    char send_buffer[BUFFERSIZE] = {};

    ClienteUDP *np = (ClienteUDP *)p_client;

	struct linger linger;
	 
	status = getnameinfo((struct sockaddr *)&clientaddr_in,sizeof(clientaddr_in),
                           hostname,MAXHOST,NULL,0,0);
    if(status){
		/* inet_ntop para interoperatividad con IPv6 */
    	if (inet_ntop(AF_INET, &(clientaddr_in.sin_addr), hostname, MAXHOST) == NULL)
           	perror(" inet_ntop \n");
    }
    /* Log a startup message. */
    time (&timevar);
	printf("\n\E[0;31m[+]\E[00m\E[1;32mStartup from %s port %u at %s\E[00m", hostname, ntohs(clientaddr_in.sin_port), (char *) ctime(&timevar));
	fflush(stdout);

	pthread_mutex_lock (&mutexFP);
	sprintf(volcado, " %s | %s | %s | %u | %s\n", nombreHost , hostname, "UDP", ntohs(clientaddr_in.sin_port), (char *) ctime(&timevar));
	fputs(volcado, fp);
	pthread_mutex_unlock (&mutexFP);
	
	linger.l_onoff  =1;
	linger.l_linger =1;
	if (setsockopt(np->sockNum, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger)) == -1) {
		errout(hostname);
	}	


	// NAMING NICK
	pthread_mutex_lock (&mutexUDP);
    if ( (i =recvfrom(np->sockNum, buf, BUFFERSIZE - 1, 0, (struct sockaddr *)&clientaddr_in, &addrlen)) <= 0) {
        perror("Error lectura nick\n");
		time (&timevar);
		printf("\n\E[0;31m[+]\E[00m\E[1;32mCompleted %s port %u, at %s\n\E[00m", hostname, ntohs(clientaddr_in.sin_port),(char *) ctime(&timevar));
	fflush(stdout);  	    
		leave_flag = 1;
    } else if ((strncmp(buf, "NICK", 4)) != 0) {
		char msg[BUFFERSIZE] = "ERROR";
		sendto(np->sockNum, msg, strlen(msg), MSG_CONFIRM, (struct sockaddr *) &clientaddr_in, addrlen);
		printf("\n\E[0;31m[+]\E[00m\E[1;32mCompleted %s port %u, at %s\n\E[00m", hostname, ntohs(clientaddr_in.sin_port),(char *) ctime(&timevar));
		fflush(stdout);		
		leave_flag = 1; 
	} else {
		buf[i] = '\0';
		char **bufSplit = split(buf, '\n');		
		char **tokenSplit = split(bufSplit[0], ' ');
		int condicion = 1;
	
		pthread_mutex_lock (&mutexFP);
		sprintf(volcado, " %s| %s\n", tokenSplit[0], (char *) ctime(&timevar));
		fputs(volcado, fp);
		pthread_mutex_unlock (&mutexFP);


		ClienteUDP *tmp = raizUDP;
		while (tmp != NULL) {
			if (strncmp(tokenSplit[1], tmp->nick, strlen(tokenSplit[1])) == 0) {
				condicion = 0;
				char msgErr[BUFFERSIZE];
				sprintf(msgErr, "\E[0;31m[+]\E[00m\E[1;33m433 ERR_NICKNAMEINUSE\E[00m");
				sendto(np->sockNum, msgErr, BUFFERSIZE, MSG_CONFIRM, (struct sockaddr *) &clientaddr_in, addrlen);
				char msg[BUFFERSIZE] = "ERROR";
				sendto(np->sockNum, msg, BUFFERSIZE, MSG_CONFIRM, (struct sockaddr *) &clientaddr_in, addrlen);
				printf("\n\E[0;31m[+]\E[00m\E[1;32mCompleted %s port %u, at %s\n\E[00m", hostname, ntohs(clientaddr_in.sin_port),(char *) ctime(&timevar));
				fflush(stdout);
				leave_flag = 1;

				break;
			}
			tmp = tmp->sig;
		}

		if (condicion) {
			strncpy(np->nick, tokenSplit[1], strlen(tokenSplit[1]));
    	}
	}
	pthread_mutex_unlock (&mutexUDP);



	// NAMING USER
	pthread_mutex_lock (&mutexUDP);
    if ( (i =recvfrom(np->sockNum, buf, BUFFERSIZE - 1, 0, (struct sockaddr *)&clientaddr_in, &addrlen)) <= 0) {
        perror("Error lectura user\n");
		time (&timevar);
		printf("\n\E[0;31m[+]\E[00m\E[1;32mCompleted %s port %u, at %s\n\E[00m", hostname, ntohs(clientaddr_in.sin_port),(char *) ctime(&timevar));
	fflush(stdout);  	    
		leave_flag = 1;
    } else if ((strncmp(buf, "USER", 4)) != 0) {
		char msg[BUFFERSIZE] = "ERROR";
		sendto(np->sockNum, msg, strlen(msg), MSG_CONFIRM, (struct sockaddr *) &clientaddr_in, addrlen);
		printf("\n\E[0;31m[+]\E[00m\E[1;32mCompleted %s port %u, at %s\n\E[00m", hostname, ntohs(clientaddr_in.sin_port),(char *) ctime(&timevar));
		fflush(stdout);		
		leave_flag = 1; 
	} else {
		buf[i] = '\0';
		char **bufSplit = split(buf, '\n');		
		char **tokenSplit = split(bufSplit[0], ' ');
		int condicion = 1;

		pthread_mutex_lock (&mutexFP);
		sprintf(volcado, " %s| %s\n", tokenSplit[0], (char *) ctime(&timevar));
		fputs(volcado, fp);
		pthread_mutex_unlock (&mutexFP);

		ClienteUDP *tmp = raizUDP;
		while (tmp != NULL) {
			if (strncmp(tokenSplit[1], tmp->user, strlen(tokenSplit[1])) == 0) {
				condicion = 0;
				char msgErr[BUFFERSIZE];
				sprintf(msgErr, "\E[0;31m[+]\E[00m\E[1;33m462 ERR_ALREADYREGISTRED\E[00m");
				sendto(np->sockNum, msgErr, BUFFERSIZE, MSG_CONFIRM, (struct sockaddr *) &clientaddr_in, addrlen);
				char msg[BUFFERSIZE] = "ERROR";
				sendto(np->sockNum, msg, BUFFERSIZE, MSG_CONFIRM, (struct sockaddr *) &clientaddr_in, addrlen);
				printf("\n\E[0;31m[+]\E[00m\E[1;32mCompleted %s port %u, at %s\n\E[00m", hostname, ntohs(clientaddr_in.sin_port),(char *) ctime(&timevar));
				fflush(stdout);
				leave_flag = 1;

				break;
			}
			tmp = tmp->sig;
		}

		if (condicion) {
			strncpy(np->user, tokenSplit[1], strlen(tokenSplit[1]));
    	}
	}
	pthread_mutex_unlock (&mutexUDP);

	// Conversacion
	   	while (1) {
		    if (leave_flag) {
		        break;
		    }
		    int receive = recvfrom(np->sockNum, buf, BUFFERSIZE - 1, 0, (struct sockaddr *)&clientaddr_in, &addrlen);
			buf[receive] = '\0';
	
			char **bufSplit = split(buf, '\n');		
			char **tokenSplit = split(bufSplit[0], ' ');
			char msg[TAM_BUFFER];

			pthread_mutex_lock (&mutexFP);
			sprintf(volcado, " %s| %s\n", tokenSplit[0], (char *) ctime(&timevar));
			fputs(volcado, fp);
			pthread_mutex_unlock (&mutexFP);


			pthread_mutex_lock (&mutex);	    
	
			//QUIT
		    if (strncmp(buf, "QUIT", 4) == 0 || strlen(buf) == 0) {
				
		        printf("%s(%d) Se ha desconectado...\n", np->nick, np->sockNum);
				sendto(np->sockNum, "SALIDA", BUFFERSIZE, MSG_CONFIRM, (struct sockaddr *) &clientaddr_in, addrlen);
		        leave_flag = 1;
				
			//PRIVMSG
			} else if (strncmp(buf, "PRIVMSG", 7) == 0) {

				char *mensaje = bufSplit[0] + strlen(tokenSplit[0]) + strlen(tokenSplit[1]) + 2;

				//DIFERENCIAMOS SI ES UN CANAL O UN NICK
				char auxiliar[200];
				strncpy(auxiliar, tokenSplit[1], 200);

				//ES UN CANAL
				if (auxiliar[0] == '#') {
					ClienteUDP *tmp = raizUDP;					
					
					//ENVIAMOS A TODOS EL MENSAJE
					while (tmp != NULL) {
					
		  	 			if (np->sockNum != tmp->sockNum) {
							for (int x=0; x<MAXHOST; x++) {
								if (strncmp(tmp->canal[x], tokenSplit[1], 200) == 0) {
		        					sprintf(msg, "[%s] %s : %s\n", tmp->canal[x], np->nick, mensaje);
									sendto(tmp->sockNum, msg, BUFFERSIZE, MSG_CONFIRM, (struct sockaddr *) &tmp->clientaddr_in, tmp->addrlen);		
								}
							}
		    			}
		    			tmp = tmp->sig;
	   			 	}	

				//ES UN NICK
				} else {
					ClienteUDP *tmp = raizUDP;
					int ex = 1;

					//OBTENEMOS SOCKET RECEPTOR
					while(tmp != NULL && ex) {

						if (strcmp(tmp->nick, tokenSplit[1]) == 0) {

							sprintf(msg, "%s : %s\n", np->nick, mensaje);
							sendto(tmp->sockNum, msg, BUFFERSIZE, MSG_CONFIRM, (struct sockaddr *) &tmp->clientaddr_in, tmp->addrlen);		
							ex = 0;
						
						}
						tmp = tmp->sig;
					}
				
					if (ex) {
						sprintf(msg, "\E[0;31m[+]\E[00m\E[1;33m401 ERR_NOSUCHNICK\E[00m");
						sendto(np->sockNum, msg, BUFFERSIZE, MSG_CONFIRM, (struct sockaddr *) &clientaddr_in, addrlen);		
					}		
				}

			//JOIN
			} else if (strncmp(buf, "JOIN", 4) == 0) {
				ClienteUDP *tmp = raizUDP;
				int aux = 1;

				//DIFERENCIAMOS SI ES UN CANAL O UN NICK
				char auxiliar[200];
				strncpy(auxiliar, tokenSplit[1], 200);

				//ES UN CANAL
				if (auxiliar[0] == '#') {
					for (int i=0; i<MAXHOST && aux; i++) {
						if (strcmp(np->canal[i], " ") == 0) {
							strcpy(np->canal[i], tokenSplit[1]);
		
							sprintf(msg, "%s se ha unido al canal %s", np->nick, np->canal[i]);	
							aux = 0;
						}
					}						

					//ENVIAMOS MENSAJE DE ENTRADA A TODOS LOS DEL CANAL MENOS AL BIENVENIDO
					while (tmp != NULL && aux == 0) {
						
						
		  	 			if (np->sockNum != tmp->sockNum) {
							for (int x=0; x<MAXHOST; x++) {
								if (strncmp(tmp->canal[x], tokenSplit[1], strlen(tmp->canal[x])) == 0) {
									sendto(tmp->sockNum, msg, BUFFERSIZE, MSG_CONFIRM, (struct sockaddr *) &tmp->clientaddr_in, tmp->addrlen);		
								}
							}
		    			}

		    			tmp = tmp->sig;
	   			 	}
				} else {
					sprintf(msg, "\E[0;31m[+]\E[00m\E[1;33mNEW ERR_SINTAXIS\E[00m");
					sendto(np->sockNum, msg, BUFFERSIZE, MSG_CONFIRM, (struct sockaddr *) &clientaddr_in, addrlen);		
				}				
			//PART
			} else if (strncmp(buf, "PART", 4) == 0) {
				ClienteUDP *tmp = raizUDP;
				int aux = 1;

				//DIFERENCIAMOS SI ES UN CANAL O UN NICK
				char auxiliar[200];
				strncpy(auxiliar, tokenSplit[1], 200);

				//ES UN CANAL
				if (auxiliar[0] == '#') {				

					for (int i=0; i<MAXHOST && aux; i++) {
						if (strcmp(np->canal[i], tokenSplit[1]) == 0) {				
							strcpy(np->canal[i], " ");			
							sprintf(msg, "%s ha salido del canal %s", np->nick, tokenSplit[1]);
							aux = 0;
						}
					}			


					//ENVIAMOS MENSAJE DE SALIDA A TODOS LOS DEL CANAL MENOS AL BIENVENIDO
					while (tmp != NULL && aux == 0) {
						
		  	 			if (np->sockNum != tmp->sockNum) {
							for (int x=0; x<MAXHOST; x++) {
								if (strncmp(tmp->canal[x], tokenSplit[1], 200) == 0) {
									sendto(tmp->sockNum, msg, BUFFERSIZE, MSG_CONFIRM, (struct sockaddr *) &tmp->clientaddr_in, tmp->addrlen);		
								}
							}
		    			}

		    			tmp = tmp->sig;
	   			 	}
				} else {
					sprintf(msg, "\E[0;31m[+]\E[00m\E[1;33m403 ERR_NOSUCHCHANNEL\E[00m");
					sendto(np->sockNum, msg, BUFFERSIZE, MSG_CONFIRM, (struct sockaddr *) &clientaddr_in, addrlen);		
				}		
			}	

			pthread_mutex_unlock (&mutex);
			sleep(1);

		}	/*Fin del while*/

    // Remove Node
	pthread_mutex_lock (&mutexUDP);
    eliminarNodoUDP(&raizUDP, np);	
	pthread_mutex_unlock (&mutexUDP);

	printf("\n\E[0;31m[+]\E[00m\E[1;32mCompleted %s port %u, at %s\n\E[00m", hostname, ntohs(clientaddr_in.sin_port),(char *) ctime(&timevar));
	fflush(stdout);

}

extern int errno;

/*
 *			M A I N
 *
 *	This routine starts the server.  It forks, leaving the child
 *	to do all the work, so it does not have to be run in the
 *	background.  It sets up the sockets.  It
 *	will loop forever, until killed by a signal.
 *
 */

int FIN = 0;             /* Para el cierre ordenado */
void finalizar(){ FIN = 1; }

int main(argc, argv)
int argc;
char *argv[];
{
    
	if ((fp = fopen ("ircd.log", "w")) == NULL) { fprintf(stderr, "Fallo al crear fichero");}

	strcpy(nombreHost, argv[0]);

    int cc;				    /* contains the number of bytes read */
	pthread_mutex_init (&mutex, NULL);
	pthread_mutex_init (&mutexUDP, NULL);
	pthread_mutex_init (&mutexFP, NULL);
     
    struct sigaction sa = {.sa_handler = SIG_IGN}; /* used to ignore SIGCHLD */
    
    struct sockaddr_in myaddr_in;		/* for local socket address */
    struct sockaddr_in clientaddr_in;	/* for peer socket address 	*/
	
	int newSocket = clientSockUDP;

    fd_set readmask;
    int numfds,s_mayor;
    
    char buffer[BUFFERSIZE];	/* buffer for packets to be read into */

    struct sigaction vec;

	//CREAMOS el SOCKET
	serverSockTCP = socket (AF_INET, SOCK_STREAM, 0);
	if (serverSockTCP == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to create socket TCP\n", argv[0]);
		exit(1);
	}
	//LIMPIAMOS las estructuras de direcciones
	memset ((char *)&myaddr_in, 0, sizeof(struct sockaddr_in));
   	memset ((char *)&clientaddr_in, 0, sizeof(struct sockaddr_in));

    addrlen = sizeof(struct sockaddr_in);

	//CONFIGURAMOS la direccion de escucha para el SOCKET
	myaddr_in.sin_family = AF_INET;				//TIPO DE SOCKET, PARA UNA MISMA MAQUINA AF_UNIX
	myaddr_in.sin_addr.s_addr = INADDR_ANY;		//DIRECCION IP DEL CLIENTE PARA HACER BROADCAST
	myaddr_in.sin_port = htons(PUERTO);

	//ENLAZADO de la direccion de escucha con el SOCKET
	if (bind(serverSockTCP, (const struct sockaddr *) &myaddr_in, sizeof(struct sockaddr_in)) == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to bind address TCP\n", argv[0]);
		exit(1);
	}

	//INICIALIZAMOS la escucha en el SOCKET para que se puedan CONECTAR los usurarios remotos (5 es el mayor numero soportado)
	if (listen(serverSockTCP, 5) == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to listen on socket\n", argv[0]);
		exit(1);
	}
	
	//PARA UDP

	/* Create the socket UDP. */
	clientSockUDP = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (clientSockUDP == -1) {
		perror(argv[0]);
		printf("%s: unable to create socket UDP\n", argv[0]);
		exit(1);
	}
	/* Bind the server's address to the socket. */
	if (bind(clientSockUDP, (struct sockaddr *) &myaddr_in, sizeof(struct sockaddr_in)) == -1) {
		perror(argv[0]);
		printf("%s: unable to bind address UDP\n", argv[0]);
		exit(1);
	}

	//PARA QUE EL DAEMON NO ESTE ASOCIADO CON LA TERMINAL DEL USUARIO
	setpgrp();

	//CREAMOS EL DAEMON
	switch (fork()) {
	case -1:		/* Unable to fork, for some reason. */
		perror(argv[0]);
		fprintf(stderr, "%s: unable to fork daemon\n", argv[0]);
		exit(1);

	case 0:
		//*******************************************
		//******EMPIEZA EL PROCESO HIJO DAEMON*******
		//*******************************************

		//CERRAMOS stdin y stderr, el daemon hará un BUCLE INFINITO esperando mas conexiones
		fclose(stdin);
		fclose(stderr);

		//REGISTRAMOS SEÑALES
		if ( sigaction(SIGCHLD, &sa, NULL) == -1) {
            perror(" sigaction(SIGCHLD)");
            fprintf(stderr,"%s: unable to register the SIGCHLD signal\n", argv[0]);
            exit(1);
       	}
            
        vec.sa_handler = (void *) finalizar;
        vec.sa_flags = 0;
        if ( sigaction(SIGTERM, &vec, (struct sigaction *) 0) == -1) {
            perror(" sigaction(SIGTERM)");
            fprintf(stderr,"%s: unable to register the SIGTERM signal\n", argv[0]);
            exit(1);
      	}

		crearVacia(&raiz);
		crearVaciaUDP(&raizUDP);

		//EMPIEZA EL BUCLE
		while (!FIN) {
            //Metemos en el conjunto de sockets los TCP y los UDP
            FD_ZERO(&readmask);
            FD_SET(serverSockTCP, &readmask);
            FD_SET(clientSockUDP, &readmask);
            //SELECCION del descriptor del SOCKET que ha cambiado
    	    if (serverSockTCP > clientSockUDP) s_mayor=serverSockTCP;
    		else s_mayor=clientSockUDP;

			//CODIGO QUEDA DORMIDO HASTA QUE SE LE PASA EL DESCRIPTOR DE UN SOCKET CUANDO UN CLIENTE REALIZA UNA PETICION
            if ( (numfds = select(s_mayor+1, &readmask, (fd_set *)0, (fd_set *)0, NULL)) < 0) {
                if (errno == EINTR) {
                    FIN=1;
		            close (serverSockTCP);
		            close (clientSockUDP); 
                    perror("\nFinalizando el servidor. Señal recibida en select\n "); 
                }
            } else {
                //TCP
                if (FD_ISSET(serverSockTCP, &readmask)) {
                	//La llamada se BLOQUEARA hasta que un nuevo CLIENTE se CONECTE
    				clientSockTCP = accept(serverSockTCP, (struct sockaddr *) &clientaddr_in, &addrlen);

					//CLIENTE conectado al servidor, su descriptor es s_TCP
    				if (clientSockTCP == -1) exit(1);
	
					// Append linked list for clients
        			Cliente *c = newNode(clientSockTCP);	
				
					pthread_mutex_lock (&mutex);
        			insertarNodoFinal(&raiz, c);	
					pthread_mutex_unlock (&mutex);

					pthread_t cliente;
		
					if (pthread_create(&cliente, NULL, (void *)client_handler, (void *)c) != 0) {
            			perror("\nFallo al crear pthread\n");
       				}
				}
	
				//UDP
       			if (FD_ISSET(clientSockUDP, &readmask)) {

					//NOS QUEDAMOS BLOQUEADOS HASTA QUE LLEGUE UN CLIENTE
					cc = recvfrom(clientSockUDP, buffer, BUFFERSIZE - 1, 0, (struct sockaddr *)&clientaddr_in, &addrlen);
					char ack[] = "ACK";
					if ( cc == -1) {
						printf("SERVER: recvfrom error\n");
						exit (1);
					}
	 		
					buffer[cc]='\0';				

					pthread_mutex_lock (&mutexUDP);
					if (strncmp(buffer, "ACK", 3) == 0) {

						puts(" ");
						int nuevoSocket = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
						if (nuevoSocket == -1) {
							perror(argv[0]);
							printf("%s: unable to create socket UDP\n", argv[0]);
							exit(1);
						}

						//CREACION DEL NODO
						ClienteUDP *u = newNodeUDP(nuevoSocket, &clientaddr_in, addrlen);
       					insertarNodoFinalUDP(&raizUDP, u);

						pthread_t clienteUDP; 

						if (pthread_create(&clienteUDP, NULL, (void *)clientUDP_handler, (void *)u) != 0) {
		        			perror("\nFallo al crear pthread\n");
						}

						puts("[SERVER]:ACK RECIBIDO");

						sendto(nuevoSocket, ack, BUFFERSIZE, MSG_CONFIRM, (struct sockaddr *) &clientaddr_in, addrlen);   
		     		}
					pthread_mutex_unlock (&mutexUDP);
				}	//FIN UDP
         	}
		}   /* Fin del bucle infinito de atención a clientes */

	fclose(fp);

   	/* Cerramos los sockets UDP y TCP */
	close(serverSockTCP);
    close(clientSockUDP);

	printf("\nFin de programa servidor!\n");
        
	default:		/* Parent process comes here. */
		exit(0);
	}

}

/*
 *	This routine aborts the child process attending the client.
 */
void errout(char *hostname)
{
	printf("Connection with %s aborted on error\n", hostname);
	exit(1);     
}


char **split (char *string, const char sep) {

	char **lista;
	char *p = string;
	int i = 0;

	int pos;
	const int len = strlen (string);

	lista = (char **) malloc (sizeof (char *));
	if (lista == NULL) { /* Cannot allocate memory */
		return NULL;
	}

	lista[pos=0] = NULL;

	while (i < len) {
		while ((p[i] == sep) && (i < len))
			i++;
		if (i < len) {
			char **tmp = (char **) realloc (lista , (pos + 2) * sizeof (char *));
		if (tmp == NULL) { /* Cannot allocate memory */ 
			free (lista); 
			return NULL; 
		}
		lista = tmp;
		tmp = NULL;	
		lista[pos + 1] = NULL;
		lista[pos] = (char *) malloc (sizeof (char));
		if (lista[pos] == NULL) { /* Cannot allocate memory */
			for (i = 0; i < pos; i++)
				free (lista[i]);
			free (lista);
			return NULL;
		}
		int j = 0;
		for (i; ((p[i] != sep) && (i < len)); i++) {
			lista[pos][j] = p[i];
			j++;
			char *tmp2 = (char *) realloc (lista[pos],(j + 1) * sizeof (char));
			if (lista[pos] == NULL) { /* Cannot allocate memory */ 
				for (i = 0; i < pos; i++) 
					free (lista[i]);
					free (lista);
					return NULL;
				}
				lista[pos] = tmp2;
				tmp2 = NULL;
			}
			lista[pos][j] = '\0';
			pos++;
		}
	}
	return lista;
}
