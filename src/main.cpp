#include <iostream>
#include "minecraft-file.hpp"
#include <math.h>
#include <set>
#include "svpng.inc"
#include "hwm/task/task_queue.hpp"
#include "colormap/colormap.h"

using namespace std;
using namespace mcfile;

struct HSV {
    float fH;
    float fS;
    float fV;
};


class Color {
public:
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
        : fR(r / 255.f)
        , fG(g / 255.f)
        , fB(b / 255.f)
        , fA(a / 255.f)
    {
    }

    uint32_t premultipliedColor() const {
        return ((uint32_t)0xFF << 24) | ((uint32_t)ToU8(fB * fA) << 16) | ((uint32_t)ToU8(fG * fA) << 8) | (uint32_t)ToU8(fR * fA);
    }

    uint32_t color() const {
        return ((uint32_t)ToU8(fA) << 24) | ((uint32_t)ToU8(fB) << 16) | ((uint32_t)ToU8(fG) << 8) | (uint32_t)ToU8(fR);
    }

    Color diffuse(float diffusion, float distance) const {
        float intensity = pow(10., -diffusion * distance);
        return FromFloat(fR, fG, fB, fA * intensity);
    }

    Color withAlphaComponent(float a) const {
        return FromFloat(fR, fG, fB, a);
    }

    static Color Add(Color a, Color b) {
        return Color(a.fR * a.fA + b.fR * b.fA,
                     a.fG * a.fA + b.fG * b.fA,
                     a.fB * a.fA + b.fB * b.fA,
                     1., true);
    }

    static Color FromFloat(float r, float g, float b, float a) {
        return Color(r, g, b, a, true);
    }

    HSV toHSV() const {
        float r = fR;
        float g = fG;
        float b = fB;
        float max = r > g ? r : g;
        max = max > b ? max : b;
        float min = r < g ? r : g;
        min = min < b ? min : b;
        float h = max - min;
        if (h > 0.0f) {
            if (max == r) {
                h = (g - b) / h;
                if (h < 0.0f) {
                    h += 6.0f;
                }
            } else if (max == g) {
                h = 2.0f + (b - r) / h;
            } else {
                h = 4.0f + (r - g) / h;
            }
        }
        h /= 6.0f;
        float s = (max - min);
        if (max != 0.0f)
            s /= max;
        float v = max;
        HSV ret;
        ret.fH = h;
        ret.fS = s;
        ret.fV = v;
        return ret;
    }

    static Color FromHSV(HSV c) {
        float v = c.fV;
        float s = c.fS;
        float h = c.fH;
        float r = v;
        float g = v;
        float b = v;
        if (s > 0.0) {
            h *= 6.0;
            int i = (int) h;
            float f = h - (float) i;
            switch (i) {
                default:
                case 0:
                    g *= 1 - s * (1 - f);
                    b *= 1 - s;
                    break;
                case 1:
                    r *= 1 - s * f;
                    b *= 1 - s;
                    break;
                case 2:
                    r *= 1 - s;
                    b *= 1 - s * (1 - f);
                    break;
                case 3:
                    r *= 1 - s;
                    g *= 1 - s * f;
                    break;
                case 4:
                    r *= 1 - s * (1 - f);
                    g *= 1 - s;
                    break;
                case 5:
                    g *= 1 - s;
                    b *= 1 - s * f;
                    break;
            }
        }
        return FromFloat(r, g, b, 1);
    }

    std::string toString() const {
        std::ostringstream ss;
        ss << "{R: " << fR << ", G: " << fG << ", B: " << fB << ", A: " << fA << "}";
        return ss.str();
    }

private:
    static uint8_t ToU8(float v) {
        float vv = v * 255;
        if (vv < 0) {
            return 0;
        } else if (255 < vv) {
            return 255;
        } else {
            return (uint8_t)vv;
        }
    }

    Color(float r, float g, float b, float a, bool)
        : fR(r)
        , fG(g)
        , fB(b)
        , fA(a)
    {
    }

public:
    float fR;
    float fG;
    float fB;
    float fA;
};

