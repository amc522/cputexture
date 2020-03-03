#include <cputex/unique_texture.h>
#include <cputex/shared_texture.h>
#include <cputex/sampler.h>

#include <vector>
#include <numeric>

int main(int argc, const char **argv) {
    cputex::TextureParams params;
    params.dimension = cputex::TextureDimension::Texture2D;
    params.extent = cputex::Extent{ 4, 4, 0 };
    params.format = gpufmt::Format::R4G4_UNORM_PACK8;
    params.mips = 15;
    params.arraySize = 1;

    std::vector<uint32_t> feces;
    feces.resize(32);
    std::iota(feces.begin(), feces.end(), 0);

    auto initData = cputex::span<const cputex::byte>(reinterpret_cast<const cputex::byte*>(feces.data()), feces.size() * sizeof(uint32_t));
    cputex::UniqueTexture unique(params, initData);
    cputex::SharedTexture shared(params, initData);

    cputex::SharedTexture newShared = std::move(shared);

    cputex::Sampler sampler{ unique };
    auto sample1 = sampler.sample({ 0.0f, 0.0f, 0.0f });
    auto sample2 = sampler.sample({ 0.5f, 0.5f, 0.0f });
    auto sample3 = sampler.sample({ 1.0f, 1.0f, 0.0f });

    return 0;
}