/*
	lib605.cpp - Library implementation
*/
#include "./include/lib605.hpp"

 #include <stdint.h>
 #include <stdio.h>
 #include <sys/types.h>
 #include <fcntl.h>
 #include <termios.h>
 #include <sys/stat.h>
 #include <string.h>
 #include <unistd.h>
 #include <errno.h>
 #include <ctype.h>
 #include <sys/ioctl.h>
 #include <signal.h>


#include <chrono>
#include <thread>


namespace lib605 {

	void MSR::CycleLED(void) noexcept {
		this->SetLED(MSR::MSR_LED::LED_RED);
		std::this_thread::sleep_for(std::chrono::milliseconds(1500));
		this->SetLED(MSR::MSR_LED::LED_YELLOW);
		std::this_thread::sleep_for(std::chrono::milliseconds(1500));
		this->SetLED(MSR::MSR_LED::LED_GREEN);
		std::this_thread::sleep_for(std::chrono::milliseconds(1500));
		this->SetLED(MSR::MSR_LED::LED_OFF);
	}

	MSR::MSR(void) noexcept {
		// Set the initial state
		this->MSRConected = false;
		this->Device = "/dev/ttyUSB0";
	}
	MSR::MSR(std::string Device) noexcept {
		// Do the same as above but the user gave us the device
		this->MSRConected = false;
		this->Device = Device;
	}

	MSR::~MSR(void) {
		if(this->MSRConected) this->Disconnect();
	}

	bool MSR::Connect(void) {
		return this->Connect(this->Device);
	}
	bool MSR::Connect(std::string Device) {
#if defined(DEBUG)
		std::cout << "[*] Connecting to device '" << Device <<"'" << std::end;
#endif
		if(Device == "") {
			std::cout << "[*] Null device name, unable to connect" << std::endl;
			return false;
		}
		struct termios options;
		if((this->devhndl = open(Device.c_str(), O_RDWR | O_NOCTTY)) < 0) {
			std::cout << "[*] Error opening device for use" << std::endl;
			return false;
		}
		tcgetattr(this->devhndl, &options);

		options.c_cflag = CS8 | CREAD | CLOCAL;
	    options.c_oflag = 0;
		options.c_iflag = 0;

	    cfsetispeed(&options, B9600);
	    cfsetospeed(&options, B9600);

	    tcsetattr(this->devhndl, TCSANOW, &options);

		return (this->MSRConected = true);
	}

	bool MSR::Initialize(void) {
		if(!this->MSRConected) {
			std::cout << "[*] Unable to initialize, not connect to device" << std::endl;
			return false;
		}
#if defined(DEBUG)
		std::cout << "[*] Initializing Device" << std::end;
#endif
		this->CycleLED();
#if defined(DEBUG)
		std::cout << "[*] Performing self test" << std::end;
#endif
		this->SetLED(MSR::MSR_LED::LED_YELLOW);
		if(this->TestCommunication() && (this->TestRAM() && this->TestSensor())) {
#if defined(DEBUG)
		std::cout << "[*] Self test succeeded" << std::end;
#endif
			this->SetLED(MSR::MSR_LED::LED_GREEN);
			this->SendReset();
			return true;
		} else {
#if defined(DEBUG)
		std::cout << "[*] Self test failed, RAM or Sensor Error" << std::end;
#endif
			this->SetLED(MSR::MSR_LED::LED_RED);
			return false;
		}
	}

	bool MSR::TestCommunication(void) {
#if defined(DEBUG)
		std::cout << "[*] Performing communication test" << std::end;
#endif
		char resp[2];
		this->WriteAutoSize(MSR_COM_TEST);
		if(this->ReadBytes((char*)&resp, 2) != 2) {
			std::cout << "[*] Communication self test failed, expected back 2 bytes" << std::endl;
			return false;
		}
		if(memcmp(resp, (MSR_ESC "\x79"), 2) != 0) {
			std::cout << "[*] Communication self test failed, expected <ESC>\\x79 got other" << std::endl;
			return false;
		}
		return true;
	}

