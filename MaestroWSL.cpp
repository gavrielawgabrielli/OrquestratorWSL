#include <windows.h>
#include <stdio.h>

char g_ServiceName[] = "OrquestradorWSLService";
#define SERVICE_NAME g_ServiceName
#define PIPE_NAME R"(\\.\pipe\OrquestradorWSLPipe)"
#define BUFFER_SIZE 128

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

VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv);
DWORD WINAPI ServiceCtrlHandlerEx(DWORD control, DWORD eventType, LPVOID eventData, LPVOID context);
DWORD WINAPI PipeWorkerThread(LPVOID lpParam);
void ProcessarComando(DWORD cmd);

int main() {
    SERVICE_TABLE_ENTRY ServiceTable[] = {
        { SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
        { NULL, NULL }
    };

    // Conecta o processo principal ao Gerenciador de Serviços do Windows (SCM)
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
    DWORD resposta = 200;


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
                ProcessarComando(comandoRecebido);
                WriteFile(hPipe, &resposta, sizeof(DWORD), &bytesWritten, NULL);
            }
        }

        DisconnectNamedPipe(hPipe);
        CloseHandle(hPipe);
    }
    return 0;
}

void ProcessarComando(DWORD cmd) {
    switch (cmd) {
        case CMD_ENABLE_WSL:
            system("dism.exe /online /enable-feature /featurename:Microsoft-Windows-Subsystem-Linux /all /norestart");
            system("dism.exe /online /enable-feature /featurename:VirtualMachinePlatform /all /norestart");
            break;
        case CMD_DISABLE_WSL:
            system("dism.exe /online /disable-feature /featurename:Microsoft-Windows-Subsystem-Linux /norestart");
            system("dism.exe /online /disable-feature /featurename:VirtualMachinePlatform /norestart");
            break;
        case CMD_INSTALL_WSL:
            system("wsl.exe --install --no-distribution");
            break;
        case CMD_UNINSTALL_WSL:
            system("wsl.exe --uninstall");
            break;
        case CMD_UPDATE_WSL:
            system("wsl.exe --update");
            break;
        case CMD_REPAIR_WSL:
            system("wsl.exe --uninstall");
            system("wsl.exe --install --no-distribution");
            break;
        default:
            break;
    }
}