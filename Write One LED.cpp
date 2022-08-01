#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>

using namespace std;



const unsigned int LW_BRIDGE_BASE = 0xFF200000;
const unsigned int FINAL_PHYSICAL_ADDRESS = 0xFFFEC700;
const unsigned int LW_BRIDGE_SPAN = FINAL_PHYSICAL_ADDRESS - LW_BRIDGE_BASE;
const unsigned int LEDR_OFFSET = 0x0;
const unsigned int SW_OFFSET = 0x40;

char *Initialize(int *fd) {
    *fd = open( "/dev/mem", (O_RDWR | O_SYNC));
    if (*fd == -1){
        cout << "ERROR: could not open /dev/mem..." << endl;
        exit(1);
    }
    char *virtual_base = (char *)mmap (NULL, LW_BRIDGE_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, *fd, LW_BRIDGE_BASE);
    if (virtual_base == MAP_FAILED){
        cout << "ERROR: mmap() failed..." << endl;
        close (*fd);
        exit(1);
    }
    return virtual_base;
}

void Finalize(char *pBase, int fd){
    if (munmap (pBase, LW_BRIDGE_SPAN) != 0){
        cout <<"ERROR:  munmap() failed.." <<endl;
        exit(1);
    }
    close (fd);
}

void RegisterWrite(char *pBase, unsigned int reg_offset, int value) {
    * (volatile unsigned int *)(pBase + reg_offset) = value;
}

int RegisterRead(char *pBase, unsigned int reg_offset) {
    return * (volatile unsigned int *)(pBase + reg_offset);
}

void WriteAllLeds(char *pBase, int value){
    RegisterWrite(pBase, LEDR_OFFSET, value);
}

void Write1Led(char *pBase, int ledNum, int state){
    int num = RegisterRead(pBase, LEDR_OFFSET);
    if (state == 0){
        num = num-(0x1<<ledNum);
        RegisterWrite(pBase, LEDR_OFFSET, num);
    } else{
        num = num|state<<ledNum;
        RegisterWrite(pBase, LEDR_OFFSET, num);
    }
}

int ReadAllSwitches(char *pBase){
    int switchValue = RegisterRead(pBase, SW_OFFSET);
    return switchValue;
}



int main (){
    int fd;
    char *pBase = Initialize(&fd);
    int a;
    int b;
    int c;

    cout<<"Please enter the LED that you want to manipulate"<<endl;
    cin>>a;
    cout<<"Please enter a 1 if you would like to turn it on or a 0 if you would like to turn it off:"<<endl;
    cin>>b;
    Write1Led(pBase, a, b);

    c = ReadAllSwitches(pBase);
    cout<<"the value of the current switch combination is "<<c<<endl;
}
