#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

// Программе передаётся два аргумента: CMD1 и CMD2.
// Необходимо запустить два процесса, выполняющих эти команды,
// и перенаправить стандартный поток вывода CMD1 на стандартный поток ввода CMD2.
// Эквивалентно: CMD1 | CMD2
// Родительский процесс должен завершаться самым последним.

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <CMD1> <CMD2>\n", argv[0]);
        return 1;
    }
    int fd[2];
    if (pipe(fd) < 0) {
        return 1;
    }
    pid_t p1 = fork();
    if (p1 < 0) {
        return 1;
    }
    if (p1 == 0) {
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
        execlp(argv[1], argv[1], NULL);
        _exit(1);
    }
    pid_t p2 = fork();
    if (p2 < 0) {
        return 1;
    }
    if (p2 == 0) {
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);
        close(fd[1]);
        execlp(argv[2], argv[2], NULL);
        _exit(1);
    }
    close(fd[0]);
    close(fd[1]);
    waitpid(p1, NULL, 0);
    waitpid(p2, NULL, 0);
    return 0;
}
