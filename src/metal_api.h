#ifdef __cplusplus
extern "C" {
#endif

// Define a function to create a MetalTranslate object
void *create_metal_translate(const char *model_path);

// Define a function to translate text
const char *translate(void *translator, const char *source,
                      const char *source_code, const char *target_code);

// Define a function to release a MetalTranslate object
void free_metal_translate(void *translator);

#ifdef __cplusplus
}
#endif
