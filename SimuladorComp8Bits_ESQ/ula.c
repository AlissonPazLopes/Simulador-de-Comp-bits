#include <stdio.h>
#include <stdint.h>

// Definições auxiliares para manipulação de bits
void bit_set(uint8_t *num, uint8_t pos) {
    *num |= (1 << pos);
}

void bit_clr(uint8_t *num, uint8_t pos) {
    *num &= ~(1 << pos);
}

uint8_t bit_get(uint8_t num, uint8_t pos) {
    return (num >> pos) & 1;
}

// Somador Completo (1 bit)
static void somadorCompleto(uint8_t A, uint8_t B, uint8_t Cin, uint8_t *co, uint8_t *s) {
    *co = (A & B) | (A & Cin) | (B & Cin);
    *s = A ^ B ^ Cin;
}

// Somador de 8 bits
static void somador8bits(uint8_t A, uint8_t B, uint8_t cin, uint8_t *co, uint8_t *s) {
    *co = cin;
    *s = 0;

    for (int i = 0; i < 8; i++) {
        uint8_t bitA = bit_get(A, i);
        uint8_t bitB = bit_get(B, i);
        uint8_t somaBit, carryBit;

        somadorCompleto(bitA, bitB, *co, &carryBit, &somaBit);
        *co = carryBit;

        if (somaBit) {
            bit_set(s, i);
        } else {
            bit_clr(s, i);
        }
    }
}

// Complemento de dois (negação + 1)
static void complementador(uint8_t *A) {
    *A = ~(*A) + 1;
}

// **Função de Adição 
void ULA_ADD(int8_t *A, int8_t *B, int8_t *overflow) {
    uint8_t co;
    uint8_t resultado;

    somador8bits((uint8_t)*A, (uint8_t)*B, 0, &co, &resultado);

    
    *overflow = ((*A > 0 && *B > 0 && (int8_t)resultado < 0) || (*A < 0 && *B < 0 && (int8_t)resultado > 0));

    *A = (int8_t)resultado; 
}

// **Função de Subtração 
void ULA_SUB(int8_t *A, int8_t *B, int8_t *overflow) {
    uint8_t co;
    uint8_t resultado;
    uint8_t complementoB = (uint8_t)(~(*B) + 1); 

    somador8bits((uint8_t)*A, complementoB, 0, &co, &resultado);

    *overflow = ((*A > 0 && *B < 0 && (int8_t)resultado < 0) || (*A < 0 && *B > 0 && (int8_t)resultado > 0));

    *A = (int8_t)resultado;
}

// **Função de Multiplicação 
void ULA_MUL(int8_t *A, int8_t *Q, int8_t *M, int8_t *overflow) {
    int8_t A_reg = 0;
    int8_t Q_reg = *Q;
    int8_t M_val = *M;
    int Q_minus1 = 0;  

    int8_t testeOverflow = 0;

    for (int i = 0; i < 8; i++) {
        int Q0 = Q_reg & 1;           
        int action = (Q0 << 1) | Q_minus1; 

        if (action == 0b01) {
            
            ULA_ADD(&A_reg, &M_val, &testeOverflow);
        } else if (action == 0b10) {
            
            ULA_SUB(&A_reg, &M_val, &testeOverflow);
        }

        // Efetua o deslocamento aritmético para a direita no conjunto 
        int new_Q_minus1 = Q_reg & 1;              
        // Desloca Q_reg para a direita e insere o LSB de A_reg como novo bit mais significativo de Q
        uint8_t temp_Q = (uint8_t)Q_reg;
        temp_Q = (temp_Q >> 1) | ((A_reg & 1) << 7);
        Q_reg = (int8_t)temp_Q;
        
        A_reg = A_reg >> 1;

        Q_minus1 = new_Q_minus1;
    }

    
    *A = A_reg;
    *Q = Q_reg;
    *overflow = 0;
}

