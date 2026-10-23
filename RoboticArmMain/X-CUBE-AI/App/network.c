/**
  ******************************************************************************
  * @file    network.c
  * @author  AST Embedded Analytics Research Platform
  * @date    2026-10-23T01:28:13+0700
  * @brief   AI Tool Automatic Code Generator for Embedded NN computing
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  */


#include "network.h"
#include "network_data.h"

#include "ai_platform.h"
#include "ai_platform_interface.h"
#include "ai_math_helpers.h"

#include "core_common.h"
#include "core_convert.h"

#include "layers.h"



#undef AI_NET_OBJ_INSTANCE
#define AI_NET_OBJ_INSTANCE g_network
 
#undef AI_NETWORK_MODEL_SIGNATURE
#define AI_NETWORK_MODEL_SIGNATURE     "0x0c1932640d6f3dbdfff54dbc818de05f"

#ifndef AI_TOOLS_REVISION_ID
#define AI_TOOLS_REVISION_ID     ""
#endif

#undef AI_TOOLS_DATE_TIME
#define AI_TOOLS_DATE_TIME   "2026-10-23T01:28:13+0700"

#undef AI_TOOLS_COMPILE_TIME
#define AI_TOOLS_COMPILE_TIME    __DATE__ " " __TIME__

#undef AI_NETWORK_N_BATCHES
#define AI_NETWORK_N_BATCHES         (1)

static ai_ptr g_network_activations_map[1] = AI_C_ARRAY_INIT;
static ai_ptr g_network_weights_map[1] = AI_C_ARRAY_INIT;



/**  Array declarations section  **********************************************/
/* Array#0 */
AI_ARRAY_OBJ_DECLARE(
  logmel_output_array, AI_ARRAY_FORMAT_FLOAT|AI_FMT_FLAG_IS_IO,
  NULL, NULL, 4040, AI_STATIC)

/* Array#1 */
AI_ARRAY_OBJ_DECLARE(
  logmel_Transpose_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 4040, AI_STATIC)

/* Array#2 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_0_Sub_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 4040, AI_STATIC)

/* Array#3 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_0_Div_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 4040, AI_STATIC)

/* Array#4 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_stem_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 1616, AI_STATIC)

/* Array#5 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_shortcut_shortcut_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 1224, AI_STATIC)

/* Array#6 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_main_main_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 1224, AI_STATIC)

/* Array#7 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_main_main_2_Relu_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 1224, AI_STATIC)

/* Array#8 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_main_main_3_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 1224, AI_STATIC)

/* Array#9 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_Add_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 1224, AI_STATIC)

/* Array#10 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_relu_Relu_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 1224, AI_STATIC)

/* Array#11 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_shortcut_shortcut_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 832, AI_STATIC)

/* Array#12 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_main_main_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 832, AI_STATIC)

/* Array#13 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_main_main_2_Relu_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 832, AI_STATIC)

/* Array#14 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_main_main_3_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 832, AI_STATIC)

/* Array#15 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_Add_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 832, AI_STATIC)

/* Array#16 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_relu_Relu_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 832, AI_STATIC)

/* Array#17 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_shortcut_shortcut_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 520, AI_STATIC)

/* Array#18 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_main_main_0_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 520, AI_STATIC)

/* Array#19 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_main_main_2_Relu_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 520, AI_STATIC)

/* Array#20 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_main_main_3_Conv_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 520, AI_STATIC)

/* Array#21 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_Add_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 520, AI_STATIC)

/* Array#22 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_relu_Relu_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 520, AI_STATIC)

/* Array#23 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_head_head_0_GlobalAveragePool_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 40, AI_STATIC)

/* Array#24 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_head_head_3_Gemm_output_0_output_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 23, AI_STATIC)

/* Array#25 */
AI_ARRAY_OBJ_DECLARE(
  probs_output_array, AI_ARRAY_FORMAT_FLOAT|AI_FMT_FLAG_IS_IO,
  NULL, NULL, 23, AI_STATIC)

/* Array#26 */
AI_ARRAY_OBJ_DECLARE(
  net_0_std_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 40, AI_STATIC)

/* Array#27 */
AI_ARRAY_OBJ_DECLARE(
  net_0_mean_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 40, AI_STATIC)

/* Array#28 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_stem_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 1920, AI_STATIC)

/* Array#29 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_stem_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 16, AI_STATIC)

/* Array#30 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_shortcut_shortcut_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 384, AI_STATIC)

/* Array#31 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_shortcut_shortcut_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 24, AI_STATIC)

/* Array#32 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_main_main_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 3456, AI_STATIC)

/* Array#33 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_main_main_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 24, AI_STATIC)

/* Array#34 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_main_main_3_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 5184, AI_STATIC)

/* Array#35 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_main_main_3_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 24, AI_STATIC)

/* Array#36 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_shortcut_shortcut_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 768, AI_STATIC)

/* Array#37 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_shortcut_shortcut_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 32, AI_STATIC)

/* Array#38 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_main_main_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 6912, AI_STATIC)

/* Array#39 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_main_main_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 32, AI_STATIC)

/* Array#40 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_main_main_3_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 9216, AI_STATIC)

/* Array#41 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_main_main_3_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 32, AI_STATIC)

/* Array#42 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_shortcut_shortcut_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 1280, AI_STATIC)

/* Array#43 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_shortcut_shortcut_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 40, AI_STATIC)

/* Array#44 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_main_main_0_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 11520, AI_STATIC)

/* Array#45 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_main_main_0_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 40, AI_STATIC)

/* Array#46 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_main_main_3_Conv_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 14400, AI_STATIC)

/* Array#47 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_main_main_3_Conv_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 40, AI_STATIC)

/* Array#48 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_head_head_3_Gemm_output_0_weights_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 920, AI_STATIC)

/* Array#49 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_head_head_3_Gemm_output_0_bias_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 23, AI_STATIC)

/* Array#50 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_stem_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 120, AI_STATIC)

/* Array#51 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_shortcut_shortcut_0_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 16, AI_STATIC)

/* Array#52 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_main_main_0_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 144, AI_STATIC)

/* Array#53 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_main_main_3_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 216, AI_STATIC)

/* Array#54 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_shortcut_shortcut_0_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 24, AI_STATIC)

/* Array#55 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_main_main_0_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 216, AI_STATIC)

/* Array#56 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_main_main_3_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 288, AI_STATIC)

/* Array#57 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_shortcut_shortcut_0_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 32, AI_STATIC)

/* Array#58 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_main_main_0_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 288, AI_STATIC)

/* Array#59 */
AI_ARRAY_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_main_main_3_Conv_output_0_scratch0_array, AI_ARRAY_FORMAT_FLOAT,
  NULL, NULL, 360, AI_STATIC)

