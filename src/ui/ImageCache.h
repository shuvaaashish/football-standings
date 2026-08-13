#ifndef IMAGE_CACHE_H
#define IMAGE_CACHE_H

#include "imgui.h"
#include <map>
#include <set>
#include <string>

namespace UI {

class ImageCache {
public:
    ImageCache();
    ~ImageCache();

    // Returns a valid ImTextureID for the URL, or 0 if the image cannot be loaded.
    ImTextureID getTexture(const std::string& url);

    // Clears all cached textures from GPU memory.
    void clear();

private:
    std::map<std::string, unsigned int> textures_;
    // Failed URLs are remembered so a broken crest cannot trigger a download
    // attempt on every frame.
    std::set<std::string> failedUrls_;
    unsigned int loadTextureFromMemory(const unsigned char* data, size_t size);
};

} // namespace UI

#endif
