#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
int main(){
    int a=0x79;
    if(!(a&(1<<1)))
        printf("%d",(a&(1<<1)));
    return 0;
}
