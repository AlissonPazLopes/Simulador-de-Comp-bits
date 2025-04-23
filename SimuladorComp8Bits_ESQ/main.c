/*                TRABALHO FEITO POR :
                    JOSÉ ALISSON        */

#include "computador.h"
#include "opcodes.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
    #include <windows.h>
    #define SLEEP(ms) Sleep(ms)
#else
    #include <unistd.h>
    #define SLEEP(ms) sleep(ms / 1000)  // Converte milissegundos para segundos
#endif


#define CAMINHO_ARQUIVO "../programas/instrucoes.txt"

void boot_system(Computador * meuComputador);
void carregar_operandos(const char *nome_arquivo, Computador *meuComputador);
void carregar_operacoes(const char *nome_arquivo, Computador *meuComputador);
uint8_t obter_opcode(const char *instrucao);
void mostrar_memoria(Computador *meuComputador);

int main() {
    Computador meuComputador;   
	boot_system(&meuComputador); 
	#ifdef _WIN32
	system("pause");
	#endif

	return 0;
}

void boot_system(Computador * meuComputador){
	/*Cabe�alho*/
	printf("\t\t\t=========================================================\n");
    printf("\t\t\tSIMULADOR DE COMPUTADOR DE 8 BITS - v0.1  by Prof. Reuber e otimazo por Alisson\n");
    printf("\t\t\t=========================================================\n");
    
    int i=0;
    printf("\nCarregando Programa na Memoria RAM");
    while(i++<3){
    	printf(" . ");
		SLEEP(1000);    	
	}
	printf("\n\n");
	
	inicializar_computador(meuComputador);
		
	 // Carregar os operandos do arquivo
    carregar_operandos(CAMINHO_ARQUIVO, meuComputador);    
    // Carregar as opera��es do arquivo
    carregar_operacoes(CAMINHO_ARQUIVO, meuComputador);	
	mostrar_memoria(meuComputador);
	
	executar_programa(meuComputador);
	
	mostrar_memoria(meuComputador);	
}


// Fun��o para carregar os operandos na mem�ria
void carregar_operandos(const char *nome_arquivo, Computador *meuComputador) {
    FILE *arquivo = fopen(nome_arquivo, "r");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    char linha[256];
    uint8_t endereco = 9;  // Comecamos a carregar os operandos a partir do endereco 9

    // Carregar operandos ate encontrar uma linha vazia ou o inicio das operadores
    while (fgets(linha, sizeof(linha), arquivo)) {
        // Ignora linhas vazias e coment�rios
        if (linha[0] == '\n' || linha[0] == '#') {
            continue;
        }

        // Verifica se encontrou uma linha indicando o inicio das operacoes
        if (strstr(linha, "Opera��es") != NULL) {
            break; // Sai do loop quando encontrar a parte de operacoes
        }

        uint8_t operando;
        if (sscanf(linha, "%hhd", &operando) == 1) {
            escrever_memoria(&meuComputador->ram, endereco++, operando);
        } 
    }

    fclose(arquivo);
}

// Funcao para carregar as operacoes e enderecos na memoria
void carregar_operacoes(const char *nome_arquivo, Computador *meuComputador) {
    FILE *arquivo = fopen(nome_arquivo, "r");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    char linha[256];
    uint8_t endereco = 0;  // As operacoes comecam a ser carregadas a partir do endereco 0

    // Carregar operacoes ate o fim do arquivo
    while (fgets(linha, sizeof(linha), arquivo)) {
        // Ignora linhas vazias e coment�rios
        if (linha[0] == '\n' || linha[0] == '#') {
            continue;
        }

        uint8_t operando;
        char instrucao[10];  // Para armazenar o nome da operacao (ex: "LDD", "ADD", etc.)

        // A primeira parte da linha e a operacao e a segunda parte e o operando
        if (sscanf(linha, "%s %hhx", instrucao, &operando) == 2) {
            uint8_t opcode = obter_opcode(instrucao);
            if (opcode != 0) {
                escrever_memoria(&meuComputador->ram, endereco++, opcode | operando);
            } else {
                printf("Instrucao invalida: %s\n", instrucao);
            }
        } 
    }

    fclose(arquivo);
}



// esse é o opcode que será usado para identificar a instrução
// a cada etapa do programa ele verifica qual instrução está sendo executada
// e retorna o opcode correspondente
uint8_t obter_opcode(const char *instrucao) {
    if (strcmp(instrucao, "NOP") == 0) return NOP;
    if (strcmp(instrucao, "LDI") == 0) return LDI;
    if (strcmp(instrucao, "LDD") == 0) return LDD;
    if (strcmp(instrucao, "LDA") == 0) return LDA;
    if (strcmp(instrucao, "STA") == 0) return STA;
    if (strcmp(instrucao, "ADD") == 0) return ADD;
    if (strcmp(instrucao, "SUB") == 0) return SUB;
    if (strcmp(instrucao, "MUL") == 0) return MUL;
    if (strcmp(instrucao, "DIV") == 0) return DIV;
    if (strcmp(instrucao, "OUT") == 0) return OUTT;
    if (strcmp(instrucao, "HLT") == 0) return HLT;
    
    return 0;  // **Correção: Retorna 0 se a instrução não for encontrada**
}


//funcao que mostra a memoria
// ela é usada para mostrar a memoria antes e depois da execucao do programa
void mostrar_memoria(Computador *meuComputador) {
    printf("\nMemoria RAM:\n");// 
    printf("+-----+--------+\n");
    printf("| End. | Valor |\n");
    printf("+-----+--------+\n");

    for (int i = 0; i < 16; i++) {
        printf("|  %2d  |  0x%02X  |\n", i, ler_memoria(&meuComputador->ram, (uint8_t)i));
    }

    printf("+-----+--------+\n");
}