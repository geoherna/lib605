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
#include <tuple>

// Allows one to redefine the default device at compile time
#if !defined(DEFAULT_DEV)
#define DEFAULT_DEV "/dev/ttyUSB0"
#endif

// Protocol defines

// Control Code
#define MSR_ESC 			"\x1B"

// Status byte values
#define MSR_G_OK			"\x30"
#define MSR_RW_ERROR		"\x31"
#define MSR_CFMT_ERROR		"\x32"
#define MSR_INVALID_CMD		"\x34"
#define MSR_INVALID_SWP 	"\x39"

// OK and FAIL values
#define MSR_OK				MSR_ESC MSR_G_OK
#define MSR_FAIL			MSR_ESC	"\x41"


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
#define MSR_EC_TRACK1		"\x00"
#define MSR_EC_TRACK2		"\x02"
#define MSR_EC_TRACK3		"\x04"
#define MSR_EC_TRACK1_2		"\x03"
#define MSR_EC_TRACK1_3		"\x05"
#define MSR_EC_TRACK2_3		"\x06"
#define MSR_EC_TRACK1_2_3	"\x07"
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

/*! \namespace lib605
	\brief MSR605 and 606 Userspace library
*/
namespace lib605 {
	/*! \class lib605::Track
		\brief Track data container
		This class contains the definition for all of the track data
		returned from read of a card.
	*/
	class Track {
		public:
			/*! \enum lib605::Track::TRACK_BIT_LEN
				The length of each entry
			*/
			enum TRACK_BIT_LEN {
				TRACK_5_BIT,	/*!< Data in track is encoded in 5 bits */
				TRACK_7_BIT,	/*!< Data in track is encoded in 7 bits */
				TRACK_8_BIT		/*!< Data in track is encoded in 8 bits */
			};
			/*! \enum lib605::Track::TRACK_BPI
				The number of bits per inch of the track
			*/
			enum TRACK_BPI {
				BPI_210,	/*!<  Track has 210 bits per inch */
				BPI_75		/*!<  Track has  75 bits per inch */
			};
		private:
			// Raw track data
			unsigned char* TrackData;
			// size of data
			int TrackDataLength;
			// Track BPC
			TRACK_BIT_LEN TrackBitLength;
		public:
			/*!
				Construct a new track

				\param data The track information
				\param data_len The length of the track
				\param bit_len The density of the track
			*/
			Track(unsigned char* data, int data_len, Track::TRACK_BIT_LEN bit_len);
			// Destructor
			~Track(void);

			/*! Returns the raw track data */
			unsigned char* GetTrackData(void);
			/*! Returns the length of the track data */
			int GetTrackDataLength(void);

			TRACK_BIT_LEN GetTrackBitLength(void);

			// Allows human-readable output of data
			friend std::ostream& operator<< (std::ostream &out, Track &sTrack);
	};

	// Magstripe data class
	class Magstripe {
		public:
			// Format of the card data contained herein
			enum CARD_DATA_FORMAT {
				RAW,
				ISO
			};
		private:
			// Tracks
			Track* Track1;
			Track* Track2;
			Track* Track3;
			CARD_DATA_FORMAT Format;
			// Creates a track from the given buffer (Might move into Track class)
			Track* CreateTrack(unsigned char* data, int data_len, Track::TRACK_BIT_LEN bit_len);
		public:
			// Constructor
			Magstripe(CARD_DATA_FORMAT Format);
			// Destructor
			~Magstripe(void);

			// Gets each track object
			Track* GetTrack1(void);
			Track* GetTrack2(void);
			Track* GetTrack3(void);

			// Sets each track object
			void SetTrack1(unsigned char* data, int data_len, Track::TRACK_BIT_LEN bit_len);
			void SetTrack2(unsigned char* data, int data_len, Track::TRACK_BIT_LEN bit_len);
			void SetTrack3(unsigned char* data, int data_len, Track::TRACK_BIT_LEN bit_len);

			// Returns the card format
			CARD_DATA_FORMAT GetCardDataFormat(void);

			// Outputs a nice human-readable representation of the Magstripe data
			friend std::ostream& operator<< (std::ostream &out, Magstripe &sMagstripe);
	};

	// Main class for interacting with the MSR device
	class MSR {
		public:
			// LED Control
			enum MSR_LED {
				LED_GREEN,
				LED_YELLOW,
				LED_RED,
				LED_ALL,
				LED_OFF
			};
			// Coercivity Values
			enum COERCIVITY {
				HI_CO,
				LO_CO,
				ERR
			};
			// Track control enum
			enum TRACK {
				TRACK_1,
				TRACK_2,
				TRACK_3,
				TRACK_1_2,
				TRACK_1_3,
				TRACK_2_3,
				TRACK_1_2_3
			};
		private:
			// Device handle
			int devhndl;
			// Connection Status
			bool MSRConected;
			// Device path '/dev/ttyUSB0' by default
			std::string Device;

			//  Cycles the LEDs used in initialization step
			void CycleLED(void) noexcept;

		public:
			// Construct a new MSR class
			MSR(void) noexcept;
			// Set a device
			MSR(std::string Device) noexcept;
			// Destructor
			~MSR(void);

			// Connect to the deice specified on class instantiation
			bool Connect(void);
			// Connect to given device
			bool Connect(std::string Device);

			// Initialize the MSR device
			bool Initialize(void);

			// Communication Self Test (Runs second)
			bool TestCommunication(void);
			// Sensor Self Test (Runs last)
			bool TestSensor(void);
			// RAM Self Test (Runs first)
			bool TestRAM(void);

			// Sends the reset code to the device
			void SendReset(void);

			// Sets the LED on the device
			void SetLED(MSR_LED LED);

			// Check the device connection
			bool IsConnected(void);
			// Disconnects from the device
			void Disconnect(void);

			// Gets the model number of the device
			std::string GetModel(void);
			// Gets the firmware version of the device
			std::string GetFirmwareVersion(void);


			// Attempts to estimate buffer size and read that many bytes from the device
			int ReadAutoBytes(char* buffer);
			// Reads an arbitrary number of bytes from the device
			int ReadBytes(char* buffer, int len);
			// Attempts to estimate the size of the write buffer and write to the device
			int WriteAutoSize(char* buffer);
			// Writes an arbitrary number of bytes to the device
			int WriteBytes(char* buffer, int len);

			// Set the bits per character on the device per tack
			bool SetBPC(char Track1, char Track2, char Track3);
			// Sets the bits per inch on the given track
			bool SetBPI(int track, Track::TRACK_BPI TrackBPI);

			// Sets the Coercivity of the device
			bool SetCoercivity(COERCIVITY co);
			// Returns the current Coercivity of the device
			COERCIVITY GetCoercivity(void);

			// Sets the leading zeros of the tracks on the device
			bool SetLeadingZero(unsigned char Track1_3, unsigned char Track2);
			// Gets the leading zeros of the device. Item one is tracks 1 and 3 item two is track 2
			std::tuple<unsigned char, unsigned char> GetLeadZero(void);

			// Sets the device to erase the given track
			// NOTE: CALL A RESET AFTER USING!!!!
			bool EraseCard(TRACK track);

			// Returns a magstripe object with card data in the given format
			Magstripe ReadCard(Magstripe::CARD_DATA_FORMAT Format);

			// Read a ISO Track into a buffer
			bool ReadISOTrackData(unsigned char* buffer, int buffer_size, Track::TRACK_BIT_LEN trackFmt);
			// Read raw track data into a buffer
			bool ReadRAWTrackData(unsigned char* buffer, int buffer_size, Track::TRACK_BIT_LEN trackFmt);
	};
}
