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
// #include <stdint.h>
// #include <stddef.h>
// #include <stdlib.h>
// #include <string.h>
// #include "src/webp/decode.h"
// #include "src/webp/encode.h"
// #include "src/webp/types.h"
// 
// // This fuzzer harness combines various aspects of image encoding, decoding, and manipulation using libwebp.
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < 1) return 0;

    // Initialize WebP configuration with default preset and quality factor.
    WebPConfig config;
    if (!WebPConfigInit(&config)) {
        return 0;
    }

    // Set quality factor based on input data to vary the configuration.
    config.quality = data[0] % 101;  // Quality factor from 0 to 100
    data += 1;
    size -= 1;

    if (size < sizeof(WebPPicture)) return 0;

    WebPPicture pic;
    if (!WebPPictureInit(&pic)) {
        return 0;
    }

    // Randomly decide to use lossless compression based on input.
    config.lossless = data[0] % 2;
    data += 1;
    size -= 1;

    // Set picture width and height from input data.
    if (size < 2) return 0;
    pic.width = (data[0] % 128) + 1;
    pic.height = (data[1] % 128) + 1;
    data += 2;
    size -= 2;

    // Set up memory writer for encoded image.
    WebPMemoryWriter writer;
    WebPMemoryWriterInit(&writer);
    pic.writer = WebPMemoryWrite;
    pic.custom_ptr = &writer;

    // Allocate and fill the picture's ARGB buffer.
    size_t argb_size = pic.width * pic.height * sizeof(uint32_t);
    if (size < argb_size) return 0;

    pic.use_argb = 1;
    pic.argb = (uint32_t*)malloc(argb_size);
    memcpy(pic.argb, data, argb_size);
    pic.argb_stride = pic.width;

    // Encode the picture.
    if (!WebPEncode(&config, &pic)) {
        WebPMemoryWriterClear(&writer);
        free(pic.argb);
        return 0;
    }

    // Decode the picture.
    WebPDecoderConfig decoder_config;
    WebPInitDecoderConfig(&decoder_config);
    decoder_config.output.colorspace = MODE_RGBA;
    VP8StatusCode decode_status = WebPDecode(writer.mem, writer.size, &decoder_config);

    // Cleanup
    WebPFreeDecBuffer(&decoder_config.output);
    WebPMemoryWriterClear(&writer);
    free(pic.argb);

    return decode_status == VP8_STATUS_OK ? 1 : 0;
}
