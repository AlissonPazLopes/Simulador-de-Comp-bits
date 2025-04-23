#include "uc.h"
#include "cpu.h"
#include <stdio.h>




//                      ---------------------- Inicialização ----------------------
// aqui inicializamos a unidade de controle, que é responsável por controlar o fluxo de dados
// entre a CPU e a memória
void inicializar_uc(UnidadeControle *uc) {
    uc->halt = 0;
}

//                      ---------------------- Ciclo de Busca ---------------------- 

// esse é o ciclo de busca é responsável por buscar a instrução na memória
// e armazená-la no registrador de instrução (IR)
// o ciclo de busca é executado até que uma interrupção seja detectada
void ciclo_busca(CPU *cpu, RAM *ram) {

    // MAR recebe o valor do PC e PC é incrementado
    cpu->reg.MAR = cpu->reg.PC;
    cpu->reg.PC++;
    // Leitura da memória: busca a instrução no endereço MAR
    enviar_dado(cpu->bus, ram, cpu->reg.MAR, 0, "READ");

    // Armazena o dado lido no MBR e carrega no IR
    cpu->reg.MBR = cpu->bus->dado & 0x00FF;
    cpu->reg.IR = cpu->reg.MBR;

    // Define o estado ICC com base na instrução carregada
    if ((cpu->reg.IR & 0xF0) == LDA) 
        cpu->reg.ICC = 1; // Necessário ciclo indireto
    else 
        cpu->reg.ICC = 2; // Pode ir direto para execução
}

//                          ---------------------- Ciclo Indireto ----------------------
void ciclo_indireto(CPU *cpu, RAM *ram) {
    printf(">>> Ciclo indireto <<<\n");

    // MAR recebe os 4 bits menos significativos de IR
    cpu->reg.MAR = cpu->reg.IR & 0x0F;

    // Busca o operando no endereço MAR
    enviar_dado(cpu->bus, ram, cpu->reg.MAR, 0, "READ");

    // Armazena o dado lido no MBR
    cpu->reg.MBR = cpu->bus->dado & 0x00FF;

    // Atualiza o IR com o valor do MBR (mantendo os 4 bits mais significativos de IR)
    cpu->reg.IR = (cpu->reg.IR & 0xF0) | (cpu->reg.MBR & 0x0F);

    // Define ICC como 2 (ciclo de execução)
    cpu->reg.ICC = 2;
}

//                         ---------------------- Ciclo de Execução ----------------------
void ciclo_execucao(CPU *cpu, RAM *ram) {
    uint8_t instrucao = cpu->reg.IR & 0xF0;

    switch (instrucao) {
        case LDI:  printf(">>> LDI Executando <<<\n");  ldi_exec(cpu); break;
        case LDD:  printf(">>> LDD Executando <<<\n");  ldd_exec(cpu, ram); break;
        case STA:  printf(">>> STA Executando <<<\n");  sta_exec(cpu, ram); break;
        case ADD:  printf(">>> ADD Executando <<<\n");  add_exec(cpu, ram); break;
        case SUB:  printf(">>> SUB Executando <<<\n");  sub_exec(cpu, ram); break;
        case MUL:  printf(">>> MUL Executando <<<\n");  mul_exec(cpu, ram); break;
        case DIV:  printf(">>> DIV Executando <<<\n");  div_exec(cpu, ram); break;
        case OUTT: printf(">>> OUT Executando <<<\n");  out_exec(cpu, ram); break;
        case HLT:  printf(">>> HALT Processor <<<\n"); cpu->uc.halt = 1; return;
        default:   printf("Erro: Instrução desconhecida: 0x%X\n", instrucao); cpu->uc.halt = 1; return;
    }
    if (cpu->reg.ICC != 3) cpu->reg.ICC = 0;
}
//                      ---------------------- Ciclo de Interrupção ----------------------
// toda vez que uma interrupção é detectada, o ciclo de interrupção é executado
// e o valor do PC é armazenado no endereço de interrupção (0x0F)
// os registradores são resetados e o ciclo de busca é executado novamente
// e o processamento continua	
void ciclo_interrupcao(CPU *cpu, RAM *ram) {
    printf("\n>>> Ciclo de Interrupcao <<<\n");
    if (cpu->reg.FLAG == 2) {
        printf(">>> ERRO: ULA -> DIVISÃO POR ZERO! CPU será interrompida.\n");
        cpu->uc.halt = 1;
        return;
    }
    else if (cpu->reg.FLAG == 1) {
        printf(">>> ERRO: ULA -> OVERFLOW!\n");
    }
    cpu->reg.MBR = cpu->reg.PC;
    cpu->reg.MAR = 0x0F;
    enviar_dado(cpu->bus, ram, cpu->reg.MAR, cpu->reg.MBR, "WRITE");
    cpu->reg.FLAG = 0;
    while ((cpu->reg.IR & 0xF0) != HLT) {
        ciclo_busca(cpu, ram);
    }
}



