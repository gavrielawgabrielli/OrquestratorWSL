#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define PIPE_NAME R"(\\.\pipe\OrquestradorWSLPipe)"

typedef enum {
    CMD_ENABLE_WSL = 1,
    CMD_DISABLE_WSL = 2,
    CMD_INSTALL_WSL = 3,
    CMD_UNINSTALL_WSL = 4,
    CMD_UPDATE_WSL = 5,
    CMD_REPAIR_WSL = 6
} WslCommand;

typedef struct {
    DWORD codigo;       // 200 = sucesso, 500 = erro, 400 = comando inválido
    DWORD statusWSL;    // 1=ativado, 2=desativado, 3=instalado, 4=desinstalado, 5=atualizado, 6=reparado, 0=indefinido
} RespostaWSL;

int main() {
    HANDLE hPipe = CreateFileA(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hPipe == INVALID_HANDLE_VALUE) {
        printf("========\n");
        printf("Serviço não está em execução.");
        printf(" Execute como admin para instalar o Maestro WSL:\n");
        printf("MaestroWSL --install\n");
        printf("========\n");
        return 1;
    }

    printf("\n");
    printf("-----------------------------------------\n");
    printf("\n");

    printf("\033[;96m");
    printf("\n===============================\n");
    printf("Bem-vindo ao Orquestrador WSL!\n");
    printf("\033[;0m");
    printf("\033[;36m");
    printf("Este programa permite ativar, desativar, instalar e desinstalar o WSL (Windows Subsystem for Linux) no Windows.\n");
    printf("Todos os comandos são executados com privilégios de administrador.\n");
    printf("Tome cuidado ao executar comandos que podem afetar o sistema.\n");
    printf("\033[;0m");
    printf("\033[;96m");
    printf("===============================\n");
    printf("\033[;0m");

    int opcao;
    while(1) {
        printf("\n");
        printf("-----------------------------------------\n");
        printf("\n");
        printf("\033[;35m");
        printf("Este e o Menu\n");
        printf("escolha uma das opções abaixo:\n");
        printf("\033[0m");

        printf("\033[;32m");
        printf("1. Habilitar WSL\n");
        printf("2. Desabilitar WSL\n");
        printf("\033[0m");

        printf("\033[;33m");
        printf("3. Instalar WSL\n");
        printf("4. Desinstalar WSL\n");
        printf("\033[0m");

        printf("\033[;31m");
        printf("5. Atualizar WSL\n");
        printf("6. Reparar WSL\n");
        printf("\033[0m");

        printf("\033[;31m");
        printf("7. Sair\n");
        printf("\033[0m");
    
        printf("Escolha uma opção: ");
        if (scanf("%d", &opcao) != 1) {
            printf("Entrada inválida. Por favor, insira um número.\n");
            // Limpa o buffer de entrada
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }
        if (opcao < 1 || opcao > 7) {
            printf("Opção inválida. Por favor, escolha uma opção entre 1 e 7.\n");
            continue;
        }

    DWORD cmd;
    switch (opcao) {
        case 1: cmd = CMD_ENABLE_WSL; 
        printf("Habilitando WSL...\n");
        break;
        case 2: cmd = CMD_DISABLE_WSL; 
        printf("Desabilitando WSL...\n"); 
        break;
        case 3: cmd = CMD_INSTALL_WSL; 
        printf("Instalando WSL...\n"); 
        break;
        case 4: cmd = CMD_UNINSTALL_WSL; 
        printf("Desinstalando WSL...\n"); 
        break;
        case 5: cmd = CMD_UPDATE_WSL; 
        printf("Atualizando WSL...\n"); 
        break;
        case 6: cmd = CMD_REPAIR_WSL; 
        printf("Reparando WSL...\n"); 
        break;
        case 7:
            printf("Saindo...\n");
            CloseHandle(hPipe);
            Sleep(10000); // Aguarda 10 segundos antes de sair
            return 0;
        default: 
        printf("Opção inválida\n"); 
        continue;
    }

     DWORD bytesWritten, bytesRead;
        RespostaWSL resposta = {0};
        if (!WriteFile(hPipe, &cmd, sizeof(DWORD), &bytesWritten, NULL)) {
            printf("Erro ao enviar comando.\n");
            continue;
        }
        if (!ReadFile(hPipe, &resposta, sizeof(RespostaWSL), &bytesRead, NULL)) {
            printf("Erro ao ler resposta.\n");
            continue;
        }

        if (resposta.codigo == 200) {
            printf("Comando executado com sucesso.\n");
            printf("Código de resposta: %lu\n", resposta.codigo);
            printf("Status do WSL: %lu\n", resposta.statusWSL);
        } else if (resposta.codigo == 400) {
            printf("Comando inválido.\n");
            printf("Código de resposta: %lu\n", resposta.codigo);
        } else if (resposta.codigo == 500) {
            printf("Erro ao executar o comando.\n");
            printf("Código de resposta: %lu\n", resposta.codigo);
        } else {
            printf("Falha na execução.\n");
            printf("Código de resposta: %lu\n", resposta.codigo);
        }       
    }  

    CloseHandle(hPipe);
    return 0;
}