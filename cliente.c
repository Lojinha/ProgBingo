#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "functions.h"

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
#define MAXNOMEJOGADOR	16
#define	IPSERVIDOR	"127.0.0.1"
#define NOVA_CARTELA	"GERA_CARTELA\n"

void init(void);
void retiraBreakLine(char[]);
void sysClear(void);
void printaCartela(char*);

char nomeJogador[MAXNOMEJOGADOR], escMenu[MAXDATASIZE], *board;
bool cartelaGerada = false;

int main(int argc, char *argv[])
{
	int Meusocket, numbytes, i;
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

	if (connect(Meusocket,(struct sockaddr *)&seu_endereco, sizeof(struct sockaddr)) ==-1) {
		perror("connect");
		exit(1);
	}
	if ((numbytes=recv(Meusocket, buf, MAXDATASIZE, 0)) == -1) {
		perror("recv");
		exit(1);
	}
	init();
	if (send(Meusocket, nomeJogador, strlen(nomeJogador), 0) == -1) {
		perror("Texto Enviado");
		close(Meusocket);
		exit(0);
	}
	if ((numbytes=recv(Meusocket, buf, MAXDATASIZE, 0)) == -1) {
		perror("recv");
		exit(1);
	}
	while(1) {
		sysClear();
		if(cartelaGerada)
			printaCartela(board);
		printf("-----------------|\tBem-Vindo %s\t|-----------------\n\n", nomeJogador);
		printf("Aguarde ate o inicio do jogo, enquanto isso voce pode:\n");
		printf("(1) Escolher uma nova cartela.\n");
		if(cartelaGerada)
			printf("(2) Excluir cartela.\n");
		printf("(3) Sair do saguao de espera.\n");
		
		printf("\nSua escolha: ");
		fgets(escMenu, MAXDATASIZE, stdin);
		retiraBreakLine(&escMenu);
		if(!strcmp(escMenu, "1")) {
			if (send(Meusocket, NOVA_CARTELA, strlen(NOVA_CARTELA), 0) == -1) {
				perror("Texto Enviado");
				close(Meusocket);
				exit(0);
			}
			if ((numbytes=recv(Meusocket, buf, MAXDATASIZE, 0)) == -1) {
				perror("recv");
				exit(1);
			}
			board = create_board(buf);
			cartelaGerada = true;
		} else if( (!strcmp(escMenu, "2")) && cartelaGerada) {
			board = 0;
			cartelaGerada = false;	
		} else if(!strcmp(escMenu, "3")) {
			if (send(Meusocket, "q\n", 3, 0) == -1) {
				perror("Texto Enviado");
				close(Meusocket);
				exit(0);
			}
			break;
		} else if(!strcmp(escMenu, "4")) {
			if (send(Meusocket, "LIST_USER\n", 11, 0) == -1) {
				perror("Texto Enviado");
				close(Meusocket);
				exit(0);
			}
			if ((numbytes=recv(Meusocket, buf, MAXDATASIZE, 0)) == -1) {
				perror("recv");
				exit(1);
			}
			printf("\n\nRecebido: %s", buf);
			getchar();
		}
		strcpy(enviar, "");
		fflush(stdin);
	}
	close(Meusocket);
	return 0;
}

void init() {
	strcpy(nomeJogador, "");
	do{
		if(strcmp(nomeJogador, ""))
			printf("Nome muito grande. Max %d caracteres", MAXNOMEJOGADOR);
		sysClear();
		printf("ProBingo 3000 -------------------------------------------\n\n");	
		printf("Insira o nome do Jogador: ");
		fgets(nomeJogador, MAXDATASIZE, stdin);
		retiraBreakLine(&nomeJogador);
	}while(strlen(nomeJogador) > MAXNOMEJOGADOR);
}

void retiraBreakLine(char text[]) {
	if( text[strlen(text) - 1] == '\n') text[strlen(text) - 1] = '\0';
}

void sysClear() {
	#ifdef __WIN32
		system("cls");	
	#elif __linux__
		system("clear");
	#endif
}

void printaCartela(char *board) {
	int i;
	printf("------------------\t----------");
	for(i = 0; i < strlen(nomeJogador) - 1; i++) {
		printf("-");
	}
	printf("\t------------------\n");
	
	i = 0;
	while(board[i] != '\0') {
		printf("%c", board[i]);
		i++;
	} 
	/* printf("%s", board);*/
	
	printf("\n");
}
