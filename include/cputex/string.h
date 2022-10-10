#pragma once

#include <cputex/definitions.h>
#include <string_view>

namespace cputex
{
[[nodiscard]] constexpr std::string_view toString(TextureDimension dimension)
{
	switch (dimension)
	{
	case cputex::TextureDimension::Texture1D:
		return "1D";
	case cputex::TextureDimension::Texture2D:
		return "2D";
	case cputex::TextureDimension::Texture3D:
		return "3D";
	case cputex::TextureDimension::TextureCube:
		return "Cube";
	default:
		return "Unknown TextureDimension";
	}
}
}