#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <cassert>
#include <climits>
#include <emmintrin.h>
#include "src/dsp/dsp.h"
#include "src/dsp/cpu.h"
#include "src/dsp/lossless_common.h"
#include "src/dsp/mips_macro.h"
#include "src/dsp/quant.h"
#include "src/dsp/lossless.h"
#include "src/dsp/common_sse41.h"
#include "src/dsp/msa_macro.h"
#include "src/dsp/yuv.h"
#include "src/dsp/neon.h"
#include "src/dsp/common_sse2.h"
#include "src/webp/encode.h"
#include "src/webp/decode.h"
#include "src/webp/demux.h"
#include "src/webp/format_constants.h"
#include "src/webp/mux_types.h"
#include "src/webp/mux.h"
#include "src/webp/types.h"
#include "src/dec/webpi_dec.h"
#include "src/enc/vp8li_enc.h"
#include "src/enc/histogram_enc.h"
#include "src/enc/backward_references_enc.h"
#include "src/enc/cost_enc.h"
#include "src/enc/vp8i_enc.h"
#include "src/utils/huffman_encode_utils.h"
#include "src/utils/color_cache_utils.h"
#include "src/utils/utils.h"
#include "src/utils/huffman_utils.h"
#include "src/utils/bit_writer_utils.h"
#include "src/utils/bit_reader_utils.h"
#include "src/utils/filters_utils.h"
#include "src/utils/quant_levels_utils.h"
#include "src/utils/rescaler_utils.h"
#include "src/utils/endian_inl_utils.h"
#include "src/utils/palette.h"
#include "src/utils/bit_reader_inl_utils.h"
#include "src/utils/thread_utils.h"
#include "src/utils/random_utils.h"
#include "src/utils/quant_levels_dec_utils.h"
#include "src/mux/animi.h"
#include "src/mux/muxi.h"
// #include <cstdint>
// #include <cstddef>
// #include <webp/decode.h>
// 
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    WebPDecoderConfig config;
    if (!WebPInitDecoderConfig(&config)) {
        return 0;
    }

    // Set decoding parameters to simulate various scenarios
    config.output.colorspace = MODE_RGBA;
    config.options.use_threads = size % 2;  // Randomly use threading based on input size
    config.options.no_fancy_upsampling = (size % 3 == 0);
    config.options.bypass_filtering = (size % 5 == 0);
    config.options.use_scaling = (size % 7 == 0);
    config.options.scaled_width = size % 2000;   // Ensure width is within a reasonable range
    config.options.scaled_height = size % 2000; // Ensure height is within a reasonable range

    VP8StatusCode status = WebPDecode(data, size, &config);

    // Clean up any allocated data by WebPDecode
    WebPFreeDecBuffer(&config.output);

    return 0;  // Non-zero return values are reserved for future use.
}
