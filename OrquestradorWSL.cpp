#include <windows.h>
#include <stdio.h>

#define PIPE_NAME R"(\\.\pipe\OrquestradorWSLPipe)"

typedef enum {
    CMD_ENABLE_WSL = 1,
    CMD_DISABLE_WSL = 2,
    CMD_INSTALL_WSL = 3,
    CMD_UNINSTALL_WSL = 4,
    CMD_UPDATE_WSL = 5,
    CMD_REPAIR_WSL = 6
} WslCommand;

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
    char buffer[10];
    fgets(buffer, sizeof(buffer), stdin);
    opcao = atoi(buffer);
    scanf("%d", &opcao);

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
            return 0;
        default: 
        printf("Opção inválida\n"); 
        CloseHandle(hPipe);
            return 1;
    }

    DWORD bytesWritten, bytesRead;
    DWORD resposta = 0;
    WriteFile(hPipe, &cmd, sizeof(DWORD), &bytesWritten, NULL);
    ReadFile(hPipe, &resposta, sizeof(DWORD), &bytesRead, NULL);
    CloseHandle(hPipe);

    if (resposta == 200) {
        printf("Comando executado com sucesso.\n");
        printf("Código de resposta: %lu\n", resposta);
    } else {
        printf("Falha na execução.\n");
        printf("Código de resposta: %lu\n", resposta);
    }

}