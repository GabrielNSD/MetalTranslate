#include "metal_api.h"

#include <cstring>
#include <iostream>
#include <string>

class MetalTranslateWrapper {
public:
    MetalTranslateWrapper(const std::string& model_path, const MetalTranslate::ModelType model_type, const int max_tokens)
        : config(model_path, model_type, max_tokens), translator(config) {
    }

    std::string Translate(const std::string& source, const std::string& source_code, const std::string& target_code) {
        return translator.Translate(source, source_code, target_code);
    }

private:
    MetalTranslate::MetalTranslateConfig config;
    MetalTranslate::MetalTranslate translator;
};

extern "C" {
void* create_metal_translate(const char* model_path, const MetalTranslate::ModelType model_type, const int max_tokens) {
    return new MetalTranslateWrapper(model_path, model_type, max_tokens);
}

char* translate(void* translator, const char* source, const char* source_code, const char* target_code) {
    try {
        MetalTranslateWrapper* trans = static_cast<MetalTranslateWrapper*>(translator);

        std::string str_source(source);
        std::string str_source_code(source_code);
        std::string str_target_code(target_code);

        // Call the Translate method and handle potential exceptions
        std::string result;
        try {
            result = trans->Translate(str_source, str_source_code, str_target_code);
        } catch (const std::exception& e) {
            std::cerr << "Exception caught in MetalTranslateWrapper::Translate: " << e.what() << std::endl;
            throw;  // Re-throw the exception to be caught by the outer catch block
        }

        // Allocate memory for the result string
        char* c_result = new char[result.size() + 1];
        std::strcpy(c_result, result.c_str());

        return c_result;  // The caller must free this memory
    } catch (const std::exception& e) {
        std::cerr << "Exception caught in translate function: " << e.what() << std::endl;
        return nullptr;
    } catch (...) {
        std::cerr << "Unknown exception caught in translate function." << std::endl;
        return nullptr;
    }
}

void free_translated_string(char* translated_str) {
    delete[] translated_str;
}

void free_metal_translate(void* translator) {
    delete static_cast<MetalTranslateWrapper*>(translator);
}
}