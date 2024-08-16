#include <string>

namespace MetalTranslate {
enum ModelType {
    M2M = 1,
    BART = 2,
    NLLB = 3
};
class MetalTranslateConfig {
public:
    MetalTranslateConfig() = default;

    MetalTranslateConfig(const std::string& model_path, const ModelType model_type, const int max_tokens) : ModelPath(model_path), Type(model_type), MaxTokens(max_tokens) {}

    std::string ModelPath = "";
    const ModelType Type = ModelType::M2M;
    const int MaxTokens = 64;
};
}  // namespace MetalTranslate
