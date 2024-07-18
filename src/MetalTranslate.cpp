#include "MetalTranslate.h"

#include <ctranslate2/translator.h>
#include <iostream>
#include <onmt/Tokenizer.h>

#include <stdexcept>
#include <string>

namespace MetalTranslate
{


    MetalTranslate::MetalTranslate(MetalTranslateConfig config)
        : _config(std::move(config)) // Use member initializer list
    {
    }

  std::string MetalTranslate::Translate(std::string source,
                                        std::string source_code,
                                        std::string target_code)
  {
    try
    {
      // Tokenizer
      onmt::Tokenizer tokenizer(this->_config.ModelPath + "sentencepiece.model");
      std::vector<std::string> tokens;
      tokenizer.tokenize(source, tokens);

      std::string source_prefix = "__" + source_code + "__";
      tokens.insert(tokens.begin(), source_prefix);

      // CTranslate2
      const size_t num_translators = 1;
      const size_t num_threads_per_translator = 1; // Unused with DNNL

      ctranslate2::Translator translator(this->_config.ModelPath + "model", ctranslate2::Device::CPU);

      //ctranslate2::init_profiling(ctranslate2::Device::CPU, translator.num_replicas());

      const std::vector<std::vector<std::string>> batch = {tokens};
      //const std::vector<std::vector<std::string>> batch = { {"__en__", "Hello", "world"} };
      const std::vector<std::vector<std::string>> target_prefix = {
          {"__" + target_code + "__"}};
      const size_t max_batch_size = 1024;

      const ctranslate2::TranslationOptions options = ctranslate2::TranslationOptions();

      const std::vector<ctranslate2::TranslationResult> results = std::move(translator.translate_batch(batch, target_prefix));
      //const std::vector<ctranslate2::TranslationResult> results =
          //std::move(translator.translate_batch(batch, target_prefix, options, max_batch_size));

      //ctranslate2::dump_profiling(std::cerr);
      
      const std::vector<std::string> translatedtokens = results[0].output();

      std::string result = std::move(tokenizer.detokenize(translatedtokens));
      // Remove target prefix
      // __es__ Traducción de texto con MetalTranslate
      // -> Traducción de texto con MetalTranslate
      std::string final_result = result.substr(7);


      return final_result;
    }
    catch (const std::exception &e)
    {
      std::cerr << "Exception caught in Translate method: " << e.what() << std::endl;
      return "";
    }
    catch (...)
    {
      std::cerr << "Unknown exception caught in Translate method." << std::endl;
      return "";
    }
  }

} // namespace MetalTranslate
