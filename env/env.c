#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

extern char **environ;

// print environment function
void print_env()
{
    char **s = environ;
    for (; *s; s++)
    {
        if (printf("%s\n", *s) != strlen(*s) + 1)
        {
            perror("printf\n");
            exit(EXIT_FAILURE);
        }
    }
}

// add variable to environment function
void add_variable(char *var)
{
    if (putenv(var) == -1)
    {
        perror("putenv\n");
        exit(EXIT_FAILURE);
    }
}

//print the trace on the stderr and check for errors
void print_v(char* text, char *arg){
    int var = fprintf(stderr, "%s %s\n", text, arg);
    if(var != strlen(text) + strlen(arg) + 2){
        perror("fprintf");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv)
{
    int opt, v_executed = 0, i;
    if (argc == 1)
    {
        print_env();
        return EXIT_SUCCESS;
    }
    else
    {
        while ((opt = getopt(argc, argv, "vu:")) != -1)
        {
            switch (opt)
            {
            case 'v':
                v_executed = 1;
                break;

            case 'u':
                if (v_executed)
                {
                    print_v("env unset:", optarg);
                }
                if (unsetenv(optarg) == -1)
                {
                    perror("unsetenv");
                    return EXIT_FAILURE;
                }
            default:
                break;
            }
        }

        for (i = optind; i < argc; i++)
        {
            opterr = 0;
            if (strchr(argv[i], '=') != NULL)
            {
                if (v_executed)
                {
                    print_v("env set:", argv[i]);
                }
                add_variable(argv[i]);
            }
            else
                break;
        }

        if (i == argc) //check if there are no extra commands passed as input
        {
            print_env();
            return EXIT_SUCCESS;
        }
        else
        {
            // NOTE: If you want to execute a command that takes arguments, after you have written your
            // "-u" or "-v" commands, type "--". This way, you can "force end" the getopt() function so
            // it will not look for other commands after "-u" and "-v", and the program will be able to
            // take the other arguments for other commands into consideration.
            // EXAMPLE: ./env -v -u NAME -- HELLO=THERE GENERAL=KENOBI ls -la
            if (v_executed)
            {
                print_v("env execute command:", argv[i]);
            }
            char **command_arg = argv + i;
            execvp(command_arg[0], command_arg);

            // if the command execvp executes succesfully, the program will continue to execute
            // if it continues, the command execvp must have failed
            perror("execvp");
        }
    }
    return EXIT_FAILURE;
}