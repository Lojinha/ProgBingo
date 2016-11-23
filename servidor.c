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
	#define	ENTRADA	1
	#define	SAIDA	0
	#define	HIGH	1
	#define	LOW	0

	FILE *arquivo;
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
		int 	arquivo;
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
char * converteCartela(int *);
void * thread_func(void*);
void printaClientes(void);
bool comparaBoard(char*);
int retornaRodadaAtual(void);
int retornaID(char*);
char * retornaNumerosSorteados(int);
char * retornaCartelaCliente(int);
char * retornaCartelaCliente_IdChar(char *);
char * retornaGanhador(int);
char * retornaNomeCliente(int);
bool retornaStatusCartela(int);
bool retornaStatusCliente(int);

struct CLIENT{
	int ID_CLIENT;
	char NOME_CLIENT[MAXNAMEUSER];
	char *BOARD_CLIENT;
	bool BOARD_ACTIVE;
	bool ATIVO_CLIENT;
};
struct CLIENT *ACTIVE_CLIENTS;

struct BINGO{
	int TOT_SORTEADOS;
	int *NUMEROS_SORTEADOS;
	int ID_GANHADOR;
};
struct BINGO *RODADA_BINGO;
int RODADA_ATUAL = 0;

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
	sysClear();	

	int tamanho;
	
	ACTIVE_CLIENTS = (struct CLIENT*)malloc( (ID_CONT_CLIENT + 1) * sizeof(struct CLIENT) );
	strcpy(ACTIVE_CLIENTS[ID_CONT_CLIENT - 1].NOME_CLIENT, "ADMIN");
	ACTIVE_CLIENTS[ID_CONT_CLIENT - 1].ID_CLIENT = ID_CONT_CLIENT - 1;
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
		if ((Socket_Cliente = accept(Socket_Server, (struct sockaddr *)&ECliente,&tamanho)) < 0) {
			perror("Conexao aceita");
			continue;
		}
		sysClear(); /* ta dando erro*/
		printf("Servidor: chegando conexao de %d\nID THREAD: %d\n\n",inet_ntoa(ECliente.sin_addr), ID_CONT_CLIENT);
		travaThread = true;
		
		pthread_t sniffer_thread;
        new_sock = malloc(1);
        new_sock = Socket_Cliente;
		        
        if( pthread_create( &sniffer_thread , NULL ,  thread_func , (void*) new_sock) < 0) {
            perror("could not create thread");
            return 1;
        }
        while(travaThread);
        ID_CONT_CLIENT++;
        
		/*USER_ON = realloc(USER_ON, (ID_CONT_CLIENT + 1)*sizeof(char));
        USER_ON[ID_CONT_CLIENT] = (char*)malloc(MAXNAMEUSER);*/
        
        ACTIVE_CLIENTS = (struct CLIENT*)realloc(ACTIVE_CLIENTS, (ID_CONT_CLIENT + 1) * sizeof(struct CLIENT) );
        ACTIVE_CLIENTS[ID_CONT_CLIENT - 1].ID_CLIENT = ID_CONT_CLIENT - 1;
        ACTIVE_CLIENTS[ID_CONT_CLIENT - 1].ATIVO_CLIENT = false;
	}
	close(Socket_Cliente); 
	return 0;
}

/*============================================================================================================*/

void printaClientes() {
	int i;
	
	printf("/----------------------------------------------------------------------\\\n");
	printf("\tClientes Ativos:\n");
	for(i = 0; i < ID_CONT_CLIENT; i++) {
		/*printf("\t%s\n", USER_ON[i]);	*/
		if(ACTIVE_CLIENTS[i].ATIVO_CLIENT) {
			if(strlen(ACTIVE_CLIENTS[i].NOME_CLIENT) < 7)
				printf("\tJogador: %s\t\t\tID: %d\t", ACTIVE_CLIENTS[i].NOME_CLIENT, ACTIVE_CLIENTS[i].ID_CLIENT);	
			else
				printf("\tJogador: %s\t\tID: %d\t", ACTIVE_CLIENTS[i].NOME_CLIENT, ACTIVE_CLIENTS[i].ID_CLIENT);
			if(ACTIVE_CLIENTS[i].BOARD_ACTIVE)
				printf("Cartela: S\n");
			else
				printf("Cartela: N\n");
		}
				
	}
	printf("\\----------------------------------------------------------------------/\n\n");
}

void *thread_hardware(void *arg) {
	
}

