EXTERN char *gszArgv[];
EXTERN char gszCommandLine[];

EXTERN int __fastcall ConvertCmdLineToArgcArgv(char *lpCmdLine);

#define MAX_CMDLINE_ARGS   64
// Max NT/2K limit is 2048, but I'm going lower to 64*12=768 - good enough!
#define MAX_CMDLINE_LENGTH 768
