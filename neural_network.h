#include <stdint.h>
#include "math/matrix.h"
#include "layers/cells.h"
#include "layers/layers.h"
#include "utils/utils.h"
#include "math/matrix_ops.h"
#include "math/fixed_point_ops.h"
#include "neural_network_parameters.h"

#ifndef NEURAL_NETWORK_GUARD
#define NEURAL_NETWORK_GUARD


matrix *apply_transformation(matrix *result, matrix *input, matrix *state, uint16_t precision);
matrix *compute_logits(matrix *result, matrix *input, uint16_t precision);


#endif
