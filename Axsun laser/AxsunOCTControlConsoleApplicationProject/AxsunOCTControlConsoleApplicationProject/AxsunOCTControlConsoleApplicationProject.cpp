// AxsunOCTControlConsoleApplicationProject.cpp : Defines the entry point for the console application.
//
// NOTE: This example program is minimal on error checking for educational simplicity. 
// Production code based on this example should encorporate error checking as required.
//
// This program is compatible with AxsunOCTControl.dll version 1.16.7 or later.

// includes and defines
#include "stdafx.h"
#include <iostream>
#import "AxsunOCTControl.tlb" named_guids raw_interfaces_only
using namespace AxsunOCTControl;
#define maxDevices 5
#define DAQ_device 42
#define Laser_device 40

// function declarations
long EnumerateDevices(unsigned long * myDeviceList, IAxsunOCTControlPtr pAxsunOCTControl);
long SearchDeviceList(long whichDevice, unsigned long * myDeviceList);

// main program
int main() {
	
	// initialize variable(s)
	unsigned long retvallong = 0;
	VARIANT_BOOL isConnected = 0;

	// initialize COM and bind to the AxsunOCTControl library
	CoInitialize(NULL);	
	IAxsunOCTControlPtr pAxsunOCTControl(__uuidof(struct AxsunOCTControl));		// if this step fails, make sure you've performed the REGASM step according to the instructions and that all dependencies are present in the same location as the main AxsunOCTControl library	

	// initialize a simple static array to be used as a user device list
	// (This can be done a variety of ways, such as a linked list or other dynamic array if desired.)
	unsigned long myDeviceList[maxDevices] = { 0 };

	// open network interface and wait 2 seconds (time for connection to be established)
	pAxsunOCTControl->StartNetworkControlInterface(&retvallong);		// retvallong = 0 if successful or = 1047 if the network interface is already open (possibly from a different application)
	Sleep(2000);

	// Enumerate the device list (redo this step whenever devices are connected or disconnected)
	// More robust architectures would occasionally poll for device list changes or utilize the "OCTDeviceConnectOrDisconnectEvent" callback to re-enumerate devices
	long numDevices = EnumerateDevices(myDeviceList, pAxsunOCTControl);

	// Perform some example AxsunOCTControl library functionality (turn on laser and DAQ live imaging, wait for key press, then turn off)
	// Start Laser Emission
	pAxsunOCTControl->ConnectToOCTDevice(SearchDeviceList(Laser_device,myDeviceList), &isConnected);		// search device list and connect to laser
	if (isConnected == -1)
		pAxsunOCTControl->StartScan(&retvallong);

	// Start DAQ Live Imaging via FPGA Registers
	pAxsunOCTControl->ConnectToOCTDevice(SearchDeviceList(DAQ_device, myDeviceList), &isConnected);		// search device list and connect to DAQ
	if (isConnected == -1)
	{
		pAxsunOCTControl->SetFPGARegister(19, 0x8000, &retvallong);
		pAxsunOCTControl->SetFPGARegister(2, 0x0604, &retvallong);
	}

	std::cout << "\nPress any key to stop...\n";
	std::cin.get();	// wait for user to hit key

	// Stop DAQ Live Imaging via FPGA Registers
	pAxsunOCTControl->ConnectToOCTDevice(SearchDeviceList(DAQ_device, myDeviceList), &isConnected);		// search device list and connect to DAQ
	if (isConnected == -1)
	{
		pAxsunOCTControl->SetFPGARegister(2, 0x0000, &retvallong);
		pAxsunOCTControl->SetFPGARegister(19, 0x0000, &retvallong);
	}

	// Stop Laser Emission
	pAxsunOCTControl->ConnectToOCTDevice(SearchDeviceList(Laser_device, myDeviceList), &isConnected);		// search device list and connect to laser
	if (isConnected == -1)
		pAxsunOCTControl->StopScan(&retvallong);

	// close network interface before exiting
	pAxsunOCTControl->StopNetworkControlInterface(&retvallong);

	// un-initialize COM
	CoUninitialize();	

	std::cout << "\nPress any key to exit program...";
	std::cin.get();	// wait for user to hit key

	return 0;
}


long SearchDeviceList(long whichDevice, unsigned long * myDeviceList) {
	// this function searches the user device list for the first device number matching whichDevice and returns its index within the list if matched
	// if the desired device is not found in the list, the function returns -1
	long deviceIndex = 0;
	while (1) {
		if (myDeviceList[deviceIndex] == whichDevice)
			return deviceIndex;
		else if (deviceIndex >= maxDevices)
			return -1;
		else
			deviceIndex++;
	}
}

long EnumerateDevices(unsigned long * myDeviceList, IAxsunOCTControlPtr pAxsunOCTControl) {
	// query the number of Axsun devices successfully connected to the Control library (includes USB and Ethernet network devices)
	long numDev = 0;
	pAxsunOCTControl->GetNumberOfOCTDevicesPresent(&numDev);

	// allocate some temporary string memory to be used later in GetSystemType() function call
	BSTR systemTypeString = SysAllocString(L"");

	// enumerate connected Axsun devices by looping through numDev devices and storing each device type in the myDeviceList array
	VARIANT_BOOL isConnected = 0;
	unsigned long retvallong = 0;
	for (long i = 0; i < numDev; i++) {
		pAxsunOCTControl->ConnectToOCTDevice(i, &isConnected);	// isConnected should be -1 if connected successfully to device i
		if (isConnected == -1)
			pAxsunOCTControl->GetSystemType(&myDeviceList[i], &systemTypeString, &retvallong);	// populate ith element of device list array with information about the board type (e.g. DAQ = 42 vs. Laser = 40)
	}

	// free string memory
	SysFreeString(systemTypeString);

	return numDev;
}