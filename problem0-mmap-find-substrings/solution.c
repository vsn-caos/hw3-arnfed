#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

// Программе передаются два аргумента: имя файла и строка для поиска.
// Необходимо найти все вхождения строки в текстовом файле,
// используя отображение на память с помощью системного вызова mmap.
// На стандартный поток вывода вывести список всех позиций (с нуля),
// где встречается искомая строка, по одной на стоку.

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <filename> <search_string>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    const char *search = argv[2];

    size_t search_len = strlen(search);

    if (search_len == 0) {
        return 0;
    }

    int fd = open(filename, O_RDONLY);

    if (fd < 0) {
        return 1;
    }

    struct stat st;

    if (fstat(fd, &st) < 0) {
        close(fd);
        return 1;
    }

    size_t file_size = st.st_size;

    if (file_size == 0 || search_len > file_size) {
        close(fd);
        return 0;
    }

    char *data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);

    if (data == MAP_FAILED) {
        close(fd);
        return 1;
    }

    for (size_t i = 0; i + search_len <= file_size; i++) {
        if (memcmp(data + i, search, search_len) == 0) {
            printf("%zu\n", i);
        }
    }

    munmap(data, file_size);
    close(fd);

    return 0;
}
