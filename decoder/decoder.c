#include "decoder.h"

#pragma PERSISTENT(MODEL_ARRAY_OUTPUT)
static int16_t MODEL_ARRAY_OUTPUT[512] = {0};
#pragma PERSISTENT(MODEL_ARRAY_TEMP)
static int16_t MODEL_ARRAY_TEMP[512] = {0};

matrix *apply_model(matrix *output, matrix *input){

    uint16_t i = 0;
    int16_t *array = MODEL_ARRAY;

    uint16_t array_length = MODEL_ARRAY_LENGTH;

    uint16_t layer_class, activation;
    uint16_t numFilters;
    output->data = MODEL_ARRAY_OUTPUT;

    i = 0;
    // Sequential model
    if (array[i] == 0){  // 1st element of the array tells the model type
        i ++;
        while (i != array_length){
            // next element of the array tells the layer class

            /* layer class 0 - DENSE */
            if (array[i] == 0){
                numFilters = 1;

                // extract and prepare layer parameters
                layer_class = array[i];
                activation = array[i+1];
                uint16_t kernel_numRows = array[i+2];
                uint16_t kernel_numCols = array[i+3];
                uint16_t bias_numRows = array[i+4];
                uint16_t bias_numCols = array[i+5];
                i += 6;
                uint16_t kernel_length = kernel_numRows * kernel_numCols;
                uint16_t bias_length = bias_numRows * bias_numCols;

                // extract layer weights
                int16_t *kernel_array = &array[i];
                i += kernel_length;
                int16_t *bias_array = &array[i];
                i += bias_length;

                // prepare output
                uint16_t output_numRows = kernel_numRows;
                uint16_t output_numCols = input->numCols;
                output->numRows = output_numRows;
                output->numCols = output_numCols;

                // initialize weight matrix
                matrix kernel = {kernel_array, kernel_numRows, kernel_numCols};
                matrix bias = {bias_array, bias_numRows, bias_numCols};

                // execute dense layer
                if (activation == 2){
                    dense(output, input, &kernel, &bias, &fp_relu, FIXED_POINT_PRECISION);
                }
                else{
                    dense(output, input, &kernel, &bias, &fp_linear, FIXED_POINT_PRECISION);
                }
            }

            /* layer class 1 - LeakyReLU */
            else if (array[i] == 1){
                output->numRows = input->numRows;
                output->numCols = input->numCols;
                apply_leakyrelu(output, input, FIXED_POINT_PRECISION);
                i ++;
            }

            /* layer class 2 - Conv2D */
            else if (array[i] == 2){

                // extract and prepare layer parameters
                layer_class = array[i];
                activation = array[i+1];
                numFilters = array[i+2];
                uint16_t numChannels = array[i+3];
                uint16_t filter_numRows = array[i+4];
                uint16_t filter_numCols = array[i+5];
                uint16_t stride_numRows = array[i+6];
                uint16_t stride_numCols = array[i+7];
                uint16_t filters_length = array[i+8];
                i += 9;

                // prepare outout
                output->numRows = (input->numRows - filter_numRows + 1) / stride_numRows;
                if ((input->numRows - filter_numRows + 1) % stride_numRows > 0){
                    output->numRows ++;
                }
                output->numCols = (input->numCols - filter_numCols + 1) / stride_numCols;
                if ((input->numCols - filter_numCols + 1) % stride_numCols > 0){
                    output->numCols ++;
                }

                // extract and prepare weights
                int16_t *filters_array = &(array[i]);
                matrix filters = {filters_array, filter_numRows, filter_numCols};
                i += filters_length;

                int16_t *bias_array = &(array[i]);
                i += numFilters;


                // execute conv2d layer
                if (activation == 2){
                    conv2d_multi_channel_multi_filter(output, input, &filters, numFilters, numChannels, bias_array, &fp_relu, FIXED_POINT_PRECISION, stride_numRows, stride_numCols);
                }
                else{
                    conv2d_multi_channel_multi_filter(output, input, &filters, numFilters, numChannels, bias_array, &fp_linear, FIXED_POINT_PRECISION, stride_numRows, stride_numCols);
                }

            }

            /* layer class 3 - MaxPooling2D */
            else if (array[i] == 3){
                uint16_t pool_numRows = array[i+1];
                uint16_t pool_numCols = array[i+2];
                uint16_t stride_numRows = array[i+3];
                uint16_t stride_numCols = array[i+4];
                i += 5;

                output->numRows = input->numRows / pool_numRows;
                output->numCols = input->numCols / pool_numCols;

                conv2d_maxpooling_multi_filter(output, input, numFilters, pool_numRows, pool_numCols);
            }

            /* layer class 4 - Conv2D Flatten */
            else if (array[i] == 4){
                i += 1;
                output->numRows = input->numRows * input->numCols * numFilters;
                output->numCols = 2;
                conv2d_flatten(output, input, numFilters);
            }

            // Hard copy output matrix and reference input to copied output
            memcpy(MODEL_ARRAY_TEMP, output->data, sizeof(output->data) * output->numRows * output->numCols * numFilters);
            input->data = MODEL_ARRAY_TEMP;
            input->numRows = output->numRows;
            input->numCols = output->numCols;
        }

    }

    return output;
}

