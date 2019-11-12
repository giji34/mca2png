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

static Color const kColorPotter(135, 75, 58);
static Color const kColorPlanksBirch(244, 230, 161);
static Color const kColorPlanksDarkOak(101, 75, 50);
static Color const kColorPlanksOak(127, 85, 48);
static Color const kColorPlanksJungle(149, 108, 76);
static Color const kColorPlanksSpruce(122, 89, 51);
static Color const kColorBricks(175, 98, 76);
static Color const kColorAnvil(73, 73, 73);
static Color const kColorDeadCoral(115, 105, 102);
static Color const kColorRail(154, 154, 154);
static Color const kColorStoneDiorite(252, 249, 242);
static Color const kColorStoneGranite(149, 108, 76);
static Color const kColorStoneAndesite(165, 168, 151);
static Color const kColorStone(111, 111, 111);
static Color const kColorNetherBricks(33, 17, 20);
static Color const kColorFurnace(131, 131, 131);
static Color const kColorEndStoneBricks(233, 248, 173);
static Color const kColorRedSandstone(184, 102, 33);
static Color const kColorSandstone(244, 230, 161);
static Color const kColorDragonHead(22, 22, 22);
static Color const kColorQuartz(235, 227, 219);
static Color const kColorMossyStone(115, 131, 82);
static Color const kColorPistonHead(186, 150, 97);
static Color const kColorPurPur(170, 122, 170);
static Color const kColorPrismarine(75, 125, 151);
static Color const kColorRedNetherBricks(89, 0, 0);
static Color const kColorCreaperHead(96, 202, 77);
static Color const kColorOakLog(141, 118, 71);
static Color const kColorPlayerHead(46, 31, 14);
static Color const kColorSkeltonSkull(186, 186, 186);
static Color const kColorSpruceLog(141, 118, 71);
static Color const kColorChest(141, 118, 71);
static Color const kColorWitherSkeltonSkull(31, 31, 31);
static Color const kColorZombieHead(61, 104, 45);

