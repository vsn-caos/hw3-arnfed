#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Программе передаются два аргумента:
//   argv[1] — IPv4-адрес сервера в десятичной записи (например, "127.0.0.1")
//   argv[2] — номер порта
//
// Программа должна:
//   1. Установить TCP-соединение с указанным сервером.
//   2. В цикле читать со stdin целые знаковые числа в текстовом формате.
//   3. Отправлять каждое число на сервер в бинарном виде (int32, Little Endian).
//   4. Получать от сервера int32 LE в ответ и выводить его в stdout в текстовом виде.
//   5. Если сервер закрыл соединение — завершиться с кодом возврата 0.

static int send_all(int fd, const void *buf, size_t n) {
    const char *p = buf;
    while (n > 0) {
        ssize_t k = send(fd, p, n, 0);
        if (k <= 0) {
            return -1;
        }
        p += k;
        n -= k;
    }
    return 0;
}
static int recv_all(int fd, void *buf, size_t n) {
    char *p = buf;
    while (n > 0) {
        ssize_t k = recv(fd, p, n, 0);
        if (k == 0) {
            return 0;
        }
        if (k < 0) {
            return -1;
        }
        p += k;
        n -= k;
    }
    return 1;
}
static void to_le32(unsigned char b[4], int32_t x) {
    uint32_t u = (uint32_t)x;
    b[0] = u & 255;
    b[1] = (u >> 8) & 255;
    b[2] = (u >> 16) & 255;
    b[3] = (u >> 24) & 255;
}
static int32_t from_le32(const unsigned char b[4]) {
    uint32_t u = 0;
    u |= (uint32_t)b[0];
    u |= (uint32_t)b[1] << 8;
    u |= (uint32_t)b[2] << 16;
    u |= (uint32_t)b[3] << 24;
    return (int32_t)u;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <ipv4_addr> <port>\n", argv[0]);
        return 1;
    }
    int s = socket(AF_INET, SOCK_STREAM, 0);

    if (s < 0) {
        return 1;
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));

    if (inet_pton(AF_INET, argv[1], &addr.sin_addr) != 1) {
        close(s);
        return 1;
    }

    if (connect(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(s);
        return 1;
    }

    int32_t x;
    unsigned char buf[4];

    while (scanf("%d", &x) == 1) {
        to_le32(buf, x);

        if (send_all(s, buf, 4) < 0) {
            close(s);
            return 1;
        }

        int r = recv_all(s, buf, 4);

        if (r == 0) {
            close(s);
            return 0;
        }

        if (r < 0) {
            close(s);
            return 1;
        }

        printf("%d\n", from_le32(buf));
        fflush(stdout);
    }

    close(s);

    return 0;
}
