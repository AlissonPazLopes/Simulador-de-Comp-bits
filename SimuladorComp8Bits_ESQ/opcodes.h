#ifndef  OPCODES_H
#define  OPCODES_H


// ========================================================
// --- OPCODE das instru��es ---
#define   NOP   0x00                           //no operation
#define   LDI   0x01<<4                        //carga imediata no acumulador
#define   LDD   0x02<<4                        //carga direta no acumulador
#define   LDA   0x03<<4                        //carga indireta no acumulador
#define   STA   0x04<<4                        //armazena acumulador na RAM
#define   ADD   0x05<<4                        //adi��o do AC com operando endere�ado diretamente
#define   SUB   0x06<<4                        //subtra��o do AC com operando endere�ado diretamente
#define   MUL   0x07<<4                        //Multiplica��o do AC com operando endere�ado diretamente
#define   DIV   0x08<<4                        //Divis�o do AC com operando endere�ado diretamente
#define   OUTT   0x0E<<4                        //carrega conte�do do acumulador no registrador de sa�da
#define   HLT   0x0F<<4                        //halt, "congela" o processamento


#endif
