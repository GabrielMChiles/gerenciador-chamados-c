#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "chamado.h"
#include "service.h"
#include "repository.h"

static void obter_data_atual(char *buffer, int tamanho);

// Função auxiliar (privada ao módulo) para checar o WIP (Work In Progress) - RN4
static int contar_chamados_em_andamento_tecnico(const char *tecnico)
{
    int count = 0;

    for (int i = 1; i <= 100; i++)
    {
        Chamado *c = repository_buscar_por_id(i);
        if (c != NULL)
        {
            if (strcmp(c->tecnico, tecnico) == 0 && strcmp(c->status, STATUS_ANDAMENTO) == 0)
            {
                count++;
            }
        }
    }
    return count;
}

int service_registrar_chamado(const char *descricao, const char *solicitante)
{
    if (descricao == NULL || solicitante == NULL || strlen(descricao) == 0)
    {
        return SERVICE_ERRO_VALIDACAO;
    }

    Chamado novo_chamado;
    // Segurança de Memória
    strncpy(novo_chamado.descricao, descricao, sizeof(novo_chamado.descricao) - 1);
    novo_chamado.descricao[sizeof(novo_chamado.descricao) - 1] = '\0';

    strncpy(novo_chamado.tecnico, "", sizeof(novo_chamado.tecnico));
    strncpy(novo_chamado.solucao, "", sizeof(novo_chamado.solucao));

    // RN1: Aplicação restrita de status inicial usando as constantes do chamado.h
    strncpy(novo_chamado.status, STATUS_ABERTO, sizeof(novo_chamado.status) - 1);
    novo_chamado.status[sizeof(novo_chamado.status) - 1] = '\0';

    time_t relogio = time(NULL);
    struct tm tempo_local = *localtime(&relogio);

    // snprintf é a defesa de memória contra buffer overflow. Não se usa sprintf puro.
    snprintf(novo_chamado.data_abertura, sizeof(novo_chamado.data_abertura),
             "%02d/%02d/%d %02d:%02d",
             tempo_local.tm_mday, tempo_local.tm_mon + 1,
             tempo_local.tm_year + 1900, tempo_local.tm_hour, tempo_local.tm_min);

    if (repository_inserir(novo_chamado) == 0)
    {
        return SERVICE_ERRO_VALIDACAO; // Banco cheio
    }
    return SERVICE_SUCESSO;
}

int service_atribuir_tecnico(int id_chamado, const char *tecnico)
{
    Chamado *c = repository_buscar_por_id(id_chamado);
    if (c == NULL)
        return SERVICE_ERRO_NAO_ENCONTRADO;

    // RN5: Imutabilidade de histórico.
    if (strcmp(c->status, STATUS_RESOLVIDO) == 0)
    {
        return SERVICE_ERRO_REGRA_NEGOCIO;
    }

    // RN4: Limite de Trabalho em Progresso. Máx 3 chamados.
    if (contar_chamados_em_andamento_tecnico(tecnico) >= 3)
    {
        return SERVICE_ERRO_REGRA_NEGOCIO;
    }

    strncpy(c->tecnico, tecnico, sizeof(c->tecnico) - 1);
    c->tecnico[sizeof(c->tecnico) - 1] = '\0';

    // RN2: Fluxo de Atendimento - Só vai para Em Andamento se tiver técnico.
    strncpy(c->status, STATUS_ANDAMENTO, sizeof(c->status) - 1);
    c->status[sizeof(c->status) - 1] = '\0';

    return SERVICE_SUCESSO;
}

