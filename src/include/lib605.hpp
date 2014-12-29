/*
	lib605.hpp - Main library definitions

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/
#pragma once
#include <ostream>
#include <iostream>
#include <string>

#define BAUDRATE 9600

// Protocol defines

// Control Code
#define MSR_ESC 		"\x1B"

// Status byte values
#define MSR_G_OK		"\x30"
#define MSR_RW_ERROR	"\x31"
#define MSR_CFMT_ERROR	"\x32"
#define MSR_INVALID_CMD	"\x34"
#define MSR_INVALID_SWP "\x39"

// OK and FAIL values
#define MSR_OK			MSR_ESC MSR_G_OK
#define MSR_FAIL		MSR_ESC	"\x41"


// Response: NONE
#define MSR_RESET 			MSR_ESC "\x61"
// Response: [DATA] MSR_ESC [STATUS]
#define MSR_ISO_READ 		MSR_ESC "\x72"
// Response: MSR_ESC [STATUS]
#define MSR_ISO_WRITE 		MSR_ESC "\x77"
// Response: MSR_ESC Y(\x79)
#define MSR_COM_TEST 		MSR_ESC "\x65"
// Response: None
#define MSR_ALL_LED_OFF 	MSR_ESC "\x81"
// Response: None
#define MSR_ALL_LED_ON 		MSR_ESC "\x82"
// Response: None
#define MSR_GREEN_LED_ON 	MSR_ESC "\x83"
// Response: None
#define MSR_YELLOW_LED_ON 	MSR_ESC "\x84"
// Response: None
#define MSR_RED_LED_ON 		MSR_ESC "\x85"
// Response: MSR_ESC 0 (If test succeeded)
// NOTE: Will not return unless MSR_REST or Card swiped
#define MSR_SENS_TEST		MSR_ESC "\x86"
// Response: MSR_ESC 0 (If test succeeded) MSR_ESC A on fail
#define MSR_RAM_TEST		MSR_ESC "\x87"
// Response: MSR_ESC 0 (If set succeeded) MSR_ESC A on fail
/* NOTE: This command is used to set how many leading zeros will be written before the card data starts, and
		 the space should calculated as [leading zero] X25.4 / BPI (75or210) =mm
		 Default setting of leading zero: [3D] [16]
		 TK1 & TK3: [3D] means leading zero=61
		 TK2: [16] means leading zero=22
*/
#define MSR_SET_LEAD_ZERO	MSR_ESC "\x7A"
// Response: MSR_ESC [00-FF] [00-FF]
#define MSR_CHECK_LEAD_ZERO	MSR_ESC "\x6C"
// Response: MSR_ESC 0 (If erase succeeded) MSR_ESC A on fail
#define MSR_ERASE_CARD		MSR_ESC "\x6C"
// Track constants
#define MSR_EC_TRACK1		0b00000000
#define MSR_EC_TRACK2		0b00000010
#define MSR_EC_TRACK3		0b00000100
#define MSR_EC_TRACK1_2		MSR_EC_TRACK1 | MSR_EC_TRACK2
#define MSR_EC_TRACK1_3		MSR_EC_TRACK1 | MSR_EC_TRACK3
#define MSR_EC_TRACK2_3		MSR_EC_TRACK2 | MSR_EC_TRACK3
#define MSR_EC_TRACK1_2_3	MSR_EC_TRACK1 | MSR_EC_TRACK2 | MSR_EC_TRACK3
// Response: MSR_OK (If set succeeded) MSR_FAIL on fail
#define MSR_SET_BPI			MSR_ESC "\x62"
// Sub commands per track.
#define MSR_SB_TRACK1_210	MSR_SET_BPI "\xA1"
#define MSR_SB_TRACK1_75	MSR_SET_BPI "\xA0"
#define MSR_SB_TRACK2_210	MSR_SET_BPI "\xD2"
#define MSR_SB_TRACK2_75	MSR_SET_BPI "\x4B"
#define MSR_SB_TRACK3_210	MSR_SET_BPI "\xC1"
#define MSR_SB_TRACK3_75	MSR_SET_BPI "\xC0"
// Response: [RAW_DATA] MSR_ESC [STATUS]
#define MSR_RAW_READ		MSR_ESC "\x6D"
// Response: MSR_ESC [STATUS]
#define MSR_RAW_WRITE		MSR_ESC "\x6E"
// Response: MSR_ESC [Model] S
#define MSR_REQ_MODEL		MSR_ESC "\x74"
// Response: MSR_ESC [Version]
#define MSR_REQ_FIRM_VER	MSR_ESC "\x76"
// Response: MSR_ESC \x30 [TK1] [TK2] [TK3]
// NOTE: Track sizes are between 05-08
#define MSR_SET_BPC			MSR_ESC "\x6F"
// Response: MSR_ESC 0
#define MSR_SET_HI_CO		MSR_ESC "\x78"
// Response: MSR_ESC 0
#define MSR_SET_LO_CO		MSR_ESC "\x79"
// Response: MSR_ESC H/L
#define MSR_GET_CO_STAT		MSR_ESC "\x64"

