# cputexture

- [About](#about)
  - [Features Overview](#features-overview)
  - [Textures](#textures)
  - [Texture Views and Spans](#texture-views-and-spans)
  - [Sampling](#sampling)
  - [Converting](#converting)
  - [Operations](#operations)
- [Supported Compilers](#supported-compilers)
- [Building](#building)
- [Thirdparty Libraries](#thidparty-libraries)

## About

**cputexture** provides classes that wrap texture data and provide easy access to their surfaces. There is support for 1d, 1d array, 2d, 2d array, cubemap, cubemap array, and 3d textures. **cputexture** supports all the formats supported by [gpuformat](https://www.github.com/amc522/gpuformat).

### Features Overview

- Classes wrapping a texture and all its surfaces.
- Views and spans for textures and surfaces.
- Conversions between texture formats.
- Sampling of textures. Currently only point sampling is supported.
- Basic texture operations for clearing, copying, decompressing, and flipping textures

### Textures

```
#include <cputex/unique_texture.h>
#include <cputex/shared_texture.h>
```

**cputexture** provides two basic classes for representing textures: `cputex::UniqueTexture` and `cputex::SharedTexture`. They provide identical functionality, except that `cputex::SharedTexture` is reference counted. They are analogous to `std::unique_ptr` and `std::shared_ptr`. These classes can be found in `cputex/unique_texture.h` and `cputex/shared_texture.h` respectively.

### Texture Views and Spans

```
#include <cputex/texture_view.h>
```

In addition to the main texture classes, there are also views and spans. These views and spans can represent a whole texture (collection of surfaces) or a single surface from the texture. The available views and spans are:
  - `cputex::TextureView`: Read only view of a texture and all its surfaces.
  - `cputex::TextureSpan`: Mutable view of a texture and all its surfaces.
  - `cputex::SurfaceView`: Read only view of an individual surface (mip, array slice, cubemap face, or volume slice).
  - `cputex::SurfaceSpan`: Mutable view of an individual surface (mip, array slice, cubemap face, or volume slice).

### Sampling

```
#include <cputex/sampler.h>

cputex::Sampler sampler{someTextureView};

glm::vec3 texCoords{0.1, 0.5, 0.0};
gpufmt::SampleVariant = sampler.sample(texCoords);

glm::ivec3 texel(12, 52, 0);
gpufmt::SampleVariant = sampler.load(texel);
```

### Converting

```
#include <cputex/converter.h>

cputex::Converter converter{sourceFormat, destFormat};
cputex::ConvertError error;
cputex::UniqueTexture convertedTexture = converter.convert(sourceSurfaceOrTexture, error);
```

### Operations

```
#include <cputex/texture_operations.h>
```

- `cputex::clear`
- `cputex::flipHorizontal`
- `cputex::flipVertical`
- `cputex::copySurfaceRegionTo`
- `cputex::decompressSurface`
- `cputex::decompressTexture`


## Supported Compilers

- Microsoft Visual C++ 2017
- Microsoft Visual C++ 2019

No other compilers have been tested, but there's no reason why they shouldn't work. I am more than happy integrate changes that provide compatibility for other compilers.

## Building

  1. Run [premake 5.0](https://premake.github.io/) in the cputexture directory using the `premake5.lua` file targeting your desired toolset. ex. `premake5.exe vs2019`.
  2. Then build the static libraries with your desired environment.
  3. Include all of the files in `cputex/include` in your project.

## Thirdparty Libraries

- [glm](https://github.com/g-truc/glm) - Basic vector types and some packing and unpacking functions.
- [gpuformat](https://github.com/amc522/gpuformat) - GPU format wrapper library.
- [tcbrindle-span](https://github.com/tcbrindle/span) - std::span implementation for C++11 and later.