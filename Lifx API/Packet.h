#pragma once
#include <string>
#include <sstream>
#include <cstdint>
#include <iomanip>

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
	}

	namespace Protocol {
		const uint16_t Send = 0x3400;
	}

	namespace PacketType {
		const uint16_t Invalid = 0xffff;
		const uint16_t GetPanGateway = 0x02;
		const uint16_t PanGatewayState = 0x03;
		const uint16_t GetWifiInfo = 0x10;
	}

	struct MacAddress {
		uint8_t address[6];

		MacAddress() {
			for (unsigned i = 0; i <6; ++i) {
				address[i] = 0;
			}
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
			size = 0;
			protocol = Protocol::Send;
			reserved1  = 0;
			reserved2 = 0;
			reserved3 = 0;
			timestamp = 0;
			reserved4 = 0;

			SetType(PacketType::Invalid);
		}

		uint16_t GetSize() const {
			return size;
		}

		void SetType(uint16_t type) {
			packet_type = type;
			size = sizeof(Packet) - sizeof(payload_t);
			if (type == PacketType::PanGatewayState) {
				size += sizeof(Payload::PanGatewayState);
			}
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

		MacAddress GetSiteMac() const {
			return site;
		}
	};

#pragma pack(pop) 
}