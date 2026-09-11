#include "Maps.hpp"
#include "../Components/Components.hpp"
#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Vector3, x, y, z)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Color, r, g, b)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Block, id, name, pos, size, color, wireColor)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MapData, format, version, blocks)

namespace Maps {

Entity CreateBlock(Registry &reg, Vector3 pos, Vector3 size, Color color,
                   Color wireColor) {
    Entity block = reg.CreateEntity();
    reg.AddComponent<TransformComponent>(block, {pos.x, pos.y, pos.z});
    reg.AddComponent<ColliderComponent>(block, {size.x, size.y, size.z, true});
    reg.AddComponent<BasicRenderComponent>(
        block, {size.x, size.y, size.z, color, wireColor});
    return block;
}

int LoadMapFromMapData(Registry &registry, MapData mapData) {
    std::string format = mapData.format;

    if (format != "raylib-map-builder") {
        std::cerr << "Error: Invalid map format. Expected "
                     "\"raylib-map-builder\", got \""
                  << format << "\"" << std::endl;
        return 1;
    }

    int version = mapData.version;
    if (version != 1) {
        std::cerr << "Error: Unsupported map version. Expected 1, got "
                  << version << std::endl;
        return 1;
    }

    std::vector<Block> blocks = mapData.blocks;

    registry.Clear();
    for (const auto &block : blocks) {
        float x = block.pos.x;
        float y = block.pos.y - 0.5f;
        float z = block.pos.z;

        float w = block.size.x;
        float h = block.size.y;
        float d = block.size.z;

        Color color = Color{block.color.r, block.color.g, block.color.b, 255};
        Color wireColor =
            Color{block.wireColor.r, block.wireColor.g, block.wireColor.b, 255};

        CreateBlock(registry, {x, y, z}, {w, h, d}, color, wireColor);
    }

    return 0;
}

MapData loadedMapData;

int LoadGameMap(Registry &registry) {
    std::string filename = "maps/map.json";
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return 1;
    }

