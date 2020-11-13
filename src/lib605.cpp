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

	// Track constructor
	Track::Track(unsigned char* data, int data_len, Track::TRACK_BIT_LEN bit_len) {
		// Set all of the class members
		this->TrackData = data;
		this->TrackDataLength = data_len;
		this->TrackBitLength = bit_len;
	}

	Track::~Track(void) {
		// Dont really do anything
	}

	// Return the raw track data
	unsigned char* Track::GetTrackData(void) {
		return this->TrackData;
	}

	// Return the length of the track data
	int Track::GetTrackDataLength(void) {
		return this->TrackDataLength;
	}

	// Returns the track BPC
	Track::TRACK_BIT_LEN Track::GetTrackBitLength(void) {
		return this->TrackBitLength;
	}

	// Fancy ostream output
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

/*	==== START Magstripe CLASS ====	*/

	// Create a new track
	// NOTE: Might remove
	Track* Magstripe::CreateTrack(unsigned char* data, int data_len, Track::TRACK_BIT_LEN bit_len) {
		return new Track(data, data_len, bit_len);
	}

	// Constructor
	Magstripe::Magstripe(Magstripe::CARD_DATA_FORMAT Format) {
		// Set class members
		this->Format = Format;
	}

	// Destructor
	Magstripe::~Magstripe(void) {
		// Clean up the tracks
		delete this->Track1;
		delete this->Track2;
		delete this->Track3;
	}

	// Gets the track object
	Track* Magstripe::GetTrack1(void) {
		return this->Track1;
	}

	Track* Magstripe::GetTrack2(void) {
		return this->Track2;
	}

	Track* Magstripe::GetTrack3(void) {
		return this->Track3;
	}

	// Sets the track object
	void Magstripe::SetTrack1(unsigned char* data, int data_len, Track::TRACK_BIT_LEN bit_len) {
		this->Track1 = this->CreateTrack(data, data_len, bit_len);
	}

	void Magstripe::SetTrack2(unsigned char* data, int data_len, Track::TRACK_BIT_LEN bit_len) {
		this->Track2 = this->CreateTrack(data, data_len, bit_len);
	}

	void Magstripe::SetTrack3(unsigned char* data, int data_len, Track::TRACK_BIT_LEN bit_len) {
		this->Track3 = this->CreateTrack(data, data_len, bit_len);
	}

	// Returns the card format
	Magstripe::CARD_DATA_FORMAT Magstripe::GetCardDataFormat(void) {
		return this->Format;
	}

	// Pretty output
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

/*	==== START MSR CLASS ====	*/

	// Cycles all the LEDs
	void MSR::CycleLED(void) noexcept {
		this->SetLED(MSR::MSR_LED::LED_RED);
		std::this_thread::sleep_for(std::chrono::milliseconds(1500));
		this->SetLED(MSR::MSR_LED::LED_YELLOW);
		std::this_thread::sleep_for(std::chrono::milliseconds(1500));
		this->SetLED(MSR::MSR_LED::LED_GREEN);
		std::this_thread::sleep_for(std::chrono::milliseconds(1500));
		this->SetLED(MSR::MSR_LED::LED_OFF);
	}

	// Constructor
	MSR::MSR(void) noexcept {
		// Set the initial state
		this->MSRConected = false;
		// Give the default device
		this->Device = DEFAULT_DEV;
	}

	// MSR class with the given device, allowing for multiple devices
	MSR::MSR(std::string Device) noexcept {
		// Do the same as above but the user gave us the device
		this->MSRConected = false;
		this->Device = Device;
	}

	// Destructor
	MSR::~MSR(void) {
		// Disconnect if we are connected
		if(this->MSRConected) this->Disconnect();
	}

	// Connect to the device set in the constructor
	bool MSR::Connect(void) {
		return this->Connect(this->Device);
	}

	// Connect to the given device
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
#if defined(DEBUG)
			std::cout << "[*] Error opening device for use" << std::endl;
#endif
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
#if defined(DEBUG)
			std::cout << "[*] Unable to initialize, not connect to device" << std::endl;
#endif
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
		if(!this->MSRConected) {
#if defined(DEBUG)
			std::cout << "[*] Unable to test, not connect to device" << std::endl;
#endif
			return false;
		}
#if defined(DEBUG)
		std::cout << "[*] Performing communication test" << std::end;
