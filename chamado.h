#ifndef CHAMADO_H
#define CHAMADO_H

// Constantes
#define STATUS_ABERTO "Aberto"
#define STATUS_ANDAMENTO "Em Andamento"
#define STATUS_RESOLVIDO "Resolvido"

typedef struct {
    int id;
    char descricao[100];
    char status[20];
    char tecnico[50];
    char solucao[200];
    char data_abertura[20];   // Formato: YYYY/MM/DD HH:MM
    char data_fechamento[20]; // Formato: YYYY/MM/DD HH:MM
} Chamado;

#endif // CHAMADO_H