#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#ifdef __WIN32
	#include <winsock2.h>
	#include <conio.h>
#elif __linux__
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
#endif

#define	MAX_CON	10
#define MAX_SIZE 50
#define NUM_CLIENT 5

/* run this program using the console pauser or add your own getch, system("pause") or input loop */

int socket_desc , client_sock , c , *new_sock;
struct sockaddr_in server , client;

int configuracaoServidor()
{
    int sockfd;
    struct sockaddr_in serverAddr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
      return -1;
    memset(&serverAddr, 0, sizeof (serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(1234);
    if (bind(sockfd, (struct sockaddr *) & serverAddr, sizeof (serverAddr)) < 0)
    	return -1;
    if (listen(sockfd, MAX_CON) < 0)
    	return -1;
    return sockfd;
}

void *thread_Serial(void *arg) {
    char buffer_do_cliente[256];
    int sockEntrada = *(int *) arg;
    printf("Aguardando as mensagens... ");
    for (;;) {
        read(sockEntrada, buffer_do_cliente, sizeof (buffer_do_cliente));
        if (strcmp(buffer_do_cliente, "sair") != 0) {
            printf("%s\n",buffer_do_cliente);
        }
        else {
        	closesocket(sockEntrada);
        	pthread_exit((void*) 0);
        }
    }
}

void *thread_teste(void *threadid) {
    int threadnum = (int)threadid;
    int sock_desc;
    struct sockaddr_in serv_addr;
    char sbuff[MAX_SIZE],rbuff[MAX_SIZE];
	
	sock_desc = configuracaoServidor();
    while(accept(sock_desc, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0);

    printf("Connected successfully client:%d\n", threadnum);
    while(1)
    {
        if(recv(sock_desc,rbuff,MAX_SIZE,0)==0)
            printf("Error");
        else
           fputs(rbuff,stdout);
        memset(rbuff,MAX_SIZE, sizeof(rbuff));
        sleep(2);
    }
    close(sock_desc);
    return 0;
}

int main(int argc, char *argv[]) {
    struct sockaddr_in serverAddr;
    int sockfd = configuracaoServidor();
      
//	int i;
//	pthread_t sniffer_thread;
//    for (i=1; i<=NUM_CLIENT; i++) {
//        if( pthread_create( &sniffer_thread , NULL ,  thread_teste , (void*) i) < 0)
//        {
//            perror("could not create thread");
//            return 1;
//        }
//        sleep(3);
//    }
//    pthread_exit(NULL);
//    return 0;
   
    if(sockfd == -1) {
      	printf("Erro ao criar Socket\n");
     	exit(1);
	}
	while (1)
    {
        int clienteSockfd;
        struct sockaddr_in clienteAddr;
        unsigned int clntLen;
        clntLen = sizeof (clienteAddr);
    	pthread_t thread;
        if ((clienteSockfd = accept(sockfd, (struct sockaddr *) & clienteAddr, &clntLen)) < 0) {
      		printf("Erro no Socket\n");
     		exit(1);
    	}
        pthread_create(&thread, NULL, thread_Serial, &clienteSockfd) != 0;
        pthread_detach(thread);
    }

	return 0;
}
