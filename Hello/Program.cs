using System.Diagnostics;
using System.Security.Principal;
using System.Text.Json;
using System.Text.RegularExpressions;

const string service = "OrquestradorWSLService";
const string profile = "Orquestrador WSL";

string pasta = AppContext.BaseDirectory;
string maestro = Path.Combine(pasta, "MaestroWSL.exe");
string client = Path.Combine(pasta, "OrquestradorWSL.exe");
string booststrapper = Environment.ProcessPath!;

bool administrator = new WindowsPrincipal(
    WindowsIdentity.GetCurrent()
).IsInRole(WindowsBuiltInRole.Administrator);

if (!administrator)
{
    var elevated = new ProcessStartInfo(booststrapper)
    {
        UseShellExecute = true,
        Verb = "runas"
    };

    foreach (string arg in args)
        elevated.ArgumentList.Add(arg);

    Process.Start(elevated);
    return;
}

if (!File.Exists(maestro) || !File.Exists(client))
{
    throw new FileNotFoundException("Arquivos do Orquestrador não encontrados.");
};

    if (Sc("query", service).code != 0)
    {
        VerifyResult(Sc(
            "create", service,
            $"binPath=", $"\"{maestro}\"",
            "start=", "auto"
        ));
    }

        VerifyResult(Sc(
            "config", service,
            $"binPath=", $"\"{maestro}\"",
            "start=", "auto"
        ));

var state = Sc("query", service);
if (!Regex.IsMatch(state.output, @":\s*4\s+"))
{
    VerifyResult(Sc("start", service));
}

if (args.Contains("--terminal"))
{
    using var process = Process.Start(new ProcessStartInfo(client)
    {
        UseShellExecute = false,
        WorkingDirectory = pasta
    })!;

    process.WaitForExit();
    return;
}

string dir = Path.Combine(
    Environment.GetFolderPath(Environment.SpecialFolder.CommonApplicationData),
    "Microsoft", "Windows Terminal", "Fragments", "OrquestradorWSL"
);

Directory.CreateDirectory(dir);

var config = new
{
    profiles = new[] {new{
        name = profile,
        commandline = $"\"{booststrapper}\" --terminal",
        startingDirectory = pasta,
        elevate = true,
        supressApplicationTitle = true
    }}
};

File.WriteAllText(
    Path.Combine(dir, "profile.json"),
    JsonSerializer.Serialize(config,
        new JsonSerializerOptions { WriteIndented = true })
);

var terminal = new ProcessStartInfo("wt.exe")
{
    UseShellExecute = true
};

foreach (var argumento in new[]{
    "-w", "-1", "new-tab", "-p", profile
})
    terminal.ArgumentList.Add(argumento);

Process.Start(terminal);

static (int code, string output) Sc(params string[] arguments)
{
    var initiate = new ProcessStartInfo("sc.exe")
    {
        UseShellExecute = false,
        RedirectStandardOutput = true,
        RedirectStandardError = true
    };

    foreach (string argumento in arguments)
        initiate.ArgumentList.Add(argumento);

    using var process = Process.Start(initiate)!;
    string output = process.StandardOutput.ReadToEnd();
    string error = process.StandardError.ReadToEnd();

    process.WaitForExit();
    return (process.ExitCode, output + error);
}

static void VerifyResult((int codeResult, string outputResult) result) {
    if (result.codeResult != 0)
        throw new InvalidOperationException(result.outputResult);
}
