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


int conexao(void);
int configuracaoServidor(void);
int randomInteger (int, int);
void retornaCartela(int *);
void ordenaCrescente(int *);
void printaCartela(int *);
char* converteCartela(int *);

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
		char *convert;
		
		cartela = (int*)malloc(41);
		convert = (char*)malloc(sizeof(char)*72);
		
		retornaCartela(cartela);
		printaCartela(cartela);
		printf("\n\n");
		ordenaCrescente(cartela);
		printaCartela(cartela);
		
		convert = converteCartela(cartela);
		printf("\n\nCartela Em String:\n%s\n\n", convert);
		
		send(sockfd, convert, sizeof(convert), 0);
		
		//free(cartela);
		break;
    }
	getchar();
	closesocket(sockfd);
	return 0;
}

//============================================================================================================//]

void *thread_Serial(void *arg) {
    char buffer_do_cliente[256];
    int sockEntrada = *(int *) arg; // "Recebe" socket como argumento
    
    printf("Aguardando as mensagens... ");
    for (;;) {
        read(sockEntrada, buffer_do_cliente, sizeof (buffer_do_cliente));
        if (strcmp(buffer_do_cliente, "sair") != 0) {
            printf("%s\n",buffer_do_cliente);
    	} else if (strcmp(buffer_do_cliente, "cartela") != 0) {
    		int *cartela;	
			char *convert;
			cartela = (int*)malloc(41);
			convert = (char*)malloc(sizeof(char)*72);
			retornaCartela(cartela);
			ordenaCrescente(cartela);
			convert = converteCartela(cartela);
			send(sockEntrada, convert, sizeof(convert), 0);   		
		} else {
        	closesocket(sockEntrada);
        	pthread_exit((void*) 0);
        }
    }
}

int conexao() {
	if( accept(sockfd, 0, 0) == -1)
		return 0;
	else
		return 1;
}

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