void *thread_func(void *arg) {
	char buf[MAXDATASIZE], NOME_USER[MAXNAMEUSER], *auxCommand, *valueCommand, tmpID[3];
	int numbytes, OWN_ID, i, contUser = 0, aux, numConvert;
	bool erro = false;
	
	#ifdef __WIN32
		SOCKET sock; 
	#elif __linux__
		int sock;
	#endif
	
	OWN_ID = ID_CONT_CLIENT;
	sock = (int*)arg;
	travaThread = false;
	
	strcpy(tmpID, "");
	
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
			if(strcmp(NOME_USER, "WEB"))
				ACTIVE_CLIENTS[OWN_ID].ATIVO_CLIENT = true;
			ACTIVE_CLIENTS[OWN_ID].BOARD_ACTIVE = false;
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
				if(buf[strlen(buf) - 1] == '\n') buf[strlen(buf) - 1] = '\0';
				if(!strcmp(buf, "GERA_CARTELA")) {
					int *cartela = (int*)malloc(41);
					char *convert = (char*)malloc(sizeof(char)*72);
					retornaCartela(cartela);
					ordenaCrescente(cartela);
					/*printaCartela(cartela);*/
					convert = converteCartela(cartela);
					ACTIVE_CLIENTS[OWN_ID].BOARD_CLIENT = (char*)malloc(sizeof(char)*72);
					strcpy(ACTIVE_CLIENTS[OWN_ID].BOARD_CLIENT, convert);
					ACTIVE_CLIENTS[OWN_ID].BOARD_ACTIVE = true;
					/*convert = convert + '\n';*/
					printf("Gerando e enviando nova cartela...\n%s\n\n", convert);
					if (send(sock, convert, strlen(convert), 0) == -1) {
						perror(("Enviando Texto. Thread %d", OWN_ID));
						erro = true;
						break;
					}
					/*free(cartela);
					free(convert);*/
				} else if(!strcmp(buf, "q\n") || !strcmp(buf, "q")) {
					break;
				} else if(!strcmp(buf, "CARTELA_CLIENTE")) {
					char * OWN_BOARD;
					if(retornaStatusCartela(OWN_ID)) {
						OWN_BOARD = retornaCartelaCliente(OWN_ID);
						if (send(sock, OWN_BOARD, strlen(OWN_BOARD), 0) == -1) {
							perror(("CARTELA_CLIENTE. Thread %d", OWN_ID));
							erro = true;
							break;
						}
					} else {
						if (send(sock, "NULL", 5, 0) == -1) {
							perror(("Enviando NULL. Thread %d", OWN_ID));
							erro = true;
							break;
						}
					}
				} else if(!strcmp(buf, "DEL_CARTELA\n") || !strcmp(buf, "DEL_CARTELA") || !strcmp(buf, "DEL_CARTELA\n\n") ) {
					printf("Deletando cartela...\n\n");
					ACTIVE_CLIENTS[OWN_ID].BOARD_ACTIVE = false;
					if (send(sock, "OK\n", 4, 0) == -1) {
						perror(("Enviando Texto. Thread %d", OWN_ID));
						erro = true;
						break;
					}
				} else if(!strcmp(buf, "LIST_USER")) {
					for(i = 0; i < ID_CONT_CLIENT; i++) {
						/*contUser += strlen(USER_ON[i]);*/
						if(ACTIVE_CLIENTS[i].ATIVO_CLIENT)
							contUser += strlen(ACTIVE_CLIENTS[i].NOME_CLIENT);
					}
					char *list_user = (char*)malloc( sizeof(char) * (contUser + ID_CONT_CLIENT) );
					strcpy(list_user, "");
					for(i = 0; i < ID_CONT_CLIENT; i++) {
						/*strcat(list_user, USER_ON[i]);*/
						if(ACTIVE_CLIENTS[i].ATIVO_CLIENT) {
							strcat(list_user, ACTIVE_CLIENTS[i].NOME_CLIENT);
							if(i < ID_CONT_CLIENT - 1)
								strcat(list_user, ";");
						}
					}
					if (send(sock, list_user, strlen(list_user), 0) == -1) {
						perror(("Texto Enviado. Thread %d", OWN_ID));
						erro = true;
						break;
					}
					printf("enviei relacao de active: %s\n", list_user);
				} else if(!strcmp(buf, "LIST_USER_ID")) {
					for(i = 0; i < ID_CONT_CLIENT; i++) {
						/*contUser += strlen(USER_ON[i]);*/
						if(ACTIVE_CLIENTS[i].ATIVO_CLIENT)
							contUser += strlen(ACTIVE_CLIENTS[i].NOME_CLIENT);
					}
					char *list_user = (char*)malloc( sizeof(char) * (contUser + ID_CONT_CLIENT + (ID_CONT_CLIENT * 3) ) );
					strcpy(list_user, "");
					for(i = 0; i < ID_CONT_CLIENT; i++) {
						/*strcat(list_user, USER_ON[i]);*/
						if(ACTIVE_CLIENTS[i].ATIVO_CLIENT) {
							itoa(ACTIVE_CLIENTS[i].ID_CLIENT, tmpID, 10);
							strcat(list_user, tmpID);
							strcat(list_user, "_");
							strcat(list_user, ACTIVE_CLIENTS[i].NOME_CLIENT);
							if(i < ID_CONT_CLIENT - 1)
								strcat(list_user, ";");
						}
					}
					if (send(sock, list_user, strlen(list_user), 0) == -1) {
						perror(("Texto Enviado. Thread %d", OWN_ID));
						erro = true;
						break;
					}
					printf("enviei relacao de active: %s\n", list_user);
				} else {
					for(i = 0; i < strlen(buf); i++) {
						if(buf[i] == '@')
							break;
					}
					
					if(i == strlen(buf)) {
						printf("Nao entendi o comando\n\n");
						if (send(sock, "E\n", 2, 0) == -1) {
							perror(("Texto Enviado. Thread %d", OWN_ID));
							erro = true;
							break;
						}
					} else {
						auxCommand = (char*)malloc( sizeof(char) * (i + 1) );
						for(i = 0; buf[i] != '@'; i++)
							auxCommand[i] = buf[i];
						auxCommand[i] = '\0';
						valueCommand = (char*)malloc( sizeof(char) * (strlen(buf) - i + 2) );
						aux = i + 1;
						for(i = aux; i < strlen(buf); i++)
							valueCommand[i - aux] = buf[i];
						valueCommand[i - aux] = '\0';
						if(!strcmp(auxCommand, "RET_CARTELA")) {
							if(isdigit(valueCommand)) {
								auxCommand = retornaCartelaCliente_IdChar(valueCommand);
							}
							printf("Cartela retornada: %s", auxCommand);
							if (send(sock, auxCommand, strlen(auxCommand), 0) == -1) {
								perror(("Enviando cartela. Thread %d", OWN_ID));
								erro = true;
								break;
							}
						} else if(!strcmp(auxCommand, "RET_NUM_SORT")) {
							if(isdigit(valueCommand)) {
								auxCommand = retornaNumerosSorteados(atoi(valueCommand));
							}
							if (send(sock, auxCommand, strlen(auxCommand), 0) == -1) {
								perror(("Enviando Num. Sort. Thread %d", OWN_ID));
								erro = true;
								break;
							}
						} else if(!strcmp(auxCommand, "RET_WINNER")) {
							if(isdigit(valueCommand)) {
								auxCommand = retornaGanhador(atoi(valueCommand));
							}
							if (send(sock, auxCommand, strlen(auxCommand), 0) == -1) {
								perror(("Enviando Ganhador. Thread %d", OWN_ID));
								erro = true;
								break;
							}
						} else if(!strcmp(auxCommand, "RET_NOME")) {
							if(isdigit(valueCommand)) {
								auxCommand = retornaNomeCliente(atoi(valueCommand));
								printf("AuxCommando = %s", auxCommand);
							}
							if (send(sock, auxCommand, strlen(auxCommand), 0) == -1) {
								perror(("Enviando Nome. Thread %d", OWN_ID));
								erro = true;
								break;
							}
						} else if(!strcmp(auxCommand, "RET_ID")) {
							itoa(retornaID(valueCommand), auxCommand, 10);
							if (send(sock, auxCommand, strlen(auxCommand), 0) == -1) {
								perror(("Enviando ID. Thread %d", OWN_ID));
								erro = true;
								break;
							}
						}
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

/*
	A seguinte função, cria no ponteiro passado por parâmetro, um array de 24 valores int não repetidos e desordenados.
	Utilização:
	char *ponteiro;
	int *ponteiro = (int*)malloc(41);
	retornaCartela(ponteiro);
*/
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

/*
	A seguinte função ordena um vetor dinâmico de int passado por parâmetro, de maneira crescente.
	Ex:
		Parâmetro inserido: int[] = {17, 2, 54, 20}
		Resultado:			int[] = {2, 17, 20, 54}
*/
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

/*
	A seguinte função recebe um vetor dinâmico de inteiros e converte para um vetor dinâmico de char, o qual é o retorno.
	O array de char é separado pelo char ';', e mesmo os números de uma casa, são convertidos para 2 casas. Os números máximos
	aceitados no vetor de parâmetro vão até 99.
	Ex:
		Parâmetro aplicado: int[] = {2, 4, 17, 20};
		Retorno: char* = "02;04;17;20";
*/
char* converteCartela(int* cartela) {
	int i, unidade, dezena, tamConvert = 0;
	char *pointerConvert;
	pointerConvert = (char*)malloc(sizeof(char)*72);
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

/*
	A Seguinte função quebra a string pelo char ';' retirando número a número da cartela 'board_stream' inserida
	Após isso, há um loop que corre a rodada atual do bingo, buscando se aquele valor realmente já foi sorteado,
		Caso exista, ele passa para o próximo número para verificar se já foi sorteado.
		Caso não exista, retorna false indicando que a cartela inserida difere dos números já sorteados.
	Ao fim, caso todos os números da cartela exista no array de numeros já sorteados, a função retornará true, indicando o bingo.	
*/
bool comparaBoard(char * board_stream) {
	int lenght_board_source, lenght_board_stream, i, j, CONT_STREAM, CONT_SOURCE_NUM, AUX_SPLIT = 0;
	bool bingo;
	char SPLIT_STREAM[3];
	
	lenght_board_source = RODADA_BINGO[RODADA_ATUAL].TOT_SORTEADOS;
	
	for(i = 0; board_stream[i] != '\0'; i++) {
		/* DO NOTHING */
	}
	lenght_board_stream = i - 1;
	
	if(lenght_board_source != lenght_board_stream)
		return false;
		
	for(CONT_STREAM = 0; CONT_STREAM <= lenght_board_stream; CONT_STREAM++) {
		if(board_stream[CONT_STREAM] != ';') {
			SPLIT_STREAM[AUX_SPLIT] = board_stream[CONT_STREAM];
			AUX_SPLIT++;
		} else {
			AUX_SPLIT = 0;
			bingo = false;
			for(CONT_SOURCE_NUM = 0; CONT_SOURCE_NUM < RODADA_BINGO[RODADA_ATUAL].TOT_SORTEADOS - 1; CONT_SOURCE_NUM++)	{
				if(atoi(SPLIT_STREAM) == RODADA_BINGO[RODADA_ATUAL].NUMEROS_SORTEADOS[CONT_SOURCE_NUM]) {
					bingo = true;
					break;
				}
			}
			if(!bingo)
				return false;
		}
	}
	return true;
}

int retornaRodadaAtual() {
	return RODADA_ATUAL;	
}

/*
	A Seguinte função retorna um array dinâmico de char que indica quais números ja foram sorteados na rodada passada por parâmetro.
	O Parâmetro é do tipo INT, e o separador é o char ';'.
	Ex:
		Parâmetro inserido: int = 2;
		Resultado:			char* = "43;12;64;25;63";
*/
char * retornaNumerosSorteados(int rodada) {
	char * numerosSorteados;
	int i, unidade, dezena, numeroAtual;
	double auxConvert;
	
	if(rodada > RODADA_ATUAL)
		return NULL;
	
	numerosSorteados = (char*)malloc( sizeof(char) * (RODADA_BINGO[rodada].TOT_SORTEADOS*2 + RODADA_BINGO[rodada].TOT_SORTEADOS) );
	for(i = 1; RODADA_BINGO[rodada].TOT_SORTEADOS; i++) {
		numeroAtual = RODADA_BINGO[rodada].NUMEROS_SORTEADOS[i - 1];
		if(numeroAtual < 10) {
			unidade = numeroAtual;
			numerosSorteados[(i*3) - 3] = '0';
			numerosSorteados[(i*3) - 2] = '0' + unidade;
		} else {
			dezena = (numeroAtual/10);
			auxConvert = (numeroAtual%10);
			unidade = auxConvert;
			numerosSorteados[(i*3) - 3] = '0' + dezena;
			numerosSorteados[(i*3) - 2] = '0' + unidade;
		}	
		numerosSorteados[i*3] = ';';
	}
	
}

char * retornaCartelaCliente(int ID_CLIENTE) {
	if(ID_CLIENTE > ID_CONT_CLIENT)
		return NULL;
	return ACTIVE_CLIENTS[ID_CLIENTE].BOARD_CLIENT;
}

bool retornaStatusCartela(int ID_CLIENTE) {
	if(ID_CLIENTE > ID_CONT_CLIENT)
		return false;
	return ACTIVE_CLIENTS[ID_CLIENTE].BOARD_ACTIVE;
}

bool retornaStatusCliente(int ID_CLIENTE) {
	if(ID_CLIENTE > ID_CONT_CLIENT)
		return false;
	return ACTIVE_CLIENTS[ID_CLIENTE].ATIVO_CLIENT;
}

char * retornaGanhador(int rodada) {
	if(rodada > RODADA_ATUAL || rodada < 0)
		return NULL;
	return retornaNomeCliente(RODADA_BINGO[rodada].ID_GANHADOR);
}

char * retornaNomeCliente(int ID_CLIENTE) {
	if(ID_CLIENTE > ID_CONT_CLIENT || ID_CLIENTE < 0)
		return NULL;
	return ACTIVE_CLIENTS[ID_CLIENTE].NOME_CLIENT;
}

int retornaID(char * NOME_CLIENTE) {
	int i;
	for(i = 0; i < ID_CONT_CLIENT; i++) {
		if(!strcmp(ACTIVE_CLIENTS[i].NOME_CLIENT, NOME_CLIENTE))
			return ACTIVE_CLIENTS[i].ID_CLIENT;
	}
	return -1;
}
