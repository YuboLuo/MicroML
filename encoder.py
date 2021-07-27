import tensorflow as tf
from tensorflow.keras import datasets, layers, models, Input, activations
from tensorflow.keras import initializers
from tensorflow import keras
from fxpmath import Fxp
import numpy as np
import math

### Call export_model(<keras.model>) to export model into csv file

def encode_model(model) -> int:
    if isinstance(model, models.Sequential): return 0
    return -1

def encode_layer(layer) -> int:
    if isinstance(layer, layers.Dense): return 0
    if isinstance(layer, layers.LeakyReLU): return 1
    if isinstance(layer, layers.Conv2D): return 2
    if isinstance(layer, layers.MaxPooling2D): return 3
    if isinstance(layer, layers.Flatten): return 4
    if isinstance(layer, layers.Dropout): return 5
    return -1

def encode_activation(layer) -> int:
    activation_name = layer.activation.__name__
    map_activation = {"linear":0, "sigmoid":1, "relu":2, "tanh":3}
    return map_activation[activation_name]

def encode_dense_data(layer) -> list:
    data = []
    data.append(encode_activation(layer))
    fixed_point_arr = []
    weight = layer.get_weights()[0]
    if len(weight.shape) == 1:
        data.append(weight.shape[0])
        data.append(1)
    else:
        weight = weight.T
        data.append(weight.shape[0])
        data.append(weight.shape[1])
    
    bias = layer.get_weights()[1]
    if len(bias.shape) == 1:
        data.append(bias.shape[0])
        data.append(1)
    else:
        bias = bias.transpose(1, 0)
        data.append(bias.shape[0])
        data.append(bias.shape[1])
    
    fixed_point_weight = Fxp(weight, signed = True, n_word = 16, n_frac = 10).val.flatten().tolist()
    fixed_point_bias = Fxp(bias, signed = True, n_word = 16, n_frac = 10).val.flatten().tolist()
    data = data + fixed_point_weight + fixed_point_bias
    
    return data

def encode_padding(layer) -> int:
    if layer.padding == "valid":
        return 0
    if layer.padding == "same":
        return 1
    if layer.padding == "full":
        return 2
    return -1
    

def encode_conv2d_data(layer) -> list:
    data = []
    data.append(encode_activation(layer))       # activation function
    data.append(layer.filters)                  # num of filters
    if layer.data_format == "channels_last":    # num of channels
        data.append(layer.input_shape[3])
    else:
        data.append(layer.input_shape[0])
    data.append(layer.kernel_size[0])           # filter rows
    data.append(layer.kernel_size[1])           # filter cols
    
    data.append(layer.strides[0])               # stride rows
    data.append(layer.strides[1])               # stride cols
    data.append(layer.kernel_size[0] * layer.kernel_size[1] * layer.input_shape[3] * layer.filters)

    data.append(encode_padding(layer))          # padding

    weight = layer.get_weights()[0]
    weight = weight.transpose(3, 2, 0, 1)
    fixed_point_weight = Fxp(weight, signed = True, n_word = 16, n_frac = 10).val.flatten().tolist()

    bias = layer.get_weights()[1]
    fixed_point_bias = Fxp(bias, signed = True, n_word = 16, n_frac = 10).val.flatten().tolist()

    data = data + fixed_point_weight + fixed_point_bias

    return data


def encode_maxpooling2d_data(layer) -> list:
    data = []
    data.append(layer.pool_size[0])
    data.append(layer.pool_size[1])
    data.append(layer.strides[0])
    data.append(layer.strides[1])
    data.append(encode_padding(layer))          # padding
    return data


def encode(model) -> list:
    encode_list = []
    if encode_model(model) == -1:
        return encode_list
    encode_list.append(encode_model(model))

    if encode_model(model) == 0:
        for layer in model.layers:
            if encode_layer(layer) == -1:
                return encode_list
            encode_list.append(encode_layer(layer))
            if encode_layer(layer) == 0:
                encode_list = encode_list + encode_dense_data(layer)
            elif encode_layer(layer) == 1:
                pass
            elif encode_layer(layer) == 2:
                encode_list = encode_list + encode_conv2d_data(layer)
            elif encode_layer(layer) == 3:
                encode_list = encode_list + encode_maxpooling2d_data(layer)
            elif encode_layer(layer) == 4:
                pass
    return encode_list

def export_model(model):
    count = 0
    output_str = ""
    for element in encode(model):
        output_str = output_str + f"{element},"
        count += 1
    f = open("model.csv", "w")
    f.write(output_str)
    f.close()
    print(count)
