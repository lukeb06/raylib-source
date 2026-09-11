#pragma once
#include "../Registry/Registry.hpp"
#include "raylib.h"
#include <string>

struct Block {
    int id;
    std::string name;
    Vector3 pos;
    Vector3 size;
    Color color;
    Color wireColor;
};

struct MapData {
    std::string format;
    int version;
    std::vector<Block> blocks;
};

namespace Maps {
Entity CreateBlock(Registry &reg, Vector3 pos, Vector3 size, Color color = GRAY,
                   Color wireColor = DARKGRAY);

extern MapData loadedMapData;
int LoadMapFromMapData(Registry &registry, MapData mapData);
int LoadGameMap(Registry &registry);
void LoadMap1(Registry &registry);
void LoadMap2(Registry &registry);
void LoadMap3(Registry &registry);
void LoadMap4(Registry &registry);
} // namespace Maps
