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
        printf("Service is not running or pipe is not available.\n");
        printf("Execute has admin the following command on Powershell:\n");
        printf("\n.\\MaestroWSL --install\n");
        printf("========\n");
        return 1;
    }

    printf("\n");
    printf("-----------------------------------------\n");
    printf("\n");

    printf("\033[;96m");
    printf("\n===============================\n");
    printf("Welcome to the WSL Orchestrator!\n");
    printf("\033[;0m");
    printf("\033[;36m");
    printf("This program allows you to enable, disable, install and uninstall WSL (Windows Subsystem for Linux) on Windows.\n");
    printf("All commands are executed with administrator privileges.\n");
    printf("Be careful when executing commands that may affect the system.\n");
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
        printf("This is the Menu\n");
        printf("choose one of the options below:\n");
        printf("\033[0m");

        printf("\033[;32m");
        printf("1. Enable WSL\n");
        printf("2. Disable WSL\n");
        printf("\033[0m");

        printf("\033[;33m");
        printf("3. Install WSL\n");
        printf("4. Uninstall WSL\n");
        printf("\033[0m");

        printf("\033[;31m");
        printf("5. Update WSL\n");
        printf("6. Repair WSL\n");
        printf("\033[0m");

        printf("\033[;31m");
        printf("7. Exit\n");
        printf("\033[0m");
    
        printf("Choose an option: ");
        if (scanf("%d", &opcao) != 1) {
            printf("Invalid input. Please enter a number.\n");
            // Limpa o buffer de entrada
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }
        if (opcao < 1 || opcao > 7) {
            printf("Invalid option. Please choose an option between 1 and 7.\n");
            continue;
        }

    DWORD cmd;
    switch (opcao) {
        case 1: cmd = CMD_ENABLE_WSL; 
        printf("Enabling WSL...\n");
        break;
        case 2: cmd = CMD_DISABLE_WSL; 
        printf("Disabling WSL...\n"); 
        break;
        case 3: cmd = CMD_INSTALL_WSL; 
        printf("Installing WSL...\n"); 
        break;
        case 4: cmd = CMD_UNINSTALL_WSL; 
        printf("Uninstalling WSL...\n"); 
        break;
        case 5: cmd = CMD_UPDATE_WSL; 
        printf("Updating WSL...\n"); 
        break;
        case 6: cmd = CMD_REPAIR_WSL; 
        printf("Repairing WSL...\n"); 
        break;
        case 7:
            printf("Exiting...\n");
            CloseHandle(hPipe);
            Sleep(10000); // Aguarda 10 segundos antes de sair
            return 0;
        default: 
        printf("Invalid option\n"); 
        continue;
    }

     DWORD bytesWritten, bytesRead;
        RespostaWSL resposta = {0};
        if (!WriteFile(hPipe, &cmd, sizeof(DWORD), &bytesWritten, NULL)) {
            printf("Error sending command.\n");
            continue;
        }
        if (!ReadFile(hPipe, &resposta, sizeof(RespostaWSL), &bytesRead, NULL)) {
            printf("Error reading response.\n");
            continue;
        }

        if (resposta.codigo == 200) {
            printf("Command executed successfully.\n");
            printf("Response code: %lu\n", resposta.codigo);
            printf("Status do WSL: %lu\n", resposta.statusWSL);
        } else if (resposta.codigo == 400) {
            printf("Invalid command.\n");
            printf("Response code: %lu\n", resposta.codigo);
        } else if (resposta.codigo == 500) {
            printf("Error executing command.\n");
            printf("Response code: %lu\n", resposta.codigo);
        } else {
            printf("Failed to execute command.\n");
            printf("Response code: %lu\n", resposta.codigo);
        }       
    }  

    CloseHandle(hPipe);
    return 0;
}