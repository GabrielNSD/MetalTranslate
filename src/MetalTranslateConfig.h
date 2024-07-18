#include <string>

namespace MetalTranslate {
class MetalTranslateConfig {
public:
  MetalTranslateConfig() = default;

  MetalTranslateConfig(const std::string& model_path) : ModelPath(model_path) {}


  std::string ModelPath = "C:\\Users\\test1\\Desktop\\models\\translate-fairseq_m2m_100_418M\\";
};
} // namespace MetalTranslate
