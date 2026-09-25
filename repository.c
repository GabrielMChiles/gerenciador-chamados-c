#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chamado.h"
#include "repository.h"

#define ARQUIVO_BANCO "banco_chamados.dat"

// 'static' encapsula as variáveis impedindo que o main.c ou o service.c as acessem diretamente.
static Chamado banco_chamados[MAX_CHAMADOS];
static int total_chamados = 0;
static int proximo_id = 1;

void repository_carregar_dados()
{
    FILE *arquivo = fopen(ARQUIVO_BANCO, "rb");
    if (arquivo != NULL)
    {
        // Lê a quantidade total primeiro
        fread(&total_chamados, sizeof(int), 1, arquivo);
        fread(&proximo_id, sizeof(int), 1, arquivo);
        // Lê o array de structs inteiro de uma vez
        fread(banco_chamados, sizeof(Chamado), total_chamados, arquivo);
        fclose(arquivo);
    }
}

// Salva a memória no disco antes de fechar o sistema
void repository_salvar_dados()
{
    FILE *arquivo = fopen(ARQUIVO_BANCO, "wb");
    if (arquivo != NULL)
    {
        fwrite(&total_chamados, sizeof(int), 1, arquivo);
        fwrite(&proximo_id, sizeof(int), 1, arquivo);
        fwrite(banco_chamados, sizeof(Chamado), total_chamados, arquivo);
        fclose(arquivo);
    }
}

int repository_inserir(Chamado novo_chamado)
{
    if (total_chamados >= MAX_CHAMADOS)
    {
        return 0; // Erro de banco cheio
    }

    // Atribui o ID automático
    novo_chamado.id = proximo_id++;

    // Copia a struct para o array
    banco_chamados[total_chamados] = novo_chamado;
    total_chamados++;

    return 1;
}

Chamado *repository_buscar_por_id(int id)
{
    for (int i = 0; i < total_chamados; i++)
    {
        if (banco_chamados[i].id == id)
        {
            return &banco_chamados[i];
        }
    }
    return NULL; // Não encontrou
}

int repository_listar_todos(Chamado *buffer_saida, int capacidade_maxima)
{
    int cont = 0;
    for (int i = 0; i < total_chamados && cont < capacidade_maxima; i++)
    {
        buffer_saida[cont] = banco_chamados[i]; // Copia a struct inteira por valor
        cont++;
    }
    return cont;
}