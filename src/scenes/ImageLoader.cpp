#include "ImageLoader.h"

#include <stb_image.h>
#include <stdexcept>
#include <string>

#include "helper/io.h"

ImageData loadImage(const std::filesystem::path& path)
{
  std::string bytes;
  try
  {
    bytes = mdEngine::io::readFileAsString(path); // reads through std::filesystem, so unicode paths work
  }
  catch (const std::exception&)
  {
    throw std::runtime_error("failed to open " + path.filename().string());
  }

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
