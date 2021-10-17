#include <iostream>
#include "minecraft-file.hpp"
#include <math.h>
#include <set>
#include <fstream>
#include "zopflipng_lib.h"
#include "lodepng.h"
#include "colormap/colormap.h"
#include "block_color.h"

using namespace std;
using namespace mcfile;
using namespace mcfile::je;
namespace fs = std::filesystem;

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
    mcfile::blocks::minecraft::blue_banner,
    mcfile::blocks::minecraft::blue_wall_banner,
    mcfile::blocks::minecraft::brown_banner,
    mcfile::blocks::minecraft::gray_wall_banner,
    mcfile::blocks::minecraft::cyan_banner,
    mcfile::blocks::minecraft::cyan_wall_banner,
    mcfile::blocks::minecraft::gray_banner,
    mcfile::blocks::minecraft::green_banner,
    mcfile::blocks::minecraft::green_wall_banner,
    mcfile::blocks::minecraft::light_blue_banner,
    mcfile::blocks::minecraft::light_blue_wall_banner,
    mcfile::blocks::minecraft::light_gray_banner,
    mcfile::blocks::minecraft::light_gray_wall_banner,
    mcfile::blocks::minecraft::lime_banner,
    mcfile::blocks::minecraft::lime_wall_banner,
    mcfile::blocks::minecraft::magenta_banner,
    mcfile::blocks::minecraft::magenta_wall_banner,
    mcfile::blocks::minecraft::orange_banner,
    mcfile::blocks::minecraft::orange_wall_banner,
    mcfile::blocks::minecraft::pink_banner,
    mcfile::blocks::minecraft::pink_wall_banner,
    mcfile::blocks::minecraft::purple_banner,
    mcfile::blocks::minecraft::purple_wall_banner,
    mcfile::blocks::minecraft::red_banner,
    mcfile::blocks::minecraft::red_wall_banner,
    mcfile::blocks::minecraft::white_banner,
    mcfile::blocks::minecraft::white_wall_banner,
    mcfile::blocks::minecraft::yellow_banner,
    mcfile::blocks::minecraft::yellow_wall_banner,
    mcfile::blocks::minecraft::void_air,
    mcfile::blocks::minecraft::structure_void,
    mcfile::blocks::minecraft::tripwire,
};

struct Landmark {
    int dimension;
    int x;
    int z;
};

static int const kVisibleRadius = 128;
static vector<Landmark> kLandmarks;

static shared_ptr<Chunk> LoadChunk(fs::path const& chunkFilePath, int chunkX, int chunkZ) {
    int const fLength = fs::file_size(chunkFilePath);
    vector<uint8_t> buffer(fLength);
    {
        auto stream = make_shared<mcfile::stream::FileInputStream>(chunkFilePath);
        mcfile::stream::InputStreamReader reader(stream);
        if (!reader.read(buffer)) {
            return nullptr;
        }
        if (!mcfile::Compression::decompress(buffer)) {
            return nullptr;
        }
    }
    auto root = make_shared<nbt::CompoundTag>();
    auto bs = make_shared<mcfile::stream::ByteStream>(buffer);
    vector<uint8_t>().swap(buffer);
    auto sr = make_shared<mcfile::stream::InputStreamReader>(bs);
    if (!root->read(*sr)) {
        return nullptr;
    }
    return Chunk::MakeChunk(chunkX, chunkZ, root);
}

static string ChunkFileName(int chunkX, int chunkZ) {
    ostringstream name;
    name << "c." << chunkX << "." << chunkZ << ".nbt.z";
    return name.str();
}

static float BrightnessByDistanceFromLandmark(float distance) {
    if (distance <= kVisibleRadius) {
        return 1;
    } else if (distance > 2 * kVisibleRadius) {
        return 0;
    } else {
        float x = 1 - (distance - kVisibleRadius) / kVisibleRadius;
        return (erff(sqrtf(M_PI) * 2 * x - 2) + 1) * 0.5;
    }
}

static int SkyLevel(int dimension, Chunk const& chunk, int x, int z) {
    if (dimension != -1) {
        return chunk.maxBlockY();
    }
    for (int y = 127; y >= 0; y--) {
        auto block = chunk.blockIdAt(x, y, z);
        if (!block) continue;
        if (block == mcfile::blocks::minecraft::air) {
            return y;
        }
    }
    return 0;
}

static bool IsSlab(Block const& block) {
    auto const found = block.fName.rfind("_slab");
    return found == block.fName.size() - 5;
}