namespace lib605 {

	class Track {
		public:
			enum TRACK_BIT_LEN {
				TRACK_5_BIT,
				TRACK_7_BIT,
				TRACK_8_BIT
			};
			enum TRACK_BPI {
				BPI_210,
				BPI_75
			};
		private:
			unsigned char* TrackData;
			int TrackDataLength;
			TRACK_BIT_LEN TrackBitLength;
		public:
			Track(unsigned char* data, int data_len, Track::TRACK_BIT_LEN bit_len);
			~Track(void);

			unsigned char* GetTrackData(void);
			int GetTrackDataLength(void);
			TRACK_BIT_LEN GetTrackBitLength(void);

			friend std::ostream& operator<< (std::ostream &out, Track &sTrack);
	};

	class Magstripe {
		public:
			enum CARD_DATA_FORMAT {
				RAW,
				ISO
			};
		private:
			Track* Track1;
			Track* Track2;
			Track* Track3;
			CARD_DATA_FORMAT Format;

			Track* CreateTrack(unsigned char* data, int data_len, Track::TRACK_BIT_LEN bit_len);
		public:
			Magstripe(CARD_DATA_FORMAT Format);
			~Magstripe(void);

			Track* GetTrack1(void);
			Track* GetTrack2(void);
			Track* GetTrack3(void);

			void SetTrack1(unsigned char* data, int data_len, Track::TRACK_BIT_LEN bit_len);
			void SetTrack2(unsigned char* data, int data_len, Track::TRACK_BIT_LEN bit_len);
			void SetTrack3(unsigned char* data, int data_len, Track::TRACK_BIT_LEN bit_len);

			CARD_DATA_FORMAT GetCardDataFormat(void);

			friend std::ostream& operator<< (std::ostream &out, Magstripe &sMagstripe);
	};

	class MSR {
		public:
			enum MSR_LED {
				LED_GREEN,
				LED_YELLOW,
				LED_RED,
				LED_ALL,
				LED_OFF
			};
			enum CO {
				HI_CO,
				LO_CO,
				ERR
			};
		private:
			int devhndl;
			bool MSRConected;
			std::string Device;

			void CycleLED(void) noexcept;

		public:
			MSR(void) noexcept;
			MSR(std::string Device) noexcept;
			~MSR(void);

			bool Connect(void);
			bool Connect(std::string Device);

			bool Initialize(void);


			bool TestCommunication(void);
			bool TestSensor(void);
			bool TestRAM(void);

			void SendReset(void);

			void SetLED(MSR_LED LED);

			bool IsConnected(void);
			void Disconnect(void);

			std::string GetModel(void);
			std::string GetFirmwareVersion(void);

			int ReadAutoBytes(char* buffer);
			int ReadBytes(char* buffer, int len);
			int WriteAutoSize(char* buffer);
			int WriteBytes(char* buffer, int len);

			bool SetBPC(char Track1, char Track2, char Track3);
			bool SetBPI(int track, Track::TRACK_BPI TrackBPI);

			bool SetCo(CO co);
			CO GetCo(void);


	};
}
