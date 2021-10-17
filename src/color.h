#pragma once

#include <cstdint>
#include <cmath>
#include <string>
#include <sstream>

struct HSV {
    float fH;
    float fS;
    float fV;
};


class Color {
public:
    Color() : fR(0), fG(0), fB(0), fA(1) {}
    
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

    static Color Blend(Color fg, Color bg) {
        return Blend_(fg, bg);
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

    static Color Blend_(Color src, Color dst) {
        float a = src.fA + dst.fA * (1 - src.fA);
        if (a <= 0) {
            return Color::FromFloat(0, 0, 0, 0);
        }
        float r = (src.fR * src.fA + dst.fR * dst.fA * (1 - src.fA)) / a;
        float g = (src.fG * src.fA + dst.fG * dst.fA * (1 - src.fA)) / a;
        float b = (src.fB * src.fA + dst.fB * dst.fA * (1 - src.fA)) / a;
        return Color::FromFloat(r, g, b, a);
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
