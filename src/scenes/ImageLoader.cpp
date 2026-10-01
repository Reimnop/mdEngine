#include "ImageLoader.h"

#include <fstream>
#include <stb_image.h>
#include <stdexcept>
#include <string>

ImageData loadImage(const std::filesystem::path& path)
{
  std::ifstream file(path, std::ios::in | std::ios::binary);
  if (!file.is_open())
    throw std::runtime_error("failed to open file: " + path.filename().string());

  const auto bytes = std::vector(std::istreambuf_iterator(file), std::istreambuf_iterator<char>());

  stbi_set_flip_vertically_on_load(1);

  int width = 0, height = 0, channels = 0;
  stbi_uc* data = stbi_load_from_memory(
    reinterpret_cast<const stbi_uc*>(bytes.data()), static_cast<int>(bytes.size()), &width, &height, &channels, 4);
  if (!data)
    throw std::runtime_error(std::string("failed to decode image: ") + stbi_failure_reason());

  ImageData image;
  image.width = width;
  image.height = height;
  image.pixels.assign(data, data + static_cast<size_t>(width) * height * 4);
  stbi_image_free(data);
  return image;
}
