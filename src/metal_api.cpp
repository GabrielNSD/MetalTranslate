#include "metal_api.h"
#include "MetalTranslate.h"

#include <string>
#include <cstring>

class MetalTranslateWrapper {
public:
    MetalTranslateWrapper(const std::string& model_path)
        : config(model_path), translator(config) {}

    std::string Translate(const std::string& source, const std::string& source_code, const std::string& target_code) {
        return translator.Translate(source, source_code, target_code);
    }

private:
    MetalTranslate::MetalTranslateConfig config;
    MetalTranslate::MetalTranslate translator;
};


extern "C" {
    void* create_metal_translate(const char* model_path){
        return new MetalTranslateWrapper(model_path);
    }

    const char* translate(void* translator, const char* source, const char* source_code, const char* target_code) {
        MetalTranslateWrapper* trans = static_cast<MetalTranslateWrapper*>(translator);
        std::string result = trans->Translate(source, source_code, target_code);
        char* c_result = new char[result.size() + 1];
        std:strcpy(c_result, result.c_str());
        return c_result;
    }

    void free_metal_translate(void* translator) {
        delete static_cast<MetalTranslateWrapper*>(translator);
    }

}