    try {
        json data = json::parse(file);

        file.close();

        MapData mapData = data.get<MapData>();

        loadedMapData = mapData;

        int loadCode = LoadMapFromMapData(registry, mapData);

        return loadCode;
    } catch (const json::parse_error &e) {
        std::cerr << "JSON Parsing Error: " << e.what() << std::endl;
        return 1;
    } catch (const json::type_error &e) {
        std::cerr << "JSON Type Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

void LoadMap1(Registry &registry) {

    CreateBlock(registry, {0.0f, -1.0f, 0.0f}, {400.0f, 1.0f, 400.0f},
                RAYWHITE);

    CreateBlock(registry, {0.0f, 0.0f, -5.0f}, {4.0f, 0.4f, 4.0f});

    CreateBlock(registry, {5.0f, 0.0f, -5.0f}, {4.0f, 1.0f, 4.0f});

    CreateBlock(registry, {-6.0f, 0.0f, -2.0f}, {1.0f, 3.0f, 8.0f});

    for (int i = 0; i < 100; i++) {
        CreateBlock(registry, {0.0f, 0.0f + i * 0.4f, 5.0f + i * 0.4f},
                    {4.0f, 0.4f, 0.4f}, LIME);
    }

    CreateBlock(registry, {0.0f, 0.0f + 99 * 0.4f, 5.0f + 99 * 0.4f + 2.2f},
                {4.0f, 0.4f, 4.0f});

    CreateBlock(registry, {4.0f, 0.0f + 99 * 0.4f, 5.0f + 99 * 0.4f + 2.2f},
                {4.0f, 0.4f, 4.0f});
    CreateBlock(registry, {-4.0f, 0.0f + 99 * 0.4f, 5.0f + 99 * 0.4f + 2.2f},
                {4.0f, 0.4f, 4.0f});

    CreateBlock(registry, {4.0f, 0.0f + 99 * 0.4f, 1.0f + 99 * 0.4f + 2.2f},
                {4.0f, 0.4f, 4.0f});
    CreateBlock(registry, {-4.0f, 0.0f + 99 * 0.4f, 1.0f + 99 * 0.4f + 2.2f},
                {4.0f, 0.4f, 4.0f});

    CreateBlock(registry, {4.0f, 0.0f + 99 * 0.4f, -3.0f + 99 * 0.4f + 2.2f},
                {4.0f, 0.4f, 4.0f});
    CreateBlock(registry, {-4.0f, 0.0f + 99 * 0.4f, -3.0f + 99 * 0.4f + 2.2f},
                {4.0f, 0.4f, 4.0f});

    CreateBlock(registry, {0.0f, 0.0f + 99 * 0.4f, -3.0f + 99 * 0.4f + 2.2f},
                {4.0f, 0.4f, 4.0f});
}
void LoadMap2(Registry &registry) {
    CreateBlock(registry, {0.0f, -1.0f, -0.1f}, {12.1f, 1.0f, 26.4f},
                {128, 39, 236, 255}, {30, 32, 38, 255});
    CreateBlock(registry, {13.2f, -0.9f, -7.6f}, {7.2f, 1.0f, 7.0f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {20.4f, -0.9f, 6.5f}, {7.2f, 1.0f, 7.0f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {55.0f, 10.7f, -13.4f}, {122.1f, 24.3f, 0.2f},
                {251, 70, 70, 255}, {30, 32, 38, 255});
    CreateBlock(registry, {34.2f, -1.0f, 0.0f}, {12.1f, 1.0f, 26.9f},
                {128, 39, 236, 255}, {30, 32, 38, 255});
    CreateBlock(registry, {93.9f, 10.7f, 13.2f}, {199.8f, 24.3f, 0.5f},
                {255, 51, 51, 255}, {30, 32, 38, 255});
    CreateBlock(registry, {-6.0f, 10.7f, 0.0f}, {0.1f, 24.4f, 26.9f},
                {255, 61, 61, 255}, {30, 32, 38, 255});
    CreateBlock(registry, {50.0f, -0.9f, 5.1f}, {7.2f, 1.0f, 7.0f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {59.6f, -0.9f, -6.5f}, {7.2f, 1.0f, 7.0f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {71.4f, -0.9f, 3.8f}, {7.2f, 1.0f, 7.0f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {87.0f, -1.0f, -0.2f}, {12.0f, 1.0f, 26.5f},
                {128, 39, 236, 255}, {30, 32, 38, 255});
    CreateBlock(registry, {104.9f, -0.9f, 6.8f}, {7.2f, 1.0f, 7.0f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {105.1f, -0.9f, -6.8f}, {7.2f, 1.0f, 7.0f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {122.1f, -1.0f, 0.0f}, {12.1f, 1.0f, 26.8f},
                {128, 39, 236, 255}, {30, 32, 38, 255});
    CreateBlock(registry, {122.1f, -1.0f, -26.4f}, {12.1f, 1.0f, 26.0f},
                {128, 39, 236, 255}, {30, 32, 38, 255});
    CreateBlock(registry, {128.6f, 10.7f, -13.0f}, {1.0f, 24.3f, 52.9f},
                {255, 56, 56, 255}, {30, 32, 38, 255});
    CreateBlock(registry, {75.2f, -1.0f, -26.3f}, {12.0f, 1.0f, 25.9f},
                {128, 39, 236, 255}, {30, 32, 38, 255});
    CreateBlock(registry, {106.4f, -0.9f, -19.8f}, {7.2f, 1.0f, 7.0f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {90.0f, -0.9f, -31.3f}, {7.2f, 1.0f, 7.0f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {60.1f, -0.9f, -31.3f}, {7.2f, 1.0f, 7.0f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {44.8f, -0.9f, -20.6f}, {7.2f, 1.0f, 7.0f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {26.9f, -0.9f, -23.7f}, {7.2f, 1.0f, 7.0f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {12.2f, -0.9f, -30.1f}, {7.2f, 1.0f, 7.0f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-0.5f, -1.0f, -26.3f}, {11.1f, 1.0f, 25.9f},
                {128, 39, 236, 255}, {30, 32, 38, 255});
    CreateBlock(registry, {36.3f, -0.9f, -34.0f}, {7.2f, 1.0f, 7.0f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {47.5f, 10.7f, -39.0f}, {163.1f, 24.3f, 0.8f},
                {251, 55, 55, 255}, {30, 32, 38, 255});
    CreateBlock(registry, {-33.8f, 10.7f, 2.4f}, {0.7f, 24.3f, 83.6f},
                {255, 56, 56, 255}, {30, 32, 38, 255});
    CreateBlock(registry, {-14.1f, -0.9f, -11.5f}, {4.3f, 1.0f, 4.2f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-15.8f, -0.9f, -29.5f}, {7.2f, 1.0f, 7.0f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-27.9f, -0.9f, -23.4f}, {7.2f, 1.0f, 7.0f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-24.0f, -0.9f, -10.5f}, {7.2f, 1.0f, 7.0f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-29.1f, -0.9f, -34.0f}, {4.3f, 1.0f, 4.2f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-16.8f, -0.9f, 3.3f}, {7.2f, 1.0f, 7.0f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-13.1f, -0.9f, 22.5f}, {7.2f, 1.0f, 7.0f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-25.5f, -0.9f, 36.0f}, {7.2f, 1.0f, 7.0f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-25.6f, -0.9f, 14.7f}, {4.3f, 1.0f, 4.2f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {3.5f, -0.9f, 34.9f}, {7.2f, 1.0f, 7.0f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {80.0f, 10.7f, 43.8f}, {228.1f, 24.3f, 0.8f},
                {255, 56, 56, 255}, {30, 32, 38, 255});
    CreateBlock(registry, {17.9f, -0.9f, 23.1f}, {7.2f, 1.0f, 7.0f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {3.2f, -0.9f, 18.4f}, {4.3f, 1.0f, 4.2f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-10.0f, -0.9f, 34.2f}, {4.3f, 1.0f, 4.2f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {34.2f, -1.0f, 29.0f}, {12.1f, 1.0f, 30.9f},
                {128, 39, 236, 255}, {30, 32, 38, 255});
    CreateBlock(registry, {21.5f, -0.9f, 37.0f}, {4.3f, 1.0f, 4.2f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {54.3f, -0.9f, 21.6f}, {9.6f, 1.0f, 9.9f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {96.4f, -0.9f, 22.0f}, {9.6f, 1.0f, 9.9f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {113.1f, -0.9f, 32.5f}, {9.6f, 1.0f, 9.9f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {128.5f, -0.9f, 22.0f}, {9.6f, 1.0f, 9.9f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {52.9f, -0.9f, 37.0f}, {4.3f, 1.0f, 4.2f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {92.8f, -0.9f, 37.0f}, {4.3f, 1.0f, 4.2f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {76.1f, -0.9f, 20.8f}, {4.3f, 1.0f, 4.2f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {111.2f, -0.9f, 18.5f}, {4.3f, 1.0f, 4.2f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {174.4f, -1.0f, 28.5f}, {38.3f, 1.0f, 30.6f},
                {103, 228, 94, 255}, {30, 32, 38, 255});
    CreateBlock(registry, {193.8f, 10.6f, 28.4f}, {0.5f, 24.4f, 30.8f},
                {46, 255, 53, 255}, {30, 32, 38, 255});
    CreateBlock(registry, {146.4f, -0.9f, 34.9f}, {9.6f, 1.0f, 9.9f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {145.2f, -0.9f, 22.6f}, {4.3f, 1.0f, 4.2f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {131.7f, -0.9f, 35.8f}, {4.3f, 1.0f, 4.2f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {48.2f, -0.9f, -33.0f}, {4.3f, 1.0f, 4.2f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {57.2f, -0.9f, -19.2f}, {4.3f, 1.0f, 4.2f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {14.6f, -0.9f, -17.6f}, {4.3f, 1.0f, 4.2f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {24.2f, -0.9f, -34.7f}, {2.9f, 1.0f, 2.9f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-28.7f, -0.9f, 0.6f}, {2.9f, 1.0f, 2.9f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-22.8f, -0.9f, 26.5f}, {2.9f, 1.0f, 2.9f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-10.7f, -0.9f, 13.0f}, {2.9f, 1.0f, 2.9f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {75.8f, -0.9f, 34.6f}, {9.6f, 1.0f, 9.9f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
}

void LoadMap3(Registry &registry) {
    CreateBlock(registry, {-0.6f, -1.0f, 0.1f}, {45.4f, 1.0f, 42.0f},
                {0, 107, 37, 255}, {30, 32, 38, 255});
    CreateBlock(registry, {-0.6f, 12.4f, -7.1f}, {45.4f, 25.8f, 0.1f},
                {15, 15, 15, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-0.6f, 12.4f, 6.1f}, {45.4f, 25.8f, 0.1f},
                {15, 15, 15, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-23.3f, 12.4f, -0.5f}, {0.1f, 25.8f, 13.3f},
                {15, 15, 15, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {22.1f, 12.4f, -0.5f}, {0.1f, 25.8f, 13.3f},
                {15, 15, 15, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-4.1f, 12.4f, -0.5f}, {0.1f, 25.8f, 13.3f},
                {15, 15, 15, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {6.8f, 0.3f, -0.8f}, {2.8f, 1.5f, 2.8f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {13.1f, 1.1f, 2.6f}, {2.8f, 3.2f, 2.8f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {15.9f, 1.9f, -3.2f}, {2.8f, 4.8f, 2.8f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {21.6f, 5.5f, 2.8f}, {0.7f, 1.0f, 6.5f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {15.9f, 7.3f, 5.5f}, {2.9f, 0.9f, 1.0f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {8.6f, 8.5f, 5.5f}, {3.4f, 0.9f, 1.0f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {0.7f, 8.5f, 5.6f}, {3.8f, 0.9f, 1.0f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-2.6f, 8.5f, -0.5f}, {2.9f, 0.9f, 13.1f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {5.0f, 10.3f, -5.5f}, {3.2f, 0.9f, 3.1f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {6.6f, 13.4f, -5.5f}, {0.1f, 7.1f, 3.1f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {8.2f, 10.3f, -5.5f}, {3.2f, 0.9f, 3.1f},
                {180, 180, 190, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {14.5f, 2.2f, -4.0f}, {0.2f, 0.9f, 0.1f},
                {5, 5, 5, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {14.5f, 1.8f, -3.7f}, {0.2f, 0.1f, 0.5f},
                {5, 5, 5, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {14.5f, 2.0f, -3.4f}, {0.2f, 0.5f, 0.1f},
                {5, 5, 5, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {14.5f, 2.2f, -3.7f}, {0.2f, 0.1f, 0.5f},
                {5, 5, 5, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {14.5f, 2.6f, -3.7f}, {0.2f, 0.1f, 0.5f},
                {5, 5, 5, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {14.5f, 2.2f, -3.0f}, {0.2f, 0.9f, 0.1f},
                {5, 5, 5, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {14.5f, 1.8f, -2.7f}, {0.2f, 0.1f, 0.5f},
                {5, 5, 5, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {14.5f, 2.4f, -2.2f}, {0.2f, 0.5f, 0.1f},
                {5, 5, 5, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {14.5f, 1.9f, -2.2f}, {0.2f, 0.1f, 0.1f},
                {5, 5, 5, 255}, {40, 40, 48, 255});
}

void LoadMap4(Registry &registry) {
    CreateBlock(registry, {-15.5f, 4.0f, -0.3f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-17.0f, 4.0f, -1.7f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-16.2f, 4.1f, -1.0f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-15.4f, 8.5f, 0.6f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-15.2f, 8.4f, 1.5f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-15.1f, 4.0f, 2.4f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-15.1f, 4.0f, 3.2f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-15.7f, 4.0f, 3.6f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 80, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-16.3f, 4.0f, 4.2f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-17.0f, 4.0f, 4.2f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-17.0f, 4.0f, 4.2f}, {0.9f, 8.3f, 1.0f},
                {220, 90, 80, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-17.7f, 4.0f, -2.1f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-18.4f, 4.0f, -2.6f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-19.3f, 4.0f, -2.7f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-20.1f, 4.0f, -2.7f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-20.8f, 4.0f, -2.7f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-21.6f, 4.0f, -2.7f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-22.3f, 4.0f, -2.4f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-23.0f, 4.0f, -1.7f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-23.8f, 4.0f, -1.2f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-24.0f, 4.0f, -0.4f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-24.0f, 4.0f, 0.3f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-24.0f, 4.0f, 0.8f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-24.0f, 4.0f, 1.5f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-17.3f, 4.0f, 4.7f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-17.8f, 4.0f, 5.3f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-18.5f, 4.0f, 5.6f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-19.0f, 4.0f, 5.9f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-19.7f, 4.0f, 5.9f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-20.5f, 4.0f, 6.0f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-21.3f, 4.0f, 6.0f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-21.9f, 4.0f, 5.5f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-22.7f, 4.0f, 5.1f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-23.3f, 4.0f, 4.7f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-23.9f, 4.0f, 4.0f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-23.9f, 4.0f, 2.7f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-24.2f, 4.0f, 2.3f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-24.1f, 4.0f, 3.3f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-16.3f, 8.9f, -1.0f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-16.7f, 8.9f, -1.4f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-17.2f, 8.9f, -2.2f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-18.3f, 8.9f, -2.4f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-19.3f, 8.9f, -2.6f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-19.9f, 8.9f, -2.6f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-20.7f, 8.9f, -2.6f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-21.3f, 8.9f, -2.6f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-23.8f, 8.9f, -1.5f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-22.8f, 8.9f, -2.2f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-23.2f, 8.9f, -2.0f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-22.3f, 8.9f, -2.0f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-24.2f, 8.9f, -0.6f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-24.5f, 8.9f, 0.4f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-24.5f, 8.9f, 1.4f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-24.5f, 8.9f, 2.1f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-24.1f, 8.9f, 3.2f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-23.8f, 8.9f, 4.0f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-23.5f, 8.9f, 4.4f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-22.8f, 8.9f, 5.2f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-21.9f, 8.9f, 5.8f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-21.0f, 8.9f, 6.3f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-20.1f, 8.9f, 6.6f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-19.2f, 8.9f, 6.6f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-18.3f, 8.9f, 6.2f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-17.4f, 8.9f, 5.6f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-16.5f, 8.9f, 4.9f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-15.6f, 8.9f, 4.0f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-15.6f, 8.9f, 3.2f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-15.2f, 8.9f, 3.2f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-14.8f, 8.9f, 2.3f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-14.5f, 8.9f, 1.7f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-15.5f, 8.9f, 0.1f}, {0.9f, 8.3f, 1.0f},
                {255, 178, 38, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-116.1f, 43.6f, -3.9f}, {1.0f, 171.4f, 184.3f},
                {0, 255, 251, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {21.3f, 43.6f, -184.6f}, {270.0f, 171.4f, 184.3f},
                {0, 255, 251, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-115.8f, 43.6f, -150.7f}, {0.1f, 171.4f, 107.3f},
                {0, 255, 251, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {291.3f, 43.6f, -28.4f}, {270.0f, 171.4f, 184.3f},
                {0, 255, 251, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {263.1f, 43.6f, 143.9f}, {270.0f, 171.4f, 184.3f},
                {0, 255, 251, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {15.3f, 43.6f, 143.9f}, {337.3f, 171.4f, 184.3f},
                {0, 255, 251, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {352.6f, 43.6f, 143.9f}, {337.3f, 171.4f, 184.3f},
                {0, 255, 251, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {689.9f, 43.6f, 143.9f}, {337.3f, 171.4f, 184.3f},
                {0, 255, 251, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {1027.2f, 43.6f, 143.9f}, {337.3f, 171.4f, 184.3f},
                {0, 255, 251, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {82.2f, -1.0f, -35.1f}, {425.5f, 1.0f, 169.1f},
                {213, 234, 153, 255}, {30, 32, 38, 255});
    CreateBlock(registry, {-15.0f, 2.3f, 1.1f}, {1.0f, 5.0f, 2.4f},
                {12, 127, 176, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-14.9f, 2.9f, 1.2f}, {1.0f, 1.1f, 1.0f},
                {3, 52, 73, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-14.8f, 8.3f, 4.1f}, {1.0f, 1.8f, 1.6f},
                {45, 164, 215, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-16.4f, 3.4f, -1.7f}, {1.0f, 1.8f, 1.6f},
                {45, 164, 215, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-16.6f, 13.3f, -0.4f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-113.6f, 52.9f, 0.0f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-15.9f, 13.4f, 0.3f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-16.4f, 14.0f, 0.0f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-16.4f, 14.0f, 1.8f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-17.3f, 14.5f, 4.5f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-115.1f, 43.6f, -3.9f}, {1.0f, 171.4f, 184.3f},
                {0, 255, 251, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-114.1f, 43.6f, -3.9f}, {1.0f, 171.4f, 184.3f},
                {0, 255, 251, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-16.3f, 14.2f, 1.9f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-16.4f, 14.3f, 0.9f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-17.2f, 15.0f, 0.1f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-17.6f, 15.2f, 0.7f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-17.3f, 15.2f, 1.7f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-16.9f, 14.5f, 3.5f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-16.3f, 14.3f, 2.8f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-15.6f, 13.5f, 2.5f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-16.1f, 13.4f, 1.7f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-16.2f, 13.5f, 3.5f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-16.7f, 14.5f, 4.1f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-17.2f, 13.5f, 4.6f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-16.3f, 13.7f, 4.1f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-17.2f, 15.2f, 2.6f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-15.4f, 13.3f, 1.8f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-17.2f, 14.4f, -0.6f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-17.2f, 15.2f, 3.3f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-18.2f, 13.4f, 6.1f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-18.8f, 13.4f, 6.1f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-20.0f, 13.4f, 6.1f}, {1.3f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-20.5f, 14.3f, 5.2f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-21.3f, 13.5f, -2.3f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-22.4f, 14.4f, -1.2f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-23.4f, 14.2f, -0.3f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-22.6f, 15.1f, -0.4f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-23.5f, 14.3f, 0.8f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-23.6f, 14.4f, 1.8f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-20.2f, 15.8f, 1.0f}, {1.0f, 1.0f, 1.0f},
                {112, 250, 0, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-18.0f, 14.3f, -1.2f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-19.5f, 14.3f, 5.2f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-18.5f, 14.3f, 5.2f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-21.1f, 13.5f, 6.0f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-20.3f, 13.5f, -2.3f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-19.3f, 13.5f, -2.3f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-18.3f, 13.5f, -2.3f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-17.3f, 13.5f, -1.5f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-21.5f, 14.3f, -1.2f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-20.5f, 14.3f, -1.2f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-19.5f, 14.3f, -1.2f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-18.5f, 14.3f, -1.2f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-15.6f, 13.3f, 1.1f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-22.0f, 13.5f, 5.4f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-22.8f, 13.5f, 4.7f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-23.1f, 13.5f, 3.7f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-23.8f, 13.5f, 2.8f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-23.8f, 13.5f, 0.8f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-23.8f, 13.5f, 1.8f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-23.5f, 13.5f, 0.0f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-23.4f, 13.5f, -0.9f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-22.4f, 13.5f, -1.3f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-21.6f, 15.1f, -0.4f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-20.6f, 15.1f, -0.4f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-19.6f, 15.1f, -0.4f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-18.6f, 15.1f, -0.4f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-17.6f, 15.1f, -0.4f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-21.5f, 14.3f, 5.2f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-22.3f, 14.3f, 4.4f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-22.2f, 15.2f, 3.7f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-21.3f, 15.2f, 4.3f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-20.3f, 15.2f, 4.3f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-19.3f, 15.2f, 4.3f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-18.3f, 15.2f, 4.3f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-17.3f, 15.2f, 4.3f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-22.3f, 15.2f, 2.6f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-21.3f, 15.2f, 2.6f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-20.3f, 15.2f, 2.6f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-19.3f, 15.2f, 2.6f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-22.4f, 15.3f, 1.6f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-21.4f, 15.3f, 1.6f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-20.4f, 15.3f, 1.6f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-19.4f, 15.3f, 1.6f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-18.4f, 15.3f, 1.6f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-17.4f, 15.3f, 1.6f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-22.6f, 15.3f, 0.7f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-21.6f, 15.3f, 0.7f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-20.6f, 15.3f, 0.7f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-19.6f, 15.3f, 0.7f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-18.6f, 15.3f, 0.7f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-17.6f, 15.3f, 0.7f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-22.6f, 15.3f, 0.2f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-21.6f, 15.3f, 0.2f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-20.6f, 15.3f, 0.2f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-19.6f, 15.3f, 0.2f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-18.6f, 15.3f, 0.2f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-17.6f, 15.3f, 0.2f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-18.3f, 15.2f, 2.6f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-21.2f, 15.2f, 3.7f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-20.2f, 15.2f, 3.7f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-19.2f, 15.2f, 3.7f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-18.2f, 15.2f, 3.7f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-22.1f, 15.2f, 3.2f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-21.1f, 15.2f, 3.2f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-20.1f, 15.2f, 3.2f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-19.1f, 15.2f, 3.2f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-18.1f, 15.2f, 3.2f}, {1.0f, 1.0f, 1.0f},
                {255, 178, 38, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-20.2f, 17.1f, 1.0f}, {1.0f, 2.6f, 1.0f},
                {112, 250, 0, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-20.2f, 18.8f, 0.3f}, {1.0f, 1.0f, 1.0f},
                {112, 250, 0, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-19.2f, 15.8f, 1.6f}, {1.0f, 1.0f, 1.0f},
                {112, 250, 0, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-19.2f, 17.1f, 1.6f}, {1.0f, 3.5f, 1.0f},
                {112, 250, 0, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-18.8f, 19.0f, 1.6f}, {1.0f, 1.0f, 1.0f},
                {112, 250, 0, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-20.0f, 17.1f, 2.5f}, {1.0f, 3.5f, 1.0f},
                {112, 250, 0, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-20.7f, 19.0f, 2.9f}, {1.0f, 1.0f, 1.0f},
                {112, 250, 0, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-21.2f, 17.2f, 1.6f}, {1.0f, 3.5f, 1.0f},
                {112, 250, 0, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-21.8f, 19.0f, 1.3f}, {1.0f, 1.0f, 1.0f},
                {112, 250, 0, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-113.6f, 53.5f, -0.8f}, {1.0f, 1.0f, 1.0f},
                {92, 92, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-113.5f, 54.2f, -1.5f}, {1.0f, 1.0f, 1.0f},
                {67, 65, 210, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-113.5f, 54.8f, -2.4f}, {1.0f, 1.0f, 1.0f},
                {67, 65, 210, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-113.4f, 55.5f, -3.0f}, {1.0f, 1.0f, 1.0f},
                {67, 65, 210, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-113.4f, 56.2f, -3.7f}, {1.0f, 1.0f, 1.0f},
                {67, 65, 210, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-113.6f, 52.3f, 0.5f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-113.3f, 51.9f, 1.3f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-113.2f, 51.4f, 1.7f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-113.2f, 51.0f, 2.2f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-113.2f, 51.4f, 3.1f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-113.1f, 52.0f, 3.6f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-113.2f, 52.5f, 4.0f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-113.6f, 52.9f, 4.5f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-113.4f, 75.6f, 4.5f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-113.5f, 53.7f, 5.5f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-113.7f, 54.6f, 6.3f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-113.0f, 50.1f, 1.6f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-113.4f, 49.5f, 1.0f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-113.0f, 49.0f, 0.3f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-112.9f, 48.3f, -0.4f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-112.8f, 47.7f, -1.5f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-113.0f, 50.1f, 3.1f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-113.0f, 49.1f, 3.5f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-112.9f, 48.1f, 4.2f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-112.9f, 46.8f, 4.8f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-113.6f, 74.8f, 5.1f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-113.6f, 74.0f, 5.8f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-113.7f, 73.0f, 6.5f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-113.8f, 71.9f, 7.2f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-113.7f, 71.3f, 7.9f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-113.7f, 76.5f, 3.8f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-114.1f, 77.4f, 3.2f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-113.8f, 77.4f, 2.8f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-113.5f, 78.0f, 2.1f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-113.1f, 74.7f, 3.9f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-113.3f, 76.3f, 5.1f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-113.0f, 77.3f, 5.5f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-113.3f, 78.1f, 6.4f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
    CreateBlock(registry, {-112.9f, 73.8f, 2.9f}, {1.0f, 1.0f, 1.0f},
                {117, 117, 255, 255}, {40, 40, 48, 255});
}

} // namespace Maps
