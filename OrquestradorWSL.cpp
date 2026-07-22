#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void displayMenu() {
    printf("\033[;35m");
    printf("\n");
    printf("Menu:\n");
    printf("\n");
    printf("\033[;32m");
    printf("1. Ativar recursos WSL\n");
    printf("2. Desativar recursos WSL\n");
    printf("\033[;33m");
    printf("3. Instalar WSL\n");
    printf("4. Desinstalar WSL\n");
    printf("5. Reparar WSL\n");
    printf("\033[;31m");
    printf("6. Sair\n");
    printf("\033[0m");
}

void activateWSL() {
    printf("Ativando recursos WSL...\n");
    int result1 = system("powershell -NoProfile -Command \"Start-Process powershell -ArgumentList \\\"Enable-WindowsOptionalFeature -Online -FeatureName Microsoft-Windows-Subsystem-Linux -NoRestart\\\" -Verb RunAs\"");
    int result2 = system("powershell -NoProfile -Command \"Start-Process powershell -ArgumentList \\\"Enable-WindowsOptionalFeature -Online -FeatureName VirtualMachinePlatform -NoRestart\\\" -Verb RunAs\"");
    if (result1 == 0 && result2 == 0) {
        printf("Recursos WSL ativados com sucesso!\n");
        printf("Por favor, reinicie a máquina para aplicar as alterações.\n");
    } else {
        printf("Erro ao ativar recursos WSL.\n");
    }
}

void deactivateWSL() {
    printf("Desativando recursos WSL...\n");
    int result1 = system("powershell -NoProfile -Command \"Start-Process powershell -ArgumentList \\\"Disable-WindowsOptionalFeature -Online -FeatureName Microsoft-Windows-Subsystem-Linux -NoRestart\\\" -Verb RunAs\"");
    int result2 = system("powershell -NoProfile -Command \"Start-Process powershell -ArgumentList \\\"Disable-WindowsOptionalFeature -Online -FeatureName VirtualMachinePlatform -NoRestart\\\" -Verb RunAs\"");
    if (result1 == 0 && result2 == 0) {
        printf("Recursos WSL desativados com sucesso!\n");
        printf("Por favor, reinicie a máquina para aplicar as alterações.\n");
    } else {
        printf("Erro ao desativar recursos WSL.\n");
    }
}

void installWSL() {
    printf("Instalando WSL...\n");
    int result = system("powershell -NoProfile -Command \"Start-Process powershell -ArgumentList \\\"wsl --install\\\" -Verb RunAs\"");
    if (result == 0) {
        printf("WSL instalado com sucesso!\n");
        printf("Por favor, reinicie a máquina para aplicar as alterações.\n");
    } else {
        printf("Erro ao instalar WSL.\n");
    }
}

void uninstallWSL() {
    printf("Desinstalando WSL...\n");
    int result = system("powershell -NoProfile -Command \"Start-Process powershell -ArgumentList \\\"wsl --uninstall\\\" -Verb RunAs\"");
    if (result == 0) {
        printf("WSL desinstalado com sucesso!\n");
        printf("Por favor, reinicie a máquina para aplicar as alterações.\n");
    } else {
        printf("Erro ao desinstalar WSL.\n");
    }
}

void repairWSL() {
    printf("Reparando WSL...\n");
    int result1 = system("powershell -NoProfile -Command \"Start-Process powershell -ArgumentList \\\"wsl --uninstall\\\" -Verb RunAs\"");
    int result2 = system("powershell -NoProfile -Command \"Start-Process powershell -ArgumentList \\\"wsl --install\\\" -Verb RunAs\"");
    if (result1 == 0 && result2 == 0) {
        printf("WSL reparado com sucesso!\n");
        printf("Por favor, reinicie a máquina para aplicar as alterações.\n");
    } else {
        printf("Erro ao reparar WSL.\n");
    }
}

int main() {

    if (system("powershell -NoProfile -Command \"Get-Command wsl\" >nul 2>&1") != 0) {
        printf("WSL não está instalado.\n");
    } else {
        printf("WSL já está instalado.\n");
        if (system("powershell -NoProfile -Command \"Get-Command wsl --version\" >nul 2>&1") != 0) {
            printf("WSL não está atualizado. Atualizando...\n");
        } else {
            printf("WSL já está atualizado.\n");
        }
        int result = system("powershell -NoProfile -Command \"Start-Process powershell -ArgumentList \\\"wsl --update\\\" -Verb RunAs\"");
        if (result == 0) {
            printf("WSL atualizado com sucesso!\n");
        } else {
            printf("Erro ao atualizar WSL.\n");
        }
    }

    printf("\n");
    printf("-----------------------------------------\n");
    printf("\n");

    int choice;
    printf("\033[96mBem-vindo ao Orquestrador WSL!\033[0m\n");
    printf("\033[36mEste programa permite ativar, desativar, instalar, desinstalar e reparar o WSL (Windows Subsystem for Linux) no Windows.\033[0m\n");
    do {
        displayMenu();
        printf("Escolha uma opção: ");
        scanf("%d", &choice);
        switch (choice) {
            case 1:
                activateWSL();
                break;
            case 2:
                deactivateWSL();
                break;
            case 3:
                installWSL();
                break;
            case 4:
                uninstallWSL();
                break;
            case 5:
                repairWSL();
                break;
            case 6:
                printf("Saindo...\n");
                break;
            default:
                printf("Opção inválida!\n");
        }
    } while (choice != 6);

    return 0;
}
