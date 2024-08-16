// @ts-check
"use strict";

/**
 * @module metal-translator
 */

const { load, DataType, open, close, define } = require("ffi-rs");
const { platform } = require("os");
const path = require("path");

console.log(process.platform);

let dynamicLib;
switch (process.platform) {
  case "linux":
    dynamicLib = "./libmetaltranslate.so";
    break;
  case "darwin":
    dynamicLib = "./libmetaltranslate.dylib";
    break;
  case "win32":
    dynamicLib = path.join(__dirname, "metaltranslate.dll");
    break;
  default:
    console.error("OS Not supported");
    process.exit(1);
    break;
}

console.log("dynamiclib: ", dynamicLib);

open({
  library: "metalTranslate",
  path: dynamicLib,
});

const metal = define({
  create_metal_translate: {
    library: "metalTranslate",
    retType: DataType.External,
    paramsType: [DataType.String, DataType.I32, DataType.I32],
  },
  translate: {
    library: "metalTranslate",
    retType: DataType.String,
    paramsType: [
      DataType.External,
      DataType.String,
      DataType.String,
      DataType.String,
    ],
  },
  free_metal_translate: {
    library: "metalTranslate",
    retType: DataType.Void,
    paramsType: [DataType.External],
  },
});

const ModelTypes = {
  M2M: 1,
  BART: 2,
  NLLB: 3,
};

class Translator {
  /**
   *
   * @param {string} modelPath
   * @param {'M2M' | 'BART' | 'NLLB'} modelType
   * @param {Number} maxTokens
   */
  constructor(modelPath, modelType, maxTokens = 32) {
    this.translator = metal.create_metal_translate([
      modelPath,
      ModelTypes[modelType],
      maxTokens,
    ]);
  }

  /**
   *
   * @param {string} text
   * @param {string} sourceCode
   * @param {string} targetCode
   */
  translate(text, sourceCode, targetCode) {
    const result = metal.translate([
      this.translator,
      text,
      sourceCode,
      targetCode,
    ]);

    return result;
  }

  free() {
    metal.free_metal_translate([this.translator]);
  }
}

// Example usage:
/* 
const modelPath =
  path.resolve("..", "models/translate-fairseq_m2m_100_418M/") +
  (process.platform === "win32" ? "\\" : "/");


const testTranslator = new Translator(modelPath, "M2M", 64);

const textToTranslate = `In the heart of an ancient forest, there was a hidden village known as Eldoria. This village was unlike any other; it was a place where magic and nature coexisted in perfect harmony. The villagers of Eldoria were known for their unique ability to communicate with the forest creatures and harness the natural energies around them.

Among the villagers was a young boy named Aiden. Aiden was curious and adventurous, always eager to learn more about the world around him. His best friend was a wise old owl named Orion, who had been his companion since he was a child. Orion had taught Aiden many things about the forest and its secrets.

One crisp autumn morning, Aiden and Orion set out on a journey to find the legendary Crystal of Light, a mystical artifact said to possess immense power. According to the village elders, the Crystal was hidden deep within the Enchanted Grove, a place where few dared to venture. The path to the Grove was treacherous, filled with twisting vines, hidden traps, and magical creatures.

As they journeyed deeper into the forest, Aiden and Orion encountered many challenges. They had to cross a rickety old bridge over a roaring river, navigate through a dense fog that played tricks on their senses, and solve ancient riddles that guarded the entrance to the Grove. Despite the obstacles, Aiden's determination never wavered, and Orion's wisdom guided them through each trial.

After days of travel, they finally reached the Enchanted Grove. The air was filled with a shimmering light, and the trees seemed to whisper secrets to one another. In the center of the Grove stood a magnificent tree with silver leaves and golden bark. At the base of the tree, nestled among the roots, was the Crystal of Light. Its radiant glow illuminated the entire Grove, casting a warm and soothing light.

As Aiden approached the Crystal, he felt a surge of energy coursing through his body. He reached out and gently touched the Crystal, and in that moment, he was filled with a profound sense of peace and understanding. The Crystal had chosen him as its guardian, and with it, he could protect Eldoria and ensure the balance between magic and nature remained intact.

With the Crystal of Light in hand, Aiden and Orion made their way back to the village. The journey home was filled with excitement and anticipation, as they knew the villagers would be overjoyed by their success. When they arrived, the entire village gathered to celebrate their return. The elders praised Aiden for his bravery and wisdom, and the villagers marveled at the Crystal's radiant beauty.

From that day forward, Aiden became a respected guardian of Eldoria, using the power of the Crystal to protect the village and maintain the harmony between magic and nature. He continued to explore the wonders of the forest with Orion by his side, always seeking new adventures and knowledge.`;

// M2M
const result = testTranslator.translate(textToTranslate, "en", "pt");

// BART
// const result = testTranslator.translate(textToTranslate, "en_XX", "pt_XX");

// NLLB
// const result = testTranslator.translate(
//   textToTranslate,
//   "eng_Latn",
//   "por_Latn"
// );

testTranslator.free();

console.log("final result: ", result);
*/

exports.Translator = Translator;
