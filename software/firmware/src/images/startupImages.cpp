#include "startupImages.h"

#include <Preferences.h>
#include <esp_random.h>

#include <array>
#include <iterator>

#include "current_build.h"
#include "dra.h"
#include "hsc1.h"
#include "hsc2.h"
#include "miku_taur.h"
#include "randr.h"
#include "services/display.h"
#include "taur1.h"
#include "zyl1.h"

namespace startupImages {
namespace {

struct Bitmap {
    const char *name;
    const uint16_t *pixels;
    uint16_t width;
    uint16_t height;
};

constexpr Bitmap buildDefault{"", CURRENT_BUILD_IMAGE, CURRENT_BUILD_WIDTH,
                              CURRENT_BUILD_HEIGHT};
// Append new images so stored selections continue to use the same indices.
constexpr Bitmap pictures[] = {
    {"HSC1", HSC1, HSC1_WIDTH, HSC1_HEIGHT},
    {"HSC2", HSC2, HSC2_WIDTH, HSC2_HEIGHT},
    {"DRA", DRA, DRA_WIDTH, DRA_HEIGHT},
    {"Miku Taur", MIKU_TAUR_H, MIKU_TAUR_H_WIDTH, MIKU_TAUR_H_HEIGHT},
    {"RANDR", RANDR_H, RANDR_H_WIDTH, RANDR_H_HEIGHT},
    {"Taur1", TAUR1_H, TAUR1_H_WIDTH, TAUR1_H_HEIGHT},
    {"Zyl1", B415DFF9B09D08A65DCAC1322940468B,
     B415DFF9B09D08A65DCAC1322940468B_WIDTH,
     B415DFF9B09D08A65DCAC1322940468B_HEIGHT},
};
static_assert(std::size(pictures) > 1 && std::size(pictures) <= 32);
constexpr uint8_t pictureCount = std::size(pictures);
constexpr uint32_t allPictures = UINT32_MAX >> (32 - pictureCount);
constexpr char preferenceNamespace[] = "startup-image";
constexpr uint8_t firstFixedImage = static_cast<uint8_t>(Mode::FixedImage);

struct ShuffleState {
    uint32_t remaining = 0;
    uint32_t last = pictureCount;
};

Selection savedSelection;

uint8_t nextInOrder(Preferences &preferences) {
    const uint8_t stored = preferences.getUChar("next", 0);
    const uint8_t index = stored < pictureCount ? stored : 0;
    preferences.putUChar("next", (index + 1) % pictureCount);
    return index;
}

uint8_t nextShuffled(Preferences &preferences) {
    ShuffleState shuffle;
    if (preferences.isKey("shuffle") &&
        preferences.getBytesLength("shuffle") == sizeof(shuffle)) {
        preferences.getBytes("shuffle", &shuffle, sizeof(shuffle));
    }
    shuffle.remaining &= allPictures;
    uint32_t available = shuffle.remaining;
    if (available == 0) {
        shuffle.remaining = allPictures;
        available = allPictures;
        // Start a fresh cycle with a different picture than the previous one.
        if (shuffle.last < pictureCount) available &= ~(1u << shuffle.last);
    }

    std::array<uint8_t, pictureCount> candidates{};
    uint8_t candidateCount = 0;
    for (uint8_t index = 0; index < pictureCount; ++index) {
        if (available & (1u << index)) candidates[candidateCount++] = index;
    }
    const uint8_t index = candidates[esp_random() % candidateCount];
    shuffle.remaining &= ~(1u << index);
    shuffle.last = index;
    preferences.putBytes("shuffle", &shuffle, sizeof(shuffle));
    return index;
}

const Bitmap &pickImage(Preferences &preferences) {
    const uint8_t stored = preferences.getUChar("mode", 0);
    if (stored >= firstFixedImage) {
        const uint8_t index = stored - firstFixedImage;
        if (index >= pictureCount) return buildDefault;
        savedSelection = {Mode::FixedImage, index};
        return pictures[index];
    }

    savedSelection = {static_cast<Mode>(stored)};
    if (savedSelection.mode == Mode::Shuffle) {
        return pictures[nextShuffled(preferences)];
    }
    if (savedSelection.mode == Mode::FixedOrder) {
        return pictures[nextInOrder(preferences)];
    }
    return buildDefault;
}

}  // namespace

void draw() {
    Preferences preferences;
    savedSelection = {};
    const Bitmap &image = preferences.begin(preferenceNamespace)
                              ? pickImage(preferences)
                              : buildDefault;
    preferences.end();
    drawImage(image.pixels, image.width, image.height);
}

uint8_t imageCount() { return pictureCount; }

const char *imageName(uint8_t index) { return pictures[index].name; }

Selection selected() {
    if (savedSelection.mode != Mode::BuildDefault) return savedSelection;
    for (uint8_t index = 0; index < pictureCount; ++index) {
        if (pictures[index].pixels == buildDefault.pixels) {
            return {Mode::FixedImage, index};
        }
    }
    return savedSelection;
}

bool select(Selection selection) {
    if (selection == savedSelection) return true;
    auto value = static_cast<uint8_t>(selection.mode);
    if (value > firstFixedImage) return false;
    if (selection.mode == Mode::FixedImage) {
        if (selection.imageIndex >= pictureCount) return false;
        value += selection.imageIndex;
    }

    Preferences preferences;
    if (!preferences.begin(preferenceNamespace) ||
        preferences.putUChar("mode", value) != sizeof(value)) {
        return false;
    }
    savedSelection = selection;
    return true;
}

}  // namespace startupImages