static map<blocks::BlockId, Color> const blockToColor {
    {blocks::minecraft::stone, Color(111, 111, 111)},
    {blocks::minecraft::granite, Color(149, 108, 76)},
    {blocks::minecraft::diorite, Color(252, 249, 242)},
    {blocks::minecraft::andesite, Color(111, 111, 111)},
    {blocks::minecraft::chest, Color(141, 118, 71)},
    {blocks::minecraft::clay, Color(162, 166, 182)},
    {blocks::minecraft::coal_ore, Color(111, 111, 111)},
    {blocks::minecraft::cobblestone, Color(111, 111, 111)},
    {blocks::minecraft::dirt, Color(149, 108, 76)},
    {blocks::minecraft::brown_mushroom, Color(0, 123, 0)},
    {blocks::minecraft::grass_block, Color(130, 148, 58)},
    {blocks::minecraft::iron_ore, Color(111, 111, 111)},
    {blocks::minecraft::sand, Color(201,192,154)}, //
    {blocks::minecraft::oak_leaves, Color(56, 95, 31)}, //
    {blocks::minecraft::jungle_leaves, Color(56, 95, 31)}, //
    {blocks::minecraft::birch_leaves, Color(67, 124, 37)},
    {blocks::minecraft::red_mushroom, Color(0, 123, 0)},
    {blocks::minecraft::mossy_cobblestone, Color(111, 111, 111)},
    {blocks::minecraft::oak_stairs, Color(127, 85, 48)},
    {blocks::minecraft::gravel, Color(111, 111, 111)},
    {blocks::minecraft::oak_log, Color(141, 118, 71)},
    {blocks::minecraft::oak_planks, Color(127, 85, 48)},
    {blocks::minecraft::wall_torch, Color(255, 255, 255)},
    {blocks::minecraft::farmland, Color(149, 108, 76)},
    {blocks::minecraft::oak_fence, Color(127, 85, 48)},
    {blocks::minecraft::cobblestone_stairs, Color(111, 111, 111)},
    {blocks::minecraft::black_wool, Color(25, 25, 25)},
    {blocks::minecraft::grass_path, Color(204, 204, 204)}, //
    {blocks::minecraft::birch_fence, Color(244, 230, 161)},
    {blocks::minecraft::birch_planks, Color(244, 230, 161)},
    {blocks::minecraft::birch_stairs, Color(244, 230, 161)},
    {blocks::minecraft::dark_oak_fence, Color(101, 75, 50)},
    {blocks::minecraft::dark_oak_log, Color(101, 75, 50)},
    {blocks::minecraft::dark_oak_planks, Color(191,152,63)}, //
    {blocks::minecraft::dark_oak_slab, Color(101, 75, 50)},
    {blocks::minecraft::dark_oak_stairs, Color(101, 75, 50)},
    {blocks::minecraft::dark_oak_trapdoor, Color(141, 118, 71)},
    {blocks::minecraft::diamond_ore, Color(111, 111, 111)},
    {blocks::minecraft::gold_ore, Color(111, 111, 111)},
    {blocks::minecraft::ice, Color(158, 158, 252)},
    {blocks::minecraft::jungle_fence, Color(149, 108, 76)},
    {blocks::minecraft::jungle_log, Color(149, 108, 76)},
    {blocks::minecraft::jungle_planks, Color(149, 108, 76)},
    {blocks::minecraft::jungle_slab, Color(149, 108, 76)},
    {blocks::minecraft::jungle_stairs, Color(149, 108, 76)},
    {blocks::minecraft::jungle_trapdoor, Color(141, 118, 71)},
    {blocks::minecraft::lapis_ore, Color(111, 111, 111)},
    {blocks::minecraft::lava, Color(179, 71, 3)},
    {blocks::minecraft::oak_door, Color(141, 118, 71)},
    {blocks::minecraft::oak_slab, Color(127, 85, 48)},
    {blocks::minecraft::oak_trapdoor, Color(141, 118, 71)},
    {blocks::minecraft::obsidian, Color(25, 25, 25)},
    {blocks::minecraft::packed_ice, Color(158, 158, 252)},
    {blocks::minecraft::polished_granite, Color(149, 108, 76)},
    {blocks::minecraft::prismarine, Color(75, 125, 151)},
    {blocks::minecraft::prismarine_bricks, Color(91, 216, 210)},
    {blocks::minecraft::rail, Color(255, 255, 255)},
    {blocks::minecraft::redstone_ore, Color(111, 111, 111)},
    {blocks::minecraft::sandstone, Color(244, 230, 161)},
    {blocks::minecraft::sea_lantern, Color(252, 249, 242)},
    {blocks::minecraft::snow, Color(229, 229, 229)}, //
    {blocks::minecraft::snow_block, Color(252, 252, 252)},
    {blocks::minecraft::spruce_door, Color(141, 118, 71)},
    {blocks::minecraft::spruce_fence, Color(141, 118, 71)},
    {blocks::minecraft::spruce_leaves, Color(56, 95, 31)}, //
    {blocks::minecraft::stone_brick_stairs, Color(111, 111, 111)},
    {blocks::minecraft::stone_bricks, Color(111, 111, 111)},
    {blocks::minecraft::stone_slab, Color(111, 111, 111)},
    {blocks::minecraft::spruce_log, Color(141, 118, 71)},
    {blocks::minecraft::spruce_planks, Color(127, 85, 48)},
    {blocks::minecraft::spruce_slab, Color(127, 85, 48)},
    {blocks::minecraft::spruce_stairs, Color(141, 118, 71)},
    {blocks::minecraft::spruce_trapdoor, Color(141, 118, 71)},
    {blocks::minecraft::mossy_stone_bricks, Color(111, 111, 111)},
    {blocks::minecraft::chiseled_stone_bricks, Color(111, 111, 111)},
    {blocks::minecraft::cracked_stone_bricks, Color(111, 111, 111)},
    {blocks::minecraft::infested_stone, Color(111, 111, 111)},
    {blocks::minecraft::cobweb, Color(255, 255, 255)},
    {blocks::minecraft::blue_ice, Color(102, 151, 246)},
    {blocks::minecraft::magma_block, Color(181, 64, 9)},

    // plants
    {blocks::minecraft::lily_pad, Color(0, 123, 0)},
    {blocks::minecraft::wheat, Color(0, 123, 0)},
    {blocks::minecraft::melon, Color(125, 202, 25)},
    {blocks::minecraft::pumpkin, Color(213, 125, 50)},
    {blocks::minecraft::grass, Color(109, 141, 35)},
    {blocks::minecraft::tall_grass, Color(109, 141, 35)},
    {blocks::minecraft::dandelion, Color(245, 238, 50)},
    {blocks::minecraft::poppy, Color(229, 31, 29)},
    {blocks::minecraft::peony, Color(232, 143, 213)},
    {blocks::minecraft::pink_tulip, Color(234, 182, 209)},
    {blocks::minecraft::orange_tulip, Color(242, 118, 33)},
    {blocks::minecraft::lilac, Color(212, 119, 197)},
    {blocks::minecraft::sunflower, Color(245, 238, 50)},
    {blocks::minecraft::allium, Color(200, 109, 241)},
    {blocks::minecraft::red_tulip, Color(229, 31, 29)},
    {blocks::minecraft::white_tulip, Color(255, 255, 255)},
    {blocks::minecraft::rose_bush, Color(136, 40, 27)},
    {blocks::minecraft::blue_orchid, Color(47, 181, 199)},
    {blocks::minecraft::oxeye_daisy, Color(236, 246, 247)},
    {blocks::minecraft::sugar_cane, Color(165, 214, 90)},
};

static set<blocks::BlockId> plantBlocks = {
    blocks::minecraft::beetroots,
    blocks::minecraft::carrots,
    blocks::minecraft::potatoes,
    blocks::minecraft::seagrass,
    blocks::minecraft::tall_seagrass,
    blocks::minecraft::fern,
    blocks::minecraft::azure_bluet,
    blocks::minecraft::kelp,
    blocks::minecraft::large_fern,
    blocks::minecraft::kelp_plant,
};

static set<blocks::BlockId> transparentBlocks = {
    blocks::minecraft::air,
    blocks::minecraft::cave_air,
    blocks::minecraft::vine, // Color(56, 95, 31)}, //
    blocks::minecraft::ladder, // Color(255, 255, 255)},
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
                    auto it = blockToColor.find(block);
                    if (it == blockToColor.end()) {
                        cerr << "Unknown block: " << block << endl;
                    } else {
                        int const idx = (z - minZ) * width + (x - minX);

                        Color const opaqeBlockColor = it->second;
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
                        auto it = blockToColor.find(block);
                        if (it == blockToColor.end()) {
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
                        auto it = blockToColor.find(block);
                        if (it == blockToColor.end()) {
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
