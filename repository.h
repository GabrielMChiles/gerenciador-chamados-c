#ifndef REPOSITORY_H
#define REPOSITORY_H

#include "chamado.h"

// Limite máximo de chamados, para simular o banco em memória
#define MAX_CHAMADOS 100

void repository_carregar_dados(void);
void repository_salvar_dados(void);

// Retorna 1 para Sucesso, 0 para Falha (banco cheio)
int repository_inserir(Chamado novo_chamado);

// Busca um chamado pelo ID. Retorna um ponteiro para ele ou NULL se não achar.
Chamado* repository_buscar_por_id(int id);

// Retorna a quantidade de chamados copiados para o buffer
int repository_listar_todos(Chamado* buffer_saida, int capacidade_maxima);



#endif // REPOSITORY_H