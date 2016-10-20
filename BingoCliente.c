#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __WIN32
	#include <winsock2.h>
	#include <conio.h>
	WSADATA data; 
	SOCKET sockfd; 
	SOCKADDR_IN serverAddr; 
	//linker -> -l wsock32
#elif __linux__
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <curses.h>
	int sockfd;
	struct sockaddr_in serverAddr;
	void closesocket(int socket) { close(socket); }
	//char getch() { return getc(); }
	//linker -> -lws2_32 -lwsock32 -L $MinGW\lib
#endif

#define		MAX_CON			1
#define		MAX_SIZE		50
#define		NUM_CLIENT		5
#define		IPSERVIDOR		"127.0.0.1"
#define		PORTA			8188

int conexao() {
	if( connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
		return 0;
	else
		return 1;
}

void configuracaoServidor() {
	
	#ifdef __WIN32
		if(WSAStartup(MAKEWORD(1,1),&data)==SOCKET_ERROR)
			return -1;
	#endif
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
      return -1;
    memset(&serverAddr, 0, sizeof (serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORTA);
    serverAddr.sin_addr.s_addr = inet_addr(IPSERVIDOR);
}

int main(int argc, char *argv[]) {
    configuracaoServidor();
    if(sockfd == -1) {
      	printf("Erro ao criar Socket\n");
     	exit(1);
	}
	while (1)
    {
		printf("Tentativa de conexão...\n");
		getchar();
		if(conexao()) {
			printf("Conectado com sucesso\n\n");
			break;
		} else
			printf("Erro na conexao\n\n");
    }
    printf("...");
    getchar();

	return 0;
}
