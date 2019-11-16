#include <iostream>
#include "minecraft-file.hpp"
#include <math.h>
#include <set>
#include "svpng.inc"
#include "hwm/task/task_queue.hpp"
#include "colormap/colormap.h"
#include "block_color.h"

using namespace std;
using namespace mcfile;

static std::set<mcfile::blocks::BlockId> const plantBlocks = {
    mcfile::blocks::minecraft::beetroots,
    mcfile::blocks::minecraft::carrots,
    mcfile::blocks::minecraft::potatoes,
    mcfile::blocks::minecraft::seagrass,
    mcfile::blocks::minecraft::tall_seagrass,
    mcfile::blocks::minecraft::fern,
    mcfile::blocks::minecraft::azure_bluet,
    mcfile::blocks::minecraft::kelp,
    mcfile::blocks::minecraft::large_fern,
    mcfile::blocks::minecraft::kelp_plant,
};

static std::set<mcfile::blocks::BlockId> const transparentBlocks = {
    mcfile::blocks::minecraft::air,
    mcfile::blocks::minecraft::cave_air,
    mcfile::blocks::minecraft::vine, // Color(56, 95, 31)}, //
    mcfile::blocks::minecraft::ladder, // Color(255, 255, 255)},
    mcfile::blocks::minecraft::glass_pane,
    mcfile::blocks::minecraft::glass,
    mcfile::blocks::minecraft::brown_wall_banner,
    mcfile::blocks::minecraft::redstone_wall_torch,
    mcfile::blocks::minecraft::wall_torch,
    mcfile::blocks::minecraft::redstone_torch,
    mcfile::blocks::minecraft::torch,
    mcfile::blocks::minecraft::barrier,
    mcfile::blocks::minecraft::black_banner,
    mcfile::blocks::minecraft::black_wall_banner,
    mcfile::blocks::minecraft::black_stained_glass,
    mcfile::blocks::minecraft::black_stained_glass_pane,
    mcfile::blocks::minecraft::blue_banner,
    mcfile::blocks::minecraft::blue_stained_glass,
    mcfile::blocks::minecraft::blue_stained_glass_pane,
    mcfile::blocks::minecraft::blue_wall_banner,
    mcfile::blocks::minecraft::brown_banner,
    mcfile::blocks::minecraft::brown_stained_glass,
    mcfile::blocks::minecraft::brown_stained_glass_pane,
    mcfile::blocks::minecraft::gray_wall_banner,
    mcfile::blocks::minecraft::cyan_banner,
    mcfile::blocks::minecraft::cyan_wall_banner,
    mcfile::blocks::minecraft::cyan_stained_glass,
    mcfile::blocks::minecraft::cyan_stained_glass_pane,
    mcfile::blocks::minecraft::gray_banner,
    mcfile::blocks::minecraft::gray_stained_glass,
    mcfile::blocks::minecraft::gray_stained_glass_pane,
    mcfile::blocks::minecraft::green_banner,
    mcfile::blocks::minecraft::green_stained_glass,
    mcfile::blocks::minecraft::green_stained_glass_pane,
    mcfile::blocks::minecraft::green_wall_banner,
    mcfile::blocks::minecraft::light_blue_banner,
    mcfile::blocks::minecraft::light_blue_stained_glass,
    mcfile::blocks::minecraft::light_blue_stained_glass_pane,
    mcfile::blocks::minecraft::light_blue_wall_banner,
    mcfile::blocks::minecraft::light_gray_banner,
    mcfile::blocks::minecraft::light_gray_stained_glass,
    mcfile::blocks::minecraft::light_gray_stained_glass_pane,
    mcfile::blocks::minecraft::light_gray_wall_banner,
    mcfile::blocks::minecraft::lime_banner,
    mcfile::blocks::minecraft::lime_stained_glass,
    mcfile::blocks::minecraft::lime_stained_glass_pane,
    mcfile::blocks::minecraft::lime_wall_banner,
    mcfile::blocks::minecraft::magenta_banner,
    mcfile::blocks::minecraft::magenta_stained_glass,
    mcfile::blocks::minecraft::magenta_stained_glass_pane,
    mcfile::blocks::minecraft::magenta_wall_banner,
    mcfile::blocks::minecraft::orange_banner,
    mcfile::blocks::minecraft::orange_stained_glass,
    mcfile::blocks::minecraft::orange_stained_glass_pane,
    mcfile::blocks::minecraft::orange_wall_banner,
    mcfile::blocks::minecraft::pink_banner,
    mcfile::blocks::minecraft::pink_stained_glass,
    mcfile::blocks::minecraft::pink_stained_glass_pane,
    mcfile::blocks::minecraft::pink_wall_banner,
    mcfile::blocks::minecraft::purple_banner,
    mcfile::blocks::minecraft::purple_stained_glass,
    mcfile::blocks::minecraft::purple_stained_glass_pane,
    mcfile::blocks::minecraft::purple_wall_banner,
    mcfile::blocks::minecraft::red_banner,
    mcfile::blocks::minecraft::red_stained_glass,
    mcfile::blocks::minecraft::red_stained_glass_pane,
    mcfile::blocks::minecraft::red_wall_banner,
    mcfile::blocks::minecraft::white_banner,
    mcfile::blocks::minecraft::white_stained_glass,
    mcfile::blocks::minecraft::white_stained_glass_pane,
    mcfile::blocks::minecraft::white_wall_banner,
    mcfile::blocks::minecraft::yellow_banner,
    mcfile::blocks::minecraft::yellow_stained_glass,
    mcfile::blocks::minecraft::yellow_stained_glass_pane,
    mcfile::blocks::minecraft::yellow_wall_banner,
    mcfile::blocks::minecraft::void_air,
    mcfile::blocks::minecraft::structure_void,
    mcfile::blocks::minecraft::tripwire,
};

