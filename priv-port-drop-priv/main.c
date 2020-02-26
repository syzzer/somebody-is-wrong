#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

int main(int argc, const char *argv[])
{
    if (argc < 2) {
        dprintf(1, "Usage: %s port\n", argv[0]);
        exit(1);
    }
    errno = 0;
    long int port = strtol(argv[1], NULL, 0);
    if (errno || port <= 0 || port > (1 << 16)) {
        perror("Failed to parse port");
        exit(1);
    } else {
        printf("Going to listen on port %ld\n", port);
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Opening socket failed");
        abort();
    }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(port)
    };

    if (bind(sockfd, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        perror("Bind failed");
        abort();
    }

    if (listen(sockfd, 2) < 0) {
        perror("Listen failed");
        abort();
    }

    if (getuid() == 0) {
        printf("We are root, let's drop privs\n");
        if (setgid(65534) != 0) {
            perror("Failed to set group to nobody");
            abort();
        }
        if (setuid(65534) != 0) {
            perror("Failed to set uid to nobody");
            abort();
        }
    }

    printf("Waiting for connection...\n");
    struct sockaddr_in cli_addr = { 0 };
    socklen_t cli_addr_len = sizeof(cli_addr);
    int conn = accept(sockfd, (struct sockaddr *)&cli_addr, &cli_addr_len);
    if (conn < 0) {
        perror("Accept failed");
        abort();
    }

    printf("Got connection, reading data...\n");
    char a = 0;
    while (read(conn, &a, sizeof(a)) == 1) {
        printf("%c", a);
    }

    printf("'nuff said. Kthnxbye.\n");
    return 0;
}