/**  Tensor declarations section  *********************************************/
/* Tensor #0 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_0_Div_output_0_output, AI_STATIC,
  0, 0x0,
  AI_SHAPE_INIT(4, 1, 40, 1, 101), AI_STRIDE_INIT(4, 4, 4, 160, 160),
  1, &_net_net_0_Div_output_0_output_array, NULL)

/* Tensor #1 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_0_Sub_output_0_output, AI_STATIC,
  1, 0x0,
  AI_SHAPE_INIT(4, 1, 40, 1, 101), AI_STRIDE_INIT(4, 4, 4, 160, 160),
  1, &_net_net_0_Sub_output_0_output_array, NULL)

/* Tensor #2 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_Add_output_0_output, AI_STATIC,
  2, 0x0,
  AI_SHAPE_INIT(4, 1, 24, 1, 51), AI_STRIDE_INIT(4, 4, 4, 96, 96),
  1, &_net_net_1_blocks_blocks_0_Add_output_0_output_array, NULL)

/* Tensor #3 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_main_main_0_Conv_output_0_bias, AI_STATIC,
  3, 0x0,
  AI_SHAPE_INIT(4, 1, 24, 1, 1), AI_STRIDE_INIT(4, 4, 4, 96, 96),
  1, &_net_net_1_blocks_blocks_0_main_main_0_Conv_output_0_bias_array, NULL)

/* Tensor #4 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_main_main_0_Conv_output_0_output, AI_STATIC,
  4, 0x0,
  AI_SHAPE_INIT(4, 1, 24, 1, 51), AI_STRIDE_INIT(4, 4, 4, 96, 96),
  1, &_net_net_1_blocks_blocks_0_main_main_0_Conv_output_0_output_array, NULL)

/* Tensor #5 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_main_main_0_Conv_output_0_scratch0, AI_STATIC,
  5, 0x0,
  AI_SHAPE_INIT(4, 1, 16, 1, 9), AI_STRIDE_INIT(4, 4, 4, 64, 64),
  1, &_net_net_1_blocks_blocks_0_main_main_0_Conv_output_0_scratch0_array, NULL)

/* Tensor #6 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_main_main_0_Conv_output_0_weights, AI_STATIC,
  6, 0x0,
  AI_SHAPE_INIT(4, 16, 1, 9, 24), AI_STRIDE_INIT(4, 4, 64, 1536, 1536),
  1, &_net_net_1_blocks_blocks_0_main_main_0_Conv_output_0_weights_array, NULL)

/* Tensor #7 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_main_main_2_Relu_output_0_output, AI_STATIC,
  7, 0x0,
  AI_SHAPE_INIT(4, 1, 24, 1, 51), AI_STRIDE_INIT(4, 4, 4, 96, 96),
  1, &_net_net_1_blocks_blocks_0_main_main_2_Relu_output_0_output_array, NULL)

/* Tensor #8 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_main_main_3_Conv_output_0_bias, AI_STATIC,
  8, 0x0,
  AI_SHAPE_INIT(4, 1, 24, 1, 1), AI_STRIDE_INIT(4, 4, 4, 96, 96),
  1, &_net_net_1_blocks_blocks_0_main_main_3_Conv_output_0_bias_array, NULL)

/* Tensor #9 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_main_main_3_Conv_output_0_output, AI_STATIC,
  9, 0x0,
  AI_SHAPE_INIT(4, 1, 24, 1, 51), AI_STRIDE_INIT(4, 4, 4, 96, 96),
  1, &_net_net_1_blocks_blocks_0_main_main_3_Conv_output_0_output_array, NULL)

/* Tensor #10 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_main_main_3_Conv_output_0_scratch0, AI_STATIC,
  10, 0x0,
  AI_SHAPE_INIT(4, 1, 24, 1, 9), AI_STRIDE_INIT(4, 4, 4, 96, 96),
  1, &_net_net_1_blocks_blocks_0_main_main_3_Conv_output_0_scratch0_array, NULL)

/* Tensor #11 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_main_main_3_Conv_output_0_weights, AI_STATIC,
  11, 0x0,
  AI_SHAPE_INIT(4, 24, 1, 9, 24), AI_STRIDE_INIT(4, 4, 96, 2304, 2304),
  1, &_net_net_1_blocks_blocks_0_main_main_3_Conv_output_0_weights_array, NULL)

/* Tensor #12 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_relu_Relu_output_0_output, AI_STATIC,
  12, 0x0,
  AI_SHAPE_INIT(4, 1, 24, 1, 51), AI_STRIDE_INIT(4, 4, 4, 96, 96),
  1, &_net_net_1_blocks_blocks_0_relu_Relu_output_0_output_array, NULL)

/* Tensor #13 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_shortcut_shortcut_0_Conv_output_0_bias, AI_STATIC,
  13, 0x0,
  AI_SHAPE_INIT(4, 1, 24, 1, 1), AI_STRIDE_INIT(4, 4, 4, 96, 96),
  1, &_net_net_1_blocks_blocks_0_shortcut_shortcut_0_Conv_output_0_bias_array, NULL)

/* Tensor #14 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_shortcut_shortcut_0_Conv_output_0_output, AI_STATIC,
  14, 0x0,
  AI_SHAPE_INIT(4, 1, 24, 1, 51), AI_STRIDE_INIT(4, 4, 4, 96, 96),
  1, &_net_net_1_blocks_blocks_0_shortcut_shortcut_0_Conv_output_0_output_array, NULL)

/* Tensor #15 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_shortcut_shortcut_0_Conv_output_0_scratch0, AI_STATIC,
  15, 0x0,
  AI_SHAPE_INIT(4, 1, 16, 1, 1), AI_STRIDE_INIT(4, 4, 4, 64, 64),
  1, &_net_net_1_blocks_blocks_0_shortcut_shortcut_0_Conv_output_0_scratch0_array, NULL)

/* Tensor #16 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_shortcut_shortcut_0_Conv_output_0_weights, AI_STATIC,
  16, 0x0,
  AI_SHAPE_INIT(4, 16, 1, 1, 24), AI_STRIDE_INIT(4, 4, 64, 1536, 1536),
  1, &_net_net_1_blocks_blocks_0_shortcut_shortcut_0_Conv_output_0_weights_array, NULL)

/* Tensor #17 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_Add_output_0_output, AI_STATIC,
  17, 0x0,
  AI_SHAPE_INIT(4, 1, 32, 1, 26), AI_STRIDE_INIT(4, 4, 4, 128, 128),
  1, &_net_net_1_blocks_blocks_1_Add_output_0_output_array, NULL)

/* Tensor #18 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_main_main_0_Conv_output_0_bias, AI_STATIC,
  18, 0x0,
  AI_SHAPE_INIT(4, 1, 32, 1, 1), AI_STRIDE_INIT(4, 4, 4, 128, 128),
  1, &_net_net_1_blocks_blocks_1_main_main_0_Conv_output_0_bias_array, NULL)

/* Tensor #19 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_main_main_0_Conv_output_0_output, AI_STATIC,
  19, 0x0,
  AI_SHAPE_INIT(4, 1, 32, 1, 26), AI_STRIDE_INIT(4, 4, 4, 128, 128),
  1, &_net_net_1_blocks_blocks_1_main_main_0_Conv_output_0_output_array, NULL)

/* Tensor #20 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_main_main_0_Conv_output_0_scratch0, AI_STATIC,
  20, 0x0,
  AI_SHAPE_INIT(4, 1, 24, 1, 9), AI_STRIDE_INIT(4, 4, 4, 96, 96),
  1, &_net_net_1_blocks_blocks_1_main_main_0_Conv_output_0_scratch0_array, NULL)

/* Tensor #21 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_main_main_0_Conv_output_0_weights, AI_STATIC,
  21, 0x0,
  AI_SHAPE_INIT(4, 24, 1, 9, 32), AI_STRIDE_INIT(4, 4, 96, 3072, 3072),
  1, &_net_net_1_blocks_blocks_1_main_main_0_Conv_output_0_weights_array, NULL)

/* Tensor #22 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_main_main_2_Relu_output_0_output, AI_STATIC,
  22, 0x0,
  AI_SHAPE_INIT(4, 1, 32, 1, 26), AI_STRIDE_INIT(4, 4, 4, 128, 128),
  1, &_net_net_1_blocks_blocks_1_main_main_2_Relu_output_0_output_array, NULL)

/* Tensor #23 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_main_main_3_Conv_output_0_bias, AI_STATIC,
  23, 0x0,
  AI_SHAPE_INIT(4, 1, 32, 1, 1), AI_STRIDE_INIT(4, 4, 4, 128, 128),
  1, &_net_net_1_blocks_blocks_1_main_main_3_Conv_output_0_bias_array, NULL)

/* Tensor #24 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_main_main_3_Conv_output_0_output, AI_STATIC,
  24, 0x0,
  AI_SHAPE_INIT(4, 1, 32, 1, 26), AI_STRIDE_INIT(4, 4, 4, 128, 128),
  1, &_net_net_1_blocks_blocks_1_main_main_3_Conv_output_0_output_array, NULL)

/* Tensor #25 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_main_main_3_Conv_output_0_scratch0, AI_STATIC,
  25, 0x0,
  AI_SHAPE_INIT(4, 1, 32, 1, 9), AI_STRIDE_INIT(4, 4, 4, 128, 128),
  1, &_net_net_1_blocks_blocks_1_main_main_3_Conv_output_0_scratch0_array, NULL)

/* Tensor #26 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_main_main_3_Conv_output_0_weights, AI_STATIC,
  26, 0x0,
  AI_SHAPE_INIT(4, 32, 1, 9, 32), AI_STRIDE_INIT(4, 4, 128, 4096, 4096),
  1, &_net_net_1_blocks_blocks_1_main_main_3_Conv_output_0_weights_array, NULL)

/* Tensor #27 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_relu_Relu_output_0_output, AI_STATIC,
  27, 0x0,
  AI_SHAPE_INIT(4, 1, 32, 1, 26), AI_STRIDE_INIT(4, 4, 4, 128, 128),
  1, &_net_net_1_blocks_blocks_1_relu_Relu_output_0_output_array, NULL)

/* Tensor #28 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_shortcut_shortcut_0_Conv_output_0_bias, AI_STATIC,
  28, 0x0,
  AI_SHAPE_INIT(4, 1, 32, 1, 1), AI_STRIDE_INIT(4, 4, 4, 128, 128),
  1, &_net_net_1_blocks_blocks_1_shortcut_shortcut_0_Conv_output_0_bias_array, NULL)

/* Tensor #29 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_shortcut_shortcut_0_Conv_output_0_output, AI_STATIC,
  29, 0x0,
  AI_SHAPE_INIT(4, 1, 32, 1, 26), AI_STRIDE_INIT(4, 4, 4, 128, 128),
  1, &_net_net_1_blocks_blocks_1_shortcut_shortcut_0_Conv_output_0_output_array, NULL)

/* Tensor #30 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_shortcut_shortcut_0_Conv_output_0_scratch0, AI_STATIC,
  30, 0x0,
  AI_SHAPE_INIT(4, 1, 24, 1, 1), AI_STRIDE_INIT(4, 4, 4, 96, 96),
  1, &_net_net_1_blocks_blocks_1_shortcut_shortcut_0_Conv_output_0_scratch0_array, NULL)

/* Tensor #31 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_shortcut_shortcut_0_Conv_output_0_weights, AI_STATIC,
  31, 0x0,
  AI_SHAPE_INIT(4, 24, 1, 1, 32), AI_STRIDE_INIT(4, 4, 96, 3072, 3072),
  1, &_net_net_1_blocks_blocks_1_shortcut_shortcut_0_Conv_output_0_weights_array, NULL)

/* Tensor #32 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_Add_output_0_output, AI_STATIC,
  32, 0x0,
  AI_SHAPE_INIT(4, 1, 40, 1, 13), AI_STRIDE_INIT(4, 4, 4, 160, 160),
  1, &_net_net_1_blocks_blocks_2_Add_output_0_output_array, NULL)

/* Tensor #33 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_main_main_0_Conv_output_0_bias, AI_STATIC,
  33, 0x0,
  AI_SHAPE_INIT(4, 1, 40, 1, 1), AI_STRIDE_INIT(4, 4, 4, 160, 160),
  1, &_net_net_1_blocks_blocks_2_main_main_0_Conv_output_0_bias_array, NULL)

/* Tensor #34 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_main_main_0_Conv_output_0_output, AI_STATIC,
  34, 0x0,
  AI_SHAPE_INIT(4, 1, 40, 1, 13), AI_STRIDE_INIT(4, 4, 4, 160, 160),
  1, &_net_net_1_blocks_blocks_2_main_main_0_Conv_output_0_output_array, NULL)

/* Tensor #35 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_main_main_0_Conv_output_0_scratch0, AI_STATIC,
  35, 0x0,
  AI_SHAPE_INIT(4, 1, 32, 1, 9), AI_STRIDE_INIT(4, 4, 4, 128, 128),
  1, &_net_net_1_blocks_blocks_2_main_main_0_Conv_output_0_scratch0_array, NULL)

/* Tensor #36 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_main_main_0_Conv_output_0_weights, AI_STATIC,
  36, 0x0,
  AI_SHAPE_INIT(4, 32, 1, 9, 40), AI_STRIDE_INIT(4, 4, 128, 5120, 5120),
  1, &_net_net_1_blocks_blocks_2_main_main_0_Conv_output_0_weights_array, NULL)

/* Tensor #37 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_main_main_2_Relu_output_0_output, AI_STATIC,
  37, 0x0,
  AI_SHAPE_INIT(4, 1, 40, 1, 13), AI_STRIDE_INIT(4, 4, 4, 160, 160),
  1, &_net_net_1_blocks_blocks_2_main_main_2_Relu_output_0_output_array, NULL)

/* Tensor #38 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_main_main_3_Conv_output_0_bias, AI_STATIC,
  38, 0x0,
  AI_SHAPE_INIT(4, 1, 40, 1, 1), AI_STRIDE_INIT(4, 4, 4, 160, 160),
  1, &_net_net_1_blocks_blocks_2_main_main_3_Conv_output_0_bias_array, NULL)

/* Tensor #39 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_main_main_3_Conv_output_0_output, AI_STATIC,
  39, 0x0,
  AI_SHAPE_INIT(4, 1, 40, 1, 13), AI_STRIDE_INIT(4, 4, 4, 160, 160),
  1, &_net_net_1_blocks_blocks_2_main_main_3_Conv_output_0_output_array, NULL)

/* Tensor #40 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_main_main_3_Conv_output_0_scratch0, AI_STATIC,
  40, 0x0,
  AI_SHAPE_INIT(4, 1, 40, 1, 9), AI_STRIDE_INIT(4, 4, 4, 160, 160),
  1, &_net_net_1_blocks_blocks_2_main_main_3_Conv_output_0_scratch0_array, NULL)

/* Tensor #41 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_main_main_3_Conv_output_0_weights, AI_STATIC,
  41, 0x0,
  AI_SHAPE_INIT(4, 40, 1, 9, 40), AI_STRIDE_INIT(4, 4, 160, 6400, 6400),
  1, &_net_net_1_blocks_blocks_2_main_main_3_Conv_output_0_weights_array, NULL)

/* Tensor #42 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_relu_Relu_output_0_output, AI_STATIC,
  42, 0x0,
  AI_SHAPE_INIT(4, 1, 40, 1, 13), AI_STRIDE_INIT(4, 4, 4, 160, 160),
  1, &_net_net_1_blocks_blocks_2_relu_Relu_output_0_output_array, NULL)

/* Tensor #43 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_shortcut_shortcut_0_Conv_output_0_bias, AI_STATIC,
  43, 0x0,
  AI_SHAPE_INIT(4, 1, 40, 1, 1), AI_STRIDE_INIT(4, 4, 4, 160, 160),
  1, &_net_net_1_blocks_blocks_2_shortcut_shortcut_0_Conv_output_0_bias_array, NULL)

/* Tensor #44 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_shortcut_shortcut_0_Conv_output_0_output, AI_STATIC,
  44, 0x0,
  AI_SHAPE_INIT(4, 1, 40, 1, 13), AI_STRIDE_INIT(4, 4, 4, 160, 160),
  1, &_net_net_1_blocks_blocks_2_shortcut_shortcut_0_Conv_output_0_output_array, NULL)

/* Tensor #45 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_shortcut_shortcut_0_Conv_output_0_scratch0, AI_STATIC,
  45, 0x0,
  AI_SHAPE_INIT(4, 1, 32, 1, 1), AI_STRIDE_INIT(4, 4, 4, 128, 128),
  1, &_net_net_1_blocks_blocks_2_shortcut_shortcut_0_Conv_output_0_scratch0_array, NULL)

/* Tensor #46 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_shortcut_shortcut_0_Conv_output_0_weights, AI_STATIC,
  46, 0x0,
  AI_SHAPE_INIT(4, 32, 1, 1, 40), AI_STRIDE_INIT(4, 4, 128, 5120, 5120),
  1, &_net_net_1_blocks_blocks_2_shortcut_shortcut_0_Conv_output_0_weights_array, NULL)

/* Tensor #47 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_head_head_0_GlobalAveragePool_output_0_output, AI_STATIC,
  47, 0x0,
  AI_SHAPE_INIT(4, 1, 40, 1, 1), AI_STRIDE_INIT(4, 4, 4, 160, 160),
  1, &_net_net_1_head_head_0_GlobalAveragePool_output_0_output_array, NULL)

/* Tensor #48 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_head_head_3_Gemm_output_0_bias, AI_STATIC,
  48, 0x0,
  AI_SHAPE_INIT(4, 1, 23, 1, 1), AI_STRIDE_INIT(4, 4, 4, 92, 92),
  1, &_net_net_1_head_head_3_Gemm_output_0_bias_array, NULL)

/* Tensor #49 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_head_head_3_Gemm_output_0_output, AI_STATIC,
  49, 0x0,
  AI_SHAPE_INIT(4, 1, 23, 1, 1), AI_STRIDE_INIT(4, 4, 4, 92, 92),
  1, &_net_net_1_head_head_3_Gemm_output_0_output_array, NULL)

/* Tensor #50 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_head_head_3_Gemm_output_0_weights, AI_STATIC,
  50, 0x0,
  AI_SHAPE_INIT(4, 40, 23, 1, 1), AI_STRIDE_INIT(4, 4, 160, 3680, 3680),
  1, &_net_net_1_head_head_3_Gemm_output_0_weights_array, NULL)

/* Tensor #51 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_stem_Conv_output_0_bias, AI_STATIC,
  51, 0x0,
  AI_SHAPE_INIT(4, 1, 16, 1, 1), AI_STRIDE_INIT(4, 4, 4, 64, 64),
  1, &_net_net_1_stem_Conv_output_0_bias_array, NULL)

/* Tensor #52 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_stem_Conv_output_0_output, AI_STATIC,
  52, 0x0,
  AI_SHAPE_INIT(4, 1, 16, 1, 101), AI_STRIDE_INIT(4, 4, 4, 64, 64),
  1, &_net_net_1_stem_Conv_output_0_output_array, NULL)

/* Tensor #53 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_stem_Conv_output_0_scratch0, AI_STATIC,
  53, 0x0,
  AI_SHAPE_INIT(4, 1, 40, 1, 3), AI_STRIDE_INIT(4, 4, 4, 160, 160),
  1, &_net_net_1_stem_Conv_output_0_scratch0_array, NULL)

/* Tensor #54 */
AI_TENSOR_OBJ_DECLARE(
  _net_net_1_stem_Conv_output_0_weights, AI_STATIC,
  54, 0x0,
  AI_SHAPE_INIT(4, 40, 1, 3, 16), AI_STRIDE_INIT(4, 4, 160, 2560, 2560),
  1, &_net_net_1_stem_Conv_output_0_weights_array, NULL)

