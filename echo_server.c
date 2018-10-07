/* A simple echo server using TCP */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>


#define SERVER_TCP_PORT 3000   /* well-known port */
#define BUFLEN    256    /* buffer length */

int sendfile(int);

void reaper(int);

int main(int argc, char **argv) {
    int sd, new_sd, client_len, port;
    struct sockaddr_in server, client;

    switch (argc) {
        case 1:
            port = SERVER_TCP_PORT;
            break;
        case 2:
            port = atoi(argv[1]);
            break;
        default:
            fprintf(stderr, "Usage: %d [port]\n", argv[0]);
            exit(1);
    }

    /* Create a stream socket  */
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Can't create a socket\n");
        exit(1);
    }

    /* Bind an address to the socket   */
    bzero((char *) &server, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sd, (struct sockaddr *) &server, sizeof(server)) == -1) {
        fprintf(stderr, "Can't bind name to socket\n");
        exit(1);
    }

    /* queue up to 5 connect requests  */
    listen(sd, 5);

    (void) signal(SIGCHLD, reaper);

    while (1) {
        client_len = sizeof(client);
        new_sd = accept(sd, (struct sockaddr *) &client, &client_len);
        if (new_sd < 0) {
            fprintf(stderr, "Can't accept client \n");
            exit(1);
        }
        switch (fork()) {
            case 0:       /* child */
                (void) close(sd);
                exit(sendfile(new_sd));
            default:      /* parent */
                (void) close(new_sd);
                break;
            case -1:
                fprintf(stderr, "fork: error\n");
        }
    }
}

/* echod program  */
int sendfile(int sd) {
    unsigned char name[BUFLEN] = {0};
    int n = read(sd, name, BUFLEN);
	



    FILE *sender = fopen(name, "r");
    printf("Filename: |%s|", name);
    if (sender == NULL) {
        fprintf(stderr, "Can't open file \n");
        exit(1);
    }
    for (;;) {
        unsigned char buf[BUFLEN] = {0};
        int bytes_to_read = fread(buf, 1, BUFLEN, sender);

        /* sending data      */
        if (bytes_to_read > 0) {
            write(sd, buf, bytes_to_read);
        }

        if (bytes_to_read < BUFLEN) {
            if (feof(sender))
                fprintf(stderr, "End of file \n");
            if (ferror(sender))
                fprintf(stderr, "Error Reading \n");
            break;
        }
    }
    fclose(sender);
    close(sd);
    return (0);
}

/* reaper    */
void reaper(int sig) {
    int status;
    while (wait3(&status, WNOHANG, (struct rusage *) 0) >= 0);
}
