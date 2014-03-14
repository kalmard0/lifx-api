#pragma once
#include <string>
#include <sstream>
#include <cstdint>
#include <iomanip>
#include <cassert>

namespace lifx {

	// from the awesome https://github.com/magicmonkey/lifxjs/blob/master/Protocol.md

#pragma pack(push, 1) 

	namespace Payload {
		struct PanGatewayState {
			enum class Service
			{
				UDP = 1,
				TCP = 2
			};

			uint8_t service;
			uint32_t port;     // LE
		};

		struct WifiInfo {
			float signal;   // LE
			int tx;         // LE
			int rx;         // LE
			short mcu_temperature;
		};

		struct BulbLabel {
			char label[32]; // UTF-8 encoded string
		};

		struct LightStatus {
			uint16_t hue;          // LE
			uint16_t saturation;   // LE
			uint16_t brightness;   // LE
			uint16_t kelvin;       // LE
			uint16_t dim;          // LE?
			uint16_t power;
			char bulb_label[32]; // UTF-8 encoded string
			uint64_t tags;
		};

		struct LightColorRGBW {
			void Initialize() {
				memset(this, 0, sizeof(*this));
			}
			uint16_t blue;
			uint16_t green;
			uint16_t red;
			uint16_t white;
		};

		struct LightColorHSL {
			void Initialize() {
				memset(this, 0, sizeof(*this));
			}
			uint8_t stream;        // Unknown, potential "streaming" mode toggle? Set to
			// 0x00 for now.
#if 1
			uint8_t reserved1;
			uint8_t hue;         // LE NOTE: Wraps around at 0xff 0xff back to 0x00 0x00
			// which is a primary red colour.
			uint8_t reserved2;
			uint8_t saturation;  // LE
			uint8_t reserved3;
			uint8_t brightness;  // LE
			uint8_t reserved4;
			uint8_t kelvin;      // LE i.e. colour temperature (whites wheel in apps)
			uint16_t reserved5;
			uint16_t fade_time;
#else
			uint16_t hue;
			uint16_t saturation;
			uint16_t brightness;
			uint16_t kelvin; 
			uint32_t fade_time;   // LE Length of fade action, in seconds
#endif	
		};

		struct SetDim {
			uint8_t reserved;
			uint8_t brightness; // LE
			uint32_t duration;  // in seconds
		};
	}

	namespace Protocol {
		const uint16_t Send = 0x3400;
	}

	namespace PacketType {
		const uint16_t Invalid = 0xffff;
		const uint16_t GetPanGateway = 0x02;
		const uint16_t PanGatewayState = 0x03;
		const uint16_t GetWifiInfo = 0x10;
		const uint16_t WifiInfo = 0x11;
		const uint16_t GetBulbLabel = 0x17;
		const uint16_t BulbLabel = 0x19;
		const uint16_t GetLightState = 0x65;
		const uint16_t SetLightColorHSL = 0x66;
		const uint16_t SetDim = 0x68;
		const uint16_t SetLightColorRGBW = 0x6a;
		const uint16_t LightStatus = 0x6b;
	}

	struct MacAddress {
		uint8_t address[6];

		MacAddress() {
			for (unsigned i = 0; i <6; ++i) {
				address[i] = 0;
			}
		}

		bool operator==(const MacAddress& other) const {
			for (unsigned i = 0; i <6; ++i) {
				if (address[i] != other.address[i]) {
					return false;
				}
			}
			return true;
		}

		bool IsNull() const {
			for (unsigned i = 0; i <6; ++i) {
				if (address[i] != 0) {
					return false;
				}
			}

			return true;
		}

		std::string ToString() const {
			std::stringstream stream;

			for (unsigned i = 0; i <6; ++i) {
				stream  << std::setw(2) << std::setfill ('0') << std::hex;
				stream << (int) address[i];
				if (i < 5) {
					stream << "::";
				}
			}
			return stream.str();
		}

		void FromString(const std::string& str) {
			unsigned fakeAddr[6];
			sscanf_s(str.c_str(), "%x::%x::%x::%x::%x::%x",
				&fakeAddr[0], &fakeAddr[1], &fakeAddr[2], &fakeAddr[3], &fakeAddr[4], &fakeAddr[5]);
			for (unsigned i = 0; i < 6; ++i) {
				address[i] = ( uint8_t) fakeAddr[i];
			}
		}
	};

	class Packet
	{
		union payload_t {
			Payload::PanGatewayState panGatewayState;
			Payload::WifiInfo wifiInfo;
			Payload::BulbLabel bulbLabel;
			Payload::LightStatus lightStatus;
			Payload::LightColorRGBW lightColorRGBW;
			Payload::LightColorHSL lightColorHSL;
			Payload::SetDim setDim;
		};