// **Função de Divisão
void ULA_DIV(int8_t *regA, int8_t *A, int8_t *B, int8_t *overflow) {
    if (*B == 0) {
        printf("\n>>> ERRO: Divisao por zero detectada na ULA!\n");
        *overflow = 2; // 🚨 Agora FLAG indica erro de divisão por zero
        *regA = 0;     // Mantemos um valor inválido
        *A = 0;        // Evita que a CPU continue com valores errados
    } else {
        *overflow = 0;
        *regA = *A % *B;
        *A = *A / *B;
    }
}

//----------------------------------------------------Operação em Ponto Flutuante-------------------------------------------------------------------------------------------------

//4 registradores de 8 bits
typedef struct {
    uint8_t reg[4];
} RegPF;

//decompor o float em sinal, expoente polarizado e mantissa
void decomposicao(RegPF *rpf, int *sinal, int *expoente, uint32_t *mantissa) {
    //combina os 4 registradores
    uint32_t bits = ((uint32_t)rpf->reg[3] << 24) | ((uint32_t)rpf->reg[2] << 16) | ((uint32_t)rpf->reg[1] << 8) | rpf->reg[0];

    //extrai bit de sinal, 8 bits do expoente e 23 bits da mantissa
    *sinal = (bits >> 31) & 1;
    *expoente = (bits >> 23) & 0xFF;
    *mantissa = bits & 0x7FFFFF;
}

//montar o numero 
void montar_float(RegPF *rpf, int sinal, int expoente, uint32_t mantissa) {
    //combinação de sinal, expoente e mantissa para formar um número de 32 bits
    uint32_t bits = (sinal << 31) | ((expoente & 0xFF) << 23) | (mantissa & 0x7FFFFF);

    //dividindo o número de 32 bits nos 4 registradores de 8 bits 
    rpf->reg[0] = bits & 0xFF;
    rpf->reg[1] = (bits >> 8) & 0xFF;
    rpf->reg[2] = (bits >> 16) & 0xFF;
    rpf->reg[3] = (bits >> 24) & 0xFF;
}


// **Função de Multiplicação em Ponto Flutuante
void ULA_MUL_PF(RegPF *A, RegPF *B, int8_t *overflow, int8_t *underflow) {
    int sinal_a, expoente_a, sinal_b, expoente_b;
    uint32_t mantissa_a, mantissa_b;
    *overflow = 0;
    *underflow = 0;
    int desloc = 0;
    
    //chamada da função "decomposicao" para decompor os números A e B
    decomposicao(A, &sinal_a, &expoente_a, &mantissa_a);
    decomposicao(B, &sinal_b, &expoente_b, &mantissa_b);
    
    //realiza um XOR dos sinais para obter o sinal resultante
    int sinal = sinal_a ^ sinal_b;

    //calcula o expoente real
    int expoente = (expoente_a ? expoente_a - 127 : -126) + (expoente_b ? expoente_b - 127 : -126) + 127;

    //acrescentando o Bit implícito nas mantissas/significandos
    if (expoente_a != 0){
        mantissa_a |= (1U << 23); 
    } 
    if (expoente_b != 0){
        mantissa_b |= (1U << 23);
    }

    //multiplicação das mantissas
    uint64_t produto = (uint64_t)mantissa_a * (uint64_t)mantissa_b;

    if (produto == 0) {
        expoente = 0;
    }
    else {
        //enquanto o bit 47 não estiver "setado" e não ter mais de 24 deslocamentos
        while(!(produto & (1ULL << 47)) && desloc < 24){
            //desloca para a esquerda e incrementa o contador de deslocamentos
            produto <<= 1;
            desloc++;
        }
        produto >>= 24;
        expoente -= (desloc - 1);
    }

    //verificação de overflow e underflow
    if (expoente >= 0xFF) {
        *overflow = 1;
        expoente = 0xFF;
        produto = 0;
    } else if (expoente <= 0) {
        *underflow = 1;
        expoente = 0;
        produto = 0;
    }

    //montando o resultado nos registradores
    montar_float(A, sinal, expoente, produto & 0x7FFFFF);
}