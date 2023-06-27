#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include <mpi.h>
#include <omp.h>

#include "run.h"
#include "initialize.h"

#define INIT 1
#define RUN  2

#define ORDERED 0
#define STATIC  1


int main(int argc, char **argv)
{
    // define defaults

    // what to do, 0=nothing 1=initialize 2=run
    int action = 0; 

    // grid border length
    int k = 100;

    // evolution type, 0=ordered, 1=static
    int e = STATIC;

    // file to read/write
    char fname[100];
    int extra_char_count;

    // number of steps to make
    int n = 100;

    // after how many steps write on file, 0=only at he end
    int s = 0;


    // passing command-line args
    char *optstring = "irk:e:f:n:s:";
    int c;

    while ((c = getopt(argc, argv, optstring)) != -1)
    {
        switch(c)
        {
            case 'i':
            action = INIT; break;
            
            case 'r':
            action = RUN; break;
            
            case 'k':
            k = atoi(optarg); break;

            case 'e':
            e = atoi(optarg); break;

            case 'f':
            //sprintf with dynamic allocation of fname caused segmentation fault hence this static aproach
            extra_char_count = snprintf(fname, 40*sizeof(char), "%s", optarg );
            if (extra_char_count > 100)
                printf("[WARNING] inputed name too long\n");
            break;

            case 'n':
            n = atoi(optarg); break;

            case 's':
            s = atoi(optarg); break;

            default :
            printf("argument -%c not known\n", c ); break;
        }
    }

    // do one of three things

    // initialize grid
    if (action == INIT)
    {
        initialize(fname, k);
        return 0;
    }

    // run game
    if (action == RUN)
    {   
        if (e == STATIC)
            run(fname, n, s);
        else if (e == ORDERED)
            run_ordered(fname, n, s);
        else
            printf("[ERROR] invalid evolution type (use 1 for static, 0 for ordered)\n");
        return 0;
    }

    // nothing
    printf("[ERROR] no action argument [use -i or-r]\n");
    return 1;    
}
