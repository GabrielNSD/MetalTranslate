#include "MetalTranslate.h"

#include <ctranslate2/translator.h>
#include <onmt/Tokenizer.h>

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

// Limit M2M100 input tokens: https://github.com/facebookresearch/fairseq/issues/2875

namespace MetalTranslate {

std::vector<std::string> split_into_sentences(const std::string &text, onmt::Tokenizer &tokenizer) {
    std::vector<std::string> sentences;
    tokenizer.tokenize(text, sentences);
    return sentences;
}

MetalTranslate::MetalTranslate(MetalTranslateConfig config)
    : _config(std::move(config))  // Use member initializer list
{
}

std::string MetalTranslate::Translate(std::string source,
                                      std::string source_code,
                                      std::string target_code) {
    try {
        std::string source_prefix;
        std::string target_prefix_str;

        switch (this->_config.Type) {
        case ModelType::M2M:
            source_prefix = "__" + source_code + "__";
            target_prefix_str = "__" + target_code + "__";
            break;
        case ModelType::BART:
            source_prefix = "[" + source_code + "]";
            target_prefix_str = "[" + target_code + "]";
            break;
        case ModelType::NLLB:
            source_prefix = source_code;
            target_prefix_str = target_code;
            break;
        default:
            source_prefix = "__" + source_code + "__";
            target_prefix_str = "__" + target_code + "__";
            break;
        }

        // Tokenizer
        onmt::Tokenizer tokenizer(this->_config.ModelPath + "sentencepiece.model");
        std::vector<std::string> tokens;
        tokenizer.tokenize(source, tokens);
        if (this->_config.Type == ModelType::NLLB) tokens.push_back("</s>");

        onmt::Tokenizer tk2(this->_config.ModelPath + "sentencepiece.model", 0, 0.1, onmt::Tokenizer::Mode::None);
        std::vector<std::string> tokens2;
        tk2.tokenize(source, tokens2);

        std::vector<std::vector<std::string>> batches;
        std::vector<std::string> tempBatch;
        std::vector<std::string> sentence;

        const int max_batch_size = this->_config.MaxTokens;

        if (tokens.size() <= max_batch_size - 1) {
            tokens.insert(tokens.begin(), source_prefix);
            batches.push_back(tokens);
        } else {
            for (const std::string &token : tokens) {
                sentence.push_back(token);
                if (!tempBatch.size()) tempBatch.insert(tempBatch.begin(), source_prefix);
                if (token == "." || token == "?" || token == "!" || sentence.size() == max_batch_size) {
                    if (tempBatch.size() + sentence.size() <= max_batch_size) {
                        tempBatch.insert(tempBatch.end(), sentence.begin(), sentence.end());
                    } else {
                        batches.push_back(tempBatch);
                        tempBatch.clear();
                        tempBatch.insert(tempBatch.end(), source_prefix);  // TODO: guarantee that this addition will not exceed limit
                        tempBatch.insert(tempBatch.end(), sentence.begin(), sentence.end());
                    }
                    sentence.clear();
                }
            }
            batches.push_back(tempBatch);
            tempBatch.clear();
            sentence.clear();
        }

        // CTranslate2
        const size_t num_translators = 1;
        const size_t num_threads_per_translator = 1;  // Unused with DNNL

        ctranslate2::Translator translator(this->_config.ModelPath + "model", ctranslate2::Device::CPU);

        const std::vector<std::vector<std::string>> batch = {tokens};
        const std::vector<std::vector<std::string>> target_prefix = {{target_prefix_str}};

        ctranslate2::TranslationOptions options = ctranslate2::TranslationOptions();
        // fairseq models need beam_size = 5
        options.beam_size = 5;
        options.max_input_length = max_batch_size + 1;
        options.max_decoding_length = max_batch_size * 2;

        std::string concatenatedResult;

        for (const std::vector<std::string> &batch : batches) {
            const std::vector<ctranslate2::TranslationResult> results = translator.translate_batch({batch}, target_prefix, options, 0);
            const std::vector<std::string> translatedtokens = results[0].output();
            std::string result = std::move(tokenizer.detokenize(translatedtokens));

            // Remove target prefix
            // __es__ Traducción de texto con MetalTranslate
            // -> Traducción de texto con MetalTranslate

            size_t pos = result.find(" ");

            if (pos != std::string::npos) {
                result.erase(0, pos + 1);
            }

            concatenatedResult += result;
        }

        return concatenatedResult;
    } catch (const std::exception &e) {
        std::cerr << "Exception caught in Translate method: " << e.what() << std::endl;
        return "";
    } catch (...) {
        std::cerr << "Unknown exception caught in Translate method." << std::endl;
        return "";
    }
}

}  // namespace MetalTranslate
