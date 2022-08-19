#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#define MAX 100000
unsigned long long combine1(int a[MAX], int b[MAX], unsigned long long sum){

    for (int i = 0; i < MAX; i++) {
        sum = a[i] + b[i];
    }
    return sum;
}

int main(void){
    int a[MAX];
    int b[MAX];
    unsigned long long sum;
    time_t t;


    srand((unsigned)time(&t));
    for (int i = 0; i < MAX; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }

    sleep(60);
    combine1(a, b, sum);
}
