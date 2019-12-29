#include <iostream>
#include "minecraft-file.hpp"
#include <math.h>
#include <set>
#include <fstream>
#include "svpng.inc"
#include "hwm/task/task_queue.hpp"
#include "colormap/colormap.h"
#include "block_color.h"

using namespace std;
using namespace mcfile;
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

struct Landmark {
    int dimension;
    int x;
    int z;
};

static int const kVisibleRadius = 128;
static vector<Landmark> kLandmarks;

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


static shared_ptr<Chunk> LoadChunk(fs::path const& chunkFilePath, int chunkX, int chunkZ) {
    int const fLength = fs::file_size(chunkFilePath);
    vector<uint8_t> buffer(fLength);
    {
        auto stream = make_shared<mcfile::detail::FileStream>(chunkFilePath.string());
        mcfile::detail::StreamReader reader(stream);
        if (!reader.read(buffer)) {
            return nullptr;
        }
        if (!mcfile::detail::Compression::decompress(buffer)) {
            return nullptr;
        }
    }
    auto root = make_shared<nbt::CompoundTag>();
    auto bs = make_shared<mcfile::detail::ByteStream>(buffer);
    vector<uint8_t>().swap(buffer);
    auto sr = make_shared<mcfile::detail::StreamReader>(bs);
    root->read(*sr);
    if (!root->valid()) {
        return nullptr;
    }
    return Chunk::MakeChunk(chunkX, chunkZ, *root);
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

static void RegionToPng2(string world, int dimension, int regionX, int regionZ, string png) {
    int const width = 513;
    int const height = 513;
    
    vector<Landmark> nearbyLandmarks;
    {
        int const minBlockX = regionX * 512 - kVisibleRadius * 2;
        int const maxBlockX = regionX * 512 + 511 + kVisibleRadius * 2;
        
        int const minBlockZ = regionZ * 512 - kVisibleRadius * 2;
        int const maxBlockZ = regionZ * 512 + 511 + kVisibleRadius * 2;
        for (int i = 0; i < kLandmarks.size(); i++) {
        }
        for (auto it = kLandmarks.begin(); it != kLandmarks.end(); it++) {
            if (dimension == it->dimension && minBlockX <= it->x && it->x <= maxBlockX && minBlockZ <= it->z && it->z <= maxBlockZ) {
                nearbyLandmarks.push_back(*it);
            }
        }
        if (nearbyLandmarks.empty()) {
            return;
        }
    }
    
    vector<uint8_t> heightMap(width * height, 0);
    vector<Color> pixels(width * height, Color::FromFloat(0, 0, 0, 1));
    vector<float> light(width * height, 0);
    uint8_t *heightMapPtr = heightMap.data();
    Color *pixelsPtr = pixels.data();
    float *lightPtr = light.data();

    int const minX = regionX * 512 - 1;
    int const minZ = regionZ * 512 - 1;
    
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

            Color const waterColor(69, 91, 211);
            float const waterDiffusion = 0.02;
            colormap::kbinani::Altitude altitude;
            int const sZ = chunk->minBlockZ();
            int const eZ = chunk->maxBlockZ();
            int const sX = chunk->minBlockX();
            int const eX = chunk->maxBlockX();
            for (int z = sZ; z <= eZ; z++) {
                for (int x = sX; x <= eX; x++) {
                    int waterDepth = 0;
                    int airDepth = 0;
                    Color translucentBlock(0, 0, 0, 0);
                    int yini = 255;
                    if (dimension == -1) {
                        yini = 127;
                        for (int y = 127; y >= 0; y--) {
                            auto block = chunk->blockIdAt(x, y, z);
                            if (!block) continue;
                            if (block == mcfile::blocks::minecraft::air) {
                                yini = y;
                                break;
                            }
                        }
                    }
                    for (int y = yini; y >= 0; y--) {
                        auto block = chunk->blockIdAt(x, y, z);
                        if (!block) {
                            airDepth++;
                            continue;
                        }
                        if (block == blocks::minecraft::water || block == blocks::minecraft::bubble_column || block == blocks::minecraft::kelp) {
                            if (waterDepth == 0) {
                                int const idx = (z - minZ) * width + (x - minX);
                                lightPtr[idx] = LightAt(*chunk, x, y + 1, z);
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
                                lightPtr[idx] = LightAt(*chunk, x, y + 1, z);
                            } else {
                                color = opaqeBlockColor;
                                heightMapPtr[idx] = y;
                                lightPtr[idx] = LightAt(*chunk, x, y + 1, z);
                            }
                            pixelsPtr[idx] = Color::Add(color, translucentBlock.withAlphaComponent(0.2));
                            break;
                        }
                    }
                }
            }
        }
    }

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
            int waterDepth = 0;
            for (int y = 255; y >= 0; y--) {
                auto block = chunk->blockIdAt(x, y, z);
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
            int waterDepth = 0;
            for (int y = 255; y >= 0; y--) {
                auto block = chunk->blockIdAt(x, y, z);
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
    }
    
    vector<uint32_t> img(512 * 512, Color(0, 0, 0, 0).color());
    bool blackout = true;

    for (int z = 1; z < height; z++) {
        int const blockZ = regionZ * 512 + z - 1;
        for (int x = 1; x < width; x++) {
            int const blockX = regionX * 512 + x - 1;
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

            float minDistance = numeric_limits<float>::max();
            for (int j = 0; j < nearbyLandmarks.size(); j++) {
                Landmark const& landmark = nearbyLandmarks[j];
                if (landmark.dimension != dimension) {
                    continue;
                }
                float const distance = hypotf(blockX - landmark.x, blockZ - landmark.z);
                minDistance = min(minDistance, distance);
            }
            float const brightness = BrightnessByDistanceFromLandmark(minDistance);

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
            
            float const l = light[idx] * brightness;
            if (l > 0) {
                blackout = false;
            }
            int i = (z - 1) * 512 + (x - 1);
            img[i] = Color::FromFloat(color.fR, color.fG, color.fB, color.fA * l).color();
        }
    }

    if (blackout) {
        return;
    }
    
    FILE *out = fopen(png.c_str(), "wb");
    if (!out) {
        return;
    }
    svpng(out, 512, 512, (unsigned char *)img.data(), 1);
    fclose(out);
}