static map<blocks::BlockId, Color> const blockToColor {
    {mcfile::blocks::minecraft::stone, Color(111, 111, 111)},
    {mcfile::blocks::minecraft::granite, kColorStoneGranite},
    {mcfile::blocks::minecraft::diorite, kColorStoneDiorite},
    {mcfile::blocks::minecraft::andesite, kColorStoneAndesite},
    {mcfile::blocks::minecraft::chest, kColorChest},
    {mcfile::blocks::minecraft::clay, Color(162, 166, 182)},
    {mcfile::blocks::minecraft::coal_ore, Color(111, 111, 111)},
    {mcfile::blocks::minecraft::cobblestone, Color(111, 111, 111)},
    {mcfile::blocks::minecraft::dirt, Color(149, 108, 76)},
    {mcfile::blocks::minecraft::brown_mushroom, Color(0, 123, 0)},
    {mcfile::blocks::minecraft::grass_block, Color(130, 148, 58)},
    {mcfile::blocks::minecraft::iron_ore, Color(111, 111, 111)},
    {mcfile::blocks::minecraft::sand, Color(201,192,154)}, //
    {mcfile::blocks::minecraft::oak_leaves, Color(56, 95, 31)}, //
    {mcfile::blocks::minecraft::jungle_leaves, Color(56, 95, 31)}, //
    {mcfile::blocks::minecraft::birch_leaves, Color(67, 124, 37)},
    {mcfile::blocks::minecraft::red_mushroom, Color(0, 123, 0)},
    {mcfile::blocks::minecraft::mossy_cobblestone, kColorMossyStone},
    {mcfile::blocks::minecraft::oak_stairs, kColorPlanksOak},
    {mcfile::blocks::minecraft::gravel, Color(111, 111, 111)},
    {mcfile::blocks::minecraft::oak_log, kColorOakLog},
    {mcfile::blocks::minecraft::oak_planks, kColorPlanksOak},
    {mcfile::blocks::minecraft::farmland, Color(149, 108, 76)},
    {mcfile::blocks::minecraft::oak_fence, kColorPlanksOak},
    {mcfile::blocks::minecraft::cobblestone_stairs, Color(111, 111, 111)},
    {mcfile::blocks::minecraft::grass_path, Color(204, 204, 204)}, //
    {mcfile::blocks::minecraft::birch_fence, kColorPlanksBirch},
    {mcfile::blocks::minecraft::birch_planks, kColorPlanksBirch},
    {mcfile::blocks::minecraft::birch_stairs, kColorPlanksBirch},
    {mcfile::blocks::minecraft::dark_oak_fence, kColorPlanksDarkOak},
    {mcfile::blocks::minecraft::dark_oak_log, Color(101, 75, 50)},
    {mcfile::blocks::minecraft::dark_oak_planks, Color(191,152,63)}, //
    {mcfile::blocks::minecraft::dark_oak_slab, kColorPlanksDarkOak},
    {mcfile::blocks::minecraft::dark_oak_stairs, kColorPlanksDarkOak},
    {mcfile::blocks::minecraft::dark_oak_trapdoor, Color(141, 118, 71)},
    {mcfile::blocks::minecraft::diamond_ore, Color(111, 111, 111)},
    {mcfile::blocks::minecraft::gold_ore, Color(111, 111, 111)},
    {mcfile::blocks::minecraft::ice, Color(158, 158, 252)},
    {mcfile::blocks::minecraft::jungle_fence, kColorPlanksJungle},
    {mcfile::blocks::minecraft::jungle_log, Color(149, 108, 76)},
    {mcfile::blocks::minecraft::jungle_planks, kColorPlanksJungle},
    {mcfile::blocks::minecraft::jungle_slab, kColorPlanksJungle},
    {mcfile::blocks::minecraft::jungle_stairs, kColorPlanksJungle},
    {mcfile::blocks::minecraft::jungle_button, kColorPlanksJungle},
    {mcfile::blocks::minecraft::jungle_door, kColorPlanksJungle},
    {mcfile::blocks::minecraft::jungle_trapdoor, Color(141, 118, 71)},
    {mcfile::blocks::minecraft::lapis_ore, Color(111, 111, 111)},
    {mcfile::blocks::minecraft::lava, Color(179, 71, 3)},
    {mcfile::blocks::minecraft::oak_door, Color(141, 118, 71)},
    {mcfile::blocks::minecraft::oak_slab, kColorPlanksOak},
    {mcfile::blocks::minecraft::oak_trapdoor, Color(141, 118, 71)},
    {mcfile::blocks::minecraft::obsidian, Color(29, 14, 52)},
    {mcfile::blocks::minecraft::packed_ice, Color(158, 158, 252)},
    {mcfile::blocks::minecraft::polished_granite, kColorStoneGranite},
    {mcfile::blocks::minecraft::prismarine, kColorPrismarine},
    {mcfile::blocks::minecraft::prismarine_bricks, Color(91, 216, 210)},
    {mcfile::blocks::minecraft::rail, kColorRail},
    {mcfile::blocks::minecraft::redstone_ore, Color(111, 111, 111)},
    {mcfile::blocks::minecraft::sandstone, kColorSandstone},
    {mcfile::blocks::minecraft::sea_lantern, Color(252, 249, 242)},
    {mcfile::blocks::minecraft::snow, Color(229, 229, 229)}, //
    {mcfile::blocks::minecraft::snow_block, Color(252, 252, 252)},
    {mcfile::blocks::minecraft::spruce_door, kColorPlanksSpruce},
    {mcfile::blocks::minecraft::spruce_fence, kColorPlanksSpruce},
    {mcfile::blocks::minecraft::spruce_leaves, Color(56, 95, 31)}, //
    {mcfile::blocks::minecraft::stone_brick_stairs, Color(111, 111, 111)},
    {mcfile::blocks::minecraft::stone_bricks, Color(111, 111, 111)},
    {mcfile::blocks::minecraft::stone_slab, Color(111, 111, 111)},
    {mcfile::blocks::minecraft::spruce_log, kColorSpruceLog},
    {mcfile::blocks::minecraft::spruce_planks, kColorPlanksSpruce},
    {mcfile::blocks::minecraft::spruce_slab, kColorPlanksSpruce},
    {mcfile::blocks::minecraft::spruce_stairs, kColorPlanksSpruce},
    {mcfile::blocks::minecraft::spruce_trapdoor, kColorPlanksSpruce},
    {mcfile::blocks::minecraft::mossy_stone_bricks, kColorMossyStone},
    {mcfile::blocks::minecraft::chiseled_stone_bricks, kColorStone},
    {mcfile::blocks::minecraft::cracked_stone_bricks, kColorStone},
    {mcfile::blocks::minecraft::infested_stone, Color(111, 111, 111)},
    {mcfile::blocks::minecraft::cobweb, Color(255, 255, 255)},
    {mcfile::blocks::minecraft::blue_ice, Color(102, 151, 246)},
    {mcfile::blocks::minecraft::magma_block, Color(181, 64, 9)},
    {mcfile::blocks::minecraft::end_stone, Color(219, 219, 172)},
    {mcfile::blocks::minecraft::end_portal, Color(4, 18, 24)},
    {mcfile::blocks::minecraft::end_portal_frame, Color(65, 114, 102)},
    {mcfile::blocks::minecraft::bedrock, Color(111, 111, 111)},
    {mcfile::blocks::minecraft::mycelium, Color(114, 96, 97)},
    {mcfile::blocks::minecraft::white_terracotta, Color(209, 180, 161)},
    {mcfile::blocks::minecraft::orange_terracotta, Color(165, 82, 40)},
    {mcfile::blocks::minecraft::magenta_terracotta, Color(147, 87, 108)},
    {mcfile::blocks::minecraft::light_blue_terracotta, Color(110, 106, 135)},
    {mcfile::blocks::minecraft::yellow_terracotta, Color(184, 129, 33)},
    {mcfile::blocks::minecraft::lime_terracotta, Color(102, 116, 52)},
    {mcfile::blocks::minecraft::pink_terracotta, Color(160, 77, 78)},
    {mcfile::blocks::minecraft::gray_terracotta, Color(57, 41, 36)},
    {mcfile::blocks::minecraft::light_gray_terracotta, Color(137, 106, 99)},
    {mcfile::blocks::minecraft::cyan_terracotta, Color(90, 94, 93)},
    {mcfile::blocks::minecraft::purple_terracotta, Color(117, 69, 86)},
    {mcfile::blocks::minecraft::blue_terracotta, Color(73, 58, 90)},
    {mcfile::blocks::minecraft::brown_terracotta, Color(76, 50, 36)},
    {mcfile::blocks::minecraft::green_terracotta, Color(75, 82, 41)},
    {mcfile::blocks::minecraft::red_terracotta, Color(139, 58, 45)},
    {mcfile::blocks::minecraft::black_terracotta, Color(37, 22, 15)},
    {mcfile::blocks::minecraft::terracotta, Color(153, 95, 68)},
    {mcfile::blocks::minecraft::red_sand, Color(201, 109, 36)},
    {mcfile::blocks::minecraft::purpur_slab, kColorPurPur},
    {mcfile::blocks::minecraft::purpur_block, kColorPurPur},
    {mcfile::blocks::minecraft::purpur_pillar, kColorPurPur},
    {mcfile::blocks::minecraft::ender_chest, Color(39, 54, 56)},
    {mcfile::blocks::minecraft::tnt, Color(216, 46, 26)},
    {mcfile::blocks::minecraft::prismarine_slab, kColorPrismarine},
    {mcfile::blocks::minecraft::prismarine_stairs, kColorPrismarine},
    {mcfile::blocks::minecraft::prismarine_bricks, Color(89, 173, 162)},
    {mcfile::blocks::minecraft::prismarine_brick_slab, Color(89, 173, 162)},
    {mcfile::blocks::minecraft::prismarine_brick_stairs, Color(89, 173, 162)},
    {mcfile::blocks::minecraft::dark_prismarine, Color(55, 97, 80)},
    {mcfile::blocks::minecraft::dark_prismarine_slab, Color(55, 97, 80)},
    {mcfile::blocks::minecraft::dark_prismarine_stairs, Color(55, 97, 80)},
    {mcfile::blocks::minecraft::netherrack, Color(86, 32, 31)},
    {mcfile::blocks::minecraft::nether_bricks, kColorNetherBricks},
    {mcfile::blocks::minecraft::nether_brick_slab, kColorNetherBricks},
    {mcfile::blocks::minecraft::nether_brick_wall, kColorNetherBricks},
    {mcfile::blocks::minecraft::red_nether_bricks, kColorRedNetherBricks},
    {mcfile::blocks::minecraft::red_nether_brick_slab, kColorRedNetherBricks},
    {mcfile::blocks::minecraft::red_nether_brick_wall, kColorRedNetherBricks},
    {mcfile::blocks::minecraft::glowstone, Color(248, 215, 115)},
    {mcfile::blocks::minecraft::nether_quartz_ore, Color(170, 112, 105)},
    {mcfile::blocks::minecraft::soul_sand, Color(72, 54, 43)},
    {mcfile::blocks::minecraft::white_wool, Color(247, 247, 247)},
    {mcfile::blocks::minecraft::orange_wool, Color(244, 122, 25)},
    {mcfile::blocks::minecraft::magenta_wool, Color(193, 73, 183)},
    {mcfile::blocks::minecraft::light_blue_wool, Color(65, 186, 220)},
    {mcfile::blocks::minecraft::yellow_wool, Color(249, 206, 47)},
    {mcfile::blocks::minecraft::lime_wool, Color(123, 193, 27)},
    {mcfile::blocks::minecraft::pink_wool, Color(241, 160, 186)},
    {mcfile::blocks::minecraft::gray_wool, Color(70, 78, 81)},
    {mcfile::blocks::minecraft::light_gray_wool, Color(151, 151, 145)},
    {mcfile::blocks::minecraft::cyan_wool, Color(22, 153, 154)},
    {mcfile::blocks::minecraft::purple_wool, Color(132, 47, 179)},
    {mcfile::blocks::minecraft::blue_wool, Color(57, 63, 164)},
    {mcfile::blocks::minecraft::brown_wool, Color(125, 79, 46)},
    {mcfile::blocks::minecraft::green_wool, Color(91, 119, 22)},
    {mcfile::blocks::minecraft::red_wool, Color(170, 42, 36)},
    {mcfile::blocks::minecraft::black_wool, Color(28, 28, 32)},
    {mcfile::blocks::minecraft::white_carpet, Color(247, 247, 247)},
    {mcfile::blocks::minecraft::orange_carpet, Color(244, 122, 25)},
    {mcfile::blocks::minecraft::magenta_carpet, Color(193, 73, 183)},
    {mcfile::blocks::minecraft::light_blue_carpet, Color(65, 186, 220)},
    {mcfile::blocks::minecraft::yellow_carpet, Color(249, 206, 47)},
    {mcfile::blocks::minecraft::lime_carpet, Color(123, 193, 27)},
    {mcfile::blocks::minecraft::pink_carpet, Color(241, 160, 186)},
    {mcfile::blocks::minecraft::gray_carpet, Color(70, 78, 81)},
    {mcfile::blocks::minecraft::light_gray_carpet, Color(151, 151, 145)},
    {mcfile::blocks::minecraft::cyan_carpet, Color(22, 153, 154)},
    {mcfile::blocks::minecraft::purple_carpet, Color(132, 47, 179)},
    {mcfile::blocks::minecraft::blue_carpet, Color(57, 63, 164)},
    {mcfile::blocks::minecraft::brown_carpet, Color(125, 79, 46)},
    {mcfile::blocks::minecraft::green_carpet, Color(91, 119, 22)},
    {mcfile::blocks::minecraft::red_carpet, Color(170, 42, 36)},
    {mcfile::blocks::minecraft::black_carpet, Color(28, 28, 32)},
    {mcfile::blocks::minecraft::white_bed, Color(247, 247, 247)},
    {mcfile::blocks::minecraft::orange_bed, Color(244, 122, 25)},
    {mcfile::blocks::minecraft::magenta_bed, Color(193, 73, 183)},
    {mcfile::blocks::minecraft::light_blue_bed, Color(65, 186, 220)},
    {mcfile::blocks::minecraft::yellow_bed, Color(249, 206, 47)},
    {mcfile::blocks::minecraft::lime_bed, Color(123, 193, 27)},
    {mcfile::blocks::minecraft::pink_bed, Color(241, 160, 186)},
    {mcfile::blocks::minecraft::gray_bed, Color(70, 78, 81)},
    {mcfile::blocks::minecraft::light_gray_bed, Color(151, 151, 145)},
    {mcfile::blocks::minecraft::cyan_bed, Color(22, 153, 154)},
    {mcfile::blocks::minecraft::purple_bed, Color(132, 47, 179)},
    {mcfile::blocks::minecraft::blue_bed, Color(57, 63, 164)},
    {mcfile::blocks::minecraft::brown_bed, Color(125, 79, 46)},
    {mcfile::blocks::minecraft::green_bed, Color(91, 119, 22)},
    {mcfile::blocks::minecraft::red_bed, Color(170, 42, 36)},
    {mcfile::blocks::minecraft::black_bed, Color(28, 28, 32)},
    {mcfile::blocks::minecraft::dried_kelp_block, Color(43, 55, 32)},
    {mcfile::blocks::minecraft::beacon, Color(72, 210, 202)},
    {mcfile::blocks::minecraft::shulker_box, Color(149, 101, 149)},
    {mcfile::blocks::minecraft::white_shulker_box, Color(225, 230, 230)},
    {mcfile::blocks::minecraft::orange_shulker_box, Color(225, 230, 230)},
    {mcfile::blocks::minecraft::magenta_shulker_box, Color(183, 61, 172)},
    {mcfile::blocks::minecraft::light_blue_shulker_box, Color(57, 177, 215)},
    {mcfile::blocks::minecraft::yellow_shulker_box, Color(249, 194, 34)},
    {mcfile::blocks::minecraft::lime_shulker_box, Color(108, 183, 24)},
    {mcfile::blocks::minecraft::pink_shulker_box, Color(239, 135, 166)},
    {mcfile::blocks::minecraft::gray_shulker_box, Color(59, 63, 67)},
    {mcfile::blocks::minecraft::light_gray_shulker_box, Color(135, 135, 126)},
    {mcfile::blocks::minecraft::cyan_shulker_box, Color(22, 133, 144)},
    {mcfile::blocks::minecraft::purple_shulker_box, Color(115, 38, 167)},
    {mcfile::blocks::minecraft::blue_shulker_box, Color(49, 52, 152)},
    {mcfile::blocks::minecraft::brown_shulker_box, Color(111, 69, 39)},
    {mcfile::blocks::minecraft::green_shulker_box, Color(83, 107, 29)},
    {mcfile::blocks::minecraft::red_shulker_box, Color(152, 35, 33)},
    {mcfile::blocks::minecraft::black_shulker_box, Color(31, 31, 34)},
    {mcfile::blocks::minecraft::bricks, kColorBricks},
    {mcfile::blocks::minecraft::cut_sandstone, kColorSandstone},
    {mcfile::blocks::minecraft::sandstone_stairs, kColorSandstone},
    {mcfile::blocks::minecraft::chiseled_sandstone, kColorSandstone},
    {mcfile::blocks::minecraft::sandstone_slab, kColorSandstone},
    {mcfile::blocks::minecraft::dark_oak_door, kColorPlanksDarkOak},
    {mcfile::blocks::minecraft::polished_diorite, kColorStoneDiorite},
    {mcfile::blocks::minecraft::coarse_dirt, Color(96, 67, 45)},
    {mcfile::blocks::minecraft::acacia_log, Color(104, 97, 88)},
    {mcfile::blocks::minecraft::oak_pressure_plate, kColorPlanksOak},
    {mcfile::blocks::minecraft::fire, Color(202, 115, 3)},
    {mcfile::blocks::minecraft::cobblestone_wall, Color(111, 111, 111)},
    {mcfile::blocks::minecraft::cobblestone_slab, Color(111, 111, 111)},
    {mcfile::blocks::minecraft::podzol, Color(105, 67, 23)},
    {mcfile::blocks::minecraft::sticky_piston, Color(131, 131, 131)},
    {mcfile::blocks::minecraft::piston_head, kColorPistonHead},
    {mcfile::blocks::minecraft::piston, Color(131, 131, 131)},
    {mcfile::blocks::minecraft::lever, Color(134, 133, 134)},
    {mcfile::blocks::minecraft::observer, Color(100, 100, 100)},
    {mcfile::blocks::minecraft::slime_block, Color(112, 187, 94)},
    {mcfile::blocks::minecraft::activator_rail, kColorRail},
    {mcfile::blocks::minecraft::oak_fence_gate, kColorPlanksOak},
    {mcfile::blocks::minecraft::dark_oak_fence_gate, kColorPlanksDarkOak},
    {mcfile::blocks::minecraft::birch_button, kColorPlanksBirch},
    {mcfile::blocks::minecraft::birch_slab, kColorPlanksBirch},
    {mcfile::blocks::minecraft::acacia_stairs, Color(184, 98, 55)},
    {mcfile::blocks::minecraft::acacia_pressure_plate, Color(184, 98, 55)},
    {mcfile::blocks::minecraft::redstone_wire, Color(75, 1, 0)},
    {mcfile::blocks::minecraft::redstone_block, Color(162, 24, 8)},
    {mcfile::blocks::minecraft::redstone_lamp, Color(173, 104, 58)},
    {mcfile::blocks::minecraft::hopper, Color(70, 70, 70)},
    {mcfile::blocks::minecraft::crafting_table, Color(156, 88, 49)},
    {mcfile::blocks::minecraft::lectern, Color(164, 128, 73)},
    {mcfile::blocks::minecraft::acacia_planks, Color(184, 98, 55)},
    {mcfile::blocks::minecraft::acacia_wood, Color(184, 98, 55)},
    {mcfile::blocks::minecraft::acacia_slab, Color(184, 98, 55)},
    {mcfile::blocks::minecraft::acacia_fence, Color(184, 98, 55)},
    {mcfile::blocks::minecraft::acacia_fence_gate, Color(184, 98, 55)},
    {mcfile::blocks::minecraft::acacia_door, Color(184, 98, 55)},
    {mcfile::blocks::minecraft::stripped_acacia_log, Color(184, 98, 55)},
    {mcfile::blocks::minecraft::brain_coral, Color(225, 125, 183)},
    {mcfile::blocks::minecraft::brain_coral_block, Color(225, 125, 183)},
    {mcfile::blocks::minecraft::brain_coral_fan, Color(225, 125, 183)},
    {mcfile::blocks::minecraft::brain_coral_wall_fan, Color(225, 125, 183)},
    {mcfile::blocks::minecraft::bubble_coral, Color(198, 25, 184)},
    {mcfile::blocks::minecraft::bubble_coral_block, Color(198, 25, 184)},
    {mcfile::blocks::minecraft::bubble_coral_fan, Color(198, 25, 184)},
    {mcfile::blocks::minecraft::bubble_coral_wall_fan, Color(198, 25, 184)},
    {mcfile::blocks::minecraft::horn_coral, Color(234, 233, 76)},
    {mcfile::blocks::minecraft::horn_coral_block, Color(234, 233, 76)},
    {mcfile::blocks::minecraft::horn_coral_fan, Color(234, 233, 76)},
    {mcfile::blocks::minecraft::horn_coral_wall_fan, Color(234, 233, 76)},
    {mcfile::blocks::minecraft::tube_coral, Color(48, 78, 218)},
    {mcfile::blocks::minecraft::tube_coral_block, Color(48, 78, 218)},
    {mcfile::blocks::minecraft::tube_coral_fan, Color(48, 78, 218)},
    {mcfile::blocks::minecraft::tube_coral_wall_fan, Color(48, 78, 218)},
    {mcfile::blocks::minecraft::fire_coral, Color(196, 42, 54)},
    {mcfile::blocks::minecraft::fire_coral_block, Color(196, 42, 54)},
    {mcfile::blocks::minecraft::fire_coral_fan, Color(196, 42, 54)},
    {mcfile::blocks::minecraft::fire_coral_wall_fan, Color(196, 42, 54)},
    {mcfile::blocks::minecraft::smooth_sandstone, kColorSandstone},
    {mcfile::blocks::minecraft::smooth_sandstone_slab, kColorSandstone},
    {mcfile::blocks::minecraft::smooth_sandstone_stairs, kColorSandstone},
    {mcfile::blocks::minecraft::sandstone_wall, kColorSandstone},
    {mcfile::blocks::minecraft::polished_andesite, kColorStoneAndesite},
    {mcfile::blocks::minecraft::carved_pumpkin, Color(213, 125, 50)},
    {mcfile::blocks::minecraft::stripped_oak_wood, Color(127, 85, 48)},
    {mcfile::blocks::minecraft::stonecutter, Color(131, 131, 131)},
    {mcfile::blocks::minecraft::smoker, Color(131, 131, 131)},
    {mcfile::blocks::minecraft::hay_block, Color(203, 176, 7)},
    {mcfile::blocks::minecraft::birch_log, Color(252, 252, 252)},
    {mcfile::blocks::minecraft::iron_trapdoor, Color(227, 227, 227)},
    {mcfile::blocks::minecraft::bell, Color(250, 211, 56)},
    {mcfile::blocks::minecraft::white_glazed_terracotta, Color(246, 252, 251)},
    {mcfile::blocks::minecraft::orange_glazed_terracotta, Color(26, 196, 197)},
    {mcfile::blocks::minecraft::magenta_glazed_terracotta, Color(201, 87, 192)},
    {mcfile::blocks::minecraft::light_blue_glazed_terracotta, Color(86, 187, 220)},
    {mcfile::blocks::minecraft::yellow_glazed_terracotta, Color(251, 219, 93)},
    {mcfile::blocks::minecraft::lime_glazed_terracotta, Color(137, 214, 35)},
    {mcfile::blocks::minecraft::pink_glazed_terracotta, Color(241, 179, 201)},
    {mcfile::blocks::minecraft::gray_glazed_terracotta, Color(94, 114, 118)},
    {mcfile::blocks::minecraft::light_gray_glazed_terracotta, Color(199, 203, 207)},
    {mcfile::blocks::minecraft::cyan_glazed_terracotta, Color(20, 159, 160)},
    {mcfile::blocks::minecraft::purple_glazed_terracotta, Color(146, 53, 198)},
    {mcfile::blocks::minecraft::blue_glazed_terracotta, Color(59, 66, 167)},
    {mcfile::blocks::minecraft::brown_glazed_terracotta, Color(167, 120, 79)},
    {mcfile::blocks::minecraft::green_glazed_terracotta, Color(111, 151, 36)},
    {mcfile::blocks::minecraft::furnace, kColorFurnace},
    {mcfile::blocks::minecraft::composter, Color(139, 91, 49)},
    {mcfile::blocks::minecraft::campfire, Color(199, 107, 3)},
    {mcfile::blocks::minecraft::cartography_table, Color(86, 53, 24)},
    {mcfile::blocks::minecraft::brewing_stand, Color(47, 47, 47)},
    {mcfile::blocks::minecraft::grindstone, Color(141, 141, 141)},
    {mcfile::blocks::minecraft::fletching_table, Color(212, 191, 131)},
    {mcfile::blocks::minecraft::iron_bars, Color(154, 154, 154)},
    {mcfile::blocks::minecraft::bookshelf, Color(192, 155, 97)},
    {mcfile::blocks::minecraft::acacia_sapling, Color(125, 150, 33)},
    {mcfile::blocks::minecraft::potted_dead_bush, kColorPotter},
    {mcfile::blocks::minecraft::potted_cactus, kColorPotter},
    {mcfile::blocks::minecraft::jack_o_lantern, Color(213, 125, 50)},
    {mcfile::blocks::minecraft::acacia_button, Color(184, 98, 55)},
    {mcfile::blocks::minecraft::acacia_sign, Color(184, 98, 55)},
    {mcfile::blocks::minecraft::acacia_trapdoor, Color(184, 98, 55)},
    {mcfile::blocks::minecraft::acacia_wall_sign, Color(184, 98, 55)},
    {mcfile::blocks::minecraft::andesite_slab, kColorStoneAndesite},
    {mcfile::blocks::minecraft::andesite_stairs, kColorStoneAndesite},
    {mcfile::blocks::minecraft::andesite_wall, kColorStoneAndesite},
    {mcfile::blocks::minecraft::anvil, kColorAnvil},
    {mcfile::blocks::minecraft::attached_melon_stem, Color(203, 196, 187)},
    {mcfile::blocks::minecraft::bamboo_sapling, Color(67, 103, 8)},
    {mcfile::blocks::minecraft::barrel, Color(137, 102, 60)},
    {mcfile::blocks::minecraft::birch_door, kColorPlanksBirch},
    {mcfile::blocks::minecraft::birch_fence_gate, kColorPlanksBirch},
    {mcfile::blocks::minecraft::birch_pressure_plate, kColorPlanksBirch},
    {mcfile::blocks::minecraft::birch_sapling, Color(107, 156, 55)},
    {mcfile::blocks::minecraft::birch_sign, kColorPlanksBirch},
    {mcfile::blocks::minecraft::birch_trapdoor, kColorPlanksBirch},
    {mcfile::blocks::minecraft::birch_wall_sign, kColorPlanksBirch},
    {mcfile::blocks::minecraft::birch_wood, Color(252, 252, 252)},
    {mcfile::blocks::minecraft::black_concrete, Color(8, 10, 15)},
    {mcfile::blocks::minecraft::black_concrete_powder, Color(22, 24, 29)},
    {mcfile::blocks::minecraft::black_glazed_terracotta, Color(24, 24, 27)},
    {mcfile::blocks::minecraft::blast_furnace, Color(131, 131, 131)},
    {mcfile::blocks::minecraft::coal_block, Color(13, 13, 13)},
    {mcfile::blocks::minecraft::diamond_block, Color(100, 242, 224)},
    {mcfile::blocks::minecraft::emerald_block, Color(62, 240, 130)},
    {mcfile::blocks::minecraft::gold_block, Color(251, 221, 72)},
    {mcfile::blocks::minecraft::iron_block, Color(227, 227, 227)},
    {mcfile::blocks::minecraft::iron_door, Color(227, 227, 227)},
    {mcfile::blocks::minecraft::potted_acacia_sapling, kColorPotter},
    {mcfile::blocks::minecraft::potted_allium, kColorPotter},
    {mcfile::blocks::minecraft::potted_azure_bluet, kColorPotter},
    {mcfile::blocks::minecraft::potted_bamboo, kColorPotter},
    {mcfile::blocks::minecraft::potted_birch_sapling, kColorPotter},
    {mcfile::blocks::minecraft::potted_blue_orchid, kColorPotter},
    {mcfile::blocks::minecraft::potted_brown_mushroom, kColorPotter},
    {mcfile::blocks::minecraft::potted_cornflower, kColorPotter},
    {mcfile::blocks::minecraft::potted_dandelion, kColorPotter},
    {mcfile::blocks::minecraft::potted_dark_oak_sapling, kColorPotter},
    {mcfile::blocks::minecraft::potted_fern, kColorPotter},
    {mcfile::blocks::minecraft::potted_jungle_sapling, kColorPotter},
    {mcfile::blocks::minecraft::potted_lily_of_the_valley, kColorPotter},
    {mcfile::blocks::minecraft::potted_oak_sapling, kColorPotter},
    {mcfile::blocks::minecraft::potted_orange_tulip, kColorPotter},
    {mcfile::blocks::minecraft::potted_oxeye_daisy, kColorPotter},
    {mcfile::blocks::minecraft::potted_pink_tulip, kColorPotter},
    {mcfile::blocks::minecraft::potted_poppy, kColorPotter},
    {mcfile::blocks::minecraft::potted_red_mushroom, kColorPotter},
    {mcfile::blocks::minecraft::potted_red_tulip, kColorPotter},
    {mcfile::blocks::minecraft::potted_spruce_sapling, kColorPotter},
    {mcfile::blocks::minecraft::potted_white_tulip, kColorPotter},
    {mcfile::blocks::minecraft::potted_wither_rose, kColorPotter},
    {mcfile::blocks::minecraft::dark_oak_button, kColorPlanksDarkOak},
    {mcfile::blocks::minecraft::dark_oak_pressure_plate, kColorPlanksDarkOak},
    {mcfile::blocks::minecraft::dark_oak_sign,kColorPlanksDarkOak},
    {mcfile::blocks::minecraft::dark_oak_wall_sign, kColorPlanksDarkOak},
    {mcfile::blocks::minecraft::oak_button, kColorPlanksOak},
    {mcfile::blocks::minecraft::oak_sign, kColorPlanksOak},
    {mcfile::blocks::minecraft::oak_wall_sign, kColorPlanksOak},
    {mcfile::blocks::minecraft::brick_slab, kColorBricks},
    {mcfile::blocks::minecraft::brick_stairs, kColorBricks},
    {mcfile::blocks::minecraft::brick_wall, kColorBricks},
    {mcfile::blocks::minecraft::chipped_anvil, kColorAnvil},
    {mcfile::blocks::minecraft::damaged_anvil, kColorAnvil},
    {mcfile::blocks::minecraft::daylight_detector, Color(188, 168, 140)},
    {mcfile::blocks::minecraft::dead_brain_coral, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_brain_coral_block, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_brain_coral_fan, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_brain_coral_wall_fan, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_bubble_coral, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_bubble_coral_block, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_bubble_coral_fan, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_bubble_coral_wall_fan, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_fire_coral, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_fire_coral_block, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_fire_coral_fan, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_fire_coral_wall_fan, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_horn_coral, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_horn_coral_block, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_horn_coral_fan, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_horn_coral_wall_fan, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_tube_coral, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_tube_coral_block, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_tube_coral_fan, kColorDeadCoral},
    {mcfile::blocks::minecraft::dead_tube_coral_wall_fan, kColorDeadCoral},
    {mcfile::blocks::minecraft::detector_rail, kColorRail},
    {mcfile::blocks::minecraft::powered_rail, kColorRail},
    {mcfile::blocks::minecraft::diorite_slab, kColorStoneDiorite},
    {mcfile::blocks::minecraft::diorite_stairs, kColorStoneDiorite},
    {mcfile::blocks::minecraft::diorite_wall, kColorStoneDiorite},
    {mcfile::blocks::minecraft::polished_diorite_slab, kColorStoneDiorite},
    {mcfile::blocks::minecraft::polished_diorite_stairs, kColorStoneDiorite},
    {mcfile::blocks::minecraft::granite_slab, kColorStoneGranite},
    {mcfile::blocks::minecraft::granite_stairs, kColorStoneGranite},
    {mcfile::blocks::minecraft::granite_wall, kColorStoneGranite},
    {mcfile::blocks::minecraft::polished_granite_slab, kColorStoneGranite},
    {mcfile::blocks::minecraft::polished_granite_stairs, kColorStoneGranite},
    {mcfile::blocks::minecraft::jungle_fence_gate, kColorPlanksJungle},
    {mcfile::blocks::minecraft::jungle_pressure_plate, kColorPlanksJungle},
    {mcfile::blocks::minecraft::jungle_sign, kColorPlanksJungle},
    {mcfile::blocks::minecraft::jungle_wall_sign, kColorPlanksJungle},
    {mcfile::blocks::minecraft::nether_brick_fence, kColorNetherBricks},
    {mcfile::blocks::minecraft::nether_brick_stairs, kColorNetherBricks},
    {mcfile::blocks::minecraft::stone_button, kColorStone},
    {mcfile::blocks::minecraft::stone_pressure_plate, kColorStone},
    {mcfile::blocks::minecraft::stone_stairs, kColorStone},
    {mcfile::blocks::minecraft::spruce_button, kColorPlanksSpruce},
    {mcfile::blocks::minecraft::spruce_fence_gate, kColorPlanksSpruce},
    {mcfile::blocks::minecraft::spruce_pressure_plate, kColorPlanksSpruce},
    {mcfile::blocks::minecraft::spruce_sign, kColorPlanksSpruce},
    {mcfile::blocks::minecraft::spruce_wall_sign, kColorPlanksSpruce},
    {mcfile::blocks::minecraft::dispenser, kColorFurnace},
    {mcfile::blocks::minecraft::dropper, kColorFurnace},
    {mcfile::blocks::minecraft::quartz_block, kColorQuartz},
    {mcfile::blocks::minecraft::blue_concrete_powder, Color(72, 75, 175)},
    {mcfile::blocks::minecraft::brown_concrete_powder, Color(120, 81, 50)},
    {mcfile::blocks::minecraft::cyan_concrete_powder, Color(37, 154, 160)},
    {mcfile::blocks::minecraft::gray_concrete_powder, Color(75, 79, 82)},
    {mcfile::blocks::minecraft::green_concrete_powder, Color(103, 126, 37)},
    {mcfile::blocks::minecraft::light_blue_concrete_powder, Color(91, 194, 216)},
    {mcfile::blocks::minecraft::light_gray_concrete_powder, Color(154, 154, 148)},
    {mcfile::blocks::minecraft::lime_concrete_powder, Color(138, 197, 45)},
    {mcfile::blocks::minecraft::magenta_concrete_powder, Color(200, 93, 193)},
    {mcfile::blocks::minecraft::orange_concrete_powder, Color(230, 128, 20)},
    {mcfile::blocks::minecraft::pink_concrete_powder, Color(236, 172, 195)},
    {mcfile::blocks::minecraft::purple_concrete_powder, Color(138, 58, 186)},
    {mcfile::blocks::minecraft::red_concrete_powder, Color(180, 58, 55)},
    {mcfile::blocks::minecraft::white_concrete_powder, Color(222, 223, 224)},
    {mcfile::blocks::minecraft::yellow_concrete_powder, Color(235, 209, 64)},
    {mcfile::blocks::minecraft::end_stone_brick_slab, kColorEndStoneBricks},
    {mcfile::blocks::minecraft::end_stone_brick_stairs, kColorEndStoneBricks},
    {mcfile::blocks::minecraft::end_stone_brick_wall, kColorEndStoneBricks},
    {mcfile::blocks::minecraft::end_stone_bricks, kColorEndStoneBricks},
    {mcfile::blocks::minecraft::blue_concrete, Color(44, 46, 142)},
    {mcfile::blocks::minecraft::bone_block, Color(199, 195, 165)},
    {mcfile::blocks::minecraft::brown_concrete, Color(95, 58, 31)},
    {mcfile::blocks::minecraft::cake, Color(238, 229, 203)},
    {mcfile::blocks::minecraft::cauldron, Color(53, 52, 52)},
    {mcfile::blocks::minecraft::chain_command_block, Color(159, 193, 178)},
    {mcfile::blocks::minecraft::chiseled_quartz_block, kColorQuartz},
    {mcfile::blocks::minecraft::chiseled_red_sandstone, kColorRedSandstone},
    {mcfile::blocks::minecraft::command_block, Color(196, 125, 78)},
    {mcfile::blocks::minecraft::conduit, Color(126, 113, 81)},
    {mcfile::blocks::minecraft::cut_red_sandstone, kColorRedSandstone},
    {mcfile::blocks::minecraft::cut_red_sandstone_slab, kColorRedSandstone},
    {mcfile::blocks::minecraft::red_sandstone, kColorRedSandstone},
    {mcfile::blocks::minecraft::red_sandstone_slab, kColorRedSandstone},
    {mcfile::blocks::minecraft::red_sandstone_stairs, kColorRedSandstone},
    {mcfile::blocks::minecraft::red_sandstone_wall, kColorRedSandstone},
    {mcfile::blocks::minecraft::smooth_red_sandstone, kColorRedSandstone},
    {mcfile::blocks::minecraft::smooth_red_sandstone_slab, kColorRedSandstone},
    {mcfile::blocks::minecraft::smooth_red_sandstone_stairs, kColorRedSandstone},
    {mcfile::blocks::minecraft::cut_sandstone_slab, kColorSandstone},
    {mcfile::blocks::minecraft::cyan_concrete, Color(21, 118, 134)},
    {mcfile::blocks::minecraft::dark_oak_sapling, Color(31, 100, 25)},
    {mcfile::blocks::minecraft::dark_oak_wood, Color(62, 48, 29)},
    {mcfile::blocks::minecraft::dragon_egg, Color(9, 9, 9)},
    {mcfile::blocks::minecraft::dragon_head, kColorDragonHead},
    {mcfile::blocks::minecraft::dragon_wall_head, kColorDragonHead},
    {mcfile::blocks::minecraft::quartz_pillar, kColorQuartz},
    {mcfile::blocks::minecraft::quartz_slab, kColorQuartz},
    {mcfile::blocks::minecraft::quartz_stairs, kColorQuartz},
    {mcfile::blocks::minecraft::emerald_ore, kColorStone},
    {mcfile::blocks::minecraft::polished_andesite_slab, kColorStoneAndesite},
    {mcfile::blocks::minecraft::polished_andesite_stairs, kColorStoneAndesite},
    {mcfile::blocks::minecraft::mossy_cobblestone_slab, kColorMossyStone},
    {mcfile::blocks::minecraft::mossy_cobblestone_stairs, kColorMossyStone},
    {mcfile::blocks::minecraft::mossy_cobblestone_wall, kColorMossyStone},
    {mcfile::blocks::minecraft::infested_cobblestone, kColorStone},
    {mcfile::blocks::minecraft::infested_mossy_stone_bricks, kColorMossyStone},
    {mcfile::blocks::minecraft::mossy_stone_brick_slab, kColorMossyStone},
    {mcfile::blocks::minecraft::mossy_stone_brick_stairs, kColorMossyStone},
    {mcfile::blocks::minecraft::mossy_stone_brick_wall, kColorMossyStone},
    {mcfile::blocks::minecraft::infested_chiseled_stone_bricks, kColorStone},
    {mcfile::blocks::minecraft::infested_cracked_stone_bricks, kColorStone},
    {mcfile::blocks::minecraft::infested_stone_bricks, kColorStone},
    {mcfile::blocks::minecraft::moving_piston, kColorPistonHead},
    {mcfile::blocks::minecraft::smooth_quartz, kColorQuartz},
    {mcfile::blocks::minecraft::smooth_quartz_slab, kColorQuartz},
    {mcfile::blocks::minecraft::smooth_quartz_stairs, kColorQuartz},
    {mcfile::blocks::minecraft::stone_brick_slab, kColorStone},
    {mcfile::blocks::minecraft::stone_brick_wall, kColorStone},
    {mcfile::blocks::minecraft::purpur_stairs, kColorPurPur},
    {mcfile::blocks::minecraft::prismarine_wall, kColorPrismarine},
    {mcfile::blocks::minecraft::red_nether_brick_stairs, kColorRedNetherBricks},
    {mcfile::blocks::minecraft::creeper_head, kColorCreaperHead},
    {mcfile::blocks::minecraft::creeper_wall_head, kColorCreaperHead},
    {mcfile::blocks::minecraft::enchanting_table, Color(73, 234, 207)},
    {mcfile::blocks::minecraft::end_gateway, Color(3, 13, 20)},
    {mcfile::blocks::minecraft::gray_concrete, Color(53, 57, 61)},
    {mcfile::blocks::minecraft::green_concrete, Color(72, 90, 35)},
    {mcfile::blocks::minecraft::heavy_weighted_pressure_plate, Color(182, 182, 182)},
    {mcfile::blocks::minecraft::jigsaw, Color(147, 120, 148)},
    {mcfile::blocks::minecraft::jukebox, Color(122, 79, 56)},
    {mcfile::blocks::minecraft::jungle_sapling, Color(41, 73, 12)},
    {mcfile::blocks::minecraft::jungle_wood, Color(88, 69, 26)},
    {mcfile::blocks::minecraft::lantern, Color(72, 79, 100)},
    {mcfile::blocks::minecraft::lapis_block, Color(24, 59, 115)},
    {mcfile::blocks::minecraft::light_blue_concrete, Color(37, 136, 198)},
    {mcfile::blocks::minecraft::light_gray_concrete, Color(125, 125, 115)},
    {mcfile::blocks::minecraft::light_weighted_pressure_plate, Color(202, 171, 50)},
    {mcfile::blocks::minecraft::lime_concrete, Color(93, 167, 24)},
    {mcfile::blocks::minecraft::loom, Color(200, 164, 112)},
    {mcfile::blocks::minecraft::magenta_concrete, Color(168, 49, 158)},
    {mcfile::blocks::minecraft::nether_wart_block, Color(122, 1, 0)},
    {mcfile::blocks::minecraft::note_block, Color(146, 92, 64)},
    {mcfile::blocks::minecraft::oak_sapling, Color(63, 141, 46)},
    {mcfile::blocks::minecraft::oak_wood, kColorOakLog},
    {mcfile::blocks::minecraft::orange_concrete, Color(222, 97, 0)},
    {mcfile::blocks::minecraft::petrified_oak_slab, kColorPlanksOak},
    {mcfile::blocks::minecraft::pink_concrete, Color(210, 100, 141)},
    {mcfile::blocks::minecraft::player_head, kColorPlayerHead},
    {mcfile::blocks::minecraft::player_wall_head, kColorPlayerHead},
    {mcfile::blocks::minecraft::purple_concrete, Color(99, 32, 154)},
    {mcfile::blocks::minecraft::red_concrete, Color(138, 32, 32)},
    {mcfile::blocks::minecraft::red_glazed_terracotta, Color(202, 65, 57)},
    {mcfile::blocks::minecraft::comparator, Color(185, 185, 185)},
    {mcfile::blocks::minecraft::repeater, Color(185, 185, 185)},
    {mcfile::blocks::minecraft::repeating_command_block, Color(105, 78, 197)},
    {mcfile::blocks::minecraft::scaffolding, Color(225, 196, 115)},
    {mcfile::blocks::minecraft::skeleton_skull, kColorSkeltonSkull},
    {mcfile::blocks::minecraft::skeleton_wall_skull, kColorSkeltonSkull},
    {mcfile::blocks::minecraft::smithing_table, Color(63, 65, 82)},
    {mcfile::blocks::minecraft::spawner, Color(24, 43, 56)},
    {mcfile::blocks::minecraft::sponge, Color(203, 204, 73)},
    {mcfile::blocks::minecraft::spruce_sapling, Color(34, 52, 34)},
    {mcfile::blocks::minecraft::spruce_wood, kColorSpruceLog},
    {mcfile::blocks::minecraft::stripped_acacia_wood, Color(185, 94, 61)},
    {mcfile::blocks::minecraft::stripped_birch_log, Color(205, 186, 126)},
    {mcfile::blocks::minecraft::stripped_birch_wood, Color(205, 186, 126)},
    {mcfile::blocks::minecraft::stripped_dark_oak_log, Color(107, 83, 51)},
    {mcfile::blocks::minecraft::stripped_dark_oak_wood, Color(107, 83, 51)},
    {mcfile::blocks::minecraft::stripped_jungle_log, Color(173, 126, 82)},
    {mcfile::blocks::minecraft::stripped_jungle_wood, Color(173, 126, 82)},
    {mcfile::blocks::minecraft::stripped_oak_log, Color(148, 115, 64)},
    {mcfile::blocks::minecraft::stripped_spruce_log, Color(120, 90, 54)},
    {mcfile::blocks::minecraft::stripped_spruce_wood, Color(120, 90, 54)},
    {mcfile::blocks::minecraft::structure_block, Color(147, 120, 148)},
    {mcfile::blocks::minecraft::trapped_chest, kColorChest},
    {mcfile::blocks::minecraft::tripwire_hook, Color(135, 135, 135)},
    {mcfile::blocks::minecraft::turtle_egg, Color(224, 219, 197)},
    {mcfile::blocks::minecraft::wet_sponge, Color(174, 189, 74)},
    {mcfile::blocks::minecraft::white_concrete, Color(204, 209, 210)},
    {mcfile::blocks::minecraft::wither_rose, Color(23, 18, 16)},
    {mcfile::blocks::minecraft::wither_skeleton_skull, kColorWitherSkeltonSkull},
    {mcfile::blocks::minecraft::wither_skeleton_wall_skull, kColorWitherSkeltonSkull},
    {mcfile::blocks::minecraft::yellow_concrete, Color(239, 175, 22)},
    {mcfile::blocks::minecraft::zombie_head, kColorZombieHead},
    {mcfile::blocks::minecraft::zombie_wall_head, kColorZombieHead},
    {mcfile::blocks::minecraft::end_rod, Color(202, 202, 202)},
    {mcfile::blocks::minecraft::flower_pot, kColorPotter},
    {mcfile::blocks::minecraft::frosted_ice, Color(109, 146, 193)},
    {mcfile::blocks::minecraft::nether_portal, Color(78, 30, 135)},

    // plants
    {mcfile::blocks::minecraft::lily_pad, Color(0, 123, 0)},
    {mcfile::blocks::minecraft::wheat, Color(0, 123, 0)},
    {mcfile::blocks::minecraft::melon, Color(125, 202, 25)},
    {mcfile::blocks::minecraft::pumpkin, Color(213, 125, 50)},
    {mcfile::blocks::minecraft::grass, Color(109, 141, 35)},
    {mcfile::blocks::minecraft::tall_grass, Color(109, 141, 35)},
    {mcfile::blocks::minecraft::dandelion, Color(245, 238, 50)},
    {mcfile::blocks::minecraft::poppy, Color(229, 31, 29)},
    {mcfile::blocks::minecraft::peony, Color(232, 143, 213)},
    {mcfile::blocks::minecraft::pink_tulip, Color(234, 182, 209)},
    {mcfile::blocks::minecraft::orange_tulip, Color(242, 118, 33)},
    {mcfile::blocks::minecraft::lilac, Color(212, 119, 197)},
    {mcfile::blocks::minecraft::sunflower, Color(245, 238, 50)},
    {mcfile::blocks::minecraft::allium, Color(200, 109, 241)},
    {mcfile::blocks::minecraft::red_tulip, Color(229, 31, 29)},
    {mcfile::blocks::minecraft::white_tulip, Color(255, 255, 255)},
    {mcfile::blocks::minecraft::rose_bush, Color(136, 40, 27)},
    {mcfile::blocks::minecraft::blue_orchid, Color(47, 181, 199)},
    {mcfile::blocks::minecraft::oxeye_daisy, Color(236, 246, 247)},
    {mcfile::blocks::minecraft::sugar_cane, Color(165, 214, 90)},
    {mcfile::blocks::minecraft::chorus_plant, Color(90, 51, 90)},
    {mcfile::blocks::minecraft::chorus_flower, Color(159, 119, 159)},
    {mcfile::blocks::minecraft::dark_oak_leaves, Color(58, 82, 23)},
    {mcfile::blocks::minecraft::red_mushroom_block, Color(199, 42, 41)},
    {mcfile::blocks::minecraft::mushroom_stem, Color(203, 196, 187)},
    {mcfile::blocks::minecraft::brown_mushroom_block, Color(149, 113, 80)},
    {mcfile::blocks::minecraft::acacia_leaves, Color(63, 89, 25)},
    {mcfile::blocks::minecraft::dead_bush, Color(146, 99, 40)},
    {mcfile::blocks::minecraft::cactus, Color(90, 138, 42)},
    {mcfile::blocks::minecraft::sweet_berry_bush, Color(40, 97, 63)},
    {mcfile::blocks::minecraft::cornflower, Color(69, 105, 232)},
    {mcfile::blocks::minecraft::pumpkin_stem, Color(72, 65, 9)},
    {mcfile::blocks::minecraft::nether_wart, Color(163, 35, 41)},
    {mcfile::blocks::minecraft::attached_pumpkin_stem, Color(72, 65, 9)},
    {mcfile::blocks::minecraft::lily_of_the_valley, Color(252, 252, 252)},
    {mcfile::blocks::minecraft::melon_stem, Color(72, 65, 9)},
    {mcfile::blocks::minecraft::smooth_stone, Color(111, 111, 111)},
    {mcfile::blocks::minecraft::smooth_stone_slab, Color(111, 111, 111)},
    {mcfile::blocks::minecraft::bamboo, Color(67, 103, 8)},
    {mcfile::blocks::minecraft::sea_pickle, Color(106, 113, 42)},
    {mcfile::blocks::minecraft::cocoa, Color(109, 112, 52)},
};

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
