#pragma once
#include <string>
#include <sstream>
#include <cstdint>
#include <iomanip>
#include <cassert>

namespace lifx {

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
	};

	class Packet
	{
		union payload_t {
			Payload::PanGatewayState panGatewayState;
			Payload::WifiInfo wifiInfo;
			Payload::BulbLabel bulbLabel;
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
			if (type == PacketType::PanGatewayState) {
				size += sizeof(Payload::PanGatewayState);
			}
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

		std::string ToString() const {
			std::stringstream ret;
			ret << "size: " << size;
			ret << ", packet type: " << packet_type;
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
			} else {
				ret << " (unknown)";
			}

			if (!target_mac_address.IsNull()) {
				ret << ", target mac: " << target_mac_address.ToString();
			}

			if (!site.IsNull()) {
				ret << ", site mac: " << site.ToString();
			}

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