/* Tensor #55 */
AI_TENSOR_OBJ_DECLARE(
  logmel_Transpose_output, AI_STATIC,
  55, 0x0,
  AI_SHAPE_INIT(4, 1, 40, 1, 101), AI_STRIDE_INIT(4, 4, 4, 160, 160),
  1, &logmel_Transpose_output_array, NULL)

/* Tensor #56 */
AI_TENSOR_OBJ_DECLARE(
  logmel_output, AI_STATIC,
  56, 0x0,
  AI_SHAPE_INIT(4, 1, 101, 1, 40), AI_STRIDE_INIT(4, 4, 4, 404, 404),
  1, &logmel_output_array, NULL)

/* Tensor #57 */
AI_TENSOR_OBJ_DECLARE(
  net_0_mean, AI_STATIC,
  57, 0x0,
  AI_SHAPE_INIT(4, 1, 40, 1, 1), AI_STRIDE_INIT(4, 4, 4, 160, 160),
  1, &net_0_mean_array, NULL)

/* Tensor #58 */
AI_TENSOR_OBJ_DECLARE(
  net_0_std, AI_STATIC,
  58, 0x0,
  AI_SHAPE_INIT(4, 1, 40, 1, 1), AI_STRIDE_INIT(4, 4, 4, 160, 160),
  1, &net_0_std_array, NULL)

