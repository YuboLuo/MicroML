#include <stdint.h>
#include "../math/matrix_ops.h"
#include "../math/matrix.h"
#include "../math/fixed_point_ops.h"
#include "../utils/utils.h"
#include "../neural_network_parameters.h"
#include "cells.h"

#ifndef LAYERS_GUARD
#define LAYERS_GUARD


// Standard Neural Network Functions
matrix *dense(matrix *result, matrix *input, matrix *W, matrix *b, int16_t (*activation)(int16_t, uint16_t), uint16_t precision);
matrix *apply_gate(matrix *result, matrix *gate, matrix *first, matrix *second, matrix *temp, uint16_t precision);
matrix *apply_leakyrelu(matrix *result, matrix *input, uint16_t precision);

// RNN Cell Functions
matrix *apply_gru(matrix *result, matrix *input, matrix *state, GRU *gru, GRUTempStates *temp, uint16_t precision);
matrix *apply_ugrnn(matrix *result, matrix *input, matrix *state, UGRNN *ugrnn, UGRNNTempStates *temp, uint16_t precision);

matrix *conv2d_maxpooling(matrix *result, matrix *input, uint16_t poolsize_Rows, uint16_t poolsize_Cols);
matrix *conv2d_maxpooling_multi_filter(matrix *result, matrix *input, uint16_t numFilters, uint16_t poolsize_Rows, uint16_t poolsize_Cols);
matrix *conv2d_filter(matrix* result, matrix *input, matrix *filter, uint16_t precision, uint16_t stride_numRows, uint16_t stride_numCols);
matrix *conv2d_multi_channel_filter(matrix *result, matrix *input, matrix *filter, uint16_t numChannels, int16_t b, int16_t (*activation)(int16_t, uint16_t), uint16_t precision, uint16_t stride_numRows, uint16_t stride_numCols);
matrix *conv2d_multi_channel_multi_filter(matrix *result, matrix *input, matrix *filter, uint16_t numFilters, uint16_t numChannels, int16_t *b, int16_t (*activation)(int16_t, uint16_t), uint16_t precision, uint16_t stride_numRows, uint16_t stride_numCols);
matrix *conv2d_flatten(matrix* result, matrix *input, uint16_t numFilters);

matrix *apply_lea_format(matrix *result, matrix *input);

#endif