static bool IsStairs(Block const& block) {
    auto const found = block.fName.rfind("_stairs");
    return found == block.fName.size() - 7;
}

static bool IsTrapdoor(Block const& block) {
    auto const found = block.fName.rfind("_trapdoor");
    return found == block.fName.size() - 9;
}

static bool IsWaterLike(Block const& block) {
    string const& name = block.fName;
    if (name == "minecraft:water" || name == "minecraft:bubble_column" || name == "minecraft:kelp" || name == "minecraft:seagrass" || name == "minecraft:tall_seagrass") {
        return true;
    }

    if (block.property("waterlogged") != "true") {
        return false;
    }

    if (IsSlab(block) && block.property("type") == "top") {
        return false;
    } else if (IsStairs(block) && block.property("half") == "top") {
        return false;
    } else if (block.fName == "scaffolding") {
        return false;
    } else if (IsTrapdoor(block) && block.property("open") == "close") {
        return false;
    }
    
    return true;
}

class TranslucentBlock {
private:
    TranslucentBlock() = delete;
    
public:
    static Color Air() {
        return Color(0, 0, 0, 0);
    }
    
    static Color Opaque() {
        return Color(0, 0, 0, 255);
    }
    
    static Color FromBlock(Block const& block) {
        if (IsWaterLike(block)) {
            return Color::FromFloat(69 / 255.0f, 91 / 255.0f, 211 / 255.0f, 0);
        }
        blocks::BlockId blockId = blocks::FromName(block.fName);
        int const stainedGlassAlpha = (int)(255 * 0.5);
        switch (blockId) {
            case blocks::minecraft::air:
            case blocks::minecraft::cave_air:
                return Air();
            case blocks::minecraft::glass:
            case blocks::minecraft::glass_pane:
                return Color(255, 255, 255, 4);
            case blocks::minecraft::white_stained_glass:
            case blocks::minecraft::white_stained_glass_pane:
                return Color(255, 255, 255, stainedGlassAlpha);
            case blocks::minecraft::orange_stained_glass:
            case blocks::minecraft::orange_stained_glass_pane:
                return Color(255, 165, 0, stainedGlassAlpha);
            case blocks::minecraft::magenta_stained_glass:
            case blocks::minecraft::magenta_stained_glass_pane:
                return Color(255, 0, 255, stainedGlassAlpha);
            case blocks::minecraft::light_blue_stained_glass:
            case blocks::minecraft::light_blue_stained_glass_pane:
                return Color(142, 209, 224, stainedGlassAlpha);
            case blocks::minecraft::yellow_stained_glass:
            case blocks::minecraft::yellow_stained_glass_pane:
                return Color(227, 199, 0, stainedGlassAlpha);
            case blocks::minecraft::lime_stained_glass:
            case blocks::minecraft::lime_stained_glass_pane:
                return Color(0, 255, 0, stainedGlassAlpha);
            case blocks::minecraft::pink_stained_glass:
            case blocks::minecraft::pink_stained_glass_pane:
                return Color(255, 102, 153, stainedGlassAlpha);
            case blocks::minecraft::gray_stained_glass:
            case blocks::minecraft::gray_stained_glass_pane:
                return Color(118, 118, 118, stainedGlassAlpha);
            case blocks::minecraft::light_gray_stained_glass:
            case blocks::minecraft::light_gray_stained_glass_pane:
                return Color(211, 211, 211, stainedGlassAlpha);
            case blocks::minecraft::cyan_stained_glass:
            case blocks::minecraft::cyan_stained_glass_pane:
                return Color(0, 255, 255, stainedGlassAlpha);
            case blocks::minecraft::purple_stained_glass:
            case blocks::minecraft::purple_stained_glass_pane:
                return Color(128, 0, 128, stainedGlassAlpha);
            case blocks::minecraft::blue_stained_glass:
            case blocks::minecraft::blue_stained_glass_pane:
                return Color(0, 0, 255, stainedGlassAlpha);
            case blocks::minecraft::brown_stained_glass:
            case blocks::minecraft::brown_stained_glass_pane:
                return Color(139, 69, 19, stainedGlassAlpha);
            case blocks::minecraft::green_stained_glass:
            case blocks::minecraft::green_stained_glass_pane:
                return Color(0, 128, 0, stainedGlassAlpha);
            case blocks::minecraft::red_stained_glass:
            case blocks::minecraft::red_stained_glass_pane:
                return Color(255, 0, 0, stainedGlassAlpha);
            case blocks::minecraft::black_stained_glass:
            case blocks::minecraft::black_stained_glass_pane:
                return Color(0, 0, 0, stainedGlassAlpha);
            default:
                break;
        }
        if (transparentBlocks.find(blockId) != transparentBlocks.end()) {
            return Air();
        }
        if (plantBlocks.find(blockId) != plantBlocks.end()) {
            return Air();
        }
        if (IsTrapdoor(block) && block.property("open") == "true") {
            return Air();
        }
        return Opaque();
    }