/* Tensor #59 */
AI_TENSOR_OBJ_DECLARE(
  probs_output, AI_STATIC,
  59, 0x0,
  AI_SHAPE_INIT(4, 1, 23, 1, 1), AI_STRIDE_INIT(4, 4, 4, 92, 92),
  1, &probs_output_array, NULL)



/**  Layer declarations section  **********************************************/


AI_TENSOR_CHAIN_OBJ_DECLARE(
  probs_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_head_head_3_Gemm_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &probs_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  probs_layer, 25,
  SM_TYPE, 0x0, NULL,
  sm, forward_sm,
  &probs_chain,
  NULL, &probs_layer, AI_STATIC, 
  .nl_params = NULL, 
  .axis = AI_SHAPE_CHANNEL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _net_net_1_head_head_3_Gemm_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_head_head_0_GlobalAveragePool_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_head_head_3_Gemm_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_net_net_1_head_head_3_Gemm_output_0_weights, &_net_net_1_head_head_3_Gemm_output_0_bias),
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _net_net_1_head_head_3_Gemm_output_0_layer, 24,
  DENSE_TYPE, 0x0, NULL,
  dense, forward_dense,
  &_net_net_1_head_head_3_Gemm_output_0_chain,
  NULL, &probs_layer, AI_STATIC, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _net_net_1_head_head_0_GlobalAveragePool_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_blocks_blocks_2_relu_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_head_head_0_GlobalAveragePool_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _net_net_1_head_head_0_GlobalAveragePool_output_0_layer, 22,
  POOL_TYPE, 0x0, NULL,
  pool, forward_ap,
  &_net_net_1_head_head_0_GlobalAveragePool_output_0_chain,
  NULL, &_net_net_1_head_head_3_Gemm_output_0_layer, AI_STATIC, 
  .pool_size = AI_SHAPE_2D_INIT(1, 13), 
  .pool_stride = AI_SHAPE_2D_INIT(1, 13), 
  .pool_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_relu_Relu_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_blocks_blocks_2_Add_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_blocks_blocks_2_relu_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_relu_Relu_output_0_layer, 21,
  NL_TYPE, 0x0, NULL,
  nl, forward_relu,
  &_net_net_1_blocks_blocks_2_relu_Relu_output_0_chain,
  NULL, &_net_net_1_head_head_0_GlobalAveragePool_output_0_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_Add_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_net_net_1_blocks_blocks_2_main_main_3_Conv_output_0_output, &_net_net_1_blocks_blocks_2_shortcut_shortcut_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_blocks_blocks_2_Add_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_Add_output_0_layer, 20,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_net_net_1_blocks_blocks_2_Add_output_0_chain,
  NULL, &_net_net_1_blocks_blocks_2_relu_Relu_output_0_layer, AI_STATIC, 
  .operation = ai_sum_f32, 
  .buffer_operation = ai_sum_buffer_f32, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_main_main_3_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_blocks_blocks_2_main_main_2_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_blocks_blocks_2_main_main_3_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_net_net_1_blocks_blocks_2_main_main_3_Conv_output_0_weights, &_net_net_1_blocks_blocks_2_main_main_3_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_net_net_1_blocks_blocks_2_main_main_3_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_main_main_3_Conv_output_0_layer, 18,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_net_net_1_blocks_blocks_2_main_main_3_Conv_output_0_chain,
  NULL, &_net_net_1_blocks_blocks_2_Add_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 4, 0, 4, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_SAME, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_main_main_2_Relu_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_blocks_blocks_2_main_main_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_blocks_blocks_2_main_main_2_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_main_main_2_Relu_output_0_layer, 17,
  NL_TYPE, 0x0, NULL,
  nl, forward_relu,
  &_net_net_1_blocks_blocks_2_main_main_2_Relu_output_0_chain,
  NULL, &_net_net_1_blocks_blocks_2_main_main_3_Conv_output_0_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_main_main_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_blocks_blocks_1_relu_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_blocks_blocks_2_main_main_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_net_net_1_blocks_blocks_2_main_main_0_Conv_output_0_weights, &_net_net_1_blocks_blocks_2_main_main_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_net_net_1_blocks_blocks_2_main_main_0_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_main_main_0_Conv_output_0_layer, 16,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_net_net_1_blocks_blocks_2_main_main_0_Conv_output_0_chain,
  NULL, &_net_net_1_blocks_blocks_2_main_main_2_Relu_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 2), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 4, 0, 4, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_SAME, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_shortcut_shortcut_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_blocks_blocks_1_relu_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_blocks_blocks_2_shortcut_shortcut_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_net_net_1_blocks_blocks_2_shortcut_shortcut_0_Conv_output_0_weights, &_net_net_1_blocks_blocks_2_shortcut_shortcut_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_net_net_1_blocks_blocks_2_shortcut_shortcut_0_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _net_net_1_blocks_blocks_2_shortcut_shortcut_0_Conv_output_0_layer, 19,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_net_net_1_blocks_blocks_2_shortcut_shortcut_0_Conv_output_0_chain,
  NULL, &_net_net_1_blocks_blocks_2_main_main_0_Conv_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 2), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_relu_Relu_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_blocks_blocks_1_Add_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_blocks_blocks_1_relu_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_relu_Relu_output_0_layer, 15,
  NL_TYPE, 0x0, NULL,
  nl, forward_relu,
  &_net_net_1_blocks_blocks_1_relu_Relu_output_0_chain,
  NULL, &_net_net_1_blocks_blocks_2_shortcut_shortcut_0_Conv_output_0_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_Add_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_net_net_1_blocks_blocks_1_main_main_3_Conv_output_0_output, &_net_net_1_blocks_blocks_1_shortcut_shortcut_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_blocks_blocks_1_Add_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_Add_output_0_layer, 14,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_net_net_1_blocks_blocks_1_Add_output_0_chain,
  NULL, &_net_net_1_blocks_blocks_1_relu_Relu_output_0_layer, AI_STATIC, 
  .operation = ai_sum_f32, 
  .buffer_operation = ai_sum_buffer_f32, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_main_main_3_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_blocks_blocks_1_main_main_2_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_blocks_blocks_1_main_main_3_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_net_net_1_blocks_blocks_1_main_main_3_Conv_output_0_weights, &_net_net_1_blocks_blocks_1_main_main_3_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_net_net_1_blocks_blocks_1_main_main_3_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_main_main_3_Conv_output_0_layer, 12,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_net_net_1_blocks_blocks_1_main_main_3_Conv_output_0_chain,
  NULL, &_net_net_1_blocks_blocks_1_Add_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 4, 0, 4, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_SAME, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_main_main_2_Relu_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_blocks_blocks_1_main_main_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_blocks_blocks_1_main_main_2_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_main_main_2_Relu_output_0_layer, 11,
  NL_TYPE, 0x0, NULL,
  nl, forward_relu,
  &_net_net_1_blocks_blocks_1_main_main_2_Relu_output_0_chain,
  NULL, &_net_net_1_blocks_blocks_1_main_main_3_Conv_output_0_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_main_main_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_blocks_blocks_0_relu_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_blocks_blocks_1_main_main_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_net_net_1_blocks_blocks_1_main_main_0_Conv_output_0_weights, &_net_net_1_blocks_blocks_1_main_main_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_net_net_1_blocks_blocks_1_main_main_0_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_main_main_0_Conv_output_0_layer, 10,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_net_net_1_blocks_blocks_1_main_main_0_Conv_output_0_chain,
  NULL, &_net_net_1_blocks_blocks_1_main_main_2_Relu_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 2), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 4, 0, 4, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_SAME, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_shortcut_shortcut_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_blocks_blocks_0_relu_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_blocks_blocks_1_shortcut_shortcut_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_net_net_1_blocks_blocks_1_shortcut_shortcut_0_Conv_output_0_weights, &_net_net_1_blocks_blocks_1_shortcut_shortcut_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_net_net_1_blocks_blocks_1_shortcut_shortcut_0_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _net_net_1_blocks_blocks_1_shortcut_shortcut_0_Conv_output_0_layer, 13,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_net_net_1_blocks_blocks_1_shortcut_shortcut_0_Conv_output_0_chain,
  NULL, &_net_net_1_blocks_blocks_1_main_main_0_Conv_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 2), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_relu_Relu_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_blocks_blocks_0_Add_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_blocks_blocks_0_relu_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_relu_Relu_output_0_layer, 9,
  NL_TYPE, 0x0, NULL,
  nl, forward_relu,
  &_net_net_1_blocks_blocks_0_relu_Relu_output_0_chain,
  NULL, &_net_net_1_blocks_blocks_1_shortcut_shortcut_0_Conv_output_0_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_Add_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_net_net_1_blocks_blocks_0_main_main_3_Conv_output_0_output, &_net_net_1_blocks_blocks_0_shortcut_shortcut_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_blocks_blocks_0_Add_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_Add_output_0_layer, 8,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_net_net_1_blocks_blocks_0_Add_output_0_chain,
  NULL, &_net_net_1_blocks_blocks_0_relu_Relu_output_0_layer, AI_STATIC, 
  .operation = ai_sum_f32, 
  .buffer_operation = ai_sum_buffer_f32, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_main_main_3_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_blocks_blocks_0_main_main_2_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_blocks_blocks_0_main_main_3_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_net_net_1_blocks_blocks_0_main_main_3_Conv_output_0_weights, &_net_net_1_blocks_blocks_0_main_main_3_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_net_net_1_blocks_blocks_0_main_main_3_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_main_main_3_Conv_output_0_layer, 6,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_net_net_1_blocks_blocks_0_main_main_3_Conv_output_0_chain,
  NULL, &_net_net_1_blocks_blocks_0_Add_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 4, 0, 4, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_SAME, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_main_main_2_Relu_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_blocks_blocks_0_main_main_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_blocks_blocks_0_main_main_2_Relu_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_main_main_2_Relu_output_0_layer, 5,
  NL_TYPE, 0x0, NULL,
  nl, forward_relu,
  &_net_net_1_blocks_blocks_0_main_main_2_Relu_output_0_chain,
  NULL, &_net_net_1_blocks_blocks_0_main_main_3_Conv_output_0_layer, AI_STATIC, 
  .nl_params = NULL, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_main_main_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_stem_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_blocks_blocks_0_main_main_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_net_net_1_blocks_blocks_0_main_main_0_Conv_output_0_weights, &_net_net_1_blocks_blocks_0_main_main_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_net_net_1_blocks_blocks_0_main_main_0_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_main_main_0_Conv_output_0_layer, 4,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_net_net_1_blocks_blocks_0_main_main_0_Conv_output_0_chain,
  NULL, &_net_net_1_blocks_blocks_0_main_main_2_Relu_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 2), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 4, 0, 4, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_SAME, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_shortcut_shortcut_0_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_stem_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_blocks_blocks_0_shortcut_shortcut_0_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_net_net_1_blocks_blocks_0_shortcut_shortcut_0_Conv_output_0_weights, &_net_net_1_blocks_blocks_0_shortcut_shortcut_0_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_net_net_1_blocks_blocks_0_shortcut_shortcut_0_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _net_net_1_blocks_blocks_0_shortcut_shortcut_0_Conv_output_0_layer, 7,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_net_net_1_blocks_blocks_0_shortcut_shortcut_0_Conv_output_0_chain,
  NULL, &_net_net_1_blocks_blocks_0_main_main_0_Conv_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 2), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 0, 0, 0, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _net_net_1_stem_Conv_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_0_Div_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_1_stem_Conv_output_0_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 3, &_net_net_1_stem_Conv_output_0_weights, &_net_net_1_stem_Conv_output_0_bias, NULL),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_net_net_1_stem_Conv_output_0_scratch0, NULL)
)

