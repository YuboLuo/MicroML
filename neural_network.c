#include "neural_network.h"

// Buffer for intermediate states
static dtype DATA_BUFFER[7 * STATE_SIZE * VECTOR_COLS] = {0};
static dtype STATE_BUFFER[3 * STATE_SIZE * VECTOR_COLS + NUM_OUTPUT_FEATURES * VECTOR_COLS] = {0};  // currentState, nextState, logProbs
static dtype INPUT_BUFFER[NUM_INPUT_FEATURES * VECTOR_COLS] = {0};



matrix *apply_transformation(matrix *result, matrix *input, matrix *state, uint16_t precision) {
    /**
     * Applies both the embedding layer and the transformation to the input and previous state.
     */
    uint16_t data_buffer_offset = 0;

    // Allocate intermediate state for the embedding
    matrix embedding = { DATA_BUFFER + data_buffer_offset, state->numRows, state->numCols };
    data_buffer_offset += embedding.numRows * embedding.numCols;

    // Apply the embedding layer
    dense(&embedding, input, EMBEDDING_LAYER_KERNEL_MAT, EMBEDDING_LAYER_BIAS_MAT, &fp_leaky_relu, precision);

//    // Apply the specified transformation layer
//    #ifdef GRU_TRANSFORM
//        // Allocate temporary states
//        matrix stacked = { DATA_BUFFER + data_buffer_offset, 2 * state->numRows, state->numCols };
//        data_buffer_offset += stacked.numRows * stacked.numCols;
//
//        matrix gates = { DATA_BUFFER + data_buffer_offset, 2 * state->numRows, state->numCols };
//        data_buffer_offset += gates.numRows * gates.numCols;
//
//        matrix candidate = { DATA_BUFFER + data_buffer_offset, state->numRows, state->numCols };
//        data_buffer_offset += candidate.numRows * candidate.numCols;
//
//        matrix gateTemp = { DATA_BUFFER + data_buffer_offset, state->numRows, state->numCols };
//        data_buffer_offset += gateTemp.numRows * gateTemp.numCols;
//
//        GRUTempStates rnnTemp;
//        rnnTemp.stacked = &stacked;
//        rnnTemp.gates = &gates;
//        rnnTemp.candidate = &candidate;
//        rnnTemp.gateTemp = &gateTemp;
//
//        /// Create the GRU Cell
//	    GRU rnn_cell = { RNN_CELL_W_GATES_MAT, RNN_CELL_B_GATES_MAT, RNN_CELL_W_CANDIATE_MAT, RNN_CELL_B_CANDIDATE_MAT };
//
//        // Apply the GRU Cell
//        apply_gru(result, &embedding, state, &rnn_cell, &rnnTemp, precision);
//    #elif defined(UGRNN_TRANSFORM)
        // Allocate temporary states
        matrix stacked = { DATA_BUFFER + data_buffer_offset, 2 * state->numRows, state->numCols };
        data_buffer_offset += stacked.numRows * stacked.numCols;

        matrix transformed = { DATA_BUFFER + data_buffer_offset, 2 * state->numRows, state->numCols };
        data_buffer_offset += transformed.numRows * transformed.numCols;

        matrix gateTemp = { DATA_BUFFER + data_buffer_offset, state->numRows, state->numCols };
        data_buffer_offset += gateTemp.numRows * gateTemp.numCols;

        UGRNNTempStates rnnTemp;
        rnnTemp.stacked = &stacked;
        rnnTemp.transformed = &transformed;
        rnnTemp.gateTemp = &gateTemp;

        /// Create the UGRNN Cell
	    UGRNN rnn_cell = { RNN_CELL_W_TRANSFORM_MAT, RNN_CELL_B_TRANSFORM_MAT };

        // Apply the GRU Cell
        apply_ugrnn(result, &embedding, state, &rnn_cell, &rnnTemp, precision);
//    #endif

    return result;
}


// Function to compute output (1 or 2 hidden layer depending on model type)
matrix *compute_logits(matrix *result, matrix *input, uint16_t precision) {
    /**
     * Function to compute the prediction using a feed-forward network.
     */
    // Allocate intermediate states
    uint16_t data_buffer_offset = 0;
    matrix hidden = { DATA_BUFFER + data_buffer_offset, OUTPUT_LAYER_HIDDEN_0_BIAS_MAT->numRows, input->numCols };
    data_buffer_offset += hidden.numRows * hidden.numCols;

    matrix hidden1 = { DATA_BUFFER + data_buffer_offset, OUTPUT_LAYER_HIDDEN_1_BIAS_MAT->numRows, input->numCols };


    // Apply the dense layers
    dense(&hidden, input, OUTPUT_LAYER_HIDDEN_0_KERNEL_MAT, OUTPUT_LAYER_HIDDEN_0_BIAS_MAT, &fp_leaky_relu, precision);
    dense(&hidden1, &hidden, OUTPUT_LAYER_HIDDEN_1_KERNEL_MAT, OUTPUT_LAYER_HIDDEN_1_BIAS_MAT, &fp_leaky_relu, precision);  // added by Yubo
    dense(result, &hidden1, OUTPUT_LAYER_OUTPUT_KERNEL_MAT, OUTPUT_LAYER_OUTPUT_BIAS_MAT, &fp_linear, precision);

    return result;
}



