#pragma once

#include "color.h"
#include <minecraft-file.hpp>

extern std::optional<Color> BlockColor(mcfile::je::Block const& block);
