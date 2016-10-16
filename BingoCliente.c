#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <winsock2.h>

#include <conio.h>

/* run this program using the console pauser or add your own getch, system("pause") or input loop */

pthread_t threads[1];
WSADATA data; 
SOCKET winsock; 
SOCKADDR_IN sock;

char* fgets_c(FILE*);
char enviar[1024], receber[1024]; 

void *thread_Serial(void *arg) {
	while(1) {
		printf("\nThread em andamento - Digite uma frase: ");
		fgets(enviar, 100, stdin);
		if( enviar[strlen(enviar) -1 ] == '\n' ) enviar[strlen(enviar) - 1] = '\0';
		send(winsock, enviar, strlen(enviar), 0);		
	}
}

int main(int argc, char *argv[]) {
	long t;
	char c;
	//int rc;
	
	if(WSAStartup(MAKEWORD(1,1),&data)==SOCKET_ERROR){ 
		printf("Erro ao inicializar o winsock"); 
		return 0; 
	} 
	if((winsock = socket(AF_INET,SOCK_STREAM,0))==SOCKET_ERROR){ 
		printf("Erro ao criar socket"); 
		return 0; 
	} 
	sock.sin_family=AF_INET; 
	sock.sin_port=htons(1234); 
	sock.sin_addr.s_addr=inet_addr("127.0.0.1");
	printf("Socket iniciado!\n\n");
	if(connect(winsock,(SOCKADDR*)&sock,sizeof(sock))==SOCKET_ERROR){
		printf("Erro ao se conectar");
		getchar();
		return 0;
	}
	printf("Conectado!\n\n");
	pthread_create(&(threads[0]), NULL, thread_Serial, &t);
	
	while(1) {
		//c = getch();
		//if(c == 'q')
		//	break;	
	}
	
	//pthread_exit((void *)NULL);
	closesocket(winsock);
	WSACleanup();
	return 0;
}
