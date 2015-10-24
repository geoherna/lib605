/*
	demo.cpp - lib605 demo/test application
*/
#include "lib605.hpp"
#include <iostream>

auto main(void) -> int {
	lib605::MSR device;
	
	device.Connect();
	if(!device.IsConnected()) return 1;
	if(!device.Initialize()) return 1;
	
	std::cout << "Swipe Card Now..." << std::endl;
	
	lib605::Magstripe stripe = device.ReadCard(lib605::Magstripe::CARD_DATA_FORMAT::RAW);

	std::cout << "Card Data:" << std::endl << stripe << std::endl;

	return 0;
}