#endif
		char resp[2];
		this->WriteAutoSize(MSR_COM_TEST);
		if(this->ReadBytes((char*)&resp, 2) != 2) {
#if defined(DEBUG)
			std::cout << "[*] Communication self test failed, expected back 2 bytes" << std::endl;
#endif
			return false;
		}
		if(memcmp(resp, (MSR_ESC "\x79"), 2) != 0) {
#if defined(DEBUG)
			std::cout << "[*] Communication self test failed, expected <ESC>\\x79 got other" << std::endl;
#endif
			return false;
		}
		return true;
	}

	bool MSR::TestSensor(void) {
		if(!this->MSRConected) {
#if defined(DEBUG)
			std::cout << "[*] Unable to test, not connect to device" << std::endl;
#endif
			return false;
		}
#if defined(DEBUG)
		std::cout << "[*] Performing sensor test" << std::end;
#endif
		char resp[2];
		this->WriteAutoSize(MSR_SENS_TEST);
		// The device wont respond unless a reset is issued
		this->SendReset();
		if(this->ReadBytes((char*)&resp, 2) != 2) {
#if defined(DEBUG)
			std::cout << "[*] Sensor self test failed, expected back 2 bytes" << std::endl;
#endif
			return false;
		}
		if(memcmp(resp, MSR_OK, 2) != 0) {
#if defined(DEBUG)
			std::cout << "[*] Sensor self test failed, expected MSR_OK got something else" << std::endl;
#endif
			return false;
		}
		return true;
	}
	bool MSR::TestRAM(void) {
		if(!this->MSRConected) {
#if defined(DEBUG)
			std::cout << "[*] Unable to test, not connect to device" << std::endl;
#endif
			return false;
		}
#if defined(DEBUG)
		std::cout << "[*] Performing RAM test" << std::end;
#endif
		char resp[2];
		this->WriteAutoSize(MSR_RAM_TEST);
		if(this->ReadBytes((char*)&resp, 2) != 2) {
#if defined(DEBUG)
			std::cout << "[*] Ram self test failed, expected back 2 bytes" << std::endl;
#endif
			return false;
		}
		if(memcmp(resp, MSR_OK, 2) == 0) {
			return true;
		}else if (memcmp(resp, MSR_FAIL, 2) == 0) {
#if defined(DEBUG)
			std::cout << "[*] RAM self test failed, got MSR_FAIL" << std::endl;
#endif
			return false;
		} else {
#if defined(DEBUG)
			std::cout << "[*] RAM self test failed, expected MSR_OK or MSR_FAIL, got other" << std::endl;
#endif
			return false;
		}
	}

	void MSR::SendReset(void) {
		if(!this->MSRConected) {
#if defined(DEBUG)
			std::cout << "[*] Error: Unable to reset non-connected device" << std::endl;
#endif
			return;
		}
		this->WriteAutoSize(MSR_RESET);
	}

	void MSR::SetLED(MSR_LED LED) {
		if(!this->MSRConected) {
#if defined(DEBUG)
			std::cout << "[*] Error: Unable to reset non-connected device" << std::endl;
#endif
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
		if(!this->MSRConected) {
#if defined(DEBUG)
			std::cout << "[*] Unable to disconnect, not connect to device" << std::endl;
#endif
			return;
		}
#if defined(DEBUG)
		std::cout << "[*] Disconnecting from device" << std::end;
#endif
		this->SendReset();
		close(this->devhndl);
		this->MSRConected = false;
	}

	std::string MSR::GetModel(void) {
		if(!this->MSRConected) {
#if defined(DEBUG)
			std::cout << "[*] Error: unable to get model from non-connected device" << std::endl;
#endif
			return "ERROR";
		}
		char* model = new char[3];
		this->WriteAutoSize(MSR_REQ_MODEL);
		if(this->ReadBytes(model, 3) != 3) {
#if defined(DEBUG)
			std::cout << "[*] Error: unable to read model number, expected 3 bytes" << std::endl;
#endif
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
#if defined(DEBUG)
			std::cout << "[*] Error: unable to get firmware version from non-connected device" << std::endl;
#endif
			return "ERROR";
		}
		char* version = new char[9];
		this->WriteAutoSize(MSR_REQ_FIRM_VER);
		if(this->ReadBytes(version, 9) == 9) {
#if defined(DEBUG)
			std::cout << "[*] Error: unable to get firmware version" << std::endl;
#endif
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
#if defined(DEBUG)
			std::cout << "[*] Error: unable to read from non-connected device" << std::endl;
#endif
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
#if defined(DEBUG)
			std::cout << "[*] Error: unable to write to non-connected device" << std::endl;
#endif
			return -1;
		}
		int count = 0;
		count = write(this->devhndl, buffer, len);
		return count;
	}

	bool MSR::SetBPC(char Track1, char Track2, char Track3) {
		if(!this->MSRConected) {
#if defined(DEBUG)
			std::cout << "[*] Unable to set BPC, not connect to device" << std::endl;
#endif
			return false;
		}
		char resp[5];
		char expt[5];
		char bpc_data[5];

		memcpy(bpc_data, (void*)MSR_SET_BPC, 2);
		memcpy(&bpc_data[2], (void*)&Track1, 1);
		memcpy(&bpc_data[3], (void*)&Track2, 1);
		memcpy(&bpc_data[4], (void*)&Track3, 1);

		memcpy(bpc_data, expt, 5);
		memcpy(&expt[0], MSR_OK, 2);

		this->WriteBytes(bpc_data, 5);
		if(this->ReadBytes(resp, 5) != 5) {
#if defined(DEBUG)
			std::cout << "[*] Error: Unable to set BPC, expected 5 bytes" << std::endl;
#endif
			return false;
		}

		if(memcmp(resp, expt, 5) != 0) {
#if defined(DEBUG)
			std::cout << "[*] Error: Unable to set BPC, unexpected response" << std::endl;
#endif
			return false;
		}

		return true;

	}

	bool MSR::SetBPI(int track, Track::TRACK_BPI TrackBPI) {
		if(!this->MSRConected) {
#if defined(DEBUG)
			std::cout << "[*] Unable to set BPI, not connect to device" << std::endl;
#endif
			return false;
		}
		char resp[2];
		switch(track) {
			case 1: {
				switch(TrackBPI){
					case Track::BPI_210: {
						this->WriteAutoSize(MSR_SB_TRACK1_210);
						break;
					} case Track::BPI_75: {
						this->WriteAutoSize(MSR_SB_TRACK1_75);
						break;
					} default: break;
				}
				break;
			} case 2: {
				switch(TrackBPI){
					case Track::BPI_210: {
						this->WriteAutoSize(MSR_SB_TRACK2_210);
						break;
					} case Track::BPI_75: {
						this->WriteAutoSize(MSR_SB_TRACK2_75);
						break;
					} default: break;
				}
				break;
			} case 3: {
				switch(TrackBPI){
					case Track::BPI_210: {
						this->WriteAutoSize(MSR_SB_TRACK3_210);
						break;
					} case Track::BPI_75: {
						this->WriteAutoSize(MSR_SB_TRACK3_75);
						break;
					} default: break;
				}
				break;
			} default: break;
		}
		// Check Response
		if(this->ReadBytes((char*)&resp, 2) != 2) {
#if defined(DEBUG)
			std::cout << "[*] Set BPI failed, expected back 2 bytes" << std::endl;
#endif
			return false;
		}
		if(memcmp(resp, MSR_OK, 2) == 0) {
			return true;
		}else if (memcmp(resp, MSR_FAIL, 2) == 0) {
#if defined(DEBUG)
			std::cout << "[*] Set BPI failed, got MSR_FAIL" << std::endl;
#endif
			return false;
		} else {
#if defined(DEBUG)
			std::cout << "[*] Set BPI failed, expected MSR_OK or MSR_FAIL, got other" << std::endl;
#endif
			return false;
		}
		return false;
	}

	bool MSR::SetCoercivity(COERCIVITY co) {
		if(!this->MSRConected) {
#if defined(DEBUG)
			std::cout << "[*] Unable to set Coercivity, not connect to device" << std::endl;
#endif
			return false;
		}
		char resp[2];
		switch(co) {
			case HI_CO: {
				this->WriteAutoSize(MSR_SET_HI_CO);
				break;
			} case LO_CO: {
				this->WriteAutoSize(MSR_SET_LO_CO);
				break;
			} default: break;
		}
		if(this->ReadBytes(resp, 2) != 2) {
#if defined(DEBUG)
			std::cout << "[*] Error: Unable to set Coercivity, expected 2 bytes" << std::endl;
#endif
			return false;
		}
		if(memcmp(resp, MSR_OK, 2) == 0) {
			return true;
		}
		return false;
	}

	MSR::COERCIVITY MSR::GetCoercivity(void) {
		if(!this->MSRConected) {
#if defined(DEBUG)
			std::cout << "[*] Unable to get Coercivity, not connect to device" << std::endl;
#endif
			return MSR::COERCIVITY::ERR;
		}
		char resp[2];
		this->WriteAutoSize(MSR_GET_CO_STAT);
		if(this->ReadBytes(resp, 2) != 2) {
#if defined(DEBUG)
			std::cout << "[*] Error: Unable to get Coercivity, expected 2 bytes" << std::endl;
#endif
			return MSR::COERCIVITY::ERR;
		}
		if(memcmp(resp, MSR_ESC "H", 2) == 0) {
			return MSR::COERCIVITY::HI_CO;
		} else if(memcmp(resp, MSR_ESC "L", 2) == 0) {
			return MSR::COERCIVITY::LO_CO;
		} else {
#if defined(DEBUG)
			std::cout << "[*] Error: Unable to get Coercivity, unexpected value" << std::endl;
#endif
			return MSR::COERCIVITY::ERR;
		}
	}

	bool MSR::SetLeadingZero(unsigned char Track1_3, unsigned char Track2) {
		if(!this->MSRConected) {
#if defined(DEBUG)
			std::cout << "[*] Unable to Set leading zero, not connect to device" << std::endl;
#endif
			return false;
		}
		char resp[2];
		char zero_dat[4];

		memcpy(zero_dat, (void*)MSR_SET_LEAD_ZERO, 2);
		memcpy(&zero_dat[2], (void*)&Track1_3, 1);
		memcpy(&zero_dat[3], (void*)&Track2, 1);

		this->WriteBytes(zero_dat, 4);
		if(this->ReadBytes(resp, 2) != 2) {
#if defined(DEBUG)
			std::cout << "[*] Error: Unable to set leading zero, expected 2 bytes" << std::endl;
#endif
			return false;
		}
		if(memcmp(resp, MSR_OK, 2) == 0) {
			return true;
		}
		return false;
	}

	std::tuple<unsigned char, unsigned char> MSR::GetLeadZero(void) {
		if(!this->MSRConected) {
#if defined(DEBUG)
			std::cout << "[*] Unable to get leading zero, not connect to device" << std::endl;
#endif
			return std::make_tuple(0x00, 0x00);
		}
		char resp[3];
		this->WriteAutoSize(MSR_CHECK_LEAD_ZERO);
		if(this->ReadBytes(resp, 3) != 3) {
#if defined(DEBUG)
			std::cout << "[*] Unable to get leading zero, expected 3 bytes" << std::endl;
#endif
			return std::make_tuple(0x00, 0x00);
		}
		return std::make_tuple(resp[1], resp[2]);
	}

	// CALL A RESET AFTER USING!!!!
	bool MSR::EraseCard(MSR::TRACK track) {
		if(!this->MSRConected) {
#if defined(DEBUG)
			std::cout << "[*] Unable to set erase mode, not connect to device" << std::endl;
#endif
			return false;
		}
		char resp[2];
		char erase_dat[3];

		memcpy(erase_dat, MSR_ERASE_CARD, 2);
		switch(track) {
			case MSR::TRACK::TRACK_1: {
				memcpy(&erase_dat[2], MSR_EC_TRACK1, 1);
				break;
			} case MSR::TRACK::TRACK_2: {
				memcpy(&erase_dat[2], MSR_EC_TRACK2, 1);
				break;
			} case MSR::TRACK::TRACK_3: {
				memcpy(&erase_dat[2], MSR_EC_TRACK3, 1);
				break;
			} case MSR::TRACK::TRACK_1_2: {
				memcpy(&erase_dat[2], MSR_EC_TRACK1_2, 1);
				break;
			} case MSR::TRACK::TRACK_1_3: {
				memcpy(&erase_dat[2], MSR_EC_TRACK1_3, 1);
				break;
			} case MSR::TRACK::TRACK_2_3: {
				memcpy(&erase_dat[2], MSR_EC_TRACK2_3, 1);
				break;
			} case MSR::TRACK::TRACK_1_2_3: {
				memcpy(&erase_dat[2], MSR_EC_TRACK1_2_3, 1);
				break;
			} default: break;
		}
		this->WriteBytes(erase_dat, 3);
		if(this->ReadBytes(resp, 2) != 2) {
#if defined(DEBUG)
			std::cout << "[*] Error: Unable to set erase, expected 2 bytes" << std::endl;
#endif
			return false;
		}
		if(memcmp(resp, MSR_OK, 2) == 0) {
			return true;
		}
		return false;
	}
	Magstripe MSR::ReadCard(Magstripe::CARD_DATA_FORMAT Format) {
		Magstripe ms(Format);
	}

	bool MSR::ReadISOTrackData(unsigned char* buffer, int buffer_size, Track::TRACK_BIT_LEN trackFmt) {

	}

	bool MSR::ReadRAWTrackData(unsigned char* buffer, int buffer_size, Track::TRACK_BIT_LEN trackFmt) {

	}

}
