#include "layers.h"

#pragma PERSISTENT(TEMP_RESULT_ARRAY)
static int16_t TEMP_RESULT_ARRAY[128] = {0};

#pragma PERSISTENT(PADDING_BUFFER)
static dtype PADDING_BUFFER[4096] = {0};

matrix *dense(matrix *result, matrix *input, matrix *W, matrix *b, int16_t (*activation)(int16_t, uint16_t), uint16_t precision) {
    /**
     * Implementation of a dense feed-forward layer (FC - full connected layer) using matrix operations.
     */
    result = large_matrix_multiply(result, W, input, precision);

    // Only add bias if given 
    if (b != NULL_PTR) {
        result = matrix_add(result, result, b);
    }

    result = apply_elementwise(result, result, activation, precision);
    return result;
}

matrix *conv2d_maxpooling(matrix *result, matrix *input, uint16_t poolsize_Rows, uint16_t poolsize_Cols, uint16_t padding){
    /**
     * Implementation of maxpooling layer for CNN
     */

    if (poolsize_Rows == 0 || poolsize_Cols == 0){
        return NULL_PTR;
    }

    matrix padding_mat = {PADDING_BUFFER, 0, 0};
    matrix_padding(&padding_mat, input, padding);
    input = &padding_mat;

    uint16_t i = 0, j = 0, k = 0, p_i, p_j, offset;
    int16_t *max;

    while (i < input->numRows){
        j = 0;
        while (j < input->numCols){
            // (i, j) is the coordinate of top-left element among all corresponding points in the original input matrix

            for (p_i = 0; p_i < poolsize_Rows; p_i ++){
                for (p_j = 0; p_j < poolsize_Cols; p_j ++){

                    offset = (i + p_i) * (input->numCols) + j + p_j;

                    if ((p_j == 0 && p_i == 0) || (input->data[offset] > *max)){
                        max = &(input->data[offset]);
                    }
                }
            }

            result->data[k] = *max;

            k ++;
            j += poolsize_Cols;
        }
        i += poolsize_Rows;
    }
    return result;
}


matrix *conv2d_maxpooling_multi_filter(matrix *result, matrix *input, uint16_t numFilters, uint16_t poolsize_Rows, uint16_t poolsize_Cols, uint16_t padding){
    /**
     * Iteration for each filter
     * one conv2d layer usually has multiple filters, we do maxpooling one by one
     */

    uint16_t i, filter_offset, result_offset, filter_length = input->numRows * input->numCols, result_length = result->numRows * result->numCols;
    int16_t *filter_head = input->data, *result_head = result->data;

    for (i = numFilters; i > 0; i --){
        filter_offset = (i - 1) * filter_length;
        result_offset = (i - 1) * result_length;

        input->data = &filter_head[filter_offset];
        result->data = &result_head[result_offset];

        /* process one filter at a time */
        conv2d_maxpooling(result, input, poolsize_Rows, poolsize_Cols, padding);
    }
    return result;
}



matrix *conv2d_flatten(matrix* result, matrix *input, uint16_t num_filter){
    /**
     * Implementation of flatten layer for CNN
     * the result of conv2d_maxpooling or conv2d_filter is saved in the order of filter by filter
     * however, the flatten result should be in this following way according to Tensorflow
     * f0[0], f1[0], ..., fn[0], f0[1], f1[1], ..., fn[1], ..., f0[n], f1[n], fn[n]
     */

    uint16_t i, j, inputOffset, resultOffset;
    uint16_t FilterSize;

    FilterSize = input->numCols * input->numRows;

    resultOffset = 0;
    for (i = 0; i < FilterSize; i ++ ){
        for (j = 0; j < num_filter; j ++){
            inputOffset = i + j * FilterSize;   // get the ith element of the jth filter
            result->data[resultOffset++] = input->data[inputOffset];   // append it to result
            result->data[resultOffset++] = 0; // for LEA, we have to append 0 to each number
        }
    }
    return result;

}

matrix *conv2d_filter(matrix* result, matrix *input, matrix *filter, uint16_t precision, uint16_t stride_numRows, uint16_t stride_numCols, uint16_t padding){
    /**
     * do the actual convolution operation for one filter on one channel
     */
    if (stride_numRows == 0 || stride_numCols == 0){
        return NULL_PTR;
    }

    matrix padding_mat = {PADDING_BUFFER, 0, 0};
    matrix_padding(&padding_mat, input, padding);
    input = &padding_mat;

    uint16_t i = 0, j, f_i, f_j, k = 0, input_offset, filter_offset;
    int16_t sum, mult_result, *mult0, *mult1;
    while (i <= input->numRows - filter->numRows){
        j = 0;
        while (j <= input->numCols - filter->numCols){
            sum = 0;
            for (f_i = 0; f_i < filter->numRows; f_i ++){
                for (f_j = 0; f_j < filter->numCols; f_j ++){
                    input_offset = (i + f_i) * (input->numCols) + j + f_j;
                    filter_offset = f_i * filter->numCols + f_j;

                    mult0 = &(input->data[input_offset]);
                    mult1 = &(filter->data[filter_offset]);
                    mult_result = fp_mul(*mult0, *mult1, precision);

                    sum += mult_result;
                }
            }
            result->data[k] = sum;
            k ++;
            j += stride_numCols;
        }
        i += stride_numRows;
    }
    return result;
}


