#ifndef ULA_H
#define ULA_H

#include "bit.h"
#include <stdint.h> // Biblioteca para definir tipos inteiros

// Operações com números Inteiros de 8 bits com sinal (-128 até 127)
void ULA_ADD(int8_t *A, int8_t *B, int8_t *overflow);
void ULA_SUB(int8_t *A, int8_t *B, int8_t *overflow);
void ULA_MUL(int8_t *A, int8_t *Q, int8_t *M, int8_t *overflow);
void ULA_DIV(int8_t *A, int8_t *Q, int8_t *M, int8_t *overflow);

#endif // ULA_H