void ldi_exec(CPU *cpu) {
    cpu->reg.AC = cpu->reg.IR & 0x0F; // Carrega o operando no registrador AC
    cpu->reg.ICC = 0;                 // Após executar, volta ao ciclo de busca
}


void ldd_exec(CPU *cpu, RAM *ram) {
    cpu->reg.MAR = cpu->reg.IR & 0x0F;
    enviar_dado(cpu->bus, ram, cpu->reg.MAR, 0, "READ");
    cpu->reg.AC = cpu->bus->dado & 0x00FF;
    cpu->reg.ICC = 0;
}
void sta_exec(CPU *cpu, RAM *ram) {
    cpu->reg.MAR = cpu->reg.IR & 0x0F;
    enviar_dado(cpu->bus, ram, cpu->reg.MAR, cpu->reg.AC, "WRITE");
}

void add_exec(CPU *cpu, RAM *ram) {
    cpu->reg.MAR = cpu->reg.IR & 0x0F;
    enviar_dado(cpu->bus, ram, cpu->reg.MAR, 0, "READ");
    cpu->reg.MBR = cpu->bus->dado & 0x00FF;
    
    ULA_ADD(&(cpu->reg.AC), &(cpu->reg.MBR), &(cpu->reg.FLAG));
    
    if (cpu->reg.FLAG == 1) {
        cpu->reg.ICC = 3;
    } else {
        cpu->reg.ICC = 0;
    }
}

void sub_exec(CPU *cpu, RAM *ram) {
    cpu->reg.MAR = cpu->reg.IR & 0x0F;
    enviar_dado(cpu->bus, ram, cpu->reg.MAR, 0, "READ");
    cpu->reg.MBR = cpu->bus->dado & 0x00FF;
    
    ULA_SUB(&(cpu->reg.AC), &(cpu->reg.MBR), &(cpu->reg.FLAG));
    
    if (cpu->reg.FLAG == 1) {
        cpu->reg.ICC = 3;
    } else {
        cpu->reg.ICC = 0;
    }
}

void mul_exec(CPU *cpu, RAM *ram) {
    cpu->reg.MAR = cpu->reg.IR & 0x0F;
    enviar_dado(cpu->bus, ram, cpu->reg.MAR, 0, "READ");
    cpu->reg.MBR = cpu->bus->dado & 0x00FF;

    cpu->reg.Y = cpu->reg.AC;
    ULA_MUL(&(cpu->reg.Z), &(cpu->reg.Y), &(cpu->reg.MBR), &(cpu->reg.FLAG));

    cpu->reg.AC = cpu->reg.Y;

    if (cpu->reg.FLAG == 1) {
        cpu->reg.ICC = 3;
    } else {
        cpu->reg.ICC = 0;
    }
}

void div_exec(CPU *cpu, RAM *ram) {
    cpu->reg.MAR = cpu->reg.IR & 0x0F;
    enviar_dado(cpu->bus, ram, cpu->reg.MAR, 0, "READ");
    cpu->reg.MBR = cpu->bus->dado & 0x00FF;

    if (cpu->reg.MBR == 0x00) {  // 🚨 Detecta divisão por zero corretamente
        printf("\n>>> ERRO: Tentativa de divisão por zero! Acionando interrupção.\n");

        cpu->reg.FLAG = 2;  // 🚨 FLAG = 2 indica erro de divisão por zero
        cpu->reg.ICC = 3;   // 🚨 Envia para ciclo de interrupção

        // 🚨 NÃO altera MAR, MBR e IR → Preserva valores para interrupção
        return;
    } else {
        cpu->reg.Y = cpu->reg.AC;
        ULA_DIV(&(cpu->reg.Z), &(cpu->reg.Y), &(cpu->reg.MBR), &(cpu->reg.FLAG));
        cpu->reg.AC = cpu->reg.Y;

        if (cpu->reg.FLAG == 1) {
            cpu->reg.ICC = 3;
        } else {
            cpu->reg.ICC = 0;
        }
    }
}


void out_exec(CPU *cpu, RAM * ram){
	cpu->reg.OTR = cpu->reg.AC;
	cpu->reg.PC-=2;
	ciclo_busca(cpu, ram);	
	if((cpu->reg.IR&0xF0)==MUL)
		printf("DISPLAY\tMUL: REG Z: %d\tREG OTR:%d\tZOTR: %d\n",(int8_t)cpu->reg.Z, (int8_t)cpu->reg.OTR,(int16_t) (((cpu->reg.Z & 0x00FF) << 8) | (cpu->reg.OTR & 0x00FF)));
	else if((cpu->reg.IR&0xF0)==DIV)
		printf("DISPLAY\tDIV: Resto em REG Z: %d\tQuociente em REG OTR:%d\n",(int8_t)cpu->reg.Z, (int8_t)cpu->reg.OTR);
	else
		printf(" DISPLAY: %d\n", (int8_t) cpu->reg.OTR);
	
	cpu->reg.PC++;
	cpu->reg.ICC = 0;
}

