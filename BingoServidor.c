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
	int sockfd;
	struct sockaddr_in serverAddr;
	void closesocket(int socket) { close(socket); }
	//linker -> -lws2_32 -lwsock32 -L $MinGW\lib
#endif

#define		MAX_CON		1
#define 	MAX_SIZE 	50
#define 	NUM_CLIENT 	5
#define		PORTA		8188


int conexao() {
	if( accept(sockfd, 0, 0) == -1)
		return 0;
	else
		return 1;
}

int* retornaCartela() {
	int *cartela = (int*)malloc(26);
	int i, j, numerosDiversos = 0, numeroGerado;
	while(numerosDiversos < 25) {
		if(numerosDiversos == 12) {
			cartela[numerosDiversos] = 0;
			numerosDiversos++;
			continue;
		}
		numeroGerado = randomInteger(1, 99);
		for(j = 0; j < numerosDiversos; j++)
			if(cartela[j] == numeroGerado)
				break;
		if(j == numerosDiversos) {
			cartela[numerosDiversos] = numeroGerado;
			numerosDiversos++;
		}
	}
	return cartela;
}

int* ordenaCrescente(int* cartela) {
	int *cartelaAuxiliar;
	cartelaAuxiliar = cartela;
	return cartelaAuxiliar;
}

int randomInteger (int low, int high) {
    int k;
    double d;
    d = (double) rand () / ((double) RAND_MAX + 1);
    k = d * (high - low + 1);
    return low + k;
}

void printaCartela(int* cartela) {
	int i, j, aux;
	
	for(i = 0; i < 25; i++) {
		if(cartela[i] == 0) {
			if(i%5==0) {
				printf("|	-	|", cartela[i]);
			} else if((i+1)%5==0) {
				printf("	-	|\n", cartela[i]);
			} else {
				printf("	-	|", cartela[i]);
			}
			continue;
		}
		if(i%5==0) {
			printf("|	%d	|", cartela[i]);
		} else if((i+1)%5==0) {
			printf("	%d	|\n", cartela[i]);
		} else {
			printf("	%d	|", cartela[i]);
		}
	}
}

int configuracaoServidor()
{
	#ifdef __WIN32
		WSADATA data; 
		SOCKET sockfd; 
		SOCKADDR_IN serverAddr;
		if(WSAStartup(MAKEWORD(1,1),&data)==SOCKET_ERROR){ 
			printf("Erro ao inicializar o winsock"); 
			return -1;
		}
	#elif __linux__
		int sockfd;
    	struct sockaddr_in serverAddr;
	#endif
	
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
      return -1;
    memset(&serverAddr, 0, sizeof (serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(PORTA);
    if (bind(sockfd, (struct sockaddr *) & serverAddr, sizeof(serverAddr)) < 0)
    	return -1;
    if (listen(sockfd, MAX_CON) < 0)
    	return -1;
    return sockfd;
}

int main(int argc, char *argv[]) {
	system("mode con:cols=82 lines=20"); //lines=8
	sockfd = configuracaoServidor();
    if(sockfd == -1) {
      	printf("Erro ao criar Socket\n");
     	exit(1);
	}
	printf("Aguardando Conexão!\n");
	//while(!conexao());
	printf("Conexão Aceita!\n\n");
	while (1)
    {   	
		int *cartela;
		cartela = retornaCartela();
		//printaCartela(cartela);
		//printf("\n\n");
		//cartela = ordenaCrescente(cartela);
		//printaCartela(cartela);
		//break;
    }
	getchar();
	closesocket(sockfd);
	return 0;
}