int service_finalizar_chamado(int id_chamado, const char *solucao)
{
    // 1. Validação de Parâmetros (Defesa na Entrada)
    if (id_chamado <= 0 || solucao == NULL || strlen(solucao) == 0)
    {
        return SERVICE_ERRO_VALIDACAO;
    }

    // 2. Busca da Entidade
    Chamado *c = repository_buscar_por_id(id_chamado);
    if (c == NULL)
    {
        return SERVICE_ERRO_NAO_ENCONTRADO;
    }

    // 3. Regra de Negócio: Impede finalizar sem técnico atribuído
    if (strlen(c->tecnico) == 0)
    {
        return SERVICE_ERRO_REGRA_NEGOCIO;
    }

    // 4. Regra de Negócio: Transição de estado permitida apenas se estiver "Em Andamento"
    if (strcmp(c->status, STATUS_ANDAMENTO) != 0)
    {
        return SERVICE_ERRO_REGRA_NEGOCIO;
    }

    // 5. Transição do Estado e Persistência de Dados
    strncpy(c->solucao, solucao, sizeof(c->solucao) - 1);
    c->solucao[sizeof(c->solucao) - 1] = '\0';

    strcpy(c->status, STATUS_RESOLVIDO);
    obter_data_atual(c->data_fechamento, sizeof(c->data_fechamento));

    return SERVICE_SUCESSO;
}

int service_listar_historico_resolvidos(Chamado *buffer_saida, int capacidade_maxima)
{
    // Um buffer interno grande para pegar todos,
    // e depois filtramos apenas os resolvidos para o buffer de saída.
    Chamado todos[100];
    int total = repository_listar_todos(todos, 100);

    int encontrados = 0;
    for (int i = 0; i < total && encontrados < capacidade_maxima; i++)
    {
        if (strcmp(todos[i].status, STATUS_RESOLVIDO) == 0)
        {
            buffer_saida[encontrados] = todos[i];
            encontrados++;
        }
    }
    return encontrados; // Retorna quantos foram colocados no buffer_saida
}

void service_obter_indicadores_status(int *qtd_abertos, int *qtd_andamento, int *qtd_resolvidos)
{
    // Inicializa os contadores no endereço apontado (desreferenciação)
    *qtd_abertos = 0;
    *qtd_andamento = 0;
    *qtd_resolvidos = 0;

    Chamado buffer[100];
    int total = repository_listar_todos(buffer, 100);

    for (int i = 0; i < total; i++)
    {
        if (strcmp(buffer[i].status, STATUS_ABERTO) == 0)
        {
            (*qtd_abertos)++;
        }
        else if (strcmp(buffer[i].status, STATUS_ANDAMENTO) == 0)
        {
            (*qtd_andamento)++;
        }
        else if (strcmp(buffer[i].status, STATUS_RESOLVIDO) == 0)
        {
            (*qtd_resolvidos)++;
        }
    }
}

// RF2: Permite listar chamados aplicando filtros dinâmicos opcionais
int service_listar_chamados_filtrado(Chamado *buffer, int cap, const char *filtro_status, const char *filtro_tecnico)
{
    Chamado todos[100];
    int total = repository_listar_todos(todos, 100);
    int enc = 0;

    for (int i = 0; i < total && enc < cap; i++)
    {
        int match_status = (filtro_status == NULL || strlen(filtro_status) == 0 || strcmp(todos[i].status, filtro_status) == 0);
        int match_tecnico = (filtro_tecnico == NULL || strlen(filtro_tecnico) == 0 || strcmp(todos[i].tecnico, filtro_tecnico) == 0);

        if (match_status && match_tecnico)
        {
            buffer[enc++] = todos[i];
        }
    }
    return enc;
}

// RF7: Consulta o volume de soluções concluídas por um técnico específico
int service_contar_resolvidos_por_tecnico(const char *tecnico)
{
    if (tecnico == NULL || strlen(tecnico) == 0)
        return 0;
    Chamado todos[100];
    int total = repository_listar_todos(todos, 100);
    int total_resolvidos = 0;

    for (int i = 0; i < total; i++)
    {
        if (strcmp(todos[i].tecnico, tecnico) == 0 && strcmp(todos[i].status, STATUS_RESOLVIDO) == 0)
        {
            total_resolvidos++;
        }
    }
    return total_resolvidos;
}

// Função utilitária privada
static void obter_data_atual(char *buffer, int tamanho)
{
    time_t relogio = time(NULL);
    struct tm tempo_local = *localtime(&relogio);
    snprintf(buffer, tamanho, "%04d/%02d/%02d %02d:%02d",
             tempo_local.tm_year + 1900, tempo_local.tm_mon + 1,
             tempo_local.tm_mday, tempo_local.tm_hour, tempo_local.tm_min);
}