#include <stdio.h>

int main(int argc, char *argv[]) {
    printf("\x1b[2J\x1b[0;0Hhello\n");
    
    printf("coucou\n");
    
    printf("\x1b[07m\033[2ACoucoucoucouc\n\n\x1b[00m");
    
    return 0;
}

