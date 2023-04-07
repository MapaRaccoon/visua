#pragma once

#include <bitset>
#include <cstdint>

namespace fnx
{

// Constants
const size_t max_num_components = 255;
const size_t max_num_entities   = 65535;

// Types
//

using Entity         = std::uint16_t;
using ComponentID    = std::uint8_t;
using Signature      = std::bitset< max_num_components >;
using ComponentGroup = std::size_t;

} // namespace ecs

namespace guygame
{

using BlockID = std::uint8_t;

}