static void PrintDescription() {
    cerr << "mca2png -w [world directory] -o [output directory] -l [path to 'landmarks.tsv'] -d [dimension; o:overworld, n:nether, e:theEnd]" << endl;
}

int main(int argc, char *argv[]) {
    string input;
    string output;
    string landmarksFile;
    int dimension = 100;

    int opt;
    opterr = 0;
    while ((opt = getopt(argc, argv, "w:o:l:d:")) != -1) {
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
            default:
                PrintDescription();
                return 1;
        }
    }

    if (input.empty() || output.empty() || landmarksFile.empty()) {
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
    
    if (kLandmarks.empty()) {
        cerr << "landmarks.tsv is empty" << endl;
        return 1;
    }
    
    namespace fs = std::filesystem;
    auto chunkDir = fs::path(input).append("chunk");
    set<pair<int, int>> existingRegions;
    for (auto const& path : fs::directory_iterator(chunkDir)) {
        fs::path p = path.path();
        int x, z;
        if (sscanf(p.filename().c_str(), "c.%d.%d.nbt.x", &x, &z) != 2) {
            continue;
        }
        int regionX = Coordinate::RegionFromChunk(x);
        int regionZ = Coordinate::RegionFromChunk(z);
        existingRegions.insert(make_pair(regionX, regionZ));
    }

    hwm::task_queue q(thread::hardware_concurrency());
    vector<future<void>> futures;

    for (auto it : existingRegions) {
        int regionX = it.first;
        int regionZ = it.second;
        futures.emplace_back(q.enqueue([=](int regionX, int regionZ) {
            ostringstream name;
            name << "r." << regionX << "." << regionZ << ".png";
            fs::path png = fs::path(output).append(name.str());
            RegionToPng2(input, dimension, regionX, regionZ, png.string());
        }, regionX, regionZ));
    }
    
    for (auto& f : futures) {
        f.get();
    }
    return 0;
}
