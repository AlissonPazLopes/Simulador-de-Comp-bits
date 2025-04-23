#include "cpu.h"
#include "uc.h"
#include <stdio.h>

void inicializar_cpu(CPU *cpu, Barramento *bus) {
    inicializar_registradores(&cpu->reg);
    inicializar_uc(&cpu->uc);
    cpu->bus = bus;
}

void executar_ciclo(CPU *cpu, RAM *ram) {
    imprimir_registradores(&cpu->reg);

    while (!cpu->uc.halt) {
        
        switch (cpu->reg.ICC) {
            case 0:
                printf("\n>>> Ciclo de Busca <<<\n");
                ciclo_busca(cpu, ram);
                break;
            case 1:
                printf("\n>>> Ciclo Indireto <<<\n");
                ciclo_indireto(cpu, ram);
                break;
            case 2:
                ciclo_execucao(cpu, ram);
                break;
            case 3:
                printf("\n>>> Interrupcao detectada! Executando ciclo de interrupcao. <<<\n");
                ciclo_interrupcao(cpu, ram);
                break;
            default:
                printf("\nErro: Estado ICC invalido: %d\n", cpu->reg.ICC);
                cpu->uc.halt = 1;
                break;
        }

        imprimir_registradores(&cpu->reg);

        if (cpu->uc.halt) {

            break;
        }
    }
}
