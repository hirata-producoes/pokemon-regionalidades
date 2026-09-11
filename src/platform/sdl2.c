#ifdef PLATFORM_SDL2
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#include <xinput.h>
#include <SDL2/SDL_syswm.h>

extern const void *gPlatformLastCpuSetSource;
extern void *gPlatformLastCpuSetDestination;
extern uint32_t gPlatformLastCpuSetControl;
extern const void *gPlatformLastCpuSetCaller;

static LONG CALLBACK LogNativeException(EXCEPTION_POINTERS *exception)
{
    if (exception->ExceptionRecord->ExceptionCode != EXCEPTION_ACCESS_VIOLATION
     && exception->ExceptionRecord->ExceptionCode != EXCEPTION_INT_DIVIDE_BY_ZERO)
        return EXCEPTION_CONTINUE_SEARCH;

    fprintf(stdout, "PC port exception: code=0x%08lX address=%p eip=%p module=%p\n",
            exception->ExceptionRecord->ExceptionCode,
            exception->ExceptionRecord->ExceptionAddress,
            (void *)(uintptr_t)exception->ContextRecord->Eip,
            GetModuleHandle(NULL));
    DWORD *stack = (DWORD *)(uintptr_t)exception->ContextRecord->Esp;
    fprintf(stdout, "PC port registers: eax=%08lX ebx=%08lX ecx=%08lX edx=%08lX esi=%08lX edi=%08lX esp=%p ebp=%p\n",
            exception->ContextRecord->Eax,
            exception->ContextRecord->Ebx,
            exception->ContextRecord->Ecx,
            exception->ContextRecord->Edx,
            exception->ContextRecord->Esi,
            exception->ContextRecord->Edi,
            stack,
            (void *)(uintptr_t)exception->ContextRecord->Ebp);
    fprintf(stdout, "Last CpuSet: src=%p dst=%p control=%08lX caller=%p\n",
            gPlatformLastCpuSetSource,
            gPlatformLastCpuSetDestination,
            (unsigned long)gPlatformLastCpuSetControl,
            gPlatformLastCpuSetCaller);
    for (int i = 0; i < 20; i++)
        fprintf(stdout, " stack[%02d]=%08lX%s", i, stack[i], i % 4 == 3 ? "\n" : "");
    fflush(stdout);
    return EXCEPTION_CONTINUE_SEARCH;
}
#endif

#ifdef __ANDROID__
#include <jni.h>
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif
#ifdef NATIVE_LINUX
#include <SDL2/SDL_image.h>
#endif

#include "global.h"
#include "platform.h"
#include "rtc.h"
#include "gba/defines.h"
#include "gba/m4a_internal.h"
#include "cgb_audio.h"
#include "gba/flash_internal.h"
#include "platform/dma.h"
#include "platform/framedraw.h"
#include "platform/save_file.h"
#include "resource_pack.h"

extern void (*const gIntrTable[])(void);

SDL_Thread *mainLoopThread;
SDL_Window *sdlWindow;
SDL_Renderer *sdlRenderer;
SDL_Texture *sdlTexture;
#if defined(NATIVE_LINUX) || defined(_WIN32)
#define MAX_BORDER_BACKGROUNDS 15
SDL_Texture *sdlBackgroundTextures[MAX_BORDER_BACKGROUNDS];
SDL_Texture *sdlBorderTexture;
#endif
static u8 sBorderBackgroundCount = 1;
SDL_AudioDeviceID sdlAudioDevice;
SDL_sem *vBlankSemaphore;
SDL_atomic_t isFrameAvailable;
bool speedUp = false;
unsigned int videoScale = 1;
bool isRunning = true;
bool paused = false;
double simTime = 0;
double lastGameTime = 0;
double curGameTime = 0;
double fixedTimestep = 1.0 / 60.0; // 16.666667ms
double timeScale = 1.0;
struct SiiRtcInfo internalClock;
static time_t sRtcOffsetSeconds;

static char sSavePath[1024] = "pokemon_regionalidades.sav";
static char sConfigPath[1024] = "pokemon_regionalidades.cfg";
static u8 sBorderBackground;
static bool sHasBorderBackgroundConfig;
static u8 sBackgroundOrderVersion;
static u8 sPlatformSettings[PLATFORM_SETTING_COUNT] = {0, 4, 0, 1, 1, 10, 10, 10};
static bool sWindowResizable = true;
#if defined(NATIVE_LINUX) || defined(_WIN32)
static int sAppliedFullscreen = -1;
static int sAppliedWindowScale = -1;
static int sAppliedWindowResizable = -1;
#endif

enum PcKeyAction
{
    PC_KEY_A,
    PC_KEY_B,
    PC_KEY_START,
    PC_KEY_SELECT,
    PC_KEY_L,
    PC_KEY_R,
    PC_KEY_UP,
    PC_KEY_DOWN,
    PC_KEY_LEFT,
    PC_KEY_RIGHT,
    PC_KEY_SPEED,
    PC_KEY_COUNT,
};

static SDL_Keycode sKeyboardMappings[PC_KEY_COUNT] =
{
    SDLK_z, SDLK_x, SDLK_RETURN, SDLK_BACKSPACE, SDLK_a, SDLK_s,
    SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_SPACE,
};
static const char *const sKeyboardConfigNames[PC_KEY_COUNT] =
{
    "keyA", "keyB", "keyStart", "keySelect", "keyL", "keyR",
    "keyUp", "keyDown", "keyLeft", "keyRight", "keySpeed",
};
static const u16 sKeyboardButtonMasks[PC_KEY_SPEED] =
{
    A_BUTTON, B_BUTTON, START_BUTTON, SELECT_BUTTON, L_BUTTON, R_BUTTON,
    DPAD_UP, DPAD_DOWN, DPAD_LEFT, DPAD_RIGHT,
};
static unsigned int sSpeedMultiplier = 5;

#ifdef _WIN32
#define PC_MENU_GAME_PAUSE       1001
#define PC_MENU_GAME_RESTART     1002
#define PC_MENU_GAME_PROFILES    1003
#define PC_MENU_GAME_EXIT        1004
#define PC_MENU_SETTINGS         1101
#define PC_MENU_HELP_CONTROLS    1201

static HMENU sNativeMenu;
static FILETIME sConfigWriteTime;
static bool sHasConfigWriteTime;
static Uint32 sNextConfigPoll;
static bool sRestartRequested;
static bool sReturnToProfilesRequested;

enum PcControllerInput
{
    PC_CONTROLLER_A,
    PC_CONTROLLER_B,
    PC_CONTROLLER_X,
    PC_CONTROLLER_Y,
    PC_CONTROLLER_START,
    PC_CONTROLLER_BACK,
    PC_CONTROLLER_LB,
    PC_CONTROLLER_RB,
    PC_CONTROLLER_LEFT_STICK,
    PC_CONTROLLER_RIGHT_STICK,
    PC_CONTROLLER_LT,
    PC_CONTROLLER_RT,
    PC_CONTROLLER_INPUT_COUNT,
};

enum PcControllerAction
{
    PC_CONTROLLER_ACTION_A,
    PC_CONTROLLER_ACTION_B,
    PC_CONTROLLER_ACTION_START,
    PC_CONTROLLER_ACTION_SELECT,
    PC_CONTROLLER_ACTION_L,
    PC_CONTROLLER_ACTION_R,
    PC_CONTROLLER_ACTION_SPEED,
    PC_CONTROLLER_ACTION_COUNT,
};

static u8 sControllerMappings[PC_CONTROLLER_ACTION_COUNT] =
{
    PC_CONTROLLER_A, PC_CONTROLLER_X, PC_CONTROLLER_START,
    PC_CONTROLLER_BACK, PC_CONTROLLER_LB, PC_CONTROLLER_RB, PC_CONTROLLER_RT,
};
static const char *const sControllerConfigNames[PC_CONTROLLER_ACTION_COUNT] =
{
    "controllerA", "controllerB", "controllerStart", "controllerSelect",
    "controllerL", "controllerR", "controllerSpeed",
};
static const char *const sControllerInputNames[PC_CONTROLLER_INPUT_COUNT] =
{
    "A", "B", "X", "Y", "Start", "Back", "LB", "RB",
    "LeftStick", "RightStick", "LT", "RT",
};
static const u16 sControllerButtonMasks[PC_CONTROLLER_ACTION_SPEED] =
{
    A_BUTTON, B_BUTTON, START_BUTTON, SELECT_BUTTON, L_BUTTON, R_BUTTON,
};
#endif
#ifdef __ANDROID__
static SDL_GameController *androidController;
#endif

extern void AgbMain(void);
extern void DoSoftReset(void);

int DoMain(void *param);
void ProcessEvents(void);
void VDraw(SDL_Texture *texture);

static void ReadSaveFile(const char *path);
static void ReadConfigFile(void);
static void StoreConfigFile(void);
static void ApplyPlatformSettings(void);
static bool32 StoreSaveFile(void);
static bool FileExists(const char *path);
static void CopyLegacyFileIfNeeded(const char *legacyPath, const char *newPath);
static bool ResolveConfiguredFilePath(const char *variableName, char *path, size_t pathCapacity, bool *wasConfigured);
static bool OpenRegionalidadesResourcePack(void);
static bool ReadConfigTextValue(const char *line, const char *name, char *value, size_t valueCapacity);
static bool ReadKeyboardMapping(const char *line, enum PcKeyAction action);
#ifdef _WIN32
static bool ReadControllerMapping(const char *line, enum PcControllerAction action);
static bool IsControllerInputActive(const XINPUT_STATE *state, enum PcControllerInput input);
static void InstallNativeMenu(void);
static void HandleNativeMenuCommand(WORD command);
static void PollConfigFileChanges(void);
static void LaunchPowerShellScript(const char *environmentName);
static void RelaunchCurrentExecutable(void);
#endif
#ifdef _WIN32
static void ArchivePreviousCrashLog(void);
#endif

