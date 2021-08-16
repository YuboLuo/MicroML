#include "layers.h"

#pragma PERSISTENT(PADDING_BUFFER)
static dtype PADDING_BUFFER[4096] = {0};

#pragma PERSISTENT(FILTER_BUFFER)
static dtype FILTER_BUFFER[1024] = {0};


matrix *dense(matrix *result, matrix *input, matrix *W, matrix *b, int16_t (*activation)(int16_t, uint16_t), uint16_t precision) {
    /**
     * Implementation of a dense feed-forward layer using matrix operations.
     */
    result = matrix_multiply(result, W, input, precision);

    // Only add bias if given 
    if (b != NULL_PTR) {
        result = matrix_add(result, result, b);
    }

    result = apply_elementwise(result, result, activation, precision);
    return result;
}

matrix *maxpooling(matrix* result, matrix *input, uint16_t pool_numRows, uint16_t pool_numCols){
    /**
     * Implementation of maxpooling layer
     */
    uint16_t result_numRows = input->numRows / pool_numRows;
    uint16_t result_numCols = input->numCols / pool_numCols;
    uint16_t i, j, x, y, kx, ky, input_offset, result_offset;
    int16_t max;
    for (i = 0; i < result_numRows; i ++){
        for (j = 0; j < result_numCols; j ++){

            // (i, j) is the coordinate of each element after maxpooling
            // (x, y) is the coordinate of top-left element among all corresponding points in the original input matrix

            x = i * pool_numRows;
            y = j * pool_numCols;

            max = 0;
            for (kx = 0; kx < pool_numRows; kx ++){
                for (ky = 0; ky < pool_numCols; ky ++){
                    // traverse the entire sub-block that are related to this pooling
                    input_offset = (x + kx) * input->numRows + (y + ky);
                    if (max < input->data[input_offset]){
                        max = input->data[input_offset];  // if a bigger number found, update max
                    }

                }
            }
            result_offset = i * result_numRows + j;
            result->data[result_offset] = max;

        }
    }
    return result;
}


matrix *maxpooling_filters(matrix *result, matrix *input, uint16_t numFilters, uint16_t pool_numRows, uint16_t pool_numCols){
    /**
     * Iteration for each filter
     * one conv2d layer usually has multiple filters, we do maxpooling one by one
     */

    uint16_t i, filter_offset, result_offset, filter_length = input->numRows * input->numCols, result_length = result->numRows * result->numCols;
    int16_t *filterData = input->data, *resultData = result->data;

    for (i = numFilters; i > 0; i --){
        filter_offset = (i - 1) * filter_length;
        result_offset = (i - 1) * result_length;

        input->data = filterData + filter_offset;
        result->data = resultData + result_offset;

        /* process one filter at a time */
        maxpooling(result, input, pool_numRows, pool_numCols);
    }
    return result;
}

matrix *flatten(matrix* result, matrix *input, uint16_t num_filter){
    /**
     * Implementation of flatten layer for CNN
     * the result of conv2d_maxpooling or conv2d_filter is saved in the order of filter by filter
     * however, the flatten result should be in this following way according to Tensorflow
     * f0[0], f1[0], ..., fn[0], f0[1], f1[1], ..., fn[1], ..., f0[n], f1[n], fn[n]
     */
    uint16_t i, j, input_offset, result_offset = 0;
    uint16_t filter_length;
    filter_length = input->numCols * input->numRows;
    for (i = 0; i < filter_length; i ++ ){
        for (j = 0; j < num_filter; j ++){
            input_offset = i + j * filter_length;   // get the ith element of the jth filter
            result->data[result_offset++] = input->data[input_offset];   // append it to result
            result->data[result_offset++] = 0; // for LEA, we have to append 0 to each number
        }
    }
    return result;
}

matrix *padding_same(matrix *result, matrix *input, matrix *filter, uint16_t stride_numRows, uint16_t stride_numCols) {
    uint16_t input_numRows = input->numRows, input_numCols = input->numCols, filter_numRows = filter->numRows, filter_numCols = filter->numCols;
    uint16_t pad_along_numRows, pad_along_numCols, i, input_offset, padding_offset;
    if (input_numRows % stride_numRows) {
        pad_along_numRows = filter_numRows - input_numRows % stride_numRows;
    }
    else {
        pad_along_numRows = filter_numRows - stride_numRows;
    }
    if (input_numCols % stride_numCols) {
        pad_along_numCols = filter_numCols - input_numCols % stride_numCols;
    }
    else {
        pad_along_numCols = filter_numCols - stride_numCols;
    }

    result->numRows = input->numRows + pad_along_numRows;
    result->numCols = input->numCols + pad_along_numCols;

    memset(PADDING_BUFFER, 0, result->numRows * result->numCols * sizeof(dtype));

    for (i = 0; i < input_numRows; i ++) {
        input_offset = i * input_numCols;
        padding_offset = ((pad_along_numRows >> 1) + i) * result->numCols + (pad_along_numCols >> 1);
        memcpy(PADDING_BUFFER + padding_offset, input->data + input_offset, sizeof(dtype) * input_numCols);
    }

//    memcpy(result->data = PADDING_BUFFER, result->numRows * result->numCols * sizeof(dtype));
    result->data = PADDING_BUFFER;
    return result;
}

