#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "service.h"
#include "repository.h"

static void ler_string_segura(char *buffer, int tamanho)
{
    if (fgets(buffer, tamanho, stdin) != NULL)
    {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n')
        {
            buffer[len - 1] = '\0';
        }
        else
        {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF)
                ;
        }
    }
}

// Substitui o atoi() e garante tratamento rigoroso de parsing na UI
static int ler_int_seguro(const char *mensagem)
{
    char buffer[20];
    char *endptr;
    printf("%s", mensagem);
    ler_string_segura(buffer, sizeof(buffer));

    long val = strtol(buffer, &endptr, 10);
    if (endptr == buffer || *endptr != '\0' || val <= 0)
    {
        return -1; // Flag de erro para o Controller tratar
    }
    return (int)val;
}

static void tratar_retorno_service(int codigo_retorno)
{
    switch (codigo_retorno)
    {
    case SERVICE_SUCESSO:
        printf("[SUCESSO] Operacao realizada com exito.\n");
        break;
    case SERVICE_ERRO_VALIDACAO:
        printf("[ERRO] Dados invalidos. Verifique os campos informados.\n");
        break;
    case SERVICE_ERRO_REGRA_NEGOCIO:
        printf("[ERRO DE NEGOCIO] Operacao bloqueada pelas regras do sistema.\n");
        break;
    case SERVICE_ERRO_NAO_ENCONTRADO:
        printf("[ERRO] Chamado nao encontrado no banco de dados.\n");
        break;
    default:
        printf("[ERRO FATAL] Codigo de erro desconhecido.\n");
    }
}

