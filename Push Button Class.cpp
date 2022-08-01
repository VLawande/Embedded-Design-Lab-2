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

class DE1SoCfpga{
	public:
	char *pBase;
	int fd;

	DE1SoCfpga() {
		fd = open( "/dev/mem", (O_RDWR | O_SYNC));
		if (fd == -1){ 
			cout << "ERROR: could not open /dev/mem..." << endl;
			exit(1);
		}
		char *virtual_base = (char *)mmap (NULL, LW_BRIDGE_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, LW_BRIDGE_BASE);
		if (virtual_base == MAP_FAILED){
			cout << "ERROR: mmap() failed..." << endl;
			close (fd);
			exit(1);
		}   
		pBase = virtual_base;
	}

	~DE1SoCfpga(){
		if (munmap (pBase, LW_BRIDGE_SPAN) != 0){
			cout <<"ERROR:  munmap() failed.." <<endl;
			exit(1);
		}
		close (fd);
	}


	void RegisterWrite(unsigned int reg_offset, int value) {
		* (volatile unsigned int *)(pBase + reg_offset) = value;
	} 

	int RegisterRead(unsigned int reg_offset) {
		return * (volatile unsigned int *)(pBase + reg_offset);
	}

	void WriteAllLeds(int value){
		RegisterWrite(LEDR_OFFSET, value);
	}

	void Write1Led(int ledNum, int state) {
		int num = RegisterRead(LEDR_OFFSET);
		if (state == 0){
			if (num == 0){
				cout<<"that LED is already turned off!"<<endl;
				return;
			}
			num = num-(0x1<<ledNum);
			RegisterWrite(LEDR_OFFSET, num);
		} else{
			num = num|state<<ledNum;
			RegisterWrite(LEDR_OFFSET, num);
		}
	}

	int ReadAllSwitches(){
		int switchValue = RegisterRead(SW_OFFSET);
		return switchValue;
	}

	int Read1Switch(int switchNum){
		int value = RegisterRead(SW_OFFSET);
		value =(value>>switchNum)&(0x1);
		return value; 
	}

	int PushButtonGet(){
		cout<<"Please press one of the buttons!"<<endl;
		cout<<"Button 0 will increment the value of the LEDs by 1"<<endl;
		cout<<"Button 1 will decrement the value of the LEDs by 1"<<endl;
		cout<<"Button 2 will shift the value of the LEDs right by one bit"<<endl;
		cout<<"Button 3 will shift the value of the LEDs left by one bit"<<endl;

		int value = RegisterRead(KEY_OFFSET);
		int a = 1;
		int b;
		int k;

		while (a == 1){
			b = RegisterRead(KEY_OFFSET);
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

};


int main (){
	DE1SoCfpga object;
	int a;
	int b;
	int c;

	cout<<"Please enter the LED that you want to manipulate"<<endl;
	cin>>a;
	cout<<"Please enter a 1 if you would like to turn it on or a 0 if you would like to turn it off:"<<endl;
	cin>>b;
	object.Write1Led(a, b);

	c = object.ReadAllSwitches();
	cout<<"the value of the current switch combination is "<<c<<endl;

	int d;
	int switchNum;
	cout<<"Please enter the number of the switch whose value you would like to read\n"<<endl;
	cin>>switchNum;
	d = object.Read1Switch(switchNum); 
	cout<<"The  value of switch "<<switchNum<<" is "<<d<<endl;

	int response = object.PushButtonGet();
	switch (response){
		case 0:
			a = object.RegisterRead(LEDR_OFFSET);
			a = a+1;
			object.RegisterWrite(LEDR_OFFSET, a);
			cout<<"The value of the LEDs has been incremented by 1"<<endl;	
		break;
		case 1:
			a = object.RegisterRead(LEDR_OFFSET);
			a = a-1;
			object.RegisterWrite(LEDR_OFFSET, a);
			cout<<"The value of the LEDs has decremented by 1"<<endl;
		break;
		case 2:
			a = object.RegisterRead(LEDR_OFFSET);
			a = a>>1;
			object.RegisterWrite(LEDR_OFFSET,a);
			cout<<"The value of the LEDs has been shifted 1 to the right"<<endl;
		break;
		case 3:
			a = object.RegisterRead(LEDR_OFFSET);
			a = a<<1;
			object.RegisterWrite(LEDR_OFFSET,a);
			cout<<"The value of the LEDs has been shifted 1 to the left"<<endl;
		break;
	}
}
