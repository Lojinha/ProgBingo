#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

#define MAXDATASIZE 100
#define	PORTA	8181
#define	SALA_ESPERA	"SALA_ESPERA\n"
#define	MAXNAMEUSER	16

#ifdef __WIN32
	#include <winsock2.h>
	#include <conio.h>
	WSADATA data; 
	SOCKET Socket_Server, Socket_Cliente, new_sock; 
	SOCKADDR_IN EServer;
	SOCKADDR_IN ECliente;
	/*linker -> -l wsock32*/
#elif __linux__
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	int Socket_Server, Socket_Cliente, new_sock;
	struct sockaddr_in EServer;    
	struct sockaddr_in ECliente; 
	void closesocket(int socket) { close(socket); }
	/*linker -> -lws2_32 -lwsock32 -L $MinGW\lib*/
#endif

void retornaCartela(int *);
void ordenaCrescente(int *);
void printaCartela(int *);
void printaMensagem(char *);
char* converteCartela(int *);
void *thread_func(void*);

int ID_CONT_CLIENT = 1;
char **USER_ON;
bool travaThread;

int main()
{
	#ifdef __WIN32
		if(WSAStartup(MAKEWORD(1,1),&data)==SOCKET_ERROR){ 
			printf("Erro ao inicializar o winsock"); 
			return -1;
		}
	#endif
	
	int tamanho;
	USER_ON = (char*)malloc(ID_CONT_CLIENT + 1);
	USER_ON[0] = "ADMIN";
	USER_ON[1] = (char*)malloc(MAXNAMEUSER);

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
		printf("Servidor: chegando conexao de %d\nID THREAD: %d\n\n",inet_ntoa(ECliente.sin_addr), ID_CONT_CLIENT);
		travaThread = true;
		
		pthread_t sniffer_thread;
        new_sock = malloc(1);
        new_sock = Socket_Cliente;
		        
        if( pthread_create( &sniffer_thread , NULL ,  thread_func , (void*) new_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }
        while(travaThread);
        ID_CONT_CLIENT++;
        USER_ON = realloc(USER_ON, (ID_CONT_CLIENT + 1)*sizeof(char));
        USER_ON[ID_CONT_CLIENT] = (char*)malloc(MAXNAMEUSER);
	}
	close(Socket_Cliente); 
	return 0;
}

/*============================================================================================================*/

void *thread_func(void *arg) {
	char buf[MAXDATASIZE], NOME_USER[MAXNAMEUSER];
	int numbytes, OWN_ID, i, contUser = 0;
	
	#ifdef __WIN32
		SOCKET sock; 
	#elif __linux__
		int sock;
	#endif
	
	OWN_ID = ID_CONT_CLIENT;
	sock = (int*)arg;
	travaThread = false;
	
	if(send(sock, "OK\n", 4, 0) == -1) {
		close(sock);
		perror("Texto Enviado");
	} else {
		if ((numbytes=recv(sock, NOME_USER, MAXNAMEUSER, 0)) == -1) {
			perror("Recebendo o nome");
			close(sock);
		} else {
			if(NOME_USER[strlen(NOME_USER) - 1] == '\n') NOME_USER[strlen(NOME_USER) - 1] = '\0';
			USER_ON[OWN_ID] = NOME_USER;
		}
		if (send(sock, "S\n", 2, 0) == -1) {
			perror("Texto Enviado");
			close(sock);
			exit;
		}
		while(1) {
			if ((numbytes=recv(sock, buf, MAXDATASIZE, 0)) == -1) {
				perror(("Recebimento. Thread %d", OWN_ID));
				if (send(sock, "ERRO!\n\0", 6, 0) == -1)
				{
					perror(("Texto Enviado. Thread %d", OWN_ID));
					close(sock);
					break;
				}
				break;
			}
			printf("O Que eu recebi: %s\nThread ID: %d\n", buf, OWN_ID);
			if(!strcmp(buf, "GERA_CARTELA\n")) {
				printf("Gerando e enviando nova cartela...\n\n");
				int *cartela = (int*)malloc(41);
				char *convert = (char*)malloc(sizeof(char)*72);
				retornaCartela(cartela);
				ordenaCrescente(cartela);
				/*printaCartela(cartela);*/
				convert = converteCartela(cartela);
				convert = convert + '\n';
				/*printaMensagem(convert);*/
				if (send(sock, convert, strlen(convert), 0) == -1)
				{
					perror(("Enviando Texto. Thread %d", OWN_ID));
					close(sock);
					break;
				}
				/*free(cartela);
				free(convert);*/
			} else if(!strcmp(buf, "q\n")) {
				break;
			} else if(!strcmp(buf, "LIST_USER\n")) {
				for(i = 0; i <= ID_CONT_CLIENT; i++) {
					contUser += strlen(USER_ON[i]);
				}
				char *list_user = (char*)malloc( sizeof(char) * (contUser + ID_CONT_CLIENT) );
				strcpy(list_user, "");
				for(i = 0; i <= ID_CONT_CLIENT; i++) {
					strcat(list_user, USER_ON[i]);
					if(i < ID_CONT_CLIENT)
						list_user += ';';
				}
				if (send(sock, list_user, strlen(list_user), 0) == -1)
				{
					perror(("Texto Enviado. Thread %d", OWN_ID));
					close(sock);
					break;
				}
			} else {
				printf("Nao entendi o comando\n\n");
				if (send(sock, "E\n", 2, 0) == -1)
				{
					perror(("Texto Enviado. Thread %d", OWN_ID));
					close(sock);
					break;
				}
			}
			strcpy(buf, "");
		}
		printf("Desconectando. Thread ID: %d", OWN_ID);
	}
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
