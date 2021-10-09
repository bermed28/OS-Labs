#include <stdio.h>
#include <stdlib.h>

//n=amount of numbers in the series to compute, seq=array to store series
void tribonacci(int n, int* seq){
    // Complete this function
    if(n == 0) *(seq + 0) = 0;
    else if(n == 1) *(seq + 1) = 1;
    else if(n == 2) *(seq + 2) = 1;
    else{

        *(seq + n) = *(seq + (n - 3)) + *(seq + (n - 2)) + *(seq + (n - 1));
    }

}

int main(){

    int n;
    //n, amount of series to compute
    scanf("%d",&n);

    //initialize array to 1, using malloc/calloc
    int *seq = malloc(sizeof(int) * 1); /* complete code */

    int i;
    for(i = 0; i < n; i++){

        //recompute the whole series
        tribonacci(i, seq);

        //print array
        int j;
        for(j = 0; j < i + 1; j++){
            printf("%d ", *(seq + j));
        }


        //resize array, with realloc
        int newSize=i+1;/* complete code */
        seq = realloc(seq, sizeof(int) * newSize);

        printf("\n");
    }
    //free array
    free(seq);
    return 0;
}