static void UpdateInternalClock(void);
static void SetInternalClockFromRtc(const struct SiiRtcInfo *rtc, bool includeDate);

static bool FileExists(const char *path)
{
    FILE *file = fopen(path, "rb");
    if (file == NULL)
        return false;
    fclose(file);
    return true;
}

#ifdef _WIN32
static void ArchivePreviousCrashLog(void)
{
    FILE *log = fopen("runtime-last.log", "r");
    char line[512];
    bool crashed = false;

    if (log == NULL)
        return;

    while (fgets(line, sizeof(line), log) != NULL)
    {
        if (strstr(line, "PC port exception:") != NULL)
        {
            crashed = true;
            break;
        }
    }
    fclose(log);

    if (crashed)
    {
        SYSTEMTIME timestamp;
        char archivePath[96];

        GetLocalTime(&timestamp);
        SDL_snprintf(archivePath, sizeof(archivePath),
                     "runtime-crash-%04u%02u%02u-%02u%02u%02u.log",
                     timestamp.wYear, timestamp.wMonth, timestamp.wDay,
                     timestamp.wHour, timestamp.wMinute, timestamp.wSecond);
        CopyFileA("runtime-last.log", archivePath, TRUE);
    }
}

static void LaunchPowerShellScript(const char *environmentName)
{
    const char *scriptPath = SDL_getenv(environmentName);
    char parameters[2300];

    if (scriptPath == NULL || scriptPath[0] == '\0')
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Pokemon Regionalidades",
                                 "Este comando so esta disponivel quando o jogo e aberto pela tela de perfis.",
                                 sdlWindow);
        return;
    }

    SDL_snprintf(parameters, sizeof(parameters),
                 "-NoProfile -WindowStyle Hidden -ExecutionPolicy Bypass -File \"%s\"", scriptPath);
    if ((INT_PTR)ShellExecuteA(NULL, "open", "powershell.exe", parameters, NULL, SW_HIDE) <= 32)
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Pokemon Regionalidades",
                                 "Nao foi possivel abrir a janela solicitada.", sdlWindow);
}

static void RelaunchCurrentExecutable(void)
{
    const char *profileRunner = SDL_getenv("POKEMON_REGIONALIDADES_PROFILE_RUNNER");
    const char *profileId = SDL_getenv("POKEMON_REGIONALIDADES_PROFILE_ID");
    char executable[MAX_PATH];
    char commandLine[MAX_PATH + 3];
    STARTUPINFOA startupInfo;
    PROCESS_INFORMATION processInfo;

    if (profileRunner != NULL && profileRunner[0] != '\0'
     && profileId != NULL && (profileId[0] == '1' || profileId[0] == '2') && profileId[1] == '\0')
    {
        char parameters[2300];
        SDL_snprintf(parameters, sizeof(parameters),
                     "-NoProfile -WindowStyle Hidden -ExecutionPolicy Bypass -File \"%s\" -Profile %s",
                     profileRunner, profileId);
        if ((INT_PTR)ShellExecuteA(NULL, "open", "powershell.exe", parameters, NULL, SW_HIDE) > 32)
            return;
    }

    if (GetModuleFileNameA(NULL, executable, sizeof(executable)) == 0)
        return;
    SDL_snprintf(commandLine, sizeof(commandLine), "\"%s\"", executable);
    memset(&startupInfo, 0, sizeof(startupInfo));
    memset(&processInfo, 0, sizeof(processInfo));
    startupInfo.cb = sizeof(startupInfo);
    if (CreateProcessA(executable, commandLine, NULL, NULL, FALSE, 0, NULL, NULL,
                       &startupInfo, &processInfo))
    {
        CloseHandle(processInfo.hThread);
        CloseHandle(processInfo.hProcess);
    }
}

static void UpdateNativePauseMenu(void)
{
    if (sNativeMenu != NULL)
        CheckMenuItem(sNativeMenu, PC_MENU_GAME_PAUSE,
                      MF_BYCOMMAND | (paused ? MF_CHECKED : MF_UNCHECKED));
}

static void HandleNativeMenuCommand(WORD command)
{
    switch (command)
    {
    case PC_MENU_GAME_PAUSE:
        paused = !paused;
        UpdateNativePauseMenu();
        break;
    case PC_MENU_GAME_RESTART:
        DBGPRINTF("PC shutdown: restart selected from menu\n");
        sRestartRequested = true;
        isRunning = false;
        break;
    case PC_MENU_GAME_PROFILES:
        DBGPRINTF("PC shutdown: returning to profile selection\n");
        sReturnToProfilesRequested = true;
        isRunning = false;
        break;
    case PC_MENU_GAME_EXIT:
        DBGPRINTF("PC shutdown: exit selected from menu\n");
        isRunning = false;
        break;
    case PC_MENU_SETTINGS:
        LaunchPowerShellScript("POKEMON_REGIONALIDADES_SETTINGS_SCRIPT");
        break;
    case PC_MENU_HELP_CONTROLS:
        SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_INFORMATION,
            "Controles",
            "Os controles podem ser alterados em Configuracoes > Controles, video e aceleracao.\n\n"
            "Atalhos do programa:\nCtrl+P: pausar ou continuar\nCtrl+R: reiniciar\n"
            "Alt+Enter: entrar ou sair da tela cheia",
            sdlWindow);
        break;
    }
}

static void InstallNativeMenu(void)
{
    SDL_SysWMinfo windowInfo;
    HMENU gameMenu = CreatePopupMenu();
    HMENU settingsMenu = CreatePopupMenu();
    HMENU helpMenu = CreatePopupMenu();

    SDL_VERSION(&windowInfo.version);
    if (!SDL_GetWindowWMInfo(sdlWindow, &windowInfo))
        return;

    sNativeMenu = CreateMenu();
    AppendMenuW(gameMenu, MF_STRING, PC_MENU_GAME_PAUSE, L"Pausar/Continuar\tCtrl+P");
    AppendMenuW(gameMenu, MF_STRING, PC_MENU_GAME_RESTART, L"Reiniciar\tCtrl+R");
    AppendMenuW(gameMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(gameMenu, MF_STRING, PC_MENU_GAME_PROFILES, L"Voltar aos perfis");
    AppendMenuW(gameMenu, MF_STRING, PC_MENU_GAME_EXIT, L"Sair");
    AppendMenuW(settingsMenu, MF_STRING, PC_MENU_SETTINGS, L"Controles, v\u00eddeo e acelera\u00e7\u00e3o...");
    AppendMenuW(helpMenu, MF_STRING, PC_MENU_HELP_CONTROLS, L"Controles e atalhos");
    AppendMenuW(sNativeMenu, MF_POPUP, (UINT_PTR)gameMenu, L"Jogo");
    AppendMenuW(sNativeMenu, MF_POPUP, (UINT_PTR)settingsMenu, L"Configura\u00e7\u00f5es");
    AppendMenuW(sNativeMenu, MF_POPUP, (UINT_PTR)helpMenu, L"Ajuda");
    SetMenu(windowInfo.info.win.window, sNativeMenu);
    DrawMenuBar(windowInfo.info.win.window);
    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
    UpdateNativePauseMenu();
}

static void RememberConfigWriteTime(void)
{
    WIN32_FILE_ATTRIBUTE_DATA attributes;
    if (GetFileAttributesExA(sConfigPath, GetFileExInfoStandard, &attributes))
    {
        sConfigWriteTime = attributes.ftLastWriteTime;
        sHasConfigWriteTime = true;
    }
}

static void PollConfigFileChanges(void)
{
    WIN32_FILE_ATTRIBUTE_DATA attributes;
    Uint32 now = SDL_GetTicks();

    if (!SDL_TICKS_PASSED(now, sNextConfigPoll))
        return;
    sNextConfigPoll = now + 500;
    if (!GetFileAttributesExA(sConfigPath, GetFileExInfoStandard, &attributes))
        return;
    if (!sHasConfigWriteTime || CompareFileTime(&attributes.ftLastWriteTime, &sConfigWriteTime) != 0)
    {
        sConfigWriteTime = attributes.ftLastWriteTime;
        sHasConfigWriteTime = true;
        speedUp = false;
        timeScale = 1.0;
        ReadConfigFile();
        ApplyPlatformSettings();
        DBGPRINTF("PC settings: configuration reloaded while running "
                  "(speed=%ux fullscreen=%u resizable=%u scale=%ux)\n",
                  sSpeedMultiplier,
                  sPlatformSettings[PLATFORM_SETTING_FULLSCREEN],
                  sWindowResizable,
                  sPlatformSettings[PLATFORM_SETTING_WINDOW_SCALE]);
    }
}
#endif

static bool OpenRegionalidadesResourcePack(void)
{
    const char *configuredPath = SDL_getenv("POKEMON_REGIONALIDADES_RESOURCE_PACK");
    const char *resourcePackPath = configuredPath;
    char executablePath[2048];
    char *basePath = NULL;

    if (resourcePackPath == NULL || resourcePackPath[0] == '\0')
    {
        configuredPath = SDL_getenv("POKEMON_GO_WORLD_RESOURCE_PACK");
        resourcePackPath = configuredPath;
    }

    if (resourcePackPath == NULL || resourcePackPath[0] == '\0')
    {
#ifndef __ANDROID__
        basePath = SDL_GetBasePath();
        if (basePath != NULL)
        {
            SDL_snprintf(executablePath, sizeof(executablePath),
                         "%spokemon_regionalidades.pak", basePath);
            if (FileExists(executablePath))
                resourcePackPath = executablePath;
            else
            {
                SDL_snprintf(executablePath, sizeof(executablePath),
                             "%spokemon_go_world.pak", basePath);
                if (FileExists(executablePath))
                    resourcePackPath = executablePath;
            }
        }
#endif
        if (resourcePackPath == NULL || resourcePackPath[0] == '\0')
            resourcePackPath = FileExists("pokemon_regionalidades.pak")
                ? "pokemon_regionalidades.pak"
                : "pokemon_go_world.pak";
    }

    DBGPRINTF("PC port: opening resource pack %s\n", resourcePackPath);
    bool opened = ResourcePack_Open(resourcePackPath);
    if (basePath != NULL)
        SDL_free(basePath);
    if (!opened)
    {
        SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_ERROR,
            "Pokemon Regionalidades",
            "O pacote pokemon_regionalidades.pak nao foi encontrado ou esta invalido. "
            "Coloque o arquivo ao lado do executavel ou configure "
            "POKEMON_REGIONALIDADES_RESOURCE_PACK.",
            NULL);
    }
    return opened;
}

