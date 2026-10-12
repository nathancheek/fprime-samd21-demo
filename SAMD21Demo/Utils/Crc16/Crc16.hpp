// ======================================================================
// \title  Crc16.hpp
// \brief  CRC-16/CCITT-FALSE, used by ComSpacePacket
// ======================================================================

#ifndef SAMD21Demo_Crc16_HPP
#define SAMD21Demo_Crc16_HPP

#include "Fw/Types/BasicTypes.hpp"

namespace SAMD21Demo {
namespace Crc16 {

//! Starting value of every CRC
constexpr U16 INITIAL = 0xFFFF;

//! Fold one byte into a CRC-16/CCITT-FALSE (polynomial 0x1021, initial value 0xFFFF, no final XOR), the CCSDS
//! and ECSS PUS packet error control CRC. A nibble table keeps it to 32 bytes instead of the 512-byte table in
//! Utils/Hash/libcrc.
U16 update(U16 crc, U8 byte);

}  // namespace Crc16
}  // namespace SAMD21Demo

#endif
