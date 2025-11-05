#include <stdio.h>
#include <sysctl.h>
#include <plic.h>
#include <unistd.h>
#include "bsp.h"

int main(void)
{
    plic_init();

    while(1){
        printf("Interface Magazine\n");  //unistd.h is needed
        sleep(1);  //bsp.h is needed
        }
}
