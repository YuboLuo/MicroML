/**
 * Library to interact with an HM-10 Bluetooth Module.
 */
#include "main.h"

#define MAX_STEPS 100
//#define BUDGET 9  // Energy Budget in mW. This is Total Budget / (sample rate * total budget * seq_length)
#define MILLI_FACTOR 1000
#define SAMPLE_RATE 2  // Sample period (1 / freq)

// Variable to track ADC conversion
//volatile int16_t nAdc;

//// Variable to track the inference thresholds
//static int16_t thresholds[NUM_OUTPUTS];

//// Variables to track energy consumption
//volatile int32_t prevEnergy = 0;  // Previous energy in Joules (fixed-point)
//volatile int32_t currentEnergy = 0;  // Current energy in Joules (fixed-point)
//volatile int32_t avgPower = 0;  // Average power (fixed-point)
//volatile int32_t totalEnergy = 0;  // Total consumed energy (fixed-point)
//volatile int32_t stepEnergy = 0;  // Energy in the current step (fixed-point)
//volatile int32_t stepPower = 0; // Power in mW over the last interval (fixed-point)
//volatile int16_t steps = 0;

// Allocate memory for input features
//static dtype inputData[NUM_INPUT_FEATURES * VECTOR_COLS];
static matrix inputFeatures, outputLabels;

//// Allocate memory for logits
//#pragma PERSISTENT(logitsData);
//static dtype logitsData[SEQ_LENGTH * NUM_OUTPUT_FEATURES * VECTOR_COLS] = {0};
//static matrix logits[SEQ_LENGTH];

//// Allocate memory for hidden states
//#pragma PERSISTENT(statesData);
//static dtype statesData[SEQ_LENGTH * STATE_SIZE * VECTOR_COLS] = {0};
//static matrix states[SEQ_LENGTH];

// Allocate memory for the response buffer
//static char response[5];

//// Variables for tracking the position in sequence, feature and inference
//volatile uint16_t featureIdx = 0;
//volatile uint16_t seqIdx = 0; // 1; changed by Yubo
//volatile int8_t sign = 1;
//volatile ExecutionState execState;

//void init_exec_state(void);

static dtype array[784] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,44,76,381,574,574,574,574,574,574,574,574,132,0,0,0,0,0,0,0,0,0,0,0,0,0,0,361,598,835,1015,1015,1015,1015,1015,1015,1015,1015,1015,1015,871,88,0,0,0,0,0,0,0,0,0,0,0,0,0,1024,1015,1007,891,891,646,562,397,397,397,397,574,1015,1015,453,0,0,0,0,0,0,0,0,0,0,0,0,0,871,843,345,0,0,0,0,0,0,0,116,646,1015,1015,329,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,132,815,1015,1015,698,52,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,313,931,1015,1015,1015,172,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,554,1015,1015,1015,1015,1015,771,722,722,514,128,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,698,1015,1015,1015,1015,1015,1015,1015,1015,1015,678,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,168,244,244,244,244,333,706,317,441,991,1015,783,16,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,236,771,120,0,0,200,815,1015,168,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,224,783,228,0,0,0,0,449,1015,919,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,690,819,68,0,0,0,0,232,995,1015,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,771,1015,738,248,0,0,341,658,1015,1015,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,578,1015,1015,975,895,895,1003,1015,1015,385,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,610,1015,1015,1015,1015,1015,1015,610,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,24,72,489,566,566,566,349,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
static dtype input_buffer3[100] = {0};
int16_t label;


void main(void){

    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    // Initialize GPIO System
    init_gpio();

    // Initialize the clock and baudrate
    init_clock_system();

    // Assign parameters of the input features matrix
    inputFeatures.numRows = 28;
    inputFeatures.numCols = 28;
    inputFeatures.data = array;

    outputLabels.numRows = 10;
    outputLabels.numCols = 2;
    outputLabels.data = input_buffer3;

    apply_model(&outputLabels, &inputFeatures);
    label = argmax(&outputLabels);
    __no_operation();



    /* CNN implementation */
    /* model.summary()
    _________________________________________________________________
    Layer (type)                 Output Shape              Param #
    =================================================================
    conv2d (Conv2D)              (None, 10, 10, 4)         328
    _________________________________________________________________
    max_pooling2d (MaxPooling2D) (None, 5, 5, 4)           0
    _________________________________________________________________
    flatten (Flatten)            (None, 100)               0
    _________________________________________________________________
    dense (Dense)                (None, 16)                1616
    _________________________________________________________________
    dense_1 (Dense)              (None, 10)                170
    =================================================================
    Total params: 2,114
    Trainable params: 2,114
    Non-trainable params: 0
    _________________________________________________________________
    */



}



