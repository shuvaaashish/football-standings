#include "ImageCache.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <curl/curl.h>
#include <GL/gl.h>
#include <string>

namespace {

struct DownloadBuffer {
    std::string data;
};

size_t writeCallback(void* ptr, size_t size, size_t count, void* userdata) {
    size_t total = size * count;
    DownloadBuffer* buffer = static_cast<DownloadBuffer*>(userdata);
    if (!buffer) return 0;
    buffer->data.append(static_cast<const char*>(ptr), total);
    return total;
}

} // namespace

namespace UI {

ImageCache::ImageCache() {}

ImageCache::~ImageCache() {
    clear();
}

void ImageCache::clear() {
    for (auto& kv : textures_) {
        if (kv.second != 0) {
            glDeleteTextures(1, &kv.second);
        }
    }
    textures_.clear();
    failedUrls_.clear();
}

unsigned int ImageCache::loadTextureFromMemory(const unsigned char* data, size_t size) {
    if (!data || size == 0) {
        return 0;
    }

    int width = 0;
    int height = 0;
    int channels = 0;

    unsigned char* pixels = stbi_load_from_memory(data, static_cast<int>(size), &width, &height, &channels, STBI_rgb_alpha);
    if (!pixels) {
        return 0;
    }

    GLuint textureId = 0;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(pixels);
    return textureId;
}

ImTextureID ImageCache::getTexture(const std::string& url) {
    if (url.empty()) {
        return 0;
    }

    auto it = textures_.find(url);
    if (it != textures_.end()) {
        return static_cast<ImTextureID>(it->second);
    }
    if (failedUrls_.find(url) != failedUrls_.end()) {
        return 0;
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        failedUrls_.insert(url);
        return 0;
    }

    DownloadBuffer buffer;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "FootballHub/1.0");

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK || buffer.data.empty()) {
        failedUrls_.insert(url);
        return 0;
    }

    unsigned int textureId = loadTextureFromMemory(reinterpret_cast<const unsigned char*>(buffer.data.data()), buffer.data.size());
    if (textureId == 0) {
        failedUrls_.insert(url);
        return 0;
    }

    textures_[url] = textureId;
    return static_cast<ImTextureID>(textureId);
}

} // namespace UI