static float LightAt(Chunk const& chunk, int x, int y, int z) {
    int const envLight = 8;
    int const defaultLight = 15;
    if (defaultLight >= 15) {
        return 1;
    }
    
    int const blockLight = std::max(0, chunk.blockLightAt(x, y, z));
    int const skyLight = std::max(0, chunk.skyLightAt(x, y, z));
    float const l = std::min(std::max(blockLight + envLight * (skyLight / 15.f) + defaultLight, 0.f), 15.f) / 15.f;
    return l;
}

struct Options {
    Color waterColor;
    float waterDiffusion;
};

static string RegionFileName(string world, int regionX, int regionZ) {
    ostringstream fileName;
    fileName << world << "/region/r." << regionX << "." << regionZ << ".mca";
    return fileName.str();
}

static void RegionToPng(string world, int regionX, int regionZ, string png) {
    int const width = 513;
    int const height = 513;
    
    vector<uint8_t> heightMap(width * height, 0);
    vector<Color> pixels(width * height, Color::FromFloat(0, 0, 0, 1));
    vector<float> light(width * height, 0);
    uint8_t *heightMapPtr = heightMap.data();
    Color *pixelsPtr = pixels.data();
    float *lightPtr = light.data();

    shared_ptr<Region> region = Region::MakeRegion(RegionFileName(world, regionX, regionZ));
    if (!region) {
        return;
    }

    int const minX = region->minBlockX() - 1;
    int const minZ = region->minBlockZ() - 1;
            
    bool error = false;
    region->loadAllChunks(error, [=](Chunk const& chunk) {
        Color const waterColor(69, 91, 211);
        float const waterDiffusion = 0.02;
        colormap::kbinani::Altitude altitude;
        int const sZ = chunk.minBlockZ();
        int const eZ = chunk.maxBlockZ();
        int const sX = chunk.minBlockX();
        int const eX = chunk.maxBlockX();
        for (int z = sZ; z <= eZ; z++) {
            for (int x = sX; x <= eX; x++) {
                int waterDepth = 0;
                int airDepth = 0;
                Color translucentBlock(0, 0, 0, 0);
                for (int y = 255; y >= 0; y--) {
                    auto block = chunk.blockIdAt(x, y, z);
                    if (!block) {
                        airDepth++;
                        continue;
                    }
                    if (block == blocks::minecraft::water || block == blocks::minecraft::bubble_column) {
                        if (waterDepth == 0) {
                            int const idx = (z - minZ) * width + (x - minX);
                            lightPtr[idx] = LightAt(chunk, x, y + 1, z);
                        }
                        waterDepth++;
                        continue;
                    }
                    if (transparentBlocks.find(block) != transparentBlocks.end()) {
                        airDepth++;
                        continue;
                    }
                    if (plantBlocks.find(block) != plantBlocks.end()) {
                        airDepth++;
                        continue;
                    }
                    Color c(0, 0, 0);
                    if (!BlockColor(block, c)) {
                        cerr << "Unknown block: " << block << endl;
                    } else {
                        int const idx = (z - minZ) * width + (x - minX);

                        Color const opaqeBlockColor = c;
                        Color color(0, 0, 0, 0);
                        if (waterDepth > 0) {
                            color = waterColor.diffuse(waterDiffusion, waterDepth);
                            translucentBlock = Color(0, 0, 0, 0);
                        } else if (block == blocks::minecraft::grass_block) {
                            float const v = std::min(std::max((y - 63.0) / 193.0, 0.0), 1.0);
                            auto c = altitude.getColor(v);
                            color = Color::FromFloat(c.r, c.g, c.b, 1);
                            heightMapPtr[idx] = y;
                            lightPtr[idx] = LightAt(chunk, x, y + 1, z);
                        } else {
                            color = opaqeBlockColor;
                            heightMapPtr[idx] = y;
                            lightPtr[idx] = LightAt(chunk, x, y + 1, z);
                        }
                        pixelsPtr[idx] = Color::Add(color, translucentBlock.withAlphaComponent(0.2));
                        break;
                    }
                }
            }
        }
        return true;
    });

    for (int x = 1; x < width; x++) {
        int z1 = 1;
        int i1 = z1 * width + x;
        int i0 = x;
        heightMap[i0] = heightMap[i1];
    }
    for (int z = 1; z < height; z++) {
        int x1 = 1;
        int i1 = z * width + x1;
        int i0 = (z - 1) * width + x1;
        heightMap[i0] = heightMap[i1];
    }

    shared_ptr<Region> northRegion = Region::MakeRegion(RegionFileName(world, regionX, regionZ - 1));
    if (northRegion) {
        for (int lcx = 0; lcx < 32; lcx++) {
            bool e = false;
            northRegion->loadChunk(lcx, 31, e, [=](Chunk const& chunk) {
                int const z = chunk.maxBlockZ();
                for (int lbx = 0; lbx < 16; lbx++) {
                    int const x = chunk.minBlockX() + lbx;
                    int waterDepth = 0;
                    for (int y = 255; y >= 0; y--) {
                        auto block = chunk.blockIdAt(x, y, z);
                        if (!block) {
                            continue;
                        }
                        if (block == blocks::minecraft::water || block == blocks::minecraft::bubble_column) {
                            waterDepth++;
                            continue;
                        }
                        if (transparentBlocks.find(block) != transparentBlocks.end()) {
                            continue;
                        }
                        if (plantBlocks.find(block) != plantBlocks.end()) {
                            continue;
                        }
                        Color c(0, 0, 0);
                        if (!BlockColor(block, c)) {
                            cerr << "Unknown block: " << block << endl;
                        } else {
                            int const idx = (z - minZ) * width + (x - minX);
                            if (waterDepth == 0) {
                                heightMapPtr[idx] = y;
                            }
                            break;
                        }
                    }
                }
                return true;
            });
        }
    }
    
    shared_ptr<Region> westRegion = Region::MakeRegion(RegionFileName(world, regionX - 1, regionZ));
    if (westRegion) {
        for (int lcz = 0; lcz < 32; lcz++) {
            bool e = false;
            westRegion->loadChunk(31, lcz, e, [=](Chunk const& chunk) {
                int const x = chunk.maxBlockX();
                for (int lbz = 0; lbz < 16; lbz++) {
                    int const z = chunk.minBlockZ() + lbz;
                    int waterDepth = 0;
                    for (int y = 255; y >= 0; y--) {
                        auto block = chunk.blockIdAt(x, y, z);
                        if (!block) {
                            continue;
                        }
                        if (block == blocks::minecraft::water || block == blocks::minecraft::bubble_column) {
                            waterDepth++;
                            continue;
                        }
                        if (transparentBlocks.find(block) != transparentBlocks.end()) {
                            continue;
                        }
                        if (plantBlocks.find(block) != plantBlocks.end()) {
                            continue;
                        }
                        Color c(0, 0, 0);
                        if (!BlockColor(block, c)) {
                            cerr << "Unknown block: " << block << endl;
                        } else {
                            int const idx = (z - minZ) * width + (x - minX);
                            if (waterDepth == 0) {
                                heightMapPtr[idx] = y;
                            }
                            break;
                        }
                    }
                }
                return true;
            });
        }
    }
    
    vector<uint32_t> img(512 * 512, Color(0, 0, 0, 0).color());

    for (int z = 1; z < height; z++) {
        for (int x = 1; x < width; x++) {
            int const idx = z * width + x;
            uint8_t const h = heightMap[idx];
            Color c = pixels[idx];
            Color color = c;

            uint8_t hNorth = heightMap[(z - 1) * width + x];
            uint8_t hWest = heightMap[z * width + x - 1];
            int score = 0; // +: bright, -: dark
            if (hNorth > h) score--;
            if (hNorth < h) score++;
            if (hWest > h) score--;
            if (hWest < h) score++;

            if (score > 0) {
                float coeff = 1.2;
                HSV hsv = color.toHSV();
                hsv.fV = hsv.fV * coeff;
                Color cc = Color::FromHSV(hsv);
                color = cc;
            } else if (score < 0) {
                float coeff = 0.8;
                HSV hsv = color.toHSV();
                hsv.fV = hsv.fV * coeff;
                Color cc = Color::FromHSV(hsv);
                color = cc;
            }

            float const l = light[idx];
            int i = (z - 1) * 512 + (x - 1);
            img[i] = Color::FromFloat(color.fR, color.fG, color.fB, color.fA * l).color();
        }
    }

    FILE *out = fopen(png.c_str(), "wb");
    if (!out) {
        return;
    }
    svpng(out, 512, 512, (unsigned char *)img.data(), 1);
    fclose(out);
}

static void PrintDescription() {
    
}

int main(int argc, char *argv[]) {
    string input;
    string output;

    int opt;
    opterr = 0;
    while ((opt = getopt(argc, argv, "w:o:")) != -1) {
        switch (opt) {
            case 'w':
                input = optarg;
                break;
            case 'o':
                output = optarg;
                break;
            default:
                PrintDescription();
                exit(1);
        }
    }

    World world(input);

    hwm::task_queue q(thread::hardware_concurrency());
    vector<future<void>> futures;

    world.eachRegions([=, &q, &futures](shared_ptr<Region> const& region) {
        futures.emplace_back(q.enqueue([=](shared_ptr<Region> region) {
            ostringstream name;
            name << output << "/r." << region->fX << "." << region->fZ << ".png";
            RegionToPng(input, region->fX, region->fZ, name.str());
        }, region));
    });

    for (auto& f : futures) {
        f.get();
    }

    return 0;
}
