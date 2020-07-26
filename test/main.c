#include <stdlib.h>
#include <stdio.h>       
#include <unistd.h>
#include <getopt.h>
#include <stdarg.h>
#include <string.h>
#include "test_move.h"
#include "test_tap.h"
int main( int argc, char *argv[] ){
    int opt;
    while ((opt = getopt(argc, argv, "h")) != -1) {
        switch (opt) {
        case 'h':
            fprintf(stdout, "Usage: %s [-h] name\n",
                    argv[0]);
            exit(0);
        default: /* '?' */
            fprintf(stderr, "Usage: %s [-h] name\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    if (optind >= argc) {
        fprintf(stderr, "Expected argument after options\n");
        exit(EXIT_FAILURE);
    }
    const char *test_name = argv[optind];
    if( strlen(test_name) == 0 ){
        fprintf(stderr, "not test name specified\n");
        exit(EXIT_FAILURE);
    }else{
        fprintf(stdout,"testname = %s\n",test_name);
    }




    if( strcmp(test_name,"move") == 0){
        test_move();
    }else if( strcmp(test_name,"tap") == 0 ){
        test_tap();
    }else{
        fprintf(stderr, "unknown test name.\n");
        exit(EXIT_FAILURE);
    }


    
    return 0;
}