    Color color;
};

template<class T>
static T Clamp(T v, T min, T max) {
    return std::min(std::max(v, min), max);
}

static int Altitude(int dimension, Chunk const& chunk, int x, int z) {
    int const maxY = SkyLevel(dimension, chunk, x, z);
    int const minY = chunk.minBlockY();
    int waterDepth = 0;
    for (int y = maxY; y >= minY; y--) {
        auto const& block = chunk.blockAt(x, y, z);
        if (!block) {
            continue;
        }
        if (IsWaterLike(*block)) {
            waterDepth++;
        }
        auto tb = TranslucentBlock::FromBlock(*block);
        if (tb.fA >= 1) {
            return y;
        }
    }
    return 0;
}

static Color DiffuseBlockColor(Color blockColor, int waterDepth, vector<Color> const& pillar, int pillarHeight) {
    Color base = blockColor;
    if (waterDepth > 0) {
        static float const diffusion = 0.02;
        Color water = Color(69, 91, 211).diffuse(diffusion, waterDepth);
        base = Color::Add(water, Color(0, 0, 0).withAlphaComponent(0.2));
    }
    Color result = base;
    for (int i = pillarHeight - 1; i >= 0; i--) {
        Color c = pillar[i];
        if (c.fA > 0) {
            result = Color::Blend(c, result);
        }
    }
    return result;
}

