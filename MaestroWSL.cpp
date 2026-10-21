#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char g_ServiceName[] = "OrquestradorWSLService";
#define SERVICE_NAME g_ServiceName
#define PIPE_NAME R"(\\.\pipe\OrquestradorWSLPipe)"
#define BUFFER_SIZE 128

void InstallService() {
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (!hSCM) {
        printf("Erro ao abrir SCM: %lu\n", GetLastError());
        return;
    }
    char path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);
    SC_HANDLE hService = CreateService(
        hSCM,
        SERVICE_NAME,                // nome interno (identificador)
        "Orquestrador WSL Maestro",  // nome de exibição (pode ser diferente)
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        path,
        NULL, NULL, NULL, NULL, NULL
    );
    if (hService) {
        if (!StartService(hService, 0, NULL)) {
            printf("Erro ao iniciar serviço: %lu\n", GetLastError());
        } else {
            printf("Serviço instalado e iniciado com sucesso!\n");
        }
        CloseServiceHandle(hService);
    } else {
        printf("Erro ao criar serviço: %lu\n", GetLastError());
    }
    CloseServiceHandle(hSCM);
}

void UninstallService() {
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!hSCM) return;
    SC_HANDLE hService = OpenService(hSCM, SERVICE_NAME, DELETE);
    if (hService) {
        // Para o serviço se estiver rodando
        SERVICE_STATUS status;
        ControlService(hService, SERVICE_CONTROL_STOP, &status);
        DeleteService(hService);
        CloseServiceHandle(hService);
    }
    CloseServiceHandle(hSCM);
}

typedef enum {
    CMD_NONE = 0,
    CMD_ENABLE_WSL = 1,
    CMD_DISABLE_WSL = 2,
    CMD_INSTALL_WSL = 3,
    CMD_UNINSTALL_WSL = 4,
    CMD_UPDATE_WSL = 5,
    CMD_REPAIR_WSL = 6
} WslCommand;

SERVICE_STATUS        g_ServiceStatus = {0};
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_StopEvent = NULL;

typedef struct {
    DWORD codigo;       // 200 = sucesso, 500 = erro, 400 = comando inválido
    DWORD statusWSL;    // 1=ativado, 2=desativado, 3=instalado, 4=desinstalado, 5=atualizado, 6=reparado, 0=indefinido
} RespostaWSL;

VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv);
DWORD WINAPI ServiceCtrlHandlerEx(DWORD control, DWORD eventType, LPVOID eventData, LPVOID context);
DWORD WINAPI PipeWorkerThread(LPVOID lpParam);
void ProcessarComando(DWORD cmd, RespostaWSL *pResposta);

int main(int argc, char *argv[]) {
    if (argc > 1) {
        if (strcmp(argv[1], "--install") == 0) {
            InstallService();
            return 0;
        } else if (strcmp(argv[1], "--uninstall") == 0) {
            UninstallService();
            return 0;
        }
    }

    SERVICE_TABLE_ENTRY ServiceTable[] = {
        { SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
        { NULL, NULL }
    };

    if (!StartServiceCtrlDispatcher(ServiceTable)) {
        return GetLastError();
    }
    return 0;
}


VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv) {
    g_StatusHandle = RegisterServiceCtrlHandlerExA(SERVICE_NAME, ServiceCtrlHandlerEx, NULL);
    if (g_StatusHandle == NULL) {
        return;
    }

    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

    g_StopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (g_StopEvent == NULL) {
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
        return;
    }

    // Sinaliza ao Windows que o serviço aceita comandos de parar/desligar
    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

    // Inicia a thread que escuta os comandos via Named Pipe
    HANDLE hThread = CreateThread(NULL, 0, PipeWorkerThread, NULL, 0, NULL);
    if (hThread) {
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);
    }

    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
}

