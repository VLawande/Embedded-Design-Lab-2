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
const unsigned int KEY_OFFSET = 0x50;

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

int Read1Switch(char *pBase, int switchNum){
	int value = RegisterRead(pBase, SW_OFFSET);
	value =(value>>switchNum)&(0x1);
	return value; 
}

int PushButtonGet(char *pBase){
cout<<"Please press one of the buttons!"<<endl;
cout<<"Button 0 will increment the value of the LEDs by 1"<<endl;
cout<<"Button 1 will decrement the value of the LEDs by 1"<<endl;
cout<<"Button 2 will shift the value of the LEDs right by one bit"<<endl;
cout<<"Button 3 will shift the value of the LEDs left by one bit"<<endl;
int value = RegisterRead(pBase, KEY_OFFSET);
int a = 1;
int b;
int k;
while (a==1){
	b = RegisterRead(pBase, KEY_OFFSET);
	if (b != value){
	a = 0;
	}
}
if (b == 1){
    k = 0;
}
if (b == 2){
    k = 1;
}
if (b == 4){
    k = 2;
}
if (b == 8){
    k = 3;
}
return k;
}


int main (){
	int fd;
	char *pBase = Initialize(&fd);
	int a;
	int b;
	int c;

	int response = PushButtonGet(pBase);
	switch (response){
		case 0:
			a = RegisterRead(pBase, LEDR_OFFSET);
			a = a+1;
			RegisterWrite(pBase, LEDR_OFFSET, a);
			cout<<"The value of the LEDs has been incremented by 1"<<endl;	
		break;
		case 1:
			a = RegisterRead(pBase, LEDR_OFFSET);
			a = a-1;
			RegisterWrite(pBase, LEDR_OFFSET, a);
			cout<<"The value of the LEDs has decremented by 1"<<endl;
		break;
		case 2:
			a = RegisterRead(pBase, LEDR_OFFSET);
			a = a>>1;
			RegisterWrite(pBase, LEDR_OFFSET,a);
			cout<<"The value of the LEDs has been shifted 1 to the right"<<endl;
		break;
		case 3:
			a = RegisterRead(pBase, LEDR_OFFSET);
			a = a<<1;
			RegisterWrite(pBase, LEDR_OFFSET,a);
			cout<<"The value of the LEDs has been shifted 1 to the left"<<endl;
		break;
}
return 0;
}