static void CopyLegacyFileIfNeeded(const char *legacyPath, const char *newPath)
{
    unsigned char buffer[4096];
    size_t bytesRead;
    bool failed = false;
    FILE *source;
    FILE *destination;

    if (FileExists(newPath) || !FileExists(legacyPath))
        return;

    source = fopen(legacyPath, "rb");
    destination = fopen(newPath, "wb");
    if (source == NULL || destination == NULL)
    {
        if (source != NULL)
            fclose(source);
        if (destination != NULL)
            fclose(destination);
        remove(newPath);
        return;
    }

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), source)) != 0)
    {
        if (fwrite(buffer, 1, bytesRead, destination) != bytesRead)
        {
            failed = true;
            break;
        }
    }
    if (ferror(source))
        failed = true;

    fclose(source);
    if (fclose(destination) != 0)
        failed = true;

    if (failed)
        remove(newPath);
    else
        DBGPRINTF("PC port: copied legacy file %s to %s\n", legacyPath, newPath);
}

static bool ResolveConfiguredFilePath(const char *variableName, char *path, size_t pathCapacity, bool *wasConfigured)
{
    const char *configuredPath = SDL_getenv(variableName);

    *wasConfigured = false;
    if (configuredPath == NULL || configuredPath[0] == '\0')
        return true;
    if ((size_t)SDL_snprintf(path, pathCapacity, "%s", configuredPath) >= pathCapacity)
    {
        SDL_Log("Configured path is too long: %s", variableName);
        return false;
    }

    *wasConfigured = true;
    return true;
}

#ifdef __ANDROID__
static void HandleTouchEvent(const SDL_TouchFingerEvent *event);
static void DrawTouchControls(void);
#endif

