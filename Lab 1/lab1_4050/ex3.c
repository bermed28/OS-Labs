#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#define MAX_CHARS 10000

int main() {

    char cmd[MAX_CHARS];
    int t;
    scanf("%d\n", &t);
    while(t--) {
        fgets(cmd, MAX_CHARS, stdin);	    //cmd has current string
        int idx = 0;
        int size = strlen((const char*) &cmd) - 1; //0;

        if(sqrt(size) * sqrt(size) == size){
            int dim = (int) sqrt(size);
            char matrix[dim][dim];
            for(int col = 0; col < dim; col++){
                for(int row = 0; row < dim; row++){
                    if(idx < size){
                        matrix[row][col] = cmd[idx];
                        idx++;
                    }
                }
            }

            for(int row = 0; row < dim; row++){
                for(int col = 0; col < dim; col++){
                    printf("%c",  matrix[row][col]);
                }
            }

        }else {
            printf("INVALID");
        }

        printf("\n");
        // Add you code here

    }
    return 0;
}