int main()
{
    // Bootstrapping: Carrega os dados salvos do disco para a memória
    repository_carregar_dados();

    char opcao[10];
    int executando = 1;

    while (executando)
    {
        printf("\n========================================\n");
        printf("    GERENCIADOR DE CHAMADOS BÁSICO\n");
        printf("========================================\n");
        printf("1. Registrar Chamadon");
        printf("2. Atribuir Tecnico\n");
        printf("3. Finalizar Chamado\n");
        printf("4. Listar Chamados com Filtros\n");
        printf("5. Historico de Resolvidos\n");
        printf("6. Painel Gerencial e Indicadores\n");
        printf("0. Sair\n");
        printf("Escolha uma opcao: ");

        ler_string_segura(opcao, sizeof(opcao));

        if (strcmp(opcao, "1") == 0)
        {
            char solicitante[50], descricao[100];
            printf("Nome do solicitante: ");
            ler_string_segura(solicitante, sizeof(solicitante));
            printf("Descricao do problema: ");
            ler_string_segura(descricao, sizeof(descricao));

            int status = service_registrar_chamado(descricao, solicitante);
            tratar_retorno_service(status);

            if (status == SERVICE_SUCESSO)
            {
                repository_salvar_dados();
            }
        }
        else if (strcmp(opcao, "2") == 0)
        {
            int id = ler_int_seguro("ID do chamado: ");
            if (id <= 0)
            {
                printf("[ERRO] ID invalido. Digite um numero inteiro positivo.\n");
                continue;
            }

            char tecnico[50];
            printf("Nome do tecnico: ");
            ler_string_segura(tecnico, sizeof(tecnico));

            int status = service_atribuir_tecnico(id, tecnico);
            tratar_retorno_service(status);

            if (status == SERVICE_SUCESSO)
            {
                repository_salvar_dados();
            }
        }
        else if (strcmp(opcao, "3") == 0)
        {
            int id = ler_int_seguro("ID do chamado: ");
            if (id <= 0)
            {
                printf("[ERRO] ID invalido. Digite um numero inteiro positivo.\n");
                continue;
            }

            char solucao[200];
            printf("Descricao da solucao tecnica: ");
            ler_string_segura(solucao, sizeof(solucao));

            int status = service_finalizar_chamado(id, solucao);
            tratar_retorno_service(status);

            if (status == SERVICE_SUCESSO)
            {
                repository_salvar_dados();
            }
        }
        else if (strcmp(opcao, "4") == 0)
        {
            // RF2: Listagem e Visibilidade com filtros dinâmicos
            char status_filtro[20], tecnico_filtro[50];
            printf("Filtrar por Status (Aberto/Em Andamento/Resolvido ou Enter para todos): ");
            ler_string_segura(status_filtro, sizeof(status_filtro));

            printf("Filtrar por Tecnico (Nome ou Enter para todos): ");
            ler_string_segura(tecnico_filtro, sizeof(tecnico_filtro));

            Chamado lista[50];
            int qtd = service_listar_chamados_filtrado(lista, 50, status_filtro, tecnico_filtro);

            printf("\n--- LISTAGEM DE CHAMADOS (RF2) ---\n");
            if (qtd == 0)
            {
                printf("Nenhum chamado encontrado para os filtros informados.\n");
            }
            else
            {
                for (int i = 0; i < qtd; i++)
                {
                    printf("ID: %d | Status: %s | Tecnico: %s\n", lista[i].id, lista[i].status, strlen(lista[i].tecnico) > 0 ? lista[i].tecnico : "Nao Atribuido");
                    printf("Data Abertura: %s\n", lista[i].data_abertura);
                    printf("Problema: %s\n", lista[i].descricao);
                    if (strcmp(lista[i].status, STATUS_RESOLVIDO) == 0)
                    {
                        printf("Solucao: %s (Fechado em: %s)\n", lista[i].solucao, lista[i].data_fechamento);
                    }
                    printf("----------------------------------------\n");
                }
            }
        }
        else if (strcmp(opcao, "5") == 0)
        {
            // RF6: Consulta de Histórico para evitar retrabalho
            Chamado lista_historico[50];
            int qtd = service_listar_historico_resolvidos(lista_historico, 50);

            printf("\n--- BASE DE CONHECIMENTO / HISTÓRICO (RF6) ---\n");
            if (qtd == 0)
            {
                printf("Nenhum chamado resolvido encontrado.\n");
            }
            else
            {
                for (int i = 0; i < qtd; i++)
                {
                    printf("ID: %d | Tecnico: %s | Fechado em: %s\n", lista_historico[i].id, lista_historico[i].tecnico, lista_historico[i].data_fechamento);
                    printf("Problema: %s\n", lista_historico[i].descricao);
                    printf("Solucao Tecnica: %s\n", lista_historico[i].solucao);
                    printf("----------------------------------------\n");
                }
            }
        }
        else if (strcmp(opcao, "6") == 0)
        {
            // RF7: Indicadores Básicos (Totais por status + Produtividade por Técnico)
            int abertos, andamento, resolvidos;
            service_obter_indicadores_status(&abertos, &andamento, &resolvidos);

            printf("\n========================================\n");
            printf("      PAINEL GERENCIAL (RF7)\n");
            printf("========================================\n");
            printf("Chamados Abertos      : %d\n", abertos);
            printf("Chamados Em Andamento : %d\n", andamento);
            printf("Chamados Resolvidos   : %d\n", resolvidos);
            printf("----------------------------------------\n");

            char tecnico_consulta[50];
            printf("Consultar total de resolvidos do Tecnico (Nome): ");
            ler_string_segura(tecnico_consulta, sizeof(tecnico_consulta));

            if (strlen(tecnico_consulta) > 0)
            {
                int total_tec = service_contar_resolvidos_por_tecnico(tecnico_consulta);
                printf("Chamados resolvidos por '%s': %d\n", tecnico_consulta, total_tec);
            }
            printf("========================================\n");
        }
        else if (strcmp(opcao, "0") == 0)
        {
            repository_salvar_dados();
            printf("Estado final salvo no arquivo. Encerrando o sistema...\n");
            executando = 0;
        }
        else
        {
            printf("[AVISO] Opcao invalida.\n");
        }
    }

    return 0;
}