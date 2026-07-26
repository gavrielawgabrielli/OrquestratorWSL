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
    printf("===============================\n");
    printf("Ativando recursos WSL...\n");
    printf("===============================\n");
    int result1 = system("powershell -NoProfile -WindowStyle Hidden -Command \"Enable-WindowsOptionalFeature -Online -FeatureName Microsoft-Windows-Subsystem-Linux -NoRestart\"");
    int result2 = system("powershell -NoProfile -WindowStyle Hidden -Command \"Enable-WindowsOptionalFeature -Online -FeatureName VirtualMachinePlatform -NoRestart\"");
    if (result1 == 0 && result2 == 0) {
        printf("\n===============================\n");
        printf("Recursos WSL ativados com sucesso!\n");
        printf("Por favor, reinicie a máquina para aplicar as alterações.\n");
        printf("===============================\n");
    } else {
        printf("\n===============================\n");
        printf("Erro ao ativar recursos WSL.\n");
        printf("===============================\n");
    }
}

void deactivateWSL() {
    printf("===============================\n");
    printf("Desativando recursos WSL...\n");
    printf("===============================\n");
    int result1 = system("powershell -NoProfile -WindowStyle Hidden -Command \"Disable-WindowsOptionalFeature -Online -FeatureName Microsoft-Windows-Subsystem-Linux -NoRestart\"");
    int result2 = system("powershell -NoProfile -WindowStyle Hidden -Command \"Disable-WindowsOptionalFeature -Online -FeatureName VirtualMachinePlatform -NoRestart\"");
    if (result1 == 0 && result2 == 0) {
        printf("\n===============================\n");
        printf("Recursos WSL desativados com sucesso!\n");
        printf("Por favor, reinicie a máquina para aplicar as alterações.\n");
        printf("===============================\n");
    } else {
        printf("\n===============================\n");
        printf("Erro ao desativar recursos WSL.\n");
        printf("===============================\n");
    }
}

void installWSL() {
    printf("===============================\n");
    printf("Instalando WSL...\n");
    printf("===============================\n");
    int result = system("powershell -NoProfile -WindowStyle Hidden -Command \"wsl --install\"");
    if (result == 0) {
        printf("\n===============================\n");
        printf("WSL instalado com sucesso!\n");
        printf("Por favor, reinicie a máquina para aplicar as alterações.\n");
        printf("===============================\n");
    } else {
        printf("\n===============================\n");
        printf("Erro ao instalar WSL.\n");
        printf("===============================\n");
    }
}

void uninstallWSL() {
    printf("===============================\n");
    printf("Desinstalando WSL...\n");
    printf("===============================\n");
    int result = system("powershell -NoProfile -WindowStyle Hidden -Command \"wsl --uninstall\"");
    if (result == 0) {
        printf("\n===============================\n");
        printf("WSL desinstalado com sucesso!\n");
        printf("Por favor, reinicie a máquina para aplicar as alterações.\n");
        printf("===============================\n");
    } else {
        printf("\n===============================\n");
        printf("Erro ao desinstalar WSL.\n");
        printf("===============================\n");
    }
}

void repairWSL() {
    printf("===============================\n");
    printf("Reparando WSL...\n");
    printf("===============================\n");
    int result1 = system("powershell -NoProfile -WindowStyle Hidden -Command \"wsl --uninstall\"");
    int result2 = system("powershell -NoProfile -WindowStyle Hidden -Command \"wsl --install\"");
    if (result1 == 0 && result2 == 0) {
        printf("\n===============================\n");
        printf("WSL reparado com sucesso!\n");
        printf("Por favor, reinicie a máquina para aplicar as alterações.\n");
        printf("===============================\n");
    } else {
        printf("\n===============================\n");
        printf("Erro ao reparar WSL.\n");
        printf("===============================\n");
    }
}

int main() {
    
     printf("\n===============================\n");
    if (system("powershell -NoProfile -WindowStyle Hidden -Command \"Get-Command wsl\" >nul 2>&1") != 0) {
        printf("WSL não está instalado.\n");
    } else {
        printf("WSL já está instalado.\n");
        if (system("powershell -NoProfile -WindowStyle Hidden -Command \"Get-Command wsl --version\" >nul 2>&1") != 0) {
            printf("WSL não está atualizado. Atualizando...\n");
        } else {
            printf("WSL já está atualizado.\n");
        }
        int result = system("powershell -NoProfile -WindowStyle Hidden -Command \"wsl --update\"");
        if (result == 0) {
            printf("WSL atualizado com sucesso!\n");
        } else {
            printf("Erro ao atualizar WSL.\n");
        }
    }   
     printf("\n===============================\n");

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