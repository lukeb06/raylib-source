#pragma once
#include "../Registry/Registry.hpp"
#include "raylib.h"

namespace Maps {
Entity CreateBlock(Registry &reg, Vector3 pos, Vector3 size, Color color = GRAY,
                   Color wireColor = DARKGRAY);
void LoadMap1(Registry &registry);
void LoadMap2(Registry &registry);
} // namespace Maps