AI_LAYER_OBJ_DECLARE(
  _net_net_1_stem_Conv_output_0_layer, 3,
  CONV2D_TYPE, 0x0, NULL,
  conv2d, forward_conv2d_if32of32wf32,
  &_net_net_1_stem_Conv_output_0_chain,
  NULL, &_net_net_1_blocks_blocks_0_shortcut_shortcut_0_Conv_output_0_layer, AI_STATIC, 
  .groups = 1, 
  .filter_stride = AI_SHAPE_2D_INIT(1, 1), 
  .dilation = AI_SHAPE_2D_INIT(1, 1), 
  .filter_pad = AI_SHAPE_INIT(4, 1, 0, 1, 0), 
  .in_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_SAME, 
  .out_ch_format = AI_LAYER_FORMAT_CHANNEL_LAST_VALID, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _net_net_0_Div_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &_net_net_0_Sub_output_0_output, &net_0_std),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_0_Div_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _net_net_0_Div_output_0_layer, 2,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_net_net_0_Div_output_0_chain,
  NULL, &_net_net_1_stem_Conv_output_0_layer, AI_STATIC, 
  .operation = ai_div_f32, 
  .buffer_operation = ai_div_buffer_f32, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  _net_net_0_Sub_output_0_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 2, &logmel_Transpose_output, &net_0_mean),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &_net_net_0_Sub_output_0_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  _net_net_0_Sub_output_0_layer, 1,
  ELTWISE_TYPE, 0x0, NULL,
  eltwise, forward_eltwise,
  &_net_net_0_Sub_output_0_chain,
  NULL, &_net_net_0_Div_output_0_layer, AI_STATIC, 
  .operation = ai_sub_f32, 
  .buffer_operation = ai_sub_buffer_f32, 
)

AI_TENSOR_CHAIN_OBJ_DECLARE(
  logmel_Transpose_chain, AI_STATIC_CONST, 4,
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &logmel_output),
  AI_TENSOR_LIST_OBJ_INIT(AI_FLAG_NONE, 1, &logmel_Transpose_output),
  AI_TENSOR_LIST_OBJ_EMPTY,
  AI_TENSOR_LIST_OBJ_EMPTY
)

AI_LAYER_OBJ_DECLARE(
  logmel_Transpose_layer, 2,
  TRANSPOSE_TYPE, 0x0, NULL,
  transpose, forward_transpose,
  &logmel_Transpose_chain,
  NULL, &_net_net_0_Sub_output_0_layer, AI_STATIC, 
  .out_mapping = AI_SHAPE_INIT(6, AI_SHAPE_IN_CHANNEL, AI_SHAPE_HEIGHT, AI_SHAPE_WIDTH, AI_SHAPE_CHANNEL, AI_SHAPE_DEPTH, AI_SHAPE_EXTENSION), 
)


#if (AI_TOOLS_API_VERSION < AI_TOOLS_API_VERSION_1_5)

AI_NETWORK_OBJ_DECLARE(
  AI_NET_OBJ_INSTANCE, AI_STATIC,
  AI_BUFFER_INIT(AI_FLAG_NONE,  AI_BUFFER_FORMAT_U8,
    AI_BUFFER_SHAPE_INIT(AI_SHAPE_BCWH, 4, 1, 225468, 1, 1),
    225468, NULL, NULL),
  AI_BUFFER_INIT(AI_FLAG_NONE,  AI_BUFFER_FORMAT_U8,
    AI_BUFFER_SHAPE_INIT(AI_SHAPE_BCWH, 4, 1, 32320, 1, 1),
    32320, NULL, NULL),
  AI_TENSOR_LIST_IO_OBJ_INIT(AI_FLAG_NONE, AI_NETWORK_IN_NUM, &logmel_output),
  AI_TENSOR_LIST_IO_OBJ_INIT(AI_FLAG_NONE, AI_NETWORK_OUT_NUM, &probs_output),
  &logmel_Transpose_layer, 0x8ff1a8b6, NULL)