		uint16_t size;              // LE
		uint16_t protocol;
		uint32_t reserved1;         // Always 0x0000
		MacAddress   target_mac_address;
		uint16_t reserved2;         // Always 0x00
		MacAddress   site;           // MAC address of gateway PAN controller bulb
		uint16_t reserved3;         // Always 0x00
		uint64_t timestamp;
		uint16_t packet_type;       // LE
		uint16_t reserved4;         // Always 0x0000
		payload_t payload;           // Documented below per packet type

	public:
		Packet() {
			reserved1  = 0;
			reserved2 = 0;
			reserved3 = 0;
			timestamp = 0;
			reserved4 = 0;

			Initialize(PacketType::Invalid);
		}

		uint16_t GetSize() const {
			return size;
		}

		void Initialize(uint16_t type) {
			packet_type = type;
			protocol = Protocol::Send;
			target_mac_address = MacAddress();
			site = MacAddress();
			size = sizeof(Packet) - sizeof(payload_t);
		}

		uint16_t GetType() const {
			return packet_type;
		}

		Payload::PanGatewayState GetPanGatewayState() const {
			assert (GetType() == PacketType::PanGatewayState) ;
			return payload.panGatewayState;
		}

		Payload::BulbLabel GetBulbLabel() const {
			assert(GetType() == PacketType::BulbLabel);
			return payload.bulbLabel;
		}

		Payload::LightStatus GetLightStatus() const {
			assert(GetType() == PacketType::LightStatus);
			return payload.lightStatus;
		}

		void SetLightColorRGBW(const Payload::LightColorRGBW& lightColor) {
			Initialize(PacketType::SetLightColorRGBW);
			size += sizeof(Payload::LightColorRGBW);
			payload.lightColorRGBW = lightColor;
		}

		void SetLightColorHSL(const Payload::LightColorHSL& lightColor) {
			Initialize(PacketType::SetLightColorHSL);
			size += sizeof(Payload::LightColorHSL);
			payload.lightColorHSL = lightColor;
		}

		void SetDim(const Payload::SetDim& setDim) {
			Initialize(PacketType::SetDim);
			size += sizeof(Payload::SetDim);
			payload.setDim = setDim;
		}

		std::string ToString() const {
			std::stringstream ret;
			ret << "size: " << size;
			ret << ", packet type: " << "0x" << std::hex << packet_type << std::dec;
			if (packet_type == PacketType::Invalid) {
				ret << " (invalid packet)";
			} else		if (packet_type == PacketType::GetPanGateway) {
				ret << " (GetPanGateway)";
			}else		if (packet_type == PacketType::PanGatewayState) {
				ret << " (PanGatewayState)";
				ret << ", service: " << (int) payload.panGatewayState.service;
				ret << ", port: " <<  payload.panGatewayState.port;
			} else if (packet_type == PacketType::GetWifiInfo) {
				ret << " (GetWifiInfo)";
			} else if (packet_type == PacketType::WifiInfo) {
				ret << " (WifiInfo), signal: " << payload.wifiInfo.signal;
			} else if (packet_type == PacketType::GetBulbLabel) {
				ret << " (GetBulbLabel)";
			} else if (packet_type == PacketType::BulbLabel) {
				ret << " (BulbLabel), label: " << payload.bulbLabel.label;
			} else if (packet_type == PacketType::LightStatus) {
				ret << " (LightStatus), label: " << payload.lightStatus.bulb_label;
				ret << std::hex;
				ret << ", hue: 0x" << payload.lightStatus.hue;
				ret << ", sat: 0x" << payload.lightStatus.saturation;
				ret << ", brightness: 0x" << payload.lightStatus.brightness;
				ret << ", kelvin: 0x" << payload.lightStatus.kelvin;
				ret << ", dim: 0x" << payload.lightStatus.dim;
				ret << ", power: 0x" << payload.lightStatus.power;
			} else if (packet_type == PacketType::SetLightColorRGBW) {
				ret << " (SetLightColorRGBW)";
			} else if (packet_type == PacketType::SetLightColorHSL) {
				ret << " (SetLightColorHSL)";
			} else if (packet_type == PacketType::GetLightState) {
				ret << " (GetLightState)";
			} else if (packet_type == PacketType::SetDim) {
				ret << " (SetDim)";
			} else {
				ret << " (unknown)";
			}

#if 0
			if (!target_mac_address.IsNull()) {
				ret << ", target mac: " << target_mac_address.ToString();
			}

			if (!site.IsNull()) {
				ret << ", site mac: " << site.ToString();
			}
#endif

			return ret.str();
		}

		MacAddress GetTargetMac() const {
			return target_mac_address;
		}

		void SetTargetMac(const MacAddress& address) {
			target_mac_address = address;
		}

		MacAddress GetSiteMac() const {
			return site;
		}

		void SetSiteMac(const MacAddress& address) {
			site = address;
		}
	};

#pragma pack(pop) 
}