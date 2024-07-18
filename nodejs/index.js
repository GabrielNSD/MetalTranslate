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
    paramsType: [DataType.String],
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

class Translator {
  /**
   *
   * @param {string} modelPath
   */
  constructor(modelPath) {
    this.translator = metal.create_metal_translate([modelPath]);
  }

  /**
   *
   * @param {string} text
   * @param {string} sourceCode
   * @param {string} targetCode
   */
  translate(text, sourceCode, targetCode) {
    //console.log("calling translate", { text, sourceCode, targetCode });
    const result = metal.translate([
      this.translator,
      text,
      sourceCode,
      targetCode,
    ]);
    console.log("result in method", result);
    //metal.free_translated_string([result]);
    return result;
  }

  free() {
    metal.free_metal_translate([this.translator]);
  }
}

// Example usage:

const modelPath = path.resolve("..", "models/translate-fairseq_m2m_100_418M/") + ( process.platform === 'win32' ? "\\" : "/");

console.log("modelPath: ", modelPath);

const testTranslator = new Translator(modelPath);
// console.log("instatiated");

const textToTranslate = "This is a test of translation";

const result = testTranslator.translate(textToTranslate, "en", "fr");

testTranslator.free();

console.log("final result: ", result);

exports.Translator = Translator;