#else

AI_NETWORK_OBJ_DECLARE(
  AI_NET_OBJ_INSTANCE, AI_STATIC,
  AI_BUFFER_ARRAY_OBJ_INIT_STATIC(
  	AI_FLAG_NONE, 1,
    AI_BUFFER_INIT(AI_FLAG_NONE,  AI_BUFFER_FORMAT_U8,
      AI_BUFFER_SHAPE_INIT(AI_SHAPE_BCWH, 4, 1, 225468, 1, 1),
      225468, NULL, NULL)
  ),
  AI_BUFFER_ARRAY_OBJ_INIT_STATIC(
  	AI_FLAG_NONE, 1,
    AI_BUFFER_INIT(AI_FLAG_NONE,  AI_BUFFER_FORMAT_U8,
      AI_BUFFER_SHAPE_INIT(AI_SHAPE_BCWH, 4, 1, 32320, 1, 1),
      32320, NULL, NULL)
  ),
  AI_TENSOR_LIST_IO_OBJ_INIT(AI_FLAG_NONE, AI_NETWORK_IN_NUM, &logmel_output),
  AI_TENSOR_LIST_IO_OBJ_INIT(AI_FLAG_NONE, AI_NETWORK_OUT_NUM, &probs_output),
  &logmel_Transpose_layer, 0x8ff1a8b6, NULL)

#endif	/*(AI_TOOLS_API_VERSION < AI_TOOLS_API_VERSION_1_5)*/



/******************************************************************************/
AI_DECLARE_STATIC
ai_bool network_configure_activations(
  ai_network* net_ctx, const ai_network_params* params)
{
  AI_ASSERT(net_ctx)

  if (ai_platform_get_activations_map(g_network_activations_map, 1, params)) {
    /* Updating activations (byte) offsets */
    
    logmel_output_array.data = AI_PTR(g_network_activations_map[0] + 0);
    logmel_output_array.data_start = AI_PTR(g_network_activations_map[0] + 0);
    logmel_Transpose_output_array.data = AI_PTR(g_network_activations_map[0] + 16160);
    logmel_Transpose_output_array.data_start = AI_PTR(g_network_activations_map[0] + 16160);
    _net_net_0_Sub_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 0);
    _net_net_0_Sub_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 0);
    _net_net_0_Div_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 16160);
    _net_net_0_Div_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 16160);
    _net_net_1_stem_Conv_output_0_scratch0_array.data = AI_PTR(g_network_activations_map[0] + 0);
    _net_net_1_stem_Conv_output_0_scratch0_array.data_start = AI_PTR(g_network_activations_map[0] + 0);
    _net_net_1_stem_Conv_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 480);
    _net_net_1_stem_Conv_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 480);
    _net_net_1_blocks_blocks_0_shortcut_shortcut_0_Conv_output_0_scratch0_array.data = AI_PTR(g_network_activations_map[0] + 0);
    _net_net_1_blocks_blocks_0_shortcut_shortcut_0_Conv_output_0_scratch0_array.data_start = AI_PTR(g_network_activations_map[0] + 0);
    _net_net_1_blocks_blocks_0_shortcut_shortcut_0_Conv_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 6944);
    _net_net_1_blocks_blocks_0_shortcut_shortcut_0_Conv_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 6944);
    _net_net_1_blocks_blocks_0_main_main_0_Conv_output_0_scratch0_array.data = AI_PTR(g_network_activations_map[0] + 11840);
    _net_net_1_blocks_blocks_0_main_main_0_Conv_output_0_scratch0_array.data_start = AI_PTR(g_network_activations_map[0] + 11840);
    _net_net_1_blocks_blocks_0_main_main_0_Conv_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 12416);
    _net_net_1_blocks_blocks_0_main_main_0_Conv_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 12416);
    _net_net_1_blocks_blocks_0_main_main_2_Relu_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 0);
    _net_net_1_blocks_blocks_0_main_main_2_Relu_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 0);
    _net_net_1_blocks_blocks_0_main_main_3_Conv_output_0_scratch0_array.data = AI_PTR(g_network_activations_map[0] + 4896);
    _net_net_1_blocks_blocks_0_main_main_3_Conv_output_0_scratch0_array.data_start = AI_PTR(g_network_activations_map[0] + 4896);
    _net_net_1_blocks_blocks_0_main_main_3_Conv_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 11840);
    _net_net_1_blocks_blocks_0_main_main_3_Conv_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 11840);
    _net_net_1_blocks_blocks_0_Add_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 0);
    _net_net_1_blocks_blocks_0_Add_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 0);
    _net_net_1_blocks_blocks_0_relu_Relu_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 4896);
    _net_net_1_blocks_blocks_0_relu_Relu_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 4896);
    _net_net_1_blocks_blocks_1_shortcut_shortcut_0_Conv_output_0_scratch0_array.data = AI_PTR(g_network_activations_map[0] + 0);
    _net_net_1_blocks_blocks_1_shortcut_shortcut_0_Conv_output_0_scratch0_array.data_start = AI_PTR(g_network_activations_map[0] + 0);
    _net_net_1_blocks_blocks_1_shortcut_shortcut_0_Conv_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 96);
    _net_net_1_blocks_blocks_1_shortcut_shortcut_0_Conv_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 96);
    _net_net_1_blocks_blocks_1_main_main_0_Conv_output_0_scratch0_array.data = AI_PTR(g_network_activations_map[0] + 3424);
    _net_net_1_blocks_blocks_1_main_main_0_Conv_output_0_scratch0_array.data_start = AI_PTR(g_network_activations_map[0] + 3424);
    _net_net_1_blocks_blocks_1_main_main_0_Conv_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 9792);
    _net_net_1_blocks_blocks_1_main_main_0_Conv_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 9792);
    _net_net_1_blocks_blocks_1_main_main_2_Relu_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 3424);
    _net_net_1_blocks_blocks_1_main_main_2_Relu_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 3424);
    _net_net_1_blocks_blocks_1_main_main_3_Conv_output_0_scratch0_array.data = AI_PTR(g_network_activations_map[0] + 6752);
    _net_net_1_blocks_blocks_1_main_main_3_Conv_output_0_scratch0_array.data_start = AI_PTR(g_network_activations_map[0] + 6752);
    _net_net_1_blocks_blocks_1_main_main_3_Conv_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 7904);
    _net_net_1_blocks_blocks_1_main_main_3_Conv_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 7904);
    _net_net_1_blocks_blocks_1_Add_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 3424);
    _net_net_1_blocks_blocks_1_Add_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 3424);
    _net_net_1_blocks_blocks_1_relu_Relu_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 0);
    _net_net_1_blocks_blocks_1_relu_Relu_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 0);
    _net_net_1_blocks_blocks_2_shortcut_shortcut_0_Conv_output_0_scratch0_array.data = AI_PTR(g_network_activations_map[0] + 3328);
    _net_net_1_blocks_blocks_2_shortcut_shortcut_0_Conv_output_0_scratch0_array.data_start = AI_PTR(g_network_activations_map[0] + 3328);
    _net_net_1_blocks_blocks_2_shortcut_shortcut_0_Conv_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 3456);
    _net_net_1_blocks_blocks_2_shortcut_shortcut_0_Conv_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 3456);
    _net_net_1_blocks_blocks_2_main_main_0_Conv_output_0_scratch0_array.data = AI_PTR(g_network_activations_map[0] + 5536);
    _net_net_1_blocks_blocks_2_main_main_0_Conv_output_0_scratch0_array.data_start = AI_PTR(g_network_activations_map[0] + 5536);
    _net_net_1_blocks_blocks_2_main_main_0_Conv_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 6688);
    _net_net_1_blocks_blocks_2_main_main_0_Conv_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 6688);
    _net_net_1_blocks_blocks_2_main_main_2_Relu_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 0);
    _net_net_1_blocks_blocks_2_main_main_2_Relu_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 0);
    _net_net_1_blocks_blocks_2_main_main_3_Conv_output_0_scratch0_array.data = AI_PTR(g_network_activations_map[0] + 5536);
    _net_net_1_blocks_blocks_2_main_main_3_Conv_output_0_scratch0_array.data_start = AI_PTR(g_network_activations_map[0] + 5536);
    _net_net_1_blocks_blocks_2_main_main_3_Conv_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 6976);
    _net_net_1_blocks_blocks_2_main_main_3_Conv_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 6976);
    _net_net_1_blocks_blocks_2_Add_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 0);
    _net_net_1_blocks_blocks_2_Add_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 0);
    _net_net_1_blocks_blocks_2_relu_Relu_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 2080);
    _net_net_1_blocks_blocks_2_relu_Relu_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 2080);
    _net_net_1_head_head_0_GlobalAveragePool_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 0);
    _net_net_1_head_head_0_GlobalAveragePool_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 0);
    _net_net_1_head_head_3_Gemm_output_0_output_array.data = AI_PTR(g_network_activations_map[0] + 160);
    _net_net_1_head_head_3_Gemm_output_0_output_array.data_start = AI_PTR(g_network_activations_map[0] + 160);
    probs_output_array.data = AI_PTR(g_network_activations_map[0] + 0);
    probs_output_array.data_start = AI_PTR(g_network_activations_map[0] + 0);
    return true;
  }
  AI_ERROR_TRAP(net_ctx, INIT_FAILED, NETWORK_ACTIVATIONS);
  return false;
}




