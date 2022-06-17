#pragma once

#include "color.h"
#include <minecraft-file.hpp>

std::optional<Color> BlockColor(mcfile::je::Block const& block);
bool IsPlantBlock(mcfile::blocks::BlockId);
bool IsTransparentBlock(mcfile::blocks::BlockId);