matrix *conv2d_multi_channel_filter(matrix *result, matrix *input, matrix *filter, uint16_t numChannels, int16_t b, int16_t (*activation)(int16_t, uint16_t), uint16_t precision, uint16_t stride_numRows, uint16_t stride_numCols, uint16_t padding){

    int16_t *filter_head = filter->data;
    int16_t *input_head = input->data;
    uint16_t i, result_length = result->numRows * result->numCols, input_length = input->numRows * input->numCols, filter_length = filter->numRows * filter->numCols;
    matrix temp = {TEMP_RESULT_ARRAY, result->numRows, result->numCols};

    for (i = numChannels; i > 0; i --){
        input->data = &input_head[input_length * (i - 1)];
        filter->data = &filter_head[filter_length * (i - 1)];
        conv2d_filter_LEA(&temp, input, filter, precision, stride_numRows, stride_numCols, padding);
        matrix_add(result, result, &temp);
    }
    for (i = result_length; i > 0; i --){
        result->data[i - 1] = result->data[i - 1] + b;
    }

    result = apply_elementwise(result, result, activation, precision);
    return result;
}

matrix *conv2d_multi_channel_multi_filter(matrix *result, matrix *input, matrix *filter, uint16_t numFilters, uint16_t numChannels, int16_t *b, int16_t (*activation)(int16_t, uint16_t), uint16_t precision, uint16_t stride_numRows, uint16_t stride_numCols, uint16_t padding){

    uint16_t i, result_length = result->numRows * result->numCols, filter_length = filter->numRows * filter->numCols * numChannels;
    int16_t *filter_head = filter->data, *result_head = result->data;

    for (i = numFilters; i > 0; i --){
        filter->data = &(filter_head[(i - 1) * filter_length]);
        result->data = &(result_head[(i - 1) * result_length]);

        conv2d_multi_channel_filter(result, input, filter, numChannels, b[i - 1], activation, precision, stride_numRows, stride_numCols, padding);
    }
    return result;
}

matrix *apply_lea_format(matrix *result, matrix *input){
    uint16_t i, input_length = input->numCols * input->numRows;
    for (i = input_length; i > 0; i --){
        result->data[(i - 1) * 2 + 1] = 0;
        result->data[(i - 1) * 2] = input->data[i - 1];
    }
    return result;
}

matrix *apply_leakyrelu(matrix *result, matrix *input, uint16_t precision){
    result = apply_elementwise(result, input, &fp_leaky_relu, precision);
    return result;
}

matrix *apply_gate(matrix *result, matrix *gate, matrix *first, matrix *second, matrix *temp, uint16_t precision) {
    // Create the vector for 1 - gate
    temp = matrix_neg(temp, gate, precision);
    temp = scalar_add(temp, temp, int_to_fp(1, precision));

    // temp = (1.0 - gate) * second
    temp = matrix_hadamard(temp, second, temp, precision);

    // result = gate * first
    result = matrix_hadamard(result, first, gate, precision);

    // result += temp -> result = gate * first + (1.0 - gate) * second
    result = matrix_add(result, result, temp);

    return result;
}

matrix *apply_gru(matrix *result, matrix *input, matrix *state, GRU *gru, GRUTempStates *temp, uint16_t precision) {
    /**
     * Implementation of a GRU Cell.
     */
    // Unpack memory for intermediate states
    matrix *gates = temp->gates;
    matrix *candidate = temp->candidate;
    matrix *gateTemp = temp->gateTemp;
    matrix *stacked = temp->stacked;

    // Concatenate state and inputs for combined matrix multiplication
    matrix *state_input_concat = vstack(stacked, state, input);

    // Create the gates
    gates = matrix_multiply(gates, gru->wGates, state_input_concat, precision);
    gates = matrix_add(gates, gates, gru->bGates);
    gates = apply_elementwise(gates, gates, &fp_sigmoid, precision);

    // Split gates into reset and update components
    matrix update = { gates->data, STATE_SIZE, gates->numCols };

    uint16_t offset = update.numRows * update.numCols;
    matrix reset = { gates->data + offset, STATE_SIZE, gates->numCols };
    matrix_hadamard(&reset, state, &reset, precision);

    // Concatenate state and inputs to account for reset gate
    matrix *reset_input_concat = vstack(stacked, &reset, input);

    // Create the candidate state
    candidate = matrix_multiply(candidate, gru->wCandidate, reset_input_concat, precision);
    candidate = matrix_add(candidate, candidate, gru->bCandidate);
    candidate = apply_elementwise(candidate, candidate, &fp_tanh, precision);

    // Construct the result
    result = apply_gate(result, &update, state, candidate, gateTemp, precision);
 
    return result;
}


matrix *apply_ugrnn(matrix *result, matrix *input, matrix *state, UGRNN *gru, UGRNNTempStates *temp, uint16_t precision) {
    /**
     * Implementation of a UGRNN Cell. This cell has a single update gate.
     */
    // Unpack memory for intermediate states
    matrix *stacked = temp->stacked;
    matrix *transformed = temp->transformed;
    matrix *gateTemp = temp->gateTemp;

    // Concatenate state and inputs
    matrix *state_input_concat = vstack(stacked, state, input);

    // Compute the transformation
    transformed = matrix_multiply(transformed, gru->wTransform, state_input_concat, precision);
    transformed = matrix_add(transformed, transformed, gru->bTransform);

    // Split transformation into update and candidate components
    matrix update = { transformed->data, STATE_SIZE, transformed->numCols };

    uint16_t offset = update.numRows * update.numCols;
    matrix candidate = { transformed->data + offset, STATE_SIZE, transformed->numCols };

    // Create the update gate
    scalar_add(&update, &update, int_to_fp(1, precision));  // Add initial bias term
    apply_elementwise(&update, &update, &fp_sigmoid, precision);

    // Create the candidate state
    apply_elementwise(&candidate, &candidate, &fp_tanh, precision);

    // Construct the result
    result = apply_gate(result, &update, state, &candidate, gateTemp, precision);
 
    return result;
}
