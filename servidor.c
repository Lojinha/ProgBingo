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

#define MAXDATASIZE 100
#define	PORTA	8181

void retornaCartela(int *);
void ordenaCrescente(int *);
void printaCartela(int *);
void printaMensagem(char *);
char* converteCartela(int *);

void main()
{
	char buf[MAXDATASIZE];
	int tamanho, numbytes;

	#ifdef __WIN32
		if(WSAStartup(MAKEWORD(1,1),&data)==SOCKET_ERROR){ 
			printf("Erro ao inicializar o winsock"); 
			return -1;
		}
	#endif

	if ((Socket_Server = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	EServer.sin_family = AF_INET;
	EServer.sin_port = htons(PORTA);
	EServer.sin_addr.s_addr = INADDR_ANY;   

	if (bind(Socket_Server, (struct sockaddr *)&EServer, sizeof(struct sockaddr))== -1) {
		perror("Conexao");
		exit(1);
	}
	if (listen(Socket_Server, 10) < 0) {
		perror("Aguardando");
		exit(1);
	}
	printf("Aguardando conexao...\n\n");

	while(1) {
		tamanho = sizeof(struct sockaddr_in);
		if ((Socket_Cliente = accept(Socket_Server, (struct sockaddr *)&ECliente,&tamanho)) < 0){
			perror("Conexao aceita");
			continue;
		}
		printf("Servidor: chegando conexao de %d\n",inet_ntoa(ECliente.sin_addr));
		if (send(Socket_Cliente, "OK\n\0", 4, 0) == -1)
		{
			perror("Texto Enviado");
			close(Socket_Cliente);
			exit(0);
		}
		while(1) {
			if ((numbytes=recv(Socket_Cliente, buf, MAXDATASIZE, 0)) == -1) 
			{
				perror("recv");
				if (send(Socket_Cliente, "ERRO!\n\0", 6, 0) == -1)
				{
					perror("Texto Enviado");
					close(Socket_Cliente);
					exit(0);
				}
				exit(1);
			}
			printf("O Que eu recebi: %s", buf);
			if(!strcmp(buf, "GERA_CARTELA\n")) {
				printf("Gerando e enviando nova cartela...\n");
				int *cartela = (int*)malloc(41);
				char *convert = (char*)malloc(sizeof(char)*72);
				retornaCartela(cartela);
				ordenaCrescente(cartela);
				/*printaCartela(cartela);*/
				convert = converteCartela(cartela);
				convert = convert + '\n';
				printaMensagem(convert);
				if (send(Socket_Cliente, convert, strlen(convert), 0) == -1)
				{
					perror("Texto Enviado");
					close(Socket_Cliente);
					exit(0);
				}
				/*free(cartela);
				free(convert);*/
			} else {
				printf("Nao entendi o comando\n");
				if (send(Socket_Cliente, "E\n", 2, 0) == -1)
				{
					perror("Texto Enviado");
					close(Socket_Cliente);
					exit(0);
				}
			}
			strcpy(buf, "");
		}
		close(Socket_Cliente); 
	}
}

/*============================================================================================================*/

void retornaCartela(int *cartela) {
	int j, numerosDiversos = 0, numeroGerado, tamanho = 0;
	while(numerosDiversos < 24) {
		numeroGerado = randomInteger(1, 99);
		for(j = 0; j < numerosDiversos; j++)
			if(cartela[j] == numeroGerado)
				break;
		if(j == numerosDiversos) {
			cartela[numerosDiversos] = numeroGerado;
			numerosDiversos++;
		}
	}
	cartela[numerosDiversos] = '\0';
}

void ordenaCrescente(int* cartela) {
	int i, j, aux;
	for(i = 0; i < 24; i++)	{
		for(j = 23; j > i; j--) {
			if(cartela[j] < cartela[j-1]) {
				aux = cartela[j-1];
				cartela[j-1] = cartela[j];
				cartela[j] = aux;
			}
		}
	}
}

int randomInteger (int low, int high) {
	int k;
/*	srand(time(NULL));
	k = (rand() % (high - low)) + low + 1;
	srand(k);
	k = (rand() % (high - low)) + low + 1;
	return k;*/
    double d;
    d = (double) rand () / ((double) RAND_MAX + 1);
    k = d * (high - low + 1);
    return low + k;
}

void printaCartela(int* cartela) {
	int i;
	for(i = 0; i < 24; i++)
		printf("|	%d	|", cartela[i]);
}

void printaMensagem(char* mensagem) {
	int i;
	for(i = 0; i < strlen(mensagem); i++)
		printf("%c", mensagem[i]);
}

char* converteCartela(int* cartela) {
	int i, unidade, dezena, tamConvert = 0;
	char *pointerConvert = (char*)malloc(sizeof(char)*72);
	double auxConvert;
	for(i = 0; i < 24; i++) {
		unidade = dezena = auxConvert = 0;
		tamConvert += 3;
		if(cartela[i] < 10) {
			unidade = cartela[i];
			pointerConvert[tamConvert-3] = '0';
			pointerConvert[tamConvert-2] = '0' + unidade;
		} else {
			dezena = (cartela[i]/10);
			auxConvert = (cartela[i]%10);
			unidade = auxConvert;
			pointerConvert[tamConvert-3] = '0' + dezena;
			pointerConvert[tamConvert-2] = '0' + unidade;
		}
		pointerConvert[tamConvert-1] = ';';
	}
	pointerConvert[tamConvert - 1] = '\0';
	return pointerConvert;
}
