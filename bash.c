#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define MAXLONG 1000
#define MAXARGS 50

void restaurarIO(int stdinFD, int stdoutFD);
char* generarListaArgs(char* listaArgs[], char* comando);
void ultimoProceso(char* listaArgs[], bool bg);
void procesarComando(char* comando, bool bg);
void ramificar(char* listaArgs[], char* comando, bool bg);

int main()
{
    const char PROMPT[] = "#SHELL>";
    const char TERMINAR[] = "terminar";
    const char EXIT[] = "exit";

    char comando[MAXLONG];
    bool continuar, bg;
    int i;
    const int stdinBackup = dup(STDIN_FILENO), stdoutBackup = dup(STDOUT_FILENO);

    do {
        printf(PROMPT);
        scanf(" %[^\n]", comando);

        bg = false;
        for (i = 0; comando[i] != '\0'; i++)      ///VER!!!!!
            if (comando[i] == '&') {
                comando[i] = '\0';
                bg = true;
            }

        if ( continuar = strcmp(comando, TERMINAR) && strcmp(comando, EXIT) ) {
            procesarComando(comando, bg);
            restaurarIO(stdinBackup, stdoutBackup);
        }

    } while (continuar);

    return 0;
}

void restaurarIO(int stdinFD, int stdoutFD)
{
    close(STDIN_FILENO);
    dup(stdinFD);
    close(STDOUT_FILENO);
    dup(stdoutFD);
}

char* generarListaArgs(char* listaArgs[], char* comando)
{
    char argumento[MAXLONG];
    int numArg = 0;

    while (*comando != '|' && *comando != '\0') {
        int i = 0;
        while (*comando != ' ' && *comando != '|' && *comando != '\0')
            argumento[i++] = *(comando++);
        if (i != 0) {
            argumento[i] = '\0';
            listaArgs[numArg] = malloc(i * sizeof(char));
            strcpy(listaArgs[numArg++], argumento);
        }
        if (*comando == ' ')
            comando++;
    }
    listaArgs[numArg] = NULL;

    return comando;
}

void ultimoProceso(char* listaArgs[MAXARGS], bool bg)
{
	int estadoHijo;

    if (fork() != 0){
        if (!bg)
            wait(&estadoHijo);
    }
    else {
        if (bg) {
            close(STDIN_FILENO);
            open("/dev/null", O_RDONLY);
            close(STDOUT_FILENO);
            open("/dev/null", O_CREAT | O_WRONLY, 0777);
        }
		execvp(listaArgs[0], listaArgs);
		perror(listaArgs[0]);
        exit(0);
    }
}

void procesarComando(char* comando, bool bg)
{
    int i;
    char* listaArgs[MAXARGS];

    comando = generarListaArgs(listaArgs, comando);

    if (*comando == '|')
        ramificar(listaArgs, ++comando, bg);
    else
        ultimoProceso(listaArgs, bg);
}

void ramificar(char* listaArgs[MAXARGS], char* comando, bool bg)
{
    const int READ = 0;
    const int WRITE = 1;

    int fdpipe[2], estadoHijo;

	if (fork() != 0){
        if (!bg)
            wait(&estadoHijo);
    }
    else {
        pipe(fdpipe);
        if (fork() != 0) {
            close(fdpipe[READ]);
            close(STDOUT_FILENO);
            dup(fdpipe[WRITE]);
            close(fdpipe[WRITE]);
            execvp(listaArgs[0], listaArgs);
            perror(listaArgs[0]);
            exit(0);
        }
        else {
            close(fdpipe[WRITE]);
            close(STDIN_FILENO);
            dup(fdpipe[READ]);
            close(fdpipe[READ]);
            procesarComando(comando, bg);
        }
    }
}