static void RegionToPng2(string world, int dimension, int regionX, int regionZ, string png, bool zopfli) {
    int const width = 513;
    int const height = 513;
    
    vector<Landmark> nearbyLandmarks;
    if (!kLandmarks.empty()){
        int const minBlockX = regionX * 512 - kVisibleRadius * 2;
        int const maxBlockX = regionX * 512 + 511 + kVisibleRadius * 2;
        
        int const minBlockZ = regionZ * 512 - kVisibleRadius * 2;
        int const maxBlockZ = regionZ * 512 + 511 + kVisibleRadius * 2;
        for (auto it = kLandmarks.begin(); it != kLandmarks.end(); it++) {
            if (dimension == it->dimension && minBlockX <= it->x && it->x <= maxBlockX && minBlockZ <= it->z && it->z <= maxBlockZ) {
                nearbyLandmarks.push_back(*it);
            }
        }
        if (nearbyLandmarks.empty()) {
            return;
        }
    }
    
    vector<uint8_t> altitude(width * height, 0);
    vector<Color> pixels(width * height, Color::FromFloat(0, 0, 0, 1));
    vector<Color> translucentBlockPillar(256, Color(0, 0, 0, 255));

    int const minX = regionX * 512 - 1;
    int const minZ = regionZ * 512 - 1;
    
    Block const kGrassBlock(blocks::minecraft::grass_block);
    Block const kUnknownBlock(blocks::unknown);
    
    for (int localChunkZ = 0; localChunkZ < 32; localChunkZ++) {
        int const chunkZ = regionZ * 32 + localChunkZ;
        for (int localChunkX = 0; localChunkX < 32; localChunkX++) {
            int const chunkX = regionX * 32 + localChunkX;
            fs::path chunkFilePath = fs::path(world).append("chunk").append(ChunkFileName(chunkX, chunkZ));
            if (!fs::exists(chunkFilePath)) {
                continue;
            }
            shared_ptr<Chunk> chunk = LoadChunk(chunkFilePath, chunkX, chunkZ);
            if (!chunk) {
                continue;
            }

            colormap::kbinani::Altitude colormap;
            int const sZ = chunk->minBlockZ();
            int const eZ = chunk->maxBlockZ();
            int const sX = chunk->minBlockX();
            int const eX = chunk->maxBlockX();
            for (int z = sZ; z <= eZ; z++) {
                for (int x = sX; x <= eX; x++) {
                    fill_n(translucentBlockPillar.begin(), translucentBlockPillar.size(), TranslucentBlock::Air());
                    int const maxY = SkyLevel(dimension, *chunk, x, z);
                    int const minY = chunk->minBlockY();
                    int pillarIndex = 0;
                    int pillarHeight = 0;
                    shared_ptr<Block const> opaqueBlock = nullptr;
                    
                    int elevation = 0;
                    int waterDepth = 0;
                    for (int y = maxY; y >= minY; y--, pillarIndex++) {
                        auto const& block = chunk->blockAt(x, y, z);
                        if (block) {
                            if (IsWaterLike(*block)) {
                                waterDepth++;
                            }
                            auto tb = TranslucentBlock::FromBlock(*block);
                            if (tb.fA >= 1) {
                                pillarHeight = pillarIndex;
                                elevation = y;
                                opaqueBlock = block;
                                break;
                            }
                            translucentBlockPillar[pillarIndex] = tb;
                        } else {
                            translucentBlockPillar[pillarIndex] = TranslucentBlock::Air();
                        }
                    }
                    Color opaqueBlockColor(0, 0, 0);
                    if (opaqueBlock) {
                        if (opaqueBlock->fName == kGrassBlock.fName) {
                            float const v = Clamp((elevation - 63.0) / 193.0, 0.0, 1.0);
                            auto mapped = colormap.getColor(v);
                            opaqueBlockColor = Color::FromFloat(mapped.r, mapped.g, mapped.b, 1);
                        } else {
                            auto color = BlockColor(*opaqueBlock);
                            if (color) {
                                opaqueBlockColor = *color;
                            }
                        }
                    }
                    Color c = DiffuseBlockColor(opaqueBlockColor, waterDepth, translucentBlockPillar, pillarHeight);
                    int const idx = (z - minZ) * width + (x - minX);
                    pixels[idx] = c;
                    altitude[idx] = elevation;
                }
            }
        }
    }

    // 北側のチャンクがまだ無い場合に備えて, 1 ブロック南の高度をデフォルト値に使う.
    for (int x = minX + 1; x < minX + 512; x++) {
        int const z = minZ;
        int i1 = (z + 1 - minZ) * width + (x - minX);
        int i0 = (z - minZ) * width + (x - minX);
        altitude[i0] = altitude[i1];
    }

    // 西側のチャンクがまだ無い場合に備えて, 1 ブロック東の高度をデフォルト値に使う.
    for (int z = minZ + 1; z < minZ + 512; z++) {
        int const x = minX;
        int i1 = (z - minZ) * width + (x + 1 - minX);
        int i0 = (z - minZ) * width + (x - minX);
        altitude[i0] = altitude[i1];
    }

    // 北側
    for (int lcx = 0; lcx < 32; lcx++) {
        int const chunkX = regionX * 32 + lcx;
        int const chunkZ = (regionZ - 1) * 32 + 31;
        fs::path chunkFilePath = fs::path(world).append("chunk").append(ChunkFileName(chunkX, chunkZ));
        if (!fs::exists(chunkFilePath)) {
            continue;
        }
        shared_ptr<Chunk> chunk = LoadChunk(chunkFilePath, chunkX, chunkZ);
        int const z = chunk->maxBlockZ();
        for (int lbx = 0; lbx < 16; lbx++) {
            int const x = chunk->minBlockX() + lbx;
            int const idx = (z - minZ) * width + (x - minX);
            altitude[idx] = Altitude(dimension, *chunk, x, z);
        }
    }
    
    // 西側
    for (int lcz = 0; lcz < 32; lcz++) {
        int const chunkX = (regionX - 1) * 32 + 31;
        int const chunkZ = regionZ * 32 + lcz;
        fs::path chunkFilePath = fs::path(world).append("chunk").append(ChunkFileName(chunkX, chunkZ));
        if (!fs::exists(chunkFilePath)) {
            continue;
        }
        shared_ptr<Chunk> chunk = LoadChunk(chunkFilePath, chunkX, chunkZ);
        int const x = chunk->maxBlockX();
        for (int lbz = 0; lbz < 16; lbz++) {
            int const z = chunk->minBlockZ() + lbz;
            int const idx = (z - minZ) * width + (x - minX);
            altitude[idx] = Altitude(dimension, *chunk, x, z);
        }
    }
    
    vector<uint32_t> img(512 * 512, Color(0, 0, 0, 0).color());
    bool blackout = true;

    for (int z = 1; z < height; z++) {
        int const blockZ = regionZ * 512 + z - 1;
        for (int x = 1; x < width; x++) {
            int const blockX = regionX * 512 + x - 1;
            int const idx = z * width + x;
            uint8_t const h = altitude[idx];
            Color c = pixels[idx];
            Color color = c;

            uint8_t hNorth = altitude[(z - 1) * width + x];
            uint8_t hWest = altitude[z * width + x - 1];
            int score = 0; // +: bright, -: dark
            if (hNorth > h) score--;
            if (hNorth < h) score++;
            if (hWest > h) score--;
            if (hWest < h) score++;

            float minDistance = numeric_limits<float>::max();
            float brightness = 1;
            if (!kLandmarks.empty()) {
                for (int j = 0; j < nearbyLandmarks.size(); j++) {
                    Landmark const& landmark = nearbyLandmarks[j];
                    float const distance = hypotf(blockX - landmark.x, blockZ - landmark.z);
                    minDistance = min(minDistance, distance);
                }
                brightness = BrightnessByDistanceFromLandmark(minDistance);
            }

            if (score > 0) {
                float coeff = 1.2;
                HSV hsv = color.toHSV();
                hsv.fV = hsv.fV * coeff * brightness;
                Color cc = Color::FromHSV(hsv);
                color = cc;
            } else if (score < 0) {
                float coeff = 0.8;
                HSV hsv = color.toHSV();
                hsv.fV = hsv.fV * coeff * brightness;
                Color cc = Color::FromHSV(hsv);
                color = cc;
            } else {
                HSV hsv = color.toHSV();
                hsv.fV = hsv.fV * brightness;
                color = Color::FromHSV(hsv);
            }
            
            if (brightness > 0) {
                blackout = false;
            }
            int i = (z - 1) * 512 + (x - 1);
            img[i] = Color::FromFloat(color.fR, color.fG, color.fB, color.fA * brightness).color();
        }
    }

    if (blackout) {
        return;
    }

    vector<unsigned char> in;
    copy_n((unsigned char*)img.data(), img.size() * sizeof(uint32_t), back_inserter(in));
    vector<uint32_t>().swap(img);
    
    vector<unsigned char> out;
    if (lodepng::encode(out, in, 512, 512) != 0) {
        return;
    }

    if (zopfli) {
        vector<unsigned char> result;
        ZopfliPNGOptions opt;
        opt.verbose = false;
        if (ZopfliPNGOptimize(out, opt, false, &result) != 0) {
            return;
        }
        out.swap(result);
    }
    
    FILE* file = fopen(png.c_str(), "wb");
    if (!file) {
        return;
    }
    fwrite(out.data(), 1, out.size(), file);
    fclose(file);
}