int main(int argc, char **argv)
{
    bool hasConfiguredSavePath;
    bool hasConfiguredConfigPath;

    // Open an output console on Windows
#ifdef _WIN32
    const char *diagnosticLogPath = SDL_getenv("POKEMON_REGIONALIDADES_LOG_PATH");
    const char *diagnosticDir = SDL_getenv("POKEMON_GO_WORLD_CAPTURE_DIR");
    if (diagnosticLogPath != NULL && diagnosticLogPath[0] != '\0')
    {
        freopen(diagnosticLogPath, "w", stdout);
    }
    else if (diagnosticDir != NULL && diagnosticDir[0] != '\0')
    {
        char logPath[1200];
        SDL_snprintf(logPath, sizeof(logPath), "%s/runtime.log", diagnosticDir);
        freopen(logPath, "w", stdout);
    }
    else
    {
        // Keep diagnostics available when launched by desktop shortcut.
        ArchivePreviousCrashLog();
        freopen("runtime-last.log", "w", stdout);
    }
    setvbuf(stdout, NULL, _IONBF, 0);
    AddVectoredExceptionHandler(1, LogNativeException);
#endif

    DBGPRINTF("PC port: entering SDL main\n");

#ifdef __ANDROID__
    SDL_setenv("SDL_AUDIODRIVER", "openslES", 1);
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
    SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0");
#endif
    DBGPRINTF("PC port: initializing SDL\n");
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO
#ifdef __ANDROID__
                | SDL_INIT_GAMECONTROLLER
#endif
                ) < 0)
    {
        DBGPRINTF("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    DBGPRINTF("PC port: SDL initialized\n");

    if (!OpenRegionalidadesResourcePack())
    {
        SDL_Quit();
        return 1;
    }

    if (!ResolveConfiguredFilePath("POKEMON_REGIONALIDADES_SAVE_PATH",
                                   sSavePath, sizeof(sSavePath), &hasConfiguredSavePath)
     || !ResolveConfiguredFilePath("POKEMON_REGIONALIDADES_CONFIG_PATH",
                                   sConfigPath, sizeof(sConfigPath), &hasConfiguredConfigPath))
    {
        SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_ERROR,
            "Pokemon Regionalidades",
            "O caminho configurado para os dados do jogador e muito longo.",
            NULL);
        ResourcePack_Close();
        SDL_Quit();
        return 1;
    }

#ifdef __ANDROID__
    for (int i = 0; i < SDL_NumJoysticks() && androidController == NULL; i++)
    {
        if (SDL_IsGameController(i))
            androidController = SDL_GameControllerOpen(i);
    }
#endif

#ifdef __ANDROID__
    char *prefPath = SDL_GetPrefPath("pokeemerald", "pokeemerald");
    if (prefPath != NULL)
    {
        char legacySavePath[1024];
        char legacyConfigPath[1024];
        if (!hasConfiguredSavePath)
            SDL_snprintf(sSavePath, sizeof(sSavePath), "%spokemon_regionalidades.sav", prefPath);
        if (!hasConfiguredConfigPath)
            SDL_snprintf(sConfigPath, sizeof(sConfigPath), "%spokemon_regionalidades.cfg", prefPath);
        SDL_snprintf(legacySavePath, sizeof(legacySavePath), "%spokemon_go_world.sav", prefPath);
        SDL_snprintf(legacyConfigPath, sizeof(legacyConfigPath), "%spokemon_go_world.cfg", prefPath);
        if (!hasConfiguredSavePath)
            CopyLegacyFileIfNeeded(legacySavePath, sSavePath);
        if (!hasConfiguredConfigPath)
            CopyLegacyFileIfNeeded(legacyConfigPath, sConfigPath);
        SDL_free(prefPath);
    }
#else
    if (!hasConfiguredSavePath)
        CopyLegacyFileIfNeeded("pokemon_go_world.sav", sSavePath);
    if (!hasConfiguredConfigPath)
        CopyLegacyFileIfNeeded("pokemon_go_world.cfg", sConfigPath);
#endif
    DBGPRINTF("PC port: save path %s\n", sSavePath);
    DBGPRINTF("PC port: config path %s\n", sConfigPath);
    DBGPRINTF("PC port: reading save\n");
    ReadSaveFile(sSavePath);
    DBGPRINTF("PC port: save loaded\n");
    ReadConfigFile();
#ifdef _WIN32
    RememberConfigWriteTime();
#endif
    char keyAName[32];
    char keyBName[32];
    char keySpeedName[32];
    SDL_snprintf(keyAName, sizeof(keyAName), "%s", SDL_GetKeyName(sKeyboardMappings[PC_KEY_A]));
    SDL_snprintf(keyBName, sizeof(keyBName), "%s", SDL_GetKeyName(sKeyboardMappings[PC_KEY_B]));
    SDL_snprintf(keySpeedName, sizeof(keySpeedName), "%s", SDL_GetKeyName(sKeyboardMappings[PC_KEY_SPEED]));
    DBGPRINTF("PC controls: keyA=%s keyB=%s keySpeed=%s speed=%ux\n",
              keyAName, keyBName, keySpeedName,
              sSpeedMultiplier);
    DBGPRINTF("PC video: fullscreen=%u resizable=%u scale=%ux integer=%u vsync=%u border=%u\n",
              sPlatformSettings[PLATFORM_SETTING_FULLSCREEN], sWindowResizable,
              sPlatformSettings[PLATFORM_SETTING_WINDOW_SCALE],
              sPlatformSettings[PLATFORM_SETTING_INTEGER_SCALE],
              sPlatformSettings[PLATFORM_SETTING_VSYNC],
              sPlatformSettings[PLATFORM_SETTING_BORDER]);
    DBGPRINTF("PC audio: master=%u music=%u effects=%u\n",
              sPlatformSettings[PLATFORM_SETTING_VOLUME],
              sPlatformSettings[PLATFORM_SETTING_MUSIC_VOLUME],
              sPlatformSettings[PLATFORM_SETTING_EFFECTS_VOLUME]);
#ifdef _WIN32
    DBGPRINTF("PC controller: A=%s B=%s speed=%s\n",
              sControllerInputNames[sControllerMappings[PC_CONTROLLER_ACTION_A]],
              sControllerInputNames[sControllerMappings[PC_CONTROLLER_ACTION_B]],
              sControllerInputNames[sControllerMappings[PC_CONTROLLER_ACTION_SPEED]]);
#endif
    DBGPRINTF("PC port: save and config loaded\n");

#ifdef __ANDROID__
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
#endif
#if defined(NATIVE_LINUX) || defined(_WIN32)
    sdlWindow = SDL_CreateWindow("Pokemon Regionalidades", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
#else
    sdlWindow = SDL_CreateWindow("pokeemerald", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, DISPLAY_WIDTH * videoScale, DISPLAY_HEIGHT * videoScale, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
#endif
    if (sdlWindow == NULL)
    {
        DBGPRINTF("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    DBGPRINTF("PC port: window created\n");
#ifdef _WIN32
    InstallNativeMenu();
#endif

#ifdef __ANDROID__
    sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED);
#else
    sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_PRESENTVSYNC);
#endif
    if (sdlRenderer == NULL)
    {
        DBGPRINTF("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    DBGPRINTF("PC port: renderer created\n");

    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
    SDL_RenderClear(sdlRenderer);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    for (int i = 1; i < 15; i++)
    {
        char filename[16];
#ifdef _WIN32
        snprintf(filename, sizeof(filename), "BG%d.bmp", i);
#else
        snprintf(filename, sizeof(filename), "BG%d.png", i);
#endif
        SDL_RWops *backgroundFile = SDL_RWFromFile(filename, "rb");
        if (backgroundFile == NULL)
            break;
        SDL_RWclose(backgroundFile);
        sBorderBackgroundCount++;
    }
    if (sBackgroundOrderVersion < 2)
    {
        if (sHasBorderBackgroundConfig)
        {
            if (sBorderBackground == 1)
                sBorderBackground = sBorderBackgroundCount;
            else if (sBorderBackground >= 2)
                sBorderBackground--;
        }
        sBackgroundOrderVersion = 2;
        StoreConfigFile();
    }
#ifdef NATIVE_LINUX
    SDL_RenderSetLogicalSize(sdlRenderer, 0, 0);
    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0)
    {
        SDL_Log("SDL_image could not initialize: %s", IMG_GetError());
    }
    else
    {
        for (int i = 0; i < sBorderBackgroundCount; i++)
        {
            char filename[16];
            snprintf(filename, sizeof(filename), i == 0 ? "BG.png" : "BG%d.png", i);
            sdlBackgroundTextures[i] = IMG_LoadTexture(sdlRenderer, filename);
        }
        sdlBorderTexture = IMG_LoadTexture(sdlRenderer, "Border.png");
        if (sdlBackgroundTextures[0] == NULL)
            SDL_Log("Background image could not be loaded: %s", IMG_GetError());
        if (sdlBorderTexture == NULL)
            SDL_Log("Border image could not be loaded: %s", IMG_GetError());
    }
#elif defined(_WIN32)
    SDL_RenderSetLogicalSize(sdlRenderer, 0, 0);
    SDL_Surface *borderSurface = SDL_LoadBMP("Border.bmp");
    for (int i = 0; i < sBorderBackgroundCount; i++)
    {
        char filename[16];
        snprintf(filename, sizeof(filename), i == 0 ? "BG.bmp" : "BG%d.bmp", i);
        SDL_Surface *backgroundSurface = SDL_LoadBMP(filename);
        if (backgroundSurface == NULL)
            continue;
        sdlBackgroundTextures[i] = SDL_CreateTextureFromSurface(sdlRenderer, backgroundSurface);
        SDL_FreeSurface(backgroundSurface);
    }
    if (sdlBackgroundTextures[0] == NULL)
        SDL_Log("Background image could not be loaded: %s", SDL_GetError());
    if (borderSurface == NULL)
    {
        SDL_Log("Border image could not be loaded: %s", SDL_GetError());
    }
    else
    {
        sdlBorderTexture = SDL_CreateTextureFromSurface(sdlRenderer, borderSurface);
        SDL_FreeSurface(borderSurface);
    }
#else
    SDL_RenderSetLogicalSize(sdlRenderer, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    SDL_RenderSetIntegerScale(sdlRenderer, SDL_TRUE);
#endif
    ApplyPlatformSettings();

    sdlTexture = SDL_CreateTexture(sdlRenderer,
                                   SDL_PIXELFORMAT_ARGB8888,
                                   SDL_TEXTUREACCESS_STREAMING,
                                   DISPLAY_WIDTH, DISPLAY_HEIGHT);
    if (sdlTexture == NULL)
    {
        DBGPRINTF("Texture could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    SDL_SetTextureBlendMode(sdlTexture, SDL_BLENDMODE_NONE);

    simTime = curGameTime = lastGameTime = SDL_GetPerformanceCounter();

    isFrameAvailable.value = 0;
    vBlankSemaphore = SDL_CreateSemaphore(0);

    SDL_AudioSpec want;

    SDL_memset(&want, 0, sizeof(want)); /* or SDL_zero(want) */
    want.freq = 42060;
    want.format = AUDIO_F32;
    want.channels = 2;
    want.samples = 1024;
    cgb_audio_init(want.freq);


    sdlAudioDevice = SDL_OpenAudioDevice(NULL, 0, &want, NULL, 0);
    if (sdlAudioDevice == 0)
        SDL_Log("Failed to open audio: %s", SDL_GetError());
    else
    {
        if (want.format != AUDIO_F32) /* we let this one thing change. */
            SDL_Log("We didn't get Float32 audio format.");
        SDL_PauseAudioDevice(sdlAudioDevice, 0);
    }
#ifndef __ANDROID__
    VDraw(sdlTexture);
#endif

    // Initialize the emulated cartridge RTC before the game thread can probe
    // it. The project may choose its save-backed fake clock at compile time,
    // but the native SII backend remains available and follows host time.
    memset(&internalClock, 0, sizeof(internalClock));
    internalClock.status = SIIRTCINFO_24HOUR;
    UpdateInternalClock();
    if (Platform_GetEnvironmentFlag("POKEMON_GO_WORLD_TEST_RTC"))
    {
        struct SiiRtcInfo rtc;
        SiiRtcUnprotect();
        if (SiiRtcGetDateTime(&rtc))
            DBGPRINTF("PC RTC test: SII backend read succeeded\n");
        else
            DBGPRINTF("PC RTC test: SII backend read failed\n");
    }

    mainLoopThread = SDL_CreateThread(DoMain, "AgbMain", NULL);

    double accumulator = 0.0;

    while (isRunning)
    {
        ProcessEvents();
#ifdef _WIN32
        PollConfigFileChanges();
#endif

        if (!paused)
        {
            double dt = fixedTimestep / timeScale; // TODO: Fix speedup

            curGameTime = SDL_GetPerformanceCounter();
            double deltaTime = (double)((curGameTime - lastGameTime) / (double)SDL_GetPerformanceFrequency());
            // Limit only real host stalls. Basing this guard on the accelerated
            // timestep makes every multiplier above 5x look like a stall at
            // 60 Hz and collapses acceleration back to roughly normal speed.
            if (deltaTime > (fixedTimestep * 5))
                deltaTime = fixedTimestep;
            lastGameTime = curGameTime;

            accumulator += deltaTime;

            while (accumulator >= dt)
            {
                if (SDL_AtomicGet(&isFrameAvailable))
                {
                    VDraw(sdlTexture);
                    SDL_RenderClear(sdlRenderer);
#if defined(NATIVE_LINUX) || defined(_WIN32)
                    u8 backgroundOption = Platform_GetBorderBackground();
                    if (backgroundOption < sBorderBackgroundCount
                     && sdlBackgroundTextures[backgroundOption] != NULL)
                        SDL_RenderCopy(sdlRenderer, sdlBackgroundTextures[backgroundOption], NULL, NULL);
                    int outputWidth;
                    int outputHeight;
                    SDL_GetRendererOutputSize(sdlRenderer, &outputWidth, &outputHeight);
                    int gameHeight;
                    int gameWidth;
                    if (sPlatformSettings[PLATFORM_SETTING_INTEGER_SCALE])
                    {
                        int scale = outputWidth / DISPLAY_WIDTH;
                        if (outputHeight / DISPLAY_HEIGHT < scale)
                            scale = outputHeight / DISPLAY_HEIGHT;
                        if (scale < 1)
                            scale = 1;
                        gameWidth = DISPLAY_WIDTH * scale;
                        gameHeight = DISPLAY_HEIGHT * scale;
                    }
                    else
                    {
                        // Use the largest 3:2 viewport that fits the window. A
                        // previous 8/9 inset left an unnecessary black margin.
                        gameWidth = outputWidth;
                        gameHeight = gameWidth * DISPLAY_HEIGHT / DISPLAY_WIDTH;
                        if (gameHeight > outputHeight)
                        {
                            gameHeight = outputHeight;
                            gameWidth = gameHeight * DISPLAY_WIDTH / DISPLAY_HEIGHT;
                        }
                    }
                    SDL_Rect gameViewport = {(outputWidth - gameWidth) / 2,
                                             (outputHeight - gameHeight) / 2,
                                             gameWidth, gameHeight};
                    SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &gameViewport);
                    if (sPlatformSettings[PLATFORM_SETTING_BORDER] && sdlBorderTexture != NULL)
                    {
                        SDL_Rect borderSource = {141, 18, 1000, 683};
                        int innerWidth = gameViewport.w - 2;
                        int innerHeight = gameViewport.h - 2;
                        SDL_Rect borderViewport = {
                            gameViewport.x + 1 - innerWidth * 19 / 961,
                            gameViewport.y + 1 - innerHeight * 20 / 643,
                            innerWidth * 1000 / 961,
                            innerHeight * 683 / 643
                        };
                        SDL_RenderCopy(sdlRenderer, sdlBorderTexture, &borderSource, &borderViewport);
                    }
#else
                    SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
#endif
#ifdef __ANDROID__
                    SDL_RenderPresent(sdlRenderer);
#endif
                    SDL_AtomicSet(&isFrameAvailable, 0);

                    REG_DISPSTAT |= INTR_FLAG_VBLANK;

                    RunDMAs(DMA_HBLANK);

                    if (REG_IE & INTR_FLAG_VBLANK)
                        gIntrTable[4]();
                    REG_DISPSTAT &= ~INTR_FLAG_VBLANK;

                    SDL_SemPost(vBlankSemaphore);

                    accumulator -= dt;
                }
            }
        }

#ifndef __ANDROID__
        SDL_RenderPresent(sdlRenderer);
#endif
    }

    ResourcePack_Close();

#if defined(NATIVE_LINUX) || defined(_WIN32)
    for (int i = 0; i < sBorderBackgroundCount; i++)
        SDL_DestroyTexture(sdlBackgroundTextures[i]);
    SDL_DestroyTexture(sdlBorderTexture);
#endif
#ifdef NATIVE_LINUX
    IMG_Quit();
#endif
#ifdef _WIN32
    if (sNativeMenu != NULL)
    {
        SDL_SysWMinfo windowInfo;
        SDL_VERSION(&windowInfo.version);
        if (SDL_GetWindowWMInfo(sdlWindow, &windowInfo))
            SetMenu(windowInfo.info.win.window, NULL);
        DestroyMenu(sNativeMenu);
        sNativeMenu = NULL;
    }
#endif
    SDL_DestroyWindow(sdlWindow);
    sdlWindow = NULL;
    SDL_Quit();
#ifdef _WIN32
    if (sReturnToProfilesRequested)
        LaunchPowerShellScript("POKEMON_REGIONALIDADES_PROFILE_LAUNCHER");
    else if (sRestartRequested)
        RelaunchCurrentExecutable();
#endif
    return 0;
}

static void ReadSaveFile(const char *path)
{
    if (!PlatformSave_Load(path, FLASH_BASE, sizeof(FLASH_BASE)))
        SDL_Log("Unable to open save file: %s", path);
}

static bool ReadConfigTextValue(const char *line, const char *name, char *value, size_t valueCapacity)
{
    size_t nameLength = strlen(name);
    size_t valueLength;

    if (strncmp(line, name, nameLength) != 0 || line[nameLength] != '=')
        return false;

    line += nameLength + 1;
    valueLength = strcspn(line, "\r\n");
    if (valueLength == 0 || valueLength >= valueCapacity)
        return true;

    memcpy(value, line, valueLength);
    value[valueLength] = '\0';
    return true;
}

static bool ReadKeyboardMapping(const char *line, enum PcKeyAction action)
{
    char value[64] = {0};

    if (!ReadConfigTextValue(line, sKeyboardConfigNames[action], value, sizeof(value)))
        return false;
    if (value[0] != '\0')
    {
        SDL_Keycode key = SDL_GetKeyFromName(value);
        if (key != SDLK_UNKNOWN)
            sKeyboardMappings[action] = key;
    }
    return true;
}

#ifdef _WIN32
static bool ReadControllerMapping(const char *line, enum PcControllerAction action)
{
    char value[32] = {0};

    if (!ReadConfigTextValue(line, sControllerConfigNames[action], value, sizeof(value)))
        return false;
    for (int input = 0; input < PC_CONTROLLER_INPUT_COUNT; input++)
    {
        if (strcmp(value, sControllerInputNames[input]) == 0)
        {
            sControllerMappings[action] = input;
            break;
        }
    }
    return true;
}
#endif

static void ReadConfigFile(void)
{
    FILE *configFile = fopen(sConfigPath, "r");
    char line[64];
    unsigned int value;
    long long signedValue;

    if (configFile == NULL)
        return;
    while (fgets(line, sizeof(line), configFile) != NULL)
    {
        if (sscanf(line, "borderBackground=%u", &value) == 1 && value < 16)
        {
            sBorderBackground = value;
            sHasBorderBackgroundConfig = true;
        }
        else if (sscanf(line, "backgroundOrder=%u", &value) == 1)
            sBackgroundOrderVersion = value;
        else if (sscanf(line, "fullscreen=%u", &value) == 1)
            sPlatformSettings[PLATFORM_SETTING_FULLSCREEN] = value != 0;
        else if (sscanf(line, "windowScale=%u", &value) == 1 && value >= 2 && value <= 5)
            sPlatformSettings[PLATFORM_SETTING_WINDOW_SCALE] = value;
        else if (sscanf(line, "windowResizable=%u", &value) == 1)
            sWindowResizable = value != 0;
        else if (sscanf(line, "integerScale=%u", &value) == 1)
            sPlatformSettings[PLATFORM_SETTING_INTEGER_SCALE] = value != 0;
        else if (sscanf(line, "vsync=%u", &value) == 1)
            sPlatformSettings[PLATFORM_SETTING_VSYNC] = value != 0;
        else if (sscanf(line, "border=%u", &value) == 1)
            sPlatformSettings[PLATFORM_SETTING_BORDER] = value != 0;
        else if (sscanf(line, "volume=%u", &value) == 1 && value <= 10)
            sPlatformSettings[PLATFORM_SETTING_VOLUME] = value;
        else if (sscanf(line, "musicVolume=%u", &value) == 1 && value <= 10)
            sPlatformSettings[PLATFORM_SETTING_MUSIC_VOLUME] = value;
        else if (sscanf(line, "effectsVolume=%u", &value) == 1 && value <= 10)
            sPlatformSettings[PLATFORM_SETTING_EFFECTS_VOLUME] = value;
        else if (sscanf(line, "rtcOffsetSeconds=%lld", &signedValue) == 1)
            sRtcOffsetSeconds = (time_t)signedValue;
        else if (sscanf(line, "speedMultiplier=%u", &value) == 1 && value >= 2 && value <= 10)
            sSpeedMultiplier = value;
        else
        {
            bool mappingRead = false;
            for (int action = 0; action < PC_KEY_COUNT && !mappingRead; action++)
                mappingRead = ReadKeyboardMapping(line, action);
#ifdef _WIN32
            for (int action = 0; action < PC_CONTROLLER_ACTION_COUNT && !mappingRead; action++)
                mappingRead = ReadControllerMapping(line, action);
#endif
        }
    }
    fclose(configFile);
}

static void StoreConfigFile(void)
{
    FILE *configFile = fopen(sConfigPath, "w");

    if (configFile == NULL)
        return;
    fprintf(configFile, "borderBackground=%u\n", sBorderBackground);
    fprintf(configFile, "backgroundOrder=2\n");
    fprintf(configFile, "fullscreen=%u\n", sPlatformSettings[PLATFORM_SETTING_FULLSCREEN]);
    fprintf(configFile, "windowScale=%u\n", sPlatformSettings[PLATFORM_SETTING_WINDOW_SCALE]);
    fprintf(configFile, "windowResizable=%u\n", sWindowResizable);
    fprintf(configFile, "integerScale=%u\n", sPlatformSettings[PLATFORM_SETTING_INTEGER_SCALE]);
    fprintf(configFile, "vsync=%u\n", sPlatformSettings[PLATFORM_SETTING_VSYNC]);
    fprintf(configFile, "border=%u\n", sPlatformSettings[PLATFORM_SETTING_BORDER]);
    fprintf(configFile, "volume=%u\n", sPlatformSettings[PLATFORM_SETTING_VOLUME]);
    fprintf(configFile, "musicVolume=%u\n", sPlatformSettings[PLATFORM_SETTING_MUSIC_VOLUME]);
    fprintf(configFile, "effectsVolume=%u\n", sPlatformSettings[PLATFORM_SETTING_EFFECTS_VOLUME]);
    fprintf(configFile, "rtcOffsetSeconds=%lld\n", (long long)sRtcOffsetSeconds);
    fprintf(configFile, "speedMultiplier=%u\n", sSpeedMultiplier);
    for (int action = 0; action < PC_KEY_COUNT; action++)
        fprintf(configFile, "%s=%s\n", sKeyboardConfigNames[action], SDL_GetKeyName(sKeyboardMappings[action]));
#ifdef _WIN32
    for (int action = 0; action < PC_CONTROLLER_ACTION_COUNT; action++)
        fprintf(configFile, "%s=%s\n", sControllerConfigNames[action], sControllerInputNames[sControllerMappings[action]]);
#endif
    fclose(configFile);
}

static void ApplyPlatformSettings(void)
{
    SDL_RenderSetVSync(sdlRenderer, sPlatformSettings[PLATFORM_SETTING_VSYNC]);
#if defined(NATIVE_LINUX) || defined(_WIN32)
    int fullscreen = sPlatformSettings[PLATFORM_SETTING_FULLSCREEN] != 0;
    int scale = sPlatformSettings[PLATFORM_SETTING_WINDOW_SCALE];
    int resizable = sWindowResizable != 0;
    bool fullscreenChanged = fullscreen != sAppliedFullscreen;
    bool scaleChanged = scale != sAppliedWindowScale;
    bool resizableChanged = resizable != sAppliedWindowResizable;

    if (fullscreenChanged)
        SDL_SetWindowFullscreen(sdlWindow, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    SDL_SetWindowResizable(sdlWindow, resizable ? SDL_TRUE : SDL_FALSE);
    SDL_SetWindowMinimumSize(sdlWindow, 640, 360);
    if (!fullscreen && (scaleChanged || (fullscreenChanged && sAppliedFullscreen == 1)
                     || (resizableChanged && !resizable)))
    {
        SDL_SetWindowSize(sdlWindow, 320 * scale, 180 * scale);
        SDL_SetWindowPosition(sdlWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    }
    sAppliedFullscreen = fullscreen;
    sAppliedWindowScale = scale;
    sAppliedWindowResizable = resizable;
#endif
}

static bool32 StoreSaveFile(void)
{
    if (!PlatformSave_Commit(sSavePath, FLASH_BASE, sizeof(FLASH_BASE), 3))
    {
        SDL_Log("Unable to store save file safely: %s", sSavePath);
        return FALSE;
    }

    return TRUE;
}

bool32 Platform_StoreSaveFile(void)
{
    return StoreSaveFile();
}

bool32 Platform_GetEnvironmentFlag(const char *name)
{
    const char *value = SDL_getenv(name);
    return value != NULL && value[0] != '\0' && value[0] != '0';
}

const char *Platform_GetEnvironmentValue(const char *name)
{
    return SDL_getenv(name);
}

void Platform_ReadFlash(u16 sectorNum, u32 offset, u8 *dest, u32 size)
{
    u32 sourceOffset = (sectorNum << gFlash->sector.shift) + offset;

    if (sourceOffset > sizeof(FLASH_BASE) || size > sizeof(FLASH_BASE) - sourceOffset)
        return;
    memcpy(dest, &FLASH_BASE[sourceOffset], size);
}

void Platform_QueueAudio(float *audioBuffer, s32 samplesPerFrame)
{
    static bool8 sAudioDiagnosticReported;

    if (sdlAudioDevice != 0 && timeScale <= 1.0)
    {
        int floatCount = samplesPerFrame / sizeof(float);
        float adjustedAudio[floatCount];
        float volume = sPlatformSettings[PLATFORM_SETTING_VOLUME] / 10.0f;
        float peak = 0.0f;
        for (int i = 0; i < floatCount; i++)
        {
            adjustedAudio[i] = audioBuffer[i] * volume;
            float magnitude = adjustedAudio[i] < 0.0f ? -adjustedAudio[i] : adjustedAudio[i];
            if (magnitude > peak)
                peak = magnitude;
        }
        if (!sAudioDiagnosticReported
         && peak > 0.0001f
         && Platform_GetEnvironmentFlag("POKEMON_GO_WORLD_TEST_AUDIO"))
        {
            DBGPRINTF("PC audio test: non-silent SDL buffer, peak=%f, bytes=%d\n", peak, samplesPerFrame);
            sAudioDiagnosticReported = TRUE;
        }
        if (SDL_QueueAudio(sdlAudioDevice, adjustedAudio, samplesPerFrame) < 0)
            SDL_Log("Failed to queue audio: %s", SDL_GetError());
    }
}

u8 Platform_GetBorderBackgroundCount(void)
{
    return sBorderBackgroundCount + 1;
}

u8 Platform_GetBorderBackground(void)
{
    if (sHasBorderBackgroundConfig)
        return sBorderBackground;
    return 0;
}

void Platform_SetBorderBackground(u8 selection)
{
    sBorderBackground = selection;
    sHasBorderBackgroundConfig = true;
    StoreConfigFile();
}

u8 Platform_GetSetting(enum PlatformSetting setting)
{
    return sPlatformSettings[setting];
}

void Platform_SetSetting(enum PlatformSetting setting, u8 value)
{
    sPlatformSettings[setting] = value;
    if (setting == PLATFORM_SETTING_VSYNC)
        SDL_RenderSetVSync(sdlRenderer, value);
#if defined(NATIVE_LINUX) || defined(_WIN32)
    else if (setting == PLATFORM_SETTING_FULLSCREEN)
    {
        SDL_SetWindowFullscreen(sdlWindow, value ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
        if (!value)
        {
            int scale = sPlatformSettings[PLATFORM_SETTING_WINDOW_SCALE];
            SDL_SetWindowSize(sdlWindow, 320 * scale, 180 * scale);
            SDL_SetWindowPosition(sdlWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        }
    }
    else if (setting == PLATFORM_SETTING_WINDOW_SCALE && !sPlatformSettings[PLATFORM_SETTING_FULLSCREEN])
    {
        SDL_SetWindowSize(sdlWindow, 320 * value, 180 * value);
        SDL_SetWindowPosition(sdlWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    }
#endif
    StoreConfigFile();
}

#ifdef __ANDROID__
JNIEXPORT jint JNICALL Java_com_pokeemerald_experimental_GbaControlsView_getBorderBackground(JNIEnv *env, jclass clazz)
{
    return Platform_GetBorderBackground();
}

JNIEXPORT jint JNICALL Java_com_pokeemerald_experimental_GbaControlsView_getPlatformSetting(JNIEnv *env, jclass clazz, jint setting)
{
    if (setting < 0 || setting >= PLATFORM_SETTING_COUNT)
        return 0;
    return Platform_GetSetting(setting);
}
#endif


static u16 keyboardKeys;
static u16 keyboardPressedKeys;

static u16 KeyboardButtonMask(SDL_Keycode key)
{
    u16 mask = 0;

    for (int action = 0; action < PC_KEY_SPEED; action++)
    {
        if (sKeyboardMappings[action] == key)
            mask |= sKeyboardButtonMasks[action];
    }
    return mask;
}

#ifdef __ANDROID__
#define MAX_TOUCH_FINGERS 10

struct TouchFinger
{
    SDL_FingerID id;
    float x;
    float y;
    bool active;
};

static struct TouchFinger touchFingers[MAX_TOUCH_FINGERS];
static u16 touchKeys;
static u16 controllerKeys;
static u16 controllerAxisKeys;
static Sint16 controllerAxisX;
static Sint16 controllerAxisY;

static bool IsInsideRect(int x, int y, SDL_Rect rect)
{
    SDL_Point point = {x, y};
    return SDL_PointInRect(&point, &rect);
}

static int MinInt(int a, int b)
{
    return a < b ? a : b;
}

static int GetControlSideWidth(int windowWidth, int windowHeight)
{
    int sideWidth = (windowWidth - windowHeight * 3 / 2) / 2;
    int minimumWidth = windowWidth * 14 / 100;
    return sideWidth > minimumWidth ? sideWidth : minimumWidth;
}

static void UpdateTouchKeys(void)
{
    int windowWidth;
    int windowHeight;
    SDL_GetWindowSize(sdlWindow, &windowWidth, &windowHeight);
    int sideWidth = GetControlSideWidth(windowWidth, windowHeight);
    int buttonSize = MinInt(sideWidth * 2 / 5, windowHeight / 6);
    int dpadUnit = MinInt(sideWidth / 3, windowHeight / 8);
    int dpadX = sideWidth * 2 / 3;
    int dpadY = windowHeight * 7 / 10;
    SDL_Rect dpadUp = {dpadX - dpadUnit / 2, dpadY - dpadUnit * 3 / 2,
                       dpadUnit, dpadUnit};
    SDL_Rect dpadDown = {dpadX - dpadUnit / 2, dpadY + dpadUnit / 2,
                         dpadUnit, dpadUnit};
    SDL_Rect dpadLeft = {dpadX - dpadUnit * 3 / 2, dpadY - dpadUnit / 2,
                         dpadUnit, dpadUnit};
    SDL_Rect dpadRight = {dpadX + dpadUnit / 2, dpadY - dpadUnit / 2,
                          dpadUnit, dpadUnit};
    SDL_Rect aButton = {windowWidth - sideWidth / 4 - buttonSize,
                        windowHeight * 58 / 100, buttonSize, buttonSize};
    SDL_Rect bButton = {windowWidth - sideWidth + sideWidth / 4,
                        windowHeight * 76 / 100, buttonSize, buttonSize};
    SDL_Rect selectButton = {sideWidth / 4, windowHeight / 4,
                             sideWidth / 2, windowHeight / 10};
    SDL_Rect startButton = {windowWidth - sideWidth * 3 / 4, windowHeight / 4,
                            sideWidth / 2, windowHeight / 10};
    SDL_Rect lButton = {sideWidth / 4, windowHeight / 20,
                        sideWidth / 2, windowHeight / 10};
    SDL_Rect rButton = {windowWidth - sideWidth * 3 / 4, windowHeight / 20,
                        sideWidth / 2, windowHeight / 10};

    touchKeys = 0;

    for (int i = 0; i < MAX_TOUCH_FINGERS; i++)
    {
        if (!touchFingers[i].active)
            continue;

        int x = touchFingers[i].x * windowWidth;
        int y = touchFingers[i].y * windowHeight;

        if (IsInsideRect(x, y, dpadUp)) touchKeys |= DPAD_UP;
        if (IsInsideRect(x, y, dpadDown)) touchKeys |= DPAD_DOWN;
        if (IsInsideRect(x, y, dpadLeft)) touchKeys |= DPAD_LEFT;
        if (IsInsideRect(x, y, dpadRight)) touchKeys |= DPAD_RIGHT;

        if (IsInsideRect(x, y, aButton)) touchKeys |= A_BUTTON;
        if (IsInsideRect(x, y, bButton)) touchKeys |= B_BUTTON;
        if (IsInsideRect(x, y, startButton)) touchKeys |= START_BUTTON;
        if (IsInsideRect(x, y, selectButton)) touchKeys |= SELECT_BUTTON;
        if (IsInsideRect(x, y, lButton)) touchKeys |= L_BUTTON;
        if (IsInsideRect(x, y, rButton)) touchKeys |= R_BUTTON;
    }
}

static void HandleTouchEvent(const SDL_TouchFingerEvent *event)
{
    int slot = -1;
    for (int i = 0; i < MAX_TOUCH_FINGERS; i++)
    {
        if (touchFingers[i].active && touchFingers[i].id == event->fingerId)
        {
            slot = i;
            break;
        }
        if (slot < 0 && !touchFingers[i].active)
            slot = i;
    }

    if (slot < 0)
        return;

    if (event->type == SDL_FINGERUP)
    {
        touchFingers[slot].active = false;
    }
    else
    {
        touchFingers[slot].id = event->fingerId;
        touchFingers[slot].x = event->x;
        touchFingers[slot].y = event->y;
        touchFingers[slot].active = true;
    }

    UpdateTouchKeys();
}

static const Uint8 *GetGlyph(char character)
{
    static const Uint8 glyphA[7] = {14, 17, 17, 31, 17, 17, 17};
    static const Uint8 glyphB[7] = {30, 17, 17, 30, 17, 17, 30};
    static const Uint8 glyphC[7] = {15, 16, 16, 16, 16, 16, 15};
    static const Uint8 glyphE[7] = {31, 16, 16, 30, 16, 16, 31};
    static const Uint8 glyphL[7] = {16, 16, 16, 16, 16, 16, 31};
    static const Uint8 glyphR[7] = {30, 17, 17, 30, 20, 18, 17};
    static const Uint8 glyphS[7] = {15, 16, 16, 14, 1, 1, 30};
    static const Uint8 glyphT[7] = {31, 4, 4, 4, 4, 4, 4};

    switch (character)
    {
    case 'A': return glyphA;
    case 'B': return glyphB;
    case 'C': return glyphC;
    case 'E': return glyphE;
    case 'L': return glyphL;
    case 'R': return glyphR;
    case 'S': return glyphS;
    case 'T': return glyphT;
    default:  return NULL;
    }
}

static void DrawControlLabel(SDL_Rect rect, const char *label)
{
    int length = SDL_strlen(label);
    int scale = MinInt(rect.h / 9, rect.w / (length * 6));
    if (scale < 1)
        scale = 1;
    int startX = rect.x + (rect.w - (length * 6 - 1) * scale) / 2;
    int startY = rect.y + (rect.h - 7 * scale) / 2;

    SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, 230);
    for (int character = 0; character < length; character++)
    {
        const Uint8 *glyph = GetGlyph(label[character]);
        if (glyph == NULL)
            continue;
        for (int row = 0; row < 7; row++)
        {
            for (int column = 0; column < 5; column++)
            {
                if (glyph[row] & (1 << (4 - column)))
                {
                    SDL_Rect pixel = {startX + (character * 6 + column) * scale,
                                      startY + row * scale, scale, scale};
                    SDL_RenderFillRect(sdlRenderer, &pixel);
                }
            }
        }
    }
}

static void DrawControlRect(SDL_Rect rect, bool pressed, const char *label)
{
    SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, pressed ? 150 : 65);
    SDL_RenderFillRect(sdlRenderer, &rect);
    SDL_SetRenderDrawColor(sdlRenderer, 255, 255, 255, pressed ? 230 : 130);
    SDL_RenderDrawRect(sdlRenderer, &rect);
    if (label != NULL)
        DrawControlLabel(rect, label);
}

static void DrawTouchControls(void)
{
    int windowWidth;
    int windowHeight;
    SDL_GetWindowSize(sdlWindow, &windowWidth, &windowHeight);
    int sideWidth = GetControlSideWidth(windowWidth, windowHeight);
    int buttonSize = MinInt(sideWidth * 2 / 5, windowHeight / 6);
    int dpadUnit = MinInt(sideWidth / 3, windowHeight / 8);
    int dpadX = sideWidth * 2 / 3;
    int dpadY = windowHeight * 7 / 10;

    SDL_RenderSetLogicalSize(sdlRenderer, 0, 0);
    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);

    DrawControlRect((SDL_Rect){dpadX - dpadUnit / 2, dpadY - dpadUnit * 3 / 2,
                               dpadUnit, dpadUnit}, touchKeys & DPAD_UP, NULL);
    DrawControlRect((SDL_Rect){dpadX - dpadUnit / 2, dpadY + dpadUnit / 2,
                               dpadUnit, dpadUnit}, touchKeys & DPAD_DOWN, NULL);
    DrawControlRect((SDL_Rect){dpadX - dpadUnit * 3 / 2, dpadY - dpadUnit / 2,
                               dpadUnit, dpadUnit}, touchKeys & DPAD_LEFT, NULL);
    DrawControlRect((SDL_Rect){dpadX + dpadUnit / 2, dpadY - dpadUnit / 2,
                               dpadUnit, dpadUnit}, touchKeys & DPAD_RIGHT, NULL);
    DrawControlRect((SDL_Rect){windowWidth - sideWidth / 4 - buttonSize,
                               windowHeight * 58 / 100, buttonSize, buttonSize}, touchKeys & A_BUTTON, "A");
    DrawControlRect((SDL_Rect){windowWidth - sideWidth + sideWidth / 4,
                               windowHeight * 76 / 100, buttonSize, buttonSize}, touchKeys & B_BUTTON, "B");
    DrawControlRect((SDL_Rect){windowWidth - sideWidth * 3 / 4, windowHeight / 4,
                               sideWidth / 2, windowHeight / 10}, touchKeys & START_BUTTON, "START");
    DrawControlRect((SDL_Rect){sideWidth / 4, windowHeight / 4,
                               sideWidth / 2, windowHeight / 10}, touchKeys & SELECT_BUTTON, "SELECT");
    DrawControlRect((SDL_Rect){sideWidth / 4, windowHeight / 20,
                               sideWidth / 2, windowHeight / 10}, touchKeys & L_BUTTON, "L");
    DrawControlRect((SDL_Rect){windowWidth - sideWidth * 3 / 4, windowHeight / 20,
                               sideWidth / 2, windowHeight / 10}, touchKeys & R_BUTTON, "R");

    SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
    SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_NONE);
    SDL_RenderSetLogicalSize(sdlRenderer, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    SDL_RenderSetIntegerScale(sdlRenderer, SDL_TRUE);
}

static u16 ControllerButtonMask(Uint8 button)
{
    switch (button)
    {
    case SDL_CONTROLLER_BUTTON_A:             return A_BUTTON;
    case SDL_CONTROLLER_BUTTON_B:             return B_BUTTON;
    case SDL_CONTROLLER_BUTTON_BACK:          return SELECT_BUTTON;
    case SDL_CONTROLLER_BUTTON_START:         return START_BUTTON;
    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:  return L_BUTTON;
    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return R_BUTTON;
    case SDL_CONTROLLER_BUTTON_DPAD_UP:       return DPAD_UP;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:     return DPAD_DOWN;
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:     return DPAD_LEFT;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:    return DPAD_RIGHT;
    default:                                  return 0;
    }
}
#endif

void ProcessEvents(void)
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            DBGPRINTF("PC shutdown: window close requested\n");
            isRunning = false;
            break;
#ifdef _WIN32
        case SDL_SYSWMEVENT:
            if (event.syswm.msg != NULL
             && event.syswm.msg->subsystem == SDL_SYSWM_WINDOWS
             && event.syswm.msg->msg.win.msg == WM_COMMAND)
                HandleNativeMenuCommand(LOWORD(event.syswm.msg->msg.win.wParam));
            break;
#endif
#ifdef __ANDROID__
        case SDL_CONTROLLERDEVICEADDED:
            if (androidController == NULL && SDL_IsGameController(event.cdevice.which))
                androidController = SDL_GameControllerOpen(event.cdevice.which);
            break;
        case SDL_CONTROLLERDEVICEREMOVED:
            if (androidController != NULL
             && SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(androidController)) == event.cdevice.which)
            {
                SDL_GameControllerClose(androidController);
                androidController = NULL;
                controllerKeys = 0;
                controllerAxisKeys = 0;
                controllerAxisX = 0;
                controllerAxisY = 0;
            }
            break;
        case SDL_CONTROLLERBUTTONDOWN:
            controllerKeys |= ControllerButtonMask(event.cbutton.button);
            break;
        case SDL_CONTROLLERBUTTONUP:
            controllerKeys &= ~ControllerButtonMask(event.cbutton.button);
            break;
        case SDL_CONTROLLERAXISMOTION:
            if (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX)
                controllerAxisX = event.caxis.value;
            else if (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY)
                controllerAxisY = event.caxis.value;

            controllerAxisKeys = 0;
            if (controllerAxisX < -16000) controllerAxisKeys |= DPAD_LEFT;
            if (controllerAxisX >  16000) controllerAxisKeys |= DPAD_RIGHT;
            if (controllerAxisY < -16000) controllerAxisKeys |= DPAD_UP;
            if (controllerAxisY >  16000) controllerAxisKeys |= DPAD_DOWN;
            break;
#endif
        case SDL_KEYUP:
        {
            SDL_Keycode key = event.key.keysym.sym;
            keyboardKeys &= ~KeyboardButtonMask(key);
            if (key == sKeyboardMappings[PC_KEY_SPEED] && speedUp)
            {
                speedUp = false;
                timeScale = 1.0;
                SDL_ClearQueuedAudio(sdlAudioDevice);
                SDL_PauseAudioDevice(sdlAudioDevice, 0);
            }
            break;
        }
        case SDL_KEYDOWN:
        {
            SDL_Keycode key = event.key.keysym.sym;
            u16 keyMask = KeyboardButtonMask(key);
            keyboardKeys |= keyMask;
            keyboardPressedKeys |= keyMask;
            if (key == SDLK_RETURN
             && (event.key.keysym.mod & (KMOD_LALT | KMOD_RALT))
             && event.key.repeat == 0)
            {
                sPlatformSettings[PLATFORM_SETTING_FULLSCREEN] =
                    !sPlatformSettings[PLATFORM_SETTING_FULLSCREEN];
                ApplyPlatformSettings();
                StoreConfigFile();
                DBGPRINTF("PC video: fullscreen toggled by Alt+Enter (%u)\n",
                          sPlatformSettings[PLATFORM_SETTING_FULLSCREEN]);
            }
            else if (key == SDLK_r && (event.key.keysym.mod & (KMOD_LCTRL | KMOD_RCTRL)))
            {
#ifdef _WIN32
                DBGPRINTF("PC shutdown: restart requested by Ctrl+R\n");
                sRestartRequested = true;
                isRunning = false;
#else
                DoSoftReset();
#endif
            }
            else if (key == SDLK_p && (event.key.keysym.mod & (KMOD_LCTRL | KMOD_RCTRL)))
            {
                paused = !paused;
#ifdef _WIN32
                UpdateNativePauseMenu();
#endif
            }
            if (key == sKeyboardMappings[PC_KEY_SPEED])
            {
                if (!speedUp)
                {
                    speedUp = true;
                    timeScale = sSpeedMultiplier;
                    SDL_PauseAudioDevice(sdlAudioDevice, 1);
                }
            }
            break;
        }
        }
    }
}

#ifdef _WIN32
#define STICK_THRESHOLD 0.5f
static bool IsControllerInputActive(const XINPUT_STATE *state, enum PcControllerInput input)
{
    static const WORD buttonMasks[PC_CONTROLLER_LT] =
    {
        XINPUT_GAMEPAD_A,
        XINPUT_GAMEPAD_B,
        XINPUT_GAMEPAD_X,
        XINPUT_GAMEPAD_Y,
        XINPUT_GAMEPAD_START,
        XINPUT_GAMEPAD_BACK,
        XINPUT_GAMEPAD_LEFT_SHOULDER,
        XINPUT_GAMEPAD_RIGHT_SHOULDER,
        XINPUT_GAMEPAD_LEFT_THUMB,
        XINPUT_GAMEPAD_RIGHT_THUMB,
    };

    if (input < PC_CONTROLLER_LT)
        return (state->Gamepad.wButtons & buttonMasks[input]) != 0;
    if (input == PC_CONTROLLER_LT)
        return state->Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
    return state->Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
}

u16 GetXInputKeys()
{
    XINPUT_STATE state;
    ZeroMemory(&state, sizeof(XINPUT_STATE));

    DWORD dwResult = XInputGetState(0, &state);
    u16 xinputKeys = 0;

    if (dwResult == ERROR_SUCCESS)
    {
        for (int action = 0; action < PC_CONTROLLER_ACTION_SPEED; action++)
        {
            if (IsControllerInputActive(&state, sControllerMappings[action]))
                xinputKeys |= sControllerButtonMasks[action];
        }
        /* Up */     xinputKeys |= (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) << 6;
        /* Down */   xinputKeys |= (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) << 6;
        /* Left */   xinputKeys |= (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) << 3;
        /* Right */  xinputKeys |= (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) << 1;


        /* Control Stick */
        float xAxis = (float)state.Gamepad.sThumbLX / (float)SHRT_MAX;
        float yAxis = (float)state.Gamepad.sThumbLY / (float)SHRT_MAX;

        if (xAxis < -STICK_THRESHOLD) xinputKeys |= DPAD_LEFT;
        if (xAxis >  STICK_THRESHOLD) xinputKeys |= DPAD_RIGHT;
        if (yAxis < -STICK_THRESHOLD) xinputKeys |= DPAD_DOWN;
        if (yAxis >  STICK_THRESHOLD) xinputKeys |= DPAD_UP;

    }

    /* Speedup */
    // Automated smoke tests must not depend on a physical XInput device.
    double oldTimeScale = timeScale;
    bool autoplay = Platform_GetEnvironmentFlag("POKEMON_GO_WORLD_AUTOPLAY");
    bool controllerSpeedUp = dwResult == ERROR_SUCCESS
                          && IsControllerInputActive(&state, sControllerMappings[PC_CONTROLLER_ACTION_SPEED]);
    timeScale = autoplay ? 5.0 : (controllerSpeedUp || speedUp) ? sSpeedMultiplier : 1.0;

    if (oldTimeScale != timeScale)
    {
        if (timeScale > 1.0)
        {
            SDL_PauseAudioDevice(sdlAudioDevice, 1);
        }
        else
        {
            SDL_ClearQueuedAudio(sdlAudioDevice);
            SDL_PauseAudioDevice(sdlAudioDevice, 0);
        }
    }

    return xinputKeys;
}
#endif // _WIN32

u16 Platform_GetKeyInput(void)
{
    static u32 autoplayFrame;
    u16 automatedKeys = 0;
    u16 pressedKeys = keyboardPressedKeys;
    const char *autoplay = SDL_getenv("POKEMON_GO_WORLD_AUTOPLAY");

    keyboardPressedKeys = 0;

    if (autoplay != NULL && autoplay[0] != '\0' && autoplay[0] != '0')
    {
        // Generic input smoke test only. Narrative routes and object positions
        // must be validated from an authentic save, not synthesized here.
        autoplayFrame++;
        if (autoplayFrame < 6100)
        {
            if (autoplayFrame % 12 == 1)
                automatedKeys = A_BUTTON;
            else if (autoplayFrame % 60 == 7)
                automatedKeys = START_BUTTON;
        }
        else
        {
            u32 movementPhase = (autoplayFrame - 6100) % 240;
            if (movementPhase < 45)
                automatedKeys = DPAD_DOWN;
            else if (movementPhase >= 60 && movementPhase < 105)
                automatedKeys = DPAD_LEFT;
            else if (movementPhase >= 120 && movementPhase < 165)
                automatedKeys = DPAD_UP;
            else if (movementPhase >= 180 && movementPhase < 225)
                automatedKeys = DPAD_RIGHT;
        }
    }

#ifdef _WIN32
    u16 gamepadKeys = GetXInputKeys();
    return gamepadKeys | keyboardKeys | pressedKeys | automatedKeys;
#elif defined(__ANDROID__)
    return keyboardKeys | pressedKeys | controllerKeys | controllerAxisKeys | automatedKeys;
#endif

    return keyboardKeys | pressedKeys | automatedKeys;
}

void VDraw(SDL_Texture *texture)
{
    static uint16_t gbaImage[DISPLAY_WIDTH * DISPLAY_HEIGHT];
    static uint32_t image[DISPLAY_WIDTH * DISPLAY_HEIGHT];
    static unsigned int frameNumber;

    memset(gbaImage, 0, sizeof(gbaImage));
    DrawFrame(gbaImage);
    for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++)
    {
        uint16_t color = gbaImage[i];
        uint32_t r = (color & 0x1F) * 255 / 31;
        uint32_t g = ((color >> 5) & 0x1F) * 255 / 31;
        uint32_t b = ((color >> 10) & 0x1F) * 255 / 31;
        image[i] = 0xFF000000 | (r << 16) | (g << 8) | b;
    }
    SDL_UpdateTexture(texture, NULL, image, DISPLAY_WIDTH * sizeof(Uint32));

    // Optional native-port regression capture. When the environment variable
    // points to an existing directory, save the unscaled GBA framebuffer once
    // per second. Normal players do not pay any file-I/O cost.
    const char *captureDir = SDL_getenv("POKEMON_GO_WORLD_CAPTURE_DIR");
    frameNumber++;
    if (captureDir != NULL && captureDir[0] != '\0' && frameNumber % 60 == 0)
    {
        char capturePath[1200];
        SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(
            image,
            DISPLAY_WIDTH,
            DISPLAY_HEIGHT,
            32,
            DISPLAY_WIDTH * sizeof(Uint32),
            0x00FF0000,
            0x0000FF00,
            0x000000FF,
            0xFF000000);

        if (surface != NULL)
        {
            SDL_snprintf(capturePath, sizeof(capturePath), "%s/frame_%06u.bmp", captureDir, frameNumber);
            SDL_SaveBMP(surface, capturePath);
            SDL_FreeSurface(surface);
        }
    }
    REG_VCOUNT = 161; // prep for being in VBlank period
}

int DoMain(void *data)
{
    DBGPRINTF("PC port: entering AgbMain\n");
    AgbMain();
    DBGPRINTF("PC port: AgbMain returned\n");
    return 0;
}

void VBlankIntrWait(void)
{
    SDL_AtomicSet(&isFrameAvailable, 1);
    SDL_SemWait(vBlankSemaphore);
}

static u8 BinToBcd(u8 bin)
{
    int placeCounter = 1;
    u8 out = 0;
    do
    {
        out |= (bin % 10) * placeCounter;
        placeCounter *= 16;
    }
    while ((bin /= 10) > 0);

    return out;
}

static u8 BcdToBin(u8 bcd)
{
    return ((bcd >> 4) * 10) + (bcd & 0xF);
}

void Platform_GetStatus(struct SiiRtcInfo *rtc)
{
    rtc->status = internalClock.status;
}

void Platform_SetStatus(struct SiiRtcInfo *rtc)
{
    internalClock.status = rtc->status;
}

static void UpdateInternalClock(void)
{
    time_t rawTime = time(NULL) + sRtcOffsetSeconds;
    struct tm *time = localtime(&rawTime);

    if (time == NULL)
        return;

    internalClock.year = BinToBcd(time->tm_year - 100);
    internalClock.month = BinToBcd(time->tm_mon + 1);
    internalClock.day = BinToBcd(time->tm_mday);
    internalClock.dayOfWeek = BinToBcd(time->tm_wday);
    internalClock.hour = BinToBcd(time->tm_hour);
    internalClock.minute = BinToBcd(time->tm_min);
    internalClock.second = BinToBcd(time->tm_sec);
}

static void SetInternalClockFromRtc(const struct SiiRtcInfo *rtc, bool includeDate)
{
    time_t adjustedNow = time(NULL) + sRtcOffsetSeconds;
    struct tm *current = localtime(&adjustedNow);
    struct tm desired;
    time_t desiredTime;

    if (current == NULL)
        return;

    desired = *current;
    if (includeDate)
    {
        desired.tm_year = 100 + BcdToBin(rtc->year);
        desired.tm_mon = BcdToBin(rtc->month) - 1;
        desired.tm_mday = BcdToBin(rtc->day);
    }
    desired.tm_hour = BcdToBin(rtc->hour);
    desired.tm_min = BcdToBin(rtc->minute);
    desired.tm_sec = BcdToBin(rtc->second);
    desired.tm_isdst = -1;

    desiredTime = mktime(&desired);
    if (desiredTime == (time_t)-1)
        return;

    sRtcOffsetSeconds = desiredTime - time(NULL);
    UpdateInternalClock();
    StoreConfigFile();
}

void Platform_GetDateTime(struct SiiRtcInfo *rtc)
{
    UpdateInternalClock();

    rtc->year = internalClock.year;
    rtc->month = internalClock.month;
    rtc->day = internalClock.day;
    rtc->dayOfWeek = internalClock.dayOfWeek;
    rtc->hour = internalClock.hour;
    rtc->minute = internalClock.minute;
    rtc->second = internalClock.second;
    DBGPRINTF("GetDateTime: %d-%02d-%02d %02d:%02d:%02d\n", BcdToBin(rtc->year),
                                                         BcdToBin(rtc->month),
                                                         BcdToBin(rtc->day),
                                                         BcdToBin(rtc->hour),
                                                         BcdToBin(rtc->minute),
                                                         BcdToBin(rtc->second));
}

void Platform_SetDateTime(struct SiiRtcInfo *rtc)
{
    SetInternalClockFromRtc(rtc, true);
}

void Platform_GetTime(struct SiiRtcInfo *rtc)
{
    UpdateInternalClock();

    rtc->hour = internalClock.hour;
    rtc->minute = internalClock.minute;
    rtc->second = internalClock.second;
    DBGPRINTF("GetTime: %02d:%02d:%02d\n", BcdToBin(rtc->hour),
                                        BcdToBin(rtc->minute),
                                        BcdToBin(rtc->second));
}

void Platform_SetTime(struct SiiRtcInfo *rtc)
{
    SetInternalClockFromRtc(rtc, false);
}

void Platform_SetAlarm(u8 *alarmData)
{
    // TODO
}

void SoftReset(u32 resetFlags)
{
    puts("Soft Reset called. Exiting.");
    exit(0);
}

#endif
