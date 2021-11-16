/*
* Imaginet Compiler 2.7.250
*
* MIT License
* Copyright © 2020- AFarCloud consortium,, Imagimob AB, Sam Al-Attiyah, Jens Lordén, Songyi Ma.
* 
* Generated at 6/24/2021 11:29:14 AM UTC. Any changes will be lost.
* 
* Memory    Size                      Efficiency
* Buffers   15680 bytes (RAM)         86%
* State     5024 bytes (RAM)          100%
* Readonly  35664 bytes (Flash)       100%
* 
* Layer                          Shape           Type       Function
* Vector Magnitude               [2]             float      enqueue
* Sliding Window                 [280,2]         float      dequeue
*    window_shape = [280,2]
*    stride = 100
*    buffer_multiplier = 1
* Batch Normalization            [280,2]         float      dequeue
*    epsilon = 0.001
*    gamma = float[2]
*    beta = float[2]
*    mean = float[2]
*    variance = float[2]
* Convolution 1D                 [280,6]         float      dequeue
*    filters = 6
*    kernel_size = 3
*    strides = 1
*    padding = same
*    activation = linear
*    use_bias = False
*    weight = float[3,2,6]
* Batch Normalization            [280,6]         float      dequeue
*    epsilon = 0.001
*    gamma = float[6]
*    beta = float[6]
*    mean = float[6]
*    variance = float[6]
* Activation                     [280,6]         float      dequeue
*    activation = relu
* Max pooling 1D                 [93,6]          float      dequeue
*    pool_size = 3
*    strides = 3
*    padding = valid
* Long Short-Term Memory         [1,42]          float      dequeue
*    units = 42
*    activation = tanh
*    recurrent_activation = sigmoid
*    use_bias = True
*    return_sequences = False
*    return_state = False
*    stateful = False
*    go_backwards = False
*    kernel = float[6,168]
*    recurrent_kernel = float[42,168]
*    bias = float[168]
* Dropout                        [1,42]          float      dequeue
*    rate = 0.1
* Dense                          [1,13]          float      dequeue
*    units = 13
*    use_bias = False
*    activation = linear
*    weight = float[42,13]
* Batch Normalization            [1,13]          float      dequeue
*    epsilon = 0.001
*    gamma = float[13]
*    beta = float[13]
*    mean = float[13]
*    variance = float[13]
* Activation                     [1,13]          float      dequeue
*    activation = relu
* Dense                          [1,4]           float      dequeue
*    units = 4
*    use_bias = False
*    activation = linear
*    weight = float[13,4]
* Batch Normalization            [1,4]           float      dequeue
*    epsilon = 0.001
*    gamma = float[4]
*    beta = float[4]
*    mean = float[4]
*    variance = float[4]
* Activation                     [1,4]           float      dequeue
*    activation = softmax
* 
* Exported functions:
* 
* int IMAI_dequeue(float * data_out, float * time_out)
*    Description: Dequeue features. RET_SUCCESS (0) on success, RET_NODATA (-1) if no data is available, RET_NOMEM (-2) on internal memory error
*    Parameter data_out is Output of size float[1,4].
*    Parameter time_out is Output of size float[2].
* 
* int IMAI_enqueue(const float * data_in, const float * time_in)
*    Description: Enqueue features. Returns SUCCESS (0) on success, else RET_NOMEM (-2) when low on memory.
*    Parameter data_in is Input of size float[6].
*    Parameter time_in is Input of size float[1].
* 
* void IMAI_init(void)
*    Description: Initializes buffers to initial state. This function also works as a reset function.
* 
*/

#ifndef _IMAI_MODEL12_WINDOW280STRIDE100_H_
#define _IMAI_MODEL12_WINDOW280STRIDE100_H_
#ifdef _MSC_VER
#pragma once
#endif

#define IMAI_API_QUEUE_TIME

#define IMAGINET_TYPES_NONE	(0x00000000)
#define IMAGINET_TYPES_FLOAT16	(0x00000012)
#define IMAGINET_TYPES_FLOAT32	(0x00000014)
#define IMAGINET_TYPES_FLOAT64	(0x00000018)
#define IMAGINET_TYPES_INT8	(0x00000021)
#define IMAGINET_TYPES_INT16	(0x00000022)
#define IMAGINET_TYPES_INT32	(0x00000024)
#define IMAGINET_TYPES_INT64	(0x00000028)

// DATA_IN [6] (24 bytes)
#define IMAI_DATA_IN_COUNT (6)
#define IMAI_DATA_IN_TYPE float
#define IMAI_DATA_IN_TYPE_ID IMAGINET_TYPES_FLOAT32

// TIME_IN [1] (4 bytes)
#define IMAI_TIME_IN_COUNT (1)
#define IMAI_TIME_IN_TYPE float
#define IMAI_TIME_IN_TYPE_ID IMAGINET_TYPES_FLOAT32

// DATA_OUT [1,4] (16 bytes)
#define IMAI_DATA_OUT_COUNT (4)
#define IMAI_DATA_OUT_TYPE float
#define IMAI_DATA_OUT_TYPE_ID IMAGINET_TYPES_FLOAT32

// TIME_OUT [2] (8 bytes)
#define IMAI_TIME_OUT_COUNT (2)
#define IMAI_TIME_OUT_TYPE float
#define IMAI_TIME_OUT_TYPE_ID IMAGINET_TYPES_FLOAT32

#define IMAI_RET_SUCCESS 0
#define IMAI_RET_NODATA -1
#define IMAI_RET_NOMEM -2

// Exported methods
int IMAI_dequeue(float * data_out, float * time_out);
int IMAI_enqueue(const float * data_in, const float * time_in);
void IMAI_init(void);

#endif /* _IMAI_MODEL12_WINDOW280STRIDE100_H_ */