/******************************************************************************/
AI_DECLARE_STATIC
ai_bool network_configure_weights(
  ai_network* net_ctx, const ai_network_params* params)
{
  AI_ASSERT(net_ctx)

  if (ai_platform_get_weights_map(g_network_weights_map, 1, params)) {
    /* Updating weights (byte) offsets */
    
    net_0_std_array.format |= AI_FMT_FLAG_CONST;
    net_0_std_array.data = AI_PTR(g_network_weights_map[0] + 0);
    net_0_std_array.data_start = AI_PTR(g_network_weights_map[0] + 0);
    net_0_mean_array.format |= AI_FMT_FLAG_CONST;
    net_0_mean_array.data = AI_PTR(g_network_weights_map[0] + 160);
    net_0_mean_array.data_start = AI_PTR(g_network_weights_map[0] + 160);
    _net_net_1_stem_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _net_net_1_stem_Conv_output_0_weights_array.data = AI_PTR(g_network_weights_map[0] + 320);
    _net_net_1_stem_Conv_output_0_weights_array.data_start = AI_PTR(g_network_weights_map[0] + 320);
    _net_net_1_stem_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _net_net_1_stem_Conv_output_0_bias_array.data = AI_PTR(g_network_weights_map[0] + 8000);
    _net_net_1_stem_Conv_output_0_bias_array.data_start = AI_PTR(g_network_weights_map[0] + 8000);
    _net_net_1_blocks_blocks_0_shortcut_shortcut_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _net_net_1_blocks_blocks_0_shortcut_shortcut_0_Conv_output_0_weights_array.data = AI_PTR(g_network_weights_map[0] + 8064);
    _net_net_1_blocks_blocks_0_shortcut_shortcut_0_Conv_output_0_weights_array.data_start = AI_PTR(g_network_weights_map[0] + 8064);
    _net_net_1_blocks_blocks_0_shortcut_shortcut_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _net_net_1_blocks_blocks_0_shortcut_shortcut_0_Conv_output_0_bias_array.data = AI_PTR(g_network_weights_map[0] + 9600);
    _net_net_1_blocks_blocks_0_shortcut_shortcut_0_Conv_output_0_bias_array.data_start = AI_PTR(g_network_weights_map[0] + 9600);
    _net_net_1_blocks_blocks_0_main_main_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _net_net_1_blocks_blocks_0_main_main_0_Conv_output_0_weights_array.data = AI_PTR(g_network_weights_map[0] + 9696);
    _net_net_1_blocks_blocks_0_main_main_0_Conv_output_0_weights_array.data_start = AI_PTR(g_network_weights_map[0] + 9696);
    _net_net_1_blocks_blocks_0_main_main_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _net_net_1_blocks_blocks_0_main_main_0_Conv_output_0_bias_array.data = AI_PTR(g_network_weights_map[0] + 23520);
    _net_net_1_blocks_blocks_0_main_main_0_Conv_output_0_bias_array.data_start = AI_PTR(g_network_weights_map[0] + 23520);
    _net_net_1_blocks_blocks_0_main_main_3_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _net_net_1_blocks_blocks_0_main_main_3_Conv_output_0_weights_array.data = AI_PTR(g_network_weights_map[0] + 23616);
    _net_net_1_blocks_blocks_0_main_main_3_Conv_output_0_weights_array.data_start = AI_PTR(g_network_weights_map[0] + 23616);
    _net_net_1_blocks_blocks_0_main_main_3_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _net_net_1_blocks_blocks_0_main_main_3_Conv_output_0_bias_array.data = AI_PTR(g_network_weights_map[0] + 44352);
    _net_net_1_blocks_blocks_0_main_main_3_Conv_output_0_bias_array.data_start = AI_PTR(g_network_weights_map[0] + 44352);
    _net_net_1_blocks_blocks_1_shortcut_shortcut_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _net_net_1_blocks_blocks_1_shortcut_shortcut_0_Conv_output_0_weights_array.data = AI_PTR(g_network_weights_map[0] + 44448);
    _net_net_1_blocks_blocks_1_shortcut_shortcut_0_Conv_output_0_weights_array.data_start = AI_PTR(g_network_weights_map[0] + 44448);
    _net_net_1_blocks_blocks_1_shortcut_shortcut_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _net_net_1_blocks_blocks_1_shortcut_shortcut_0_Conv_output_0_bias_array.data = AI_PTR(g_network_weights_map[0] + 47520);
    _net_net_1_blocks_blocks_1_shortcut_shortcut_0_Conv_output_0_bias_array.data_start = AI_PTR(g_network_weights_map[0] + 47520);
    _net_net_1_blocks_blocks_1_main_main_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _net_net_1_blocks_blocks_1_main_main_0_Conv_output_0_weights_array.data = AI_PTR(g_network_weights_map[0] + 47648);
    _net_net_1_blocks_blocks_1_main_main_0_Conv_output_0_weights_array.data_start = AI_PTR(g_network_weights_map[0] + 47648);
    _net_net_1_blocks_blocks_1_main_main_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _net_net_1_blocks_blocks_1_main_main_0_Conv_output_0_bias_array.data = AI_PTR(g_network_weights_map[0] + 75296);
    _net_net_1_blocks_blocks_1_main_main_0_Conv_output_0_bias_array.data_start = AI_PTR(g_network_weights_map[0] + 75296);
    _net_net_1_blocks_blocks_1_main_main_3_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _net_net_1_blocks_blocks_1_main_main_3_Conv_output_0_weights_array.data = AI_PTR(g_network_weights_map[0] + 75424);
    _net_net_1_blocks_blocks_1_main_main_3_Conv_output_0_weights_array.data_start = AI_PTR(g_network_weights_map[0] + 75424);
    _net_net_1_blocks_blocks_1_main_main_3_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _net_net_1_blocks_blocks_1_main_main_3_Conv_output_0_bias_array.data = AI_PTR(g_network_weights_map[0] + 112288);
    _net_net_1_blocks_blocks_1_main_main_3_Conv_output_0_bias_array.data_start = AI_PTR(g_network_weights_map[0] + 112288);
    _net_net_1_blocks_blocks_2_shortcut_shortcut_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _net_net_1_blocks_blocks_2_shortcut_shortcut_0_Conv_output_0_weights_array.data = AI_PTR(g_network_weights_map[0] + 112416);
    _net_net_1_blocks_blocks_2_shortcut_shortcut_0_Conv_output_0_weights_array.data_start = AI_PTR(g_network_weights_map[0] + 112416);
    _net_net_1_blocks_blocks_2_shortcut_shortcut_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _net_net_1_blocks_blocks_2_shortcut_shortcut_0_Conv_output_0_bias_array.data = AI_PTR(g_network_weights_map[0] + 117536);
    _net_net_1_blocks_blocks_2_shortcut_shortcut_0_Conv_output_0_bias_array.data_start = AI_PTR(g_network_weights_map[0] + 117536);
    _net_net_1_blocks_blocks_2_main_main_0_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _net_net_1_blocks_blocks_2_main_main_0_Conv_output_0_weights_array.data = AI_PTR(g_network_weights_map[0] + 117696);
    _net_net_1_blocks_blocks_2_main_main_0_Conv_output_0_weights_array.data_start = AI_PTR(g_network_weights_map[0] + 117696);
    _net_net_1_blocks_blocks_2_main_main_0_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _net_net_1_blocks_blocks_2_main_main_0_Conv_output_0_bias_array.data = AI_PTR(g_network_weights_map[0] + 163776);
    _net_net_1_blocks_blocks_2_main_main_0_Conv_output_0_bias_array.data_start = AI_PTR(g_network_weights_map[0] + 163776);
    _net_net_1_blocks_blocks_2_main_main_3_Conv_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _net_net_1_blocks_blocks_2_main_main_3_Conv_output_0_weights_array.data = AI_PTR(g_network_weights_map[0] + 163936);
    _net_net_1_blocks_blocks_2_main_main_3_Conv_output_0_weights_array.data_start = AI_PTR(g_network_weights_map[0] + 163936);
    _net_net_1_blocks_blocks_2_main_main_3_Conv_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _net_net_1_blocks_blocks_2_main_main_3_Conv_output_0_bias_array.data = AI_PTR(g_network_weights_map[0] + 221536);
    _net_net_1_blocks_blocks_2_main_main_3_Conv_output_0_bias_array.data_start = AI_PTR(g_network_weights_map[0] + 221536);
    _net_net_1_head_head_3_Gemm_output_0_weights_array.format |= AI_FMT_FLAG_CONST;
    _net_net_1_head_head_3_Gemm_output_0_weights_array.data = AI_PTR(g_network_weights_map[0] + 221696);
    _net_net_1_head_head_3_Gemm_output_0_weights_array.data_start = AI_PTR(g_network_weights_map[0] + 221696);
    _net_net_1_head_head_3_Gemm_output_0_bias_array.format |= AI_FMT_FLAG_CONST;
    _net_net_1_head_head_3_Gemm_output_0_bias_array.data = AI_PTR(g_network_weights_map[0] + 225376);
    _net_net_1_head_head_3_Gemm_output_0_bias_array.data_start = AI_PTR(g_network_weights_map[0] + 225376);
    return true;
  }
  AI_ERROR_TRAP(net_ctx, INIT_FAILED, NETWORK_WEIGHTS);
  return false;
}


