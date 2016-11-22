#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>

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
	void sysClear(void) { system("cls"); }
	/*linker -> -l wsock32*/
#elif __linux__
	#include <sys/stat.h>
	#include <sys/types.h>
	#include <fcntl.h>
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	int Socket_Server, Socket_Cliente, new_sock;
	struct sockaddr_in EServer;    
	struct sockaddr_in ECliente; 
	void closesocket(int socket) { close(socket); }
	void sysClear(void) { system("clear"); }
	/*      FUNCOES GPIO RASPBERRY      */
	bool GPIOexport(int pino){
		char 	buffer[3];
		int 	arquivo;
		arquivo = open("/sys/class/gpio/export", O_WRONLY);
		if( arquivo == -1 )
			return false;
		snprintf( buffer, 3, "%d", pino );
		if( write( arquivo, buffer, 3 ) == -1 )	{
			close(arquivo);
			return false;
		}	
		close(arquivo);
		return true;
	}
	bool GPIOUnexport(int pino) {
		char 	buffer[3];
		int 	arquivo
		arquivo = open("/sys/class/gpio/unexport", O_WRONLY);
		if( arquivo == -1 )
			return false;
		snprintf( buffer, 3, "%d", pino );
		if( write( arquivo, buffer, 3 ) == -1 )	{
			close(arquivo);
			return false;
		}	
		close(arquivo);
		return true;
	}
	bool GPIOdirection(int pino, int direcao) {
		char 	caminho[35];
		int	arquivo;
		snprintf( caminho, 35, "/sys/class/gpio/gpio%d/direction", pino);
		arquivo = open(caminho, O_WRONLY);
		if( arquivo == -1 )
			return false;
		if( write( arquivo, (direcao == ENTRADA) ? "in" : "out", (direcao == ENTRADA) ? 2 : 3 ) == -1 ) {
			close(arquivo);
			return false;
		}
		close(arquivo);
		return true;
	}
	bool GPIOWrite(int pino, int valor) {
		char 	caminho[35];
		int	arquivo;
		snprintf( caminho, 35, "/sys/class/gpio/gpio%d/value", pino);
		arquivo = open(caminho, O_WRONLY);
		if( arquivo == -1 )
			return false;
		if( write( arquivo, (valor == HIGH)?"1":"0", 1 ) == -1 ) {
			close(arquivo);
			return false;
		}
		close(arquivo);
		return true;
	}
	bool GPIORead(int pino) {
		char 	caminho[35], retorno[2];
		int	arquivo;	
		snprintf( caminho, 35, "/sys/class/gpio/gpio%d/value", pino);
		arquivo = open(caminho, O_RDONLY);
		if( arquivo == -1 )
			return false;
		if( read(arquivo, retorno, 2) == -1 ) {
			close(arquivo);
			return false;
		}
		close(arquivo);
		return (int)retorno[0] - 48;
	}
	/*linker -> -lws2_32 -lwsock32 -L $MinGW\lib*/
#endif

/*      FUNCOES CARTELA/SERVIDOR      */
void retornaCartela(int *);
void ordenaCrescente(int *);
void printaCartela(int *);
void printaMensagem(char *);
char* converteCartela(int *);
void *thread_func(void*);
void printaClientes(void);

