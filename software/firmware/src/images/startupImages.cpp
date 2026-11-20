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
#include "services/display.h"
#include "taur1.h"
#include "zyl1.h"

namespace startupImages {
namespace {

struct Bitmap {
    uint8_t id;
    const char *name;
    const uint16_t *pixels;
    uint16_t width;
    uint16_t height;
};

constexpr uint8_t pictureIdLimit = 32;
constexpr Bitmap buildDefault{pictureIdLimit, "", CURRENT_BUILD_IMAGE, CURRENT_BUILD_WIDTH,
                              CURRENT_BUILD_HEIGHT};
// IDs are stored in NVS. Keep them in ascending order and leave ID 4 reserved
// for the removed RANDR picture.
constexpr Bitmap pictures[] = {
    {0, "HSC1", HSC1, HSC1_WIDTH, HSC1_HEIGHT},
    {1, "HSC2", HSC2, HSC2_WIDTH, HSC2_HEIGHT},
    {2, "DRA", DRA, DRA_WIDTH, DRA_HEIGHT},
    {3, "Miku Taur", MIKU_TAUR_H, MIKU_TAUR_H_WIDTH, MIKU_TAUR_H_HEIGHT},
    {5, "Taur1", TAUR1_H, TAUR1_H_WIDTH, TAUR1_H_HEIGHT},
    {6, "Zyl1", B415DFF9B09D08A65DCAC1322940468B,
     B415DFF9B09D08A65DCAC1322940468B_WIDTH,
     B415DFF9B09D08A65DCAC1322940468B_HEIGHT},
};
static_assert(std::size(pictures) > 1 && std::size(pictures) <= pictureIdLimit);
constexpr uint8_t pictureCount = std::size(pictures);
constexpr uint32_t pictureBit(uint8_t id) { return uint32_t{1} << id; }
constexpr uint32_t allPictures = [] {
    uint32_t mask = 0;
    for (const auto &picture : pictures) mask |= pictureBit(picture.id);
    return mask;
}();
constexpr char preferenceNamespace[] = "startup-image";
constexpr uint8_t firstFixedImage = static_cast<uint8_t>(Mode::FixedImage);

struct ShuffleState {
    uint32_t remaining = 0;
    uint32_t last = pictureIdLimit;
};

Selection savedSelection;

uint8_t nextInOrder(Preferences &preferences) {
    const uint8_t storedId = preferences.getUChar("next", pictures[0].id);
    uint8_t index = 0;
    while (index < pictureCount && pictures[index].id < storedId) ++index;
    if (index == pictureCount) index = 0;

    const uint8_t nextIndex = (index + 1) % pictureCount;
    preferences.putUChar("next", pictures[nextIndex].id);
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
        if (shuffle.last < pictureIdLimit) available &= ~pictureBit(shuffle.last);
    }

    std::array<uint8_t, pictureCount> candidates{};
    uint8_t candidateCount = 0;
    for (uint8_t index = 0; index < pictureCount; ++index) {
        if (available & pictureBit(pictures[index].id)) {
            candidates[candidateCount++] = index;
        }
    }
    const uint8_t index = candidates[esp_random() % candidateCount];
    shuffle.remaining &= ~pictureBit(pictures[index].id);
    shuffle.last = pictures[index].id;
    preferences.putBytes("shuffle", &shuffle, sizeof(shuffle));
    return index;
}

const Bitmap &pickImage(Preferences &preferences) {
    const uint8_t stored = preferences.getUChar("mode", 0);
    if (stored >= firstFixedImage) {
        const uint8_t id = stored - firstFixedImage;
        for (uint8_t index = 0; index < pictureCount; ++index) {
            if (pictures[index].id == id) {
                savedSelection = {Mode::FixedImage, index};
                return pictures[index];
            }
        }
        return buildDefault;
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
        value += pictures[selection.imageIndex].id;
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