/**  PUBLIC APIs SECTION  *****************************************************/



AI_DEPRECATED
AI_API_ENTRY
ai_bool ai_network_get_info(
  ai_handle network, ai_network_report* report)
{
  ai_network* net_ctx = AI_NETWORK_ACQUIRE_CTX(network);

  if (report && net_ctx)
  {
    ai_network_report r = {
      .model_name        = AI_NETWORK_MODEL_NAME,
      .model_signature   = AI_NETWORK_MODEL_SIGNATURE,
      .model_datetime    = AI_TOOLS_DATE_TIME,
      
      .compile_datetime  = AI_TOOLS_COMPILE_TIME,
      
      .runtime_revision  = ai_platform_runtime_get_revision(),
      .runtime_version   = ai_platform_runtime_get_version(),

      .tool_revision     = AI_TOOLS_REVISION_ID,
      .tool_version      = {AI_TOOLS_VERSION_MAJOR, AI_TOOLS_VERSION_MINOR,
                            AI_TOOLS_VERSION_MICRO, 0x0},
      .tool_api_version  = AI_STRUCT_INIT,

      .api_version            = ai_platform_api_get_version(),
      .interface_api_version  = ai_platform_interface_api_get_version(),
      
      .n_macc            = 1483140,
      .n_inputs          = 0,
      .inputs            = NULL,
      .n_outputs         = 0,
      .outputs           = NULL,
      .params            = AI_STRUCT_INIT,
      .activations       = AI_STRUCT_INIT,
      .n_nodes           = 0,
      .signature         = 0x8ff1a8b6,
    };

    if (!ai_platform_api_get_network_report(network, &r)) return false;

    *report = r;
    return true;
  }
  return false;
}



AI_API_ENTRY
ai_bool ai_network_get_report(
  ai_handle network, ai_network_report* report)
{
  ai_network* net_ctx = AI_NETWORK_ACQUIRE_CTX(network);

  if (report && net_ctx)
  {
    ai_network_report r = {
      .model_name        = AI_NETWORK_MODEL_NAME,
      .model_signature   = AI_NETWORK_MODEL_SIGNATURE,
      .model_datetime    = AI_TOOLS_DATE_TIME,
      
      .compile_datetime  = AI_TOOLS_COMPILE_TIME,
      
      .runtime_revision  = ai_platform_runtime_get_revision(),
      .runtime_version   = ai_platform_runtime_get_version(),

      .tool_revision     = AI_TOOLS_REVISION_ID,
      .tool_version      = {AI_TOOLS_VERSION_MAJOR, AI_TOOLS_VERSION_MINOR,
                            AI_TOOLS_VERSION_MICRO, 0x0},
      .tool_api_version  = AI_STRUCT_INIT,

      .api_version            = ai_platform_api_get_version(),
      .interface_api_version  = ai_platform_interface_api_get_version(),
      
      .n_macc            = 1483140,
      .n_inputs          = 0,
      .inputs            = NULL,
      .n_outputs         = 0,
      .outputs           = NULL,
      .map_signature     = AI_MAGIC_SIGNATURE,
      .map_weights       = AI_STRUCT_INIT,
      .map_activations   = AI_STRUCT_INIT,
      .n_nodes           = 0,
      .signature         = 0x8ff1a8b6,
    };

    if (!ai_platform_api_get_network_report(network, &r)) return false;

    *report = r;
    return true;
  }
  return false;
}


AI_API_ENTRY
ai_error ai_network_get_error(ai_handle network)
{
  return ai_platform_network_get_error(network);
}


AI_API_ENTRY
ai_error ai_network_create(
  ai_handle* network, const ai_buffer* network_config)
{
  return ai_platform_network_create(
    network, network_config, 
    AI_CONTEXT_OBJ(&AI_NET_OBJ_INSTANCE),
    AI_TOOLS_API_VERSION_MAJOR, AI_TOOLS_API_VERSION_MINOR, AI_TOOLS_API_VERSION_MICRO);
}


AI_API_ENTRY
ai_error ai_network_create_and_init(
  ai_handle* network, const ai_handle activations[], const ai_handle weights[])
{
  ai_error err;
  ai_network_params params;

  err = ai_network_create(network, AI_NETWORK_DATA_CONFIG);
  if (err.type != AI_ERROR_NONE) {
    return err;
  }
  
  if (ai_network_data_params_get(&params) != true) {
    err = ai_network_get_error(*network);
    return err;
  }
#if defined(AI_NETWORK_DATA_ACTIVATIONS_COUNT)
  /* set the addresses of the activations buffers */
  for (ai_u16 idx=0; activations && idx<params.map_activations.size; idx++) {
    AI_BUFFER_ARRAY_ITEM_SET_ADDRESS(&params.map_activations, idx, activations[idx]);
  }
#endif
#if defined(AI_NETWORK_DATA_WEIGHTS_COUNT)
  /* set the addresses of the weight buffers */
  for (ai_u16 idx=0; weights && idx<params.map_weights.size; idx++) {
    AI_BUFFER_ARRAY_ITEM_SET_ADDRESS(&params.map_weights, idx, weights[idx]);
  }
#endif
  if (ai_network_init(*network, &params) != true) {
    err = ai_network_get_error(*network);
  }
  return err;
}


AI_API_ENTRY
ai_buffer* ai_network_inputs_get(ai_handle network, ai_u16 *n_buffer)
{
  if (network == AI_HANDLE_NULL) {
    network = (ai_handle)&AI_NET_OBJ_INSTANCE;
    AI_NETWORK_OBJ(network)->magic = AI_MAGIC_CONTEXT_TOKEN;
  }
  return ai_platform_inputs_get(network, n_buffer);
}


AI_API_ENTRY
ai_buffer* ai_network_outputs_get(ai_handle network, ai_u16 *n_buffer)
{
  if (network == AI_HANDLE_NULL) {
    network = (ai_handle)&AI_NET_OBJ_INSTANCE;
    AI_NETWORK_OBJ(network)->magic = AI_MAGIC_CONTEXT_TOKEN;
  }
  return ai_platform_outputs_get(network, n_buffer);
}


AI_API_ENTRY
ai_handle ai_network_destroy(ai_handle network)
{
  return ai_platform_network_destroy(network);
}


AI_API_ENTRY
ai_bool ai_network_init(
  ai_handle network, const ai_network_params* params)
{
  ai_network* net_ctx = AI_NETWORK_OBJ(ai_platform_network_init(network, params));
  ai_bool ok = true;

  if (!net_ctx) return false;
  ok &= network_configure_weights(net_ctx, params);
  ok &= network_configure_activations(net_ctx, params);

  ok &= ai_platform_network_post_init(network);

  return ok;
}


AI_API_ENTRY
ai_i32 ai_network_run(
  ai_handle network, const ai_buffer* input, ai_buffer* output)
{
  return ai_platform_network_process(network, input, output);
}


AI_API_ENTRY
ai_i32 ai_network_forward(ai_handle network, const ai_buffer* input)
{
  return ai_platform_network_process(network, input, NULL);
}



#undef AI_NETWORK_MODEL_SIGNATURE
#undef AI_NET_OBJ_INSTANCE
#undef AI_TOOLS_DATE_TIME
#undef AI_TOOLS_COMPILE_TIME