struct CLIENT{
	int ID_CLIENT;
	char NOME_CLIENT[MAXNAMEUSER];
	bool ATIVO_CLIENT;
};
struct CLIENT *ACTIVE_CLIENTS;

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
	
	ACTIVE_CLIENTS = (struct CLIENT*)malloc( (ID_CONT_CLIENT + 1) * sizeof(struct CLIENT) );
	strcpy(ACTIVE_CLIENTS[ID_CONT_CLIENT - 1].NOME_CLIENT, "ADMIN");
	ACTIVE_CLIENTS[ID_CONT_CLIENT - 1].ID_CLIENT = ID_CONT_CLIENT;
	ACTIVE_CLIENTS[ID_CONT_CLIENT - 1].ATIVO_CLIENT = false;
	
	/*USER_ON = (char*)malloc(ID_CONT_CLIENT + 1);
	USER_ON[0] = "ADMIN";
	USER_ON[1] = (char*)malloc(MAXNAMEUSER);*/

	if ((Socket_Server = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	EServer.sin_family = AF_INET;
	EServer.sin_port = htons(PORTA);
	EServer.sin_addr.s_addr = INADDR_ANY;   

	if (bind(Socket_Server, (struct sockaddr *)&EServer, sizeof(struct sockaddr))== -1) {
		perror("Conexao Serv");
		exit(1);
	}
	if (listen(Socket_Server, 10) < 0) {
		perror("Aguardando");
		exit(1);
	}
	printf("Aguardando conexao...\n\n");

	while(1) {
		printaClientes();
		tamanho = sizeof(struct sockaddr_in);
		if ((Socket_Cliente = accept(Socket_Server, (struct sockaddr *)&ECliente,&tamanho)) < 0){
			perror("Conexao aceita");
			continue;
		}
		sysClear(); /* ta dando erro*/
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
        
		/*USER_ON = realloc(USER_ON, (ID_CONT_CLIENT + 1)*sizeof(char));
        USER_ON[ID_CONT_CLIENT] = (char*)malloc(MAXNAMEUSER);*/
        
        ACTIVE_CLIENTS = (struct CLIENT*)realloc(ACTIVE_CLIENTS, (ID_CONT_CLIENT + 1) * sizeof(struct CLIENT) );
        ACTIVE_CLIENTS[ID_CONT_CLIENT - 1].ID_CLIENT = ID_CONT_CLIENT;
        ACTIVE_CLIENTS[ID_CONT_CLIENT - 1].ATIVO_CLIENT = false;
	}
	close(Socket_Cliente); 
	return 0;
}

/*============================================================================================================*/

void printaClientes() {
	int i;
	
	printf("/-----------------------------------------------\\\n");
	printf("\tClientes Ativos:\n");
	for(i = 0; i < ID_CONT_CLIENT; i++) {
		/*printf("\t%s\n", USER_ON[i]);	*/
		if(ACTIVE_CLIENTS[i].ATIVO_CLIENT)
			if(strlen(ACTIVE_CLIENTS[i].NOME_CLIENT) < 7)
				printf("\tJogador: %s\t\t\tID: %d\n", ACTIVE_CLIENTS[i].NOME_CLIENT, ACTIVE_CLIENTS[i].ID_CLIENT);	
			else
				printf("\tJogador: %s\t\tID: %d\n", ACTIVE_CLIENTS[i].NOME_CLIENT, ACTIVE_CLIENTS[i].ID_CLIENT);	
	}
	printf("\\-----------------------------------------------/\n\n");
}

void *thread_func(void *arg) {
	char buf[MAXDATASIZE], NOME_USER[MAXNAMEUSER];
	int numbytes, OWN_ID, i, contUser = 0;
	bool erro = false;
	
	#ifdef __WIN32
		SOCKET sock; 
	#elif __linux__
		int sock;
	#endif
	
	OWN_ID = ID_CONT_CLIENT;
	sock = (int*)arg;
	travaThread = false;
	
	if(send(sock, "OK\n", 4, 0) == -1) {
		perror("Texto Enviado");
		erro = true;
	} else {
		if ((numbytes=recv(sock, NOME_USER, MAXNAMEUSER, 0)) == -1) {
			perror("Recebendo o nome");
			erro = true;
		} else {
			if(NOME_USER[strlen(NOME_USER) - 1] == '\n') NOME_USER[strlen(NOME_USER) - 1] = '\0';
			/*USER_ON[OWN_ID] = NOME_USER;*/
			strcpy(ACTIVE_CLIENTS[OWN_ID].NOME_CLIENT, NOME_USER);
			ACTIVE_CLIENTS[OWN_ID].ATIVO_CLIENT = true;
			sysClear();
			printaClientes();
		}
		if (send(sock, "S\n", 2, 0) == -1) {
			perror("Texto Enviado");
			erro = true;
		}
		while(!erro) {
			if ((numbytes=recv(sock, buf, MAXDATASIZE, 0)) == -1) {
				perror(("Recebimento. Thread %d", OWN_ID));
				erro = true;
			} else {
				printf("Thread ID: %d\nO Que eu recebi: %s\n", OWN_ID, buf);
			}
			if(!erro) {
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
					if (send(sock, convert, strlen(convert), 0) == -1) {
						perror(("Enviando Texto. Thread %d", OWN_ID));
						erro = true;
						break;
					}
					/*free(cartela);
					free(convert);*/
				} else if(!strcmp(buf, "q\n")) {
					break;
				} else if(!strcmp(buf, "LIST_USER\n")) {
					for(i = 0; i < ID_CONT_CLIENT; i++) {
						/*contUser += strlen(USER_ON[i]);*/
						contUser += strlen(ACTIVE_CLIENTS[i].NOME_CLIENT);
					}
					char *list_user = (char*)malloc( sizeof(char) * (contUser + ID_CONT_CLIENT) );
					strcpy(list_user, "");
					for(i = 0; i < ID_CONT_CLIENT; i++) {
						/*strcat(list_user, USER_ON[i]);*/
						strcat(list_user, ACTIVE_CLIENTS[i].NOME_CLIENT);
						if(i < ID_CONT_CLIENT - 1)
							strcat(list_user, ";");
					}
					if (send(sock, list_user, strlen(list_user), 0) == -1)
					{
						perror(("Texto Enviado. Thread %d", OWN_ID));
						erro = true;
						break;
					}
					printf("enviei relacao de active: %s\n", list_user);
				} else {
					printf("Nao entendi o comando\n\n");
					if (send(sock, "E\n", 2, 0) == -1)
					{
						perror(("Texto Enviado. Thread %d", OWN_ID));
						erro = true;
						break;
					}
				}
				strcpy(buf, "");
			}
		}
	}
	ACTIVE_CLIENTS[OWN_ID].ATIVO_CLIENT = false;
	sysClear();
	printaClientes();
	printf("Desconectando. Thread ID: %d\n\n", OWN_ID);
	closesocket(socket);
	pthread_exit(NULL);
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