matrix *filter_simple(matrix *result, matrix *input, matrix *filter, uint16_t precision, uint16_t stride_numRows, uint16_t stride_numCols){
    /**
     * Implementation of one filter of a conv2d layer
     */
    uint16_t input_numRows = input->numRows;
    uint16_t input_numCols = input->numCols;
    uint16_t filter_numRows = filter->numRows;
    uint16_t filter_numCols = filter->numCols;
    uint16_t i, j, m, n, input_offset, filter_offset, result_offset = 0;
    int16_t mult_result, sum = 0, mult1, mult2;

    for (i = 0; i <= input_numRows - filter_numRows; i += stride_numRows){
        for (j = 0; j <= input_numCols - filter_numCols; j += stride_numCols){
            // (i,j) is the coordinate of the top-left element of the moving filter
            sum = 0;
            for (m = i; m < i + filter_numRows; m ++ ){
                for (n = j; n < j + filter_numCols; n ++){  // calculate element-wise matrix product between the filter and corresponding section in the input image
                    input_offset = m * input_numRows + n;
                    filter_offset = (m - i) * filter_numCols + (n - j);
                    mult1 = input->data[input_offset];
                    mult2 = filter->data[filter_offset];
                    mult_result = fp_mul(mult1, mult2, precision);
                    sum += mult_result;  // ATTENTION *** potential overflow issue ***
                }
            }
            result->data[result_offset ++] = sum;  // add bias
        }
    }
    return result;
}


matrix *filters_sum(matrix *result, matrix *input, matrix *filter, uint16_t numChannels, int16_t b, int16_t (*activation)(int16_t, uint16_t), uint16_t precision, uint16_t stride_numRows, uint16_t stride_numCols, uint16_t padding){
    int16_t *filter_head = filter->data;
    int16_t *input_head = input->data;
    uint16_t i, result_length = result->numRows * result->numCols, input_length = input->numRows * input->numCols, filter_length = filter->numRows * filter->numCols, input_numRows = input->numRows, input_numCols = input->numCols;
    matrix temp = {FILTER_BUFFER, result->numRows, result->numCols};
    memset(result->data, 0, result_length * sizeof(dtype));

    for (i = numChannels; i > 0; i --){
        input->data = input_head + input_length * (i - 1);
        filter->data = filter_head + filter_length * (i - 1);
        if (padding == 1) {
            padding_same(input, input, filter, stride_numRows, stride_numCols);
        }
        filter_LEA(&temp, input, filter, precision, stride_numRows, stride_numCols);
        matrix_add(result, result, &temp);
        input->numRows = input_numRows;
        input->numCols = input_numCols;
        input->data = input_head;
    }
    for (i = result_length; i > 0; i --){
        result->data[i - 1] = result->data[i - 1] + b;
    }

    result = apply_elementwise(result, result, activation, precision);
    return result;
}

matrix *conv2d(matrix *result, matrix *input, matrix *filter, uint16_t numFilters, uint16_t numChannels, int16_t *b, int16_t (*activation)(int16_t, uint16_t), uint16_t precision, uint16_t stride_numRows, uint16_t stride_numCols, uint16_t padding){
    uint16_t i, result_length = result->numRows * result->numCols, filter_length = filter->numRows * filter->numCols * numChannels;
    int16_t *filter_head = filter->data, *result_head = result->data;
    for (i = numFilters; i > 0; i --){
        filter->data = filter_head + (i - 1) * filter_length;
        result->data = result_head + (i - 1) * result_length;
        filters_sum(result, input, filter, numChannels, b[i - 1], activation, precision, stride_numRows, stride_numCols, padding);
    }
    return result;
}

matrix *apply_leakyrelu(matrix *result, matrix *input, uint16_t precision){
    result = apply_elementwise(result, input, &fp_leaky_relu, precision);
    return result;
}

matrix *dropout(matrix *result, matrix *input, int16_t rate, uint16_t precision){
    uint16_t i, input_length = input->numRows * input->numCols;
    for (i = input_length; i > 0; i --) {
        result->data[i] = fp_mul(input->data[i], rate, precision);
    }
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
