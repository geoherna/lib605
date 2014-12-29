/*
	lib605.cpp - Library implementation

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
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

/*	====	  START Track CLASS			====	*/

	Track::Track(unsigned char* data, int data_len, Track::TRACK_BIT_LEN bit_len) {
		this->TrackData = data;
		this->TrackDataLength = data_len;
		this->TrackBitLength = bit_len;
	}

	Track::~Track(void) {

	}

	unsigned char* Track::GetTrackData(void) {
		return this->TrackData;
	}

	int Track::GetTrackDataLength(void) {
		return this->TrackDataLength;
	}

	Track::TRACK_BIT_LEN Track::GetTrackBitLength(void) {
		return this->TrackBitLength;
	}

	std::ostream& operator<< (std::ostream &out, Track &sTrack) {
		out << "Track bit length: ";
		switch(sTrack.GetTrackBitLength()) {
			case Track::TRACK_5_BIT: {
				out << "5" << std::endl;
				break;
			} case Track::TRACK_7_BIT: {
				out << "7" << std::endl;
				break;
			} case Track::TRACK_8_BIT: {
				out << "8" << std::endl;
				break;
			} default: break;
		}
		out << "Data length: " << sTrack.GetTrackDataLength();
		return out;
	}

/*	====	START Magstripe CLASS		====	*/

	Track* Magstripe::CreateTrack(unsigned char* data, int data_len, Track::TRACK_BIT_LEN bit_len) {
		return new Track(data, data_len, bit_len);
	}

	Magstripe::Magstripe(Magstripe::CARD_DATA_FORMAT Format) {
		this->Format = Format;
	}

	Magstripe::~Magstripe(void) {
		// Clean up the tracks
		delete this->Track1;
		delete this->Track2;
		delete this->Track3;
	}

	Track* Magstripe::GetTrack1(void) {
		return this->Track1;
	}

	Track* Magstripe::GetTrack2(void) {
		return this->Track2;
	}

	Track* Magstripe::GetTrack3(void) {
		return this->Track3;
	}

	void Magstripe::SetTrack1(unsigned char* data, int data_len, Track::TRACK_BIT_LEN bit_len) {
		this->Track1 = this->CreateTrack(data, data_len, bit_len);
	}

	void Magstripe::SetTrack2(unsigned char* data, int data_len, Track::TRACK_BIT_LEN bit_len) {
		this->Track2 = this->CreateTrack(data, data_len, bit_len);
	}

	void Magstripe::SetTrack3(unsigned char* data, int data_len, Track::TRACK_BIT_LEN bit_len) {
		this->Track3 = this->CreateTrack(data, data_len, bit_len);
	}

	Magstripe::CARD_DATA_FORMAT Magstripe::GetCardDataFormat(void) {
		return this->Format;
	}

	std::ostream& operator<< (std::ostream &out, Magstripe &sMagstripe) {
		out << "Card Format: ";
		switch(sMagstripe.GetCardDataFormat()) {
			case Magstripe::ISO: {
				out << "ISO" << std::endl;
				break;
			} case Magstripe::RAW: {
				out << "Raw" << std::endl;
				break;
			} default: break;
		}

		Track* t = sMagstripe.GetTrack1();
		if(t->GetTrackDataLength() != 0) {
			out << "\t" << (*t);
			if(sMagstripe.Format == Magstripe::RAW)
				out << "\t Track Data: " << std::hex << t->GetTrackData() << std::endl;
			else
				out << "\t Track Data:" << t->GetTrackData() << std::endl;
		} else {
			out << "Track 1: EMPTY" << std::endl;
		}
		t = sMagstripe.GetTrack2();
		if(t->GetTrackDataLength() != 0) {
			out << "\t" << (*t);
			if(sMagstripe.Format == Magstripe::RAW)
				out << "\t Track Data: " << std::hex << t->GetTrackData() << std::endl;
			else
				out << "\t Track Data:" << t->GetTrackData() << std::endl;
		} else {
			out << "Track 2: EMPTY" << std::endl;
		}
		t = sMagstripe.GetTrack3();
		if(t->GetTrackDataLength() != 0) {
			out << "\t" << (*t);
			if(sMagstripe.Format == Magstripe::RAW)
				out << "\t Track Data: " << std::hex << t->GetTrackData() << std::endl;
			else
				out << "\t Track Data:" << t->GetTrackData() << std::endl;
		} else {
			out << "Track 3: EMPTY" << std::endl;
		}
		return out;
	}

/*	====		START MSR CLASS 		====	*/

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
		if(!this->MSRConected) {
			std::cout << "[*] Error: unable to get model from non-connected device" << std::endl;
			return "ERROR";
		}
		char* model = new char[3];
		this->WriteAutoSize(MSR_REQ_MODEL);
		if(this->ReadBytes(model, 3) != 3) {
			std::cout << "[*] Error: unable to read model number, expected 3 bytes" << std::endl;
			return "ERROR";
		}
		if((memcmp((char*)model[0], MSR_ESC, 1) == 0) && model[2] == 'S') {
			model[0] = model[1];
			model[1] = '\0';
			std::string mdl(model);
			delete[] model;
			return mdl;
		}
		return "ERROR";
	}

	std::string MSR::GetFirmwareVersion(void) {
		if(!this->MSRConected) {
			std::cout << "[*] Error: unable to get firmware version from non-connected device" << std::endl;
			return "ERROR";
		}
		char* version = new char[9];
		this->WriteAutoSize(MSR_REQ_FIRM_VER);
		if(this->ReadBytes(version, 9) == 9) {
			std::cout << "[*] Error: unable to get firmware version" << std::endl;
			return "ERROR";
		}
		if(memcmp((char*)version[0], MSR_ESC, 1) == 0) {
			version[8] = '\0';
			memmove(version, version+1, strlen(version));
			std::string ver(version);
			delete[] version;
			return ver;
		}
		return "ERROR";
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