DWORD WINAPI ServiceCtrlHandlerEx(DWORD control, DWORD eventType, LPVOID eventData, LPVOID context) {
    switch (control) {
        case SERVICE_CONTROL_STOP:
        case SERVICE_CONTROL_SHUTDOWN:
            g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
            SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
            SetEvent(g_StopEvent);
            break;
        default:
            break;
    }
    return NO_ERROR;
}

DWORD WINAPI PipeWorkerThread(LPVOID lpParam) {
    HANDLE hPipe;
    DWORD bytesRead, bytesWritten;
    DWORD comandoRecebido = 0;
    RespostaWSL resposta = {0};

    while (WaitForSingleObject(g_StopEvent, 0) != WAIT_OBJECT_0) {
        hPipe = CreateNamedPipeA(
            PIPE_NAME,
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
            1,
            BUFFER_SIZE,
            BUFFER_SIZE,
            0,
            NULL
        );

        if (hPipe == INVALID_HANDLE_VALUE) {
            Sleep(1000);
            continue;
        }

        BOOL connected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
        if (connected) {
            if (ReadFile(hPipe, &comandoRecebido, sizeof(DWORD), &bytesRead, NULL)) {
                if (comandoRecebido = CMD_NONE) {
                    DisconnectNamedPipe(hPipe);
                    CloseHandle(hPipe);
                    break;
                }
                ProcessarComando(comandoRecebido, &resposta);
                // Envia a estrutura completa de uma vez
                WriteFile(hPipe, &resposta, sizeof(RespostaWSL), &bytesWritten, NULL);
            }
        }

        DisconnectNamedPipe(hPipe);
        CloseHandle(hPipe);
    }
    return 0;
}

void ProcessarComando(DWORD cmd, RespostaWSL *pResposta) {
    pResposta->codigo = 200;       // sucesso por padrão
    pResposta->statusWSL = 0;      // indefinido até ser preenchido

    int result = 0;
    switch (cmd) {
        case CMD_ENABLE_WSL:
            result = system("dism.exe /online /enable-feature /featurename:Microsoft-Windows-Subsystem-Linux /all /norestart");
            if (result == 0) {
                result = system("dism.exe /online /enable-feature /featurename:VirtualMachinePlatform /all /norestart");
            }
            if (result == 0) {
                pResposta->statusWSL = 1;  // ativado
            } else {
                pResposta->codigo = 500;
            }
            break;

        case CMD_DISABLE_WSL:
            result = system("dism.exe /online /disable-feature /featurename:Microsoft-Windows-Subsystem-Linux /norestart");
            if (result == 0) {
                result = system("dism.exe /online /disable-feature /featurename:VirtualMachinePlatform /norestart");
            }
            if (result == 0) {
                pResposta->statusWSL = 2;  // desativado
            } else {
                pResposta->codigo = 500;
            }
            break;

        case CMD_INSTALL_WSL:
            if (system("wsl.exe --install --no-distribution") == 0) {
                pResposta->statusWSL = 3;  // instalado
            } else {
                pResposta->codigo = 500;
            }
            break;

        case CMD_UNINSTALL_WSL:
            if (system("wsl.exe --uninstall") == 0) {
                pResposta->statusWSL = 4;  // desinstalado
            } else {
                pResposta->codigo = 500;
            }
            break;

        case CMD_UPDATE_WSL:
            if (system("wsl.exe --update") == 0) {
                pResposta->statusWSL = 5;  // atualizado
            } else {
                pResposta->codigo = 500;
            }
            break;

        case CMD_REPAIR_WSL:
            // tenta desinstalar e instalar
            result = system("wsl.exe --uninstall");
            if (result == 0) {
                result = system("wsl.exe --install --no-distribution");
            }
            if (result == 0) {
                pResposta->statusWSL = 6;  // reparado
            } else {
                pResposta->codigo = 500;
            }
            break;

        default:
            pResposta->codigo = 400;   // comando inválido
            pResposta->statusWSL = 0;
    }
}