	bool MSR::TestSensor(void) {
#if defined(DEBUG)
		std::cout << "[*] Performing sensor test" << std::end;
#endif
		char resp[2];
		this->WriteAutoSize(MSR_SENS_TEST);
		// The device wont respond unless a reset is issued
		this->SendReset();
		if(this->ReadBytes((char*)&resp, 2) != 2) {
			std::cout << "[*] Sensor self test failed, expected back 2 bytes" << std::endl;
			return false;
		}
		if(memcmp(resp, MSR_OK, 2) != 0) {
			std::cout << "[*] Sensor self test failed, expected MSR_OK got something else" << std::endl;
			return false;
		}
		return true;
	}
	bool MSR::TestRAM(void) {
#if defined(DEBUG)
		std::cout << "[*] Performing RAM test" << std::end;
#endif
		char resp[2];
		this->WriteAutoSize(MSR_RAM_TEST);
		if(this->ReadBytes((char*)&resp, 2) != 2) {
			std::cout << "[*] Ram self test failed, expected back 2 bytes" << std::endl;
			return false;
		}
		if(memcmp(resp, MSR_OK, 2) == 0) {
			return true;
		}else if (memcmp(resp, MSR_FAIL, 2) == 0) {
			std::cout << "[*] RAM self test failed, got MSR_FAIL" << std::endl;
			return false;
		} else {
			std::cout << "[*] RAM self test failed, expected MSR_OK or MSR_FAIL, got other" << std::endl;
			return false;
		}
	}

	void MSR::SendReset(void) {
		if(!this->MSRConected) {
			std::cout << "[*] Error: Unable to reset non-connected device" << std::endl;
			return;
		}
		this->WriteAutoSize(MSR_RESET);
	}

	void MSR::SetLED(MSR_LED LED) {
		if(!this->MSRConected) {
			std::cout << "[*] Error: Unable to reset non-connected device" << std::endl;
			return;
		}
		switch(LED) {
			case LED_GREEN: {
				this->WriteAutoSize(MSR_GREEN_LED_ON);
				break;
			} case LED_YELLOW: {
				this->WriteAutoSize(MSR_YELLOW_LED_ON);
				break;
			} case LED_RED: {
				this->WriteAutoSize(MSR_RED_LED_ON);
				break;
			} case LED_ALL: {
				this->WriteAutoSize(MSR_ALL_LED_ON);
				break;
			} case LED_OFF: {
				this->WriteAutoSize(MSR_ALL_LED_OFF);
				break;
			}default: break;
		}
	}

	bool MSR::IsConnected(void) {
		return this->MSRConected;
	}

	void MSR::Disconnect(void) {
#if defined(DEBUG)
		std::cout << "[*] Disconnecting from device" << std::end;
#endif
		this->SendReset();
		close(this->devhndl);
		this->MSRConected = false;
	}

	std::string MSR::GetModel(void) {
		char* model = new char[3];
		this->WriteAutoSize(MSR_REQ_MODEL);
		return "";
	}

	std::string MSR::GetFirmwareVersion(void) {
		return "";
	}


	int MSR::ReadAutoBytes(char* buffer) {
		return this->ReadBytes(buffer, (sizeof(buffer)/sizeof(char)));
	}

	int MSR::ReadBytes(char* buffer, int len) {
		if(!this->MSRConected) {
			std::cout << "[*] Error: unable to read from non-connected device" << std::endl;
			return -1;
		}
		int temp = 0;
    	int count;

    	if(buffer == NULL) return -1;

    	while(temp != len) {
        	count = read(this->devhndl, (buffer + temp), (len - temp));
        	if(count < 0) return -1;
        	if(count > 0) temp += count;
    	}
    	return count;
	}

	int MSR::WriteAutoSize(char* buffer) {
		return this->WriteBytes(buffer, (sizeof(buffer)/sizeof(char)));
	}

	int MSR::WriteBytes(char* buffer, int len) {
		if(!this->MSRConected) {
			std::cout << "[*] Error: unable to write to non-connected device" << std::endl;
			return -1;
		}
		int count = 0;
		count = write(this->devhndl, buffer, len);
		return count;
	}
}
