#ifndef SERVICE_H
#define SERVICE_H

#include "chamado.h"

// Códigos de retorno simulando uma hierarquia de exceções
#define SERVICE_SUCESSO 0
#define SERVICE_ERRO_VALIDACAO 1
#define SERVICE_ERRO_REGRA_NEGOCIO 2
#define SERVICE_ERRO_NAO_ENCONTRADO 3

// O Service só recebe tipos primitivos da UI, valida e orquestra o Repositório
int service_registrar_chamado(const char* descricao, const char* solicitante);
int service_atribuir_tecnico(int id_chamado, const char* tecnico);
int service_finalizar_chamado(int id_chamado, const char* solucao);
int service_listar_historico_resolvidos(Chamado* buffer_saida, int capacidade_maxima);
int service_listar_chamados_filtrado(Chamado* buffer, int cap, const char* status, const char* tecnico);
int service_contar_resolvidos_por_tecnico(const char* tecnico);

// Os ponteiros (*) indicam que a função vai escrever nos endereços de memória fornecidos
void service_obter_indicadores_status(int* qtd_abertos, int* qtd_andamento, int* qtd_resolvidos);

#endif // SERVICE_H