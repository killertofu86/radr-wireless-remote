#pragma once

#include <cstdint>

namespace startupImages {

enum class Mode : uint8_t {
    BuildDefault = 0,
    Shuffle = 1,
    FixedOrder = 2,
    FixedImage = 3,
};

struct Selection {
    Mode mode = Mode::BuildDefault;
    uint8_t imageIndex = 0;

    bool operator==(const Selection &other) const {
        return mode == other.mode && imageIndex == other.imageIndex;
    }
};

void draw();
uint8_t imageCount();
const char *imageName(uint8_t index);
Selection selected();
bool select(Selection selection);

}  // namespace startupImages