static void PrintDescription() {
    cerr << "mca2png -w [world directory] -x [region x] -z [region z] -o [output directory] -l [path to 'landmarks.tsv'] -d [dimension; o:overworld, n:nether, e:theEnd] [-m(minify png with zopfli)]" << endl;
}

int main(int argc, char *argv[]) {
    string input;
    string output;
    string landmarksFile;
    int dimension = 100;
    int x = INT_MAX;
    int z = INT_MAX;
    bool zopfli = false;

    int opt;
    opterr = 0;
    while ((opt = getopt(argc, argv, "w:x:z:o:l:d:m")) != -1) {
        switch (opt) {
            case 'w':
                input = optarg;
                break;
            case 'o':
                output = optarg;
                break;
            case 'l':
                landmarksFile = optarg;
                break;
            case 'd': {
                string d(optarg);
                if (d == "o") {
                    dimension = 0;
                } else if (d == "n") {
                    dimension = -1;
                } else if (d == "e") {
                    dimension = 1;
                } else {
                    PrintDescription();
                    return 1;
                }
                break;
            }
            case 'x':
                if (sscanf(optarg, "%d", &x) != 1) {
                    PrintDescription();
                    return 1;
                }
                break;
            case 'z':
                if (sscanf(optarg, "%d", &z) != 1) {
                    PrintDescription();
                    return 1;
                }
                break;
            case 'm':
                zopfli = true;
                break;
            default:
                PrintDescription();
                return 1;
        }
    }

    if (input.empty() || output.empty() || x == INT_MAX || z == INT_MAX || dimension == 100) {
        PrintDescription();
        return 1;
    }
    
    {
        ifstream stream(landmarksFile.c_str());
        string line;
        while (getline(stream, line)) {
            int dim, x, z;
            if (sscanf(line.c_str(), "%d\t%d\t%d", &dim, &x, &z) != 3) {
                continue;
            }
            kLandmarks.push_back({.dimension = dim, .x = x, .z = z});
        }
    }

    ostringstream name;
    name << "r." << x << "." << z << ".png";
    fs::path png = fs::path(output).append(name.str());
    RegionToPng2(input, dimension, x, z, png.string(), zopfli);

    return 0;
}
