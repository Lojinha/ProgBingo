#include <stdio.h>
#include <stdlib.h>

#ifdef __WIN32
	#include <winsock2.h>
	#include <conio.h>
	WSADATA data; 
	SOCKET Socket_Server, Socket_Cliente; 
	SOCKADDR_IN EServer;
	SOCKADDR_IN ECliente;
	/*linker -> -l wsock32*/
#elif __linux__
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	int Socket_Server, Socket_Cliente;
	struct sockaddr_in EServer;    
	struct sockaddr_in ECliente; 
	void closesocket(int socket) { close(socket); }
	/*linker -> -lws2_32 -lwsock32 -L $MinGW\lib*/
#endif

#define PORTA 8181
#define MAXDATASIZE 100
#define	IPSERVIDOR	"127.0.0.1"

int main(int argc, char *argv[])
{
	int Meusocket, numbytes;
	char buf[MAXDATASIZE], enviar[MAXDATASIZE];
	struct hostent *he;
	struct sockaddr_in seu_endereco;


	#ifdef __WIN32
		if(WSAStartup(MAKEWORD(1,1),&data)==SOCKET_ERROR){ 
			printf("Erro ao inicializar o winsock"); 
			return -1;
		}
	#endif

	if ((Meusocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
	{
		perror("socket");
		exit(1);
	}

	seu_endereco.sin_family = AF_INET;
	seu_endereco.sin_port = htons(PORTA);
	seu_endereco.sin_addr.s_addr = inet_addr(IPSERVIDOR);

	if (connect(Meusocket,(struct sockaddr *)&seu_endereco, sizeof(struct sockaddr)) ==-1) 
	{
		perror("connect");
		exit(1);
	}
	while(1) {
		if ((numbytes=recv(Meusocket, buf, MAXDATASIZE, 0)) == -1) 
		{
			perror("recv");
			exit(1);
		}
		printf("Recebendo: %s\n", buf);
		printf("Insira o comando: ");
		fgets(enviar, MAXDATASIZE, stdin);
		if(!strcmp(enviar, "q\n"))
			break;
		if (send(Meusocket, enviar, strlen(enviar), 0) == -1) {
			perror("Texto Enviado");
			close(Meusocket);
			exit(0);
		}
		strcpy(enviar, "");
	}
	close(Meusocket);
	return 0;
}
