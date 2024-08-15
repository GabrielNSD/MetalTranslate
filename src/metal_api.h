#ifdef _WIN32
#ifdef METALTRANSLATE_EXPORTS
#define METALTRANSLATE_API __declspec(dllexport)
#else
#define METALTRANSLATE_API __declspec(dllimport)
#endif
#else
#define METALTRANSLATE_API
#endif

#include <string>

#include "MetalTranslate.h"

#ifdef __cplusplus
extern "C" {
#endif

// Define a function to create a MetalTranslate object
METALTRANSLATE_API void* create_metal_translate(const char* model_path, const MetalTranslate::ModelType model_type);

// Define a function to translate text
METALTRANSLATE_API char* translate(void* translator, const char* source,
                                   const char* source_code, const char* target_code);

METALTRANSLATE_API void free_translated_string(char* translated_str);

// Define a function to release a MetalTranslate object
METALTRANSLATE_API void free_metal_translate(void* translator);

#ifdef __cplusplus
}
#endif
