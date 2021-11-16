/*
* Imaginet Compiler 2.5.74
*
* MIT License
* Copyright © 2020- AFarCloud consortium,, Imagimob AB, Sam Al-Attiyah, Jens Lordén, Songyi Ma.
* 
* Generated at 21/01/2021 8:34:02 AM UTC. Any changes will be lost.
* 
* Memory    Size                      Efficiency
* Buffers   9776 bytes (RAM)          95%
* State     1992 bytes (RAM)          100%
* Readonly  74144 bytes (Flash)       100%
* 
* Layer                          Shape           Type       Function
* Vector Magnitude               [2]             float      enqueue
* Sliding Window                 [100,2]         float      dequeue
* Batch Normalization            [100,2]         float      dequeue
* Max pooling 1D                 [33,2]          float      dequeue
* Long Short-Term Memory         [33,1,44]       float      dequeue
* Long Short-Term Memory         [33,1,1,22]     float      dequeue
* Long Short-Term Memory         [1,22]          float      dequeue
* Dropout                        [1,22]          float      dequeue
* Dense                          [1,12]          float      dequeue
* Batch Normalization            [1,12]          float      dequeue
* Activation                     [1,12]          float      dequeue
* Dense                          [1,6]           float      dequeue
* Batch Normalization            [1,6]           float      dequeue
* Activation                     [1,6]           float      dequeue
* Dense                          [1,4]           float      dequeue
* Batch Normalization            [1,4]           float      dequeue
* Activation                     [1,4]           float      dequeue
* 
* Exported functions:
* 
* void IMAI_init(void)
*    Description: Initializes buffers to initial state. This function also works as a reset function.
* 
* int IMAI_enqueue(const float *restrict data_in, const float *restrict time_in)
*    Description: Enqueue features. Returns SUCCESS (0) on success, else RET_NOMEM (-2) when low on memory.
*    Parameter data_in is Input of size float[6].
*    Parameter time_in is Input of size float[1].
* 
* int IMAI_dequeue(float *restrict data_out, float *restrict time_out)
*    Description: Dequeue features. RET_SUCCESS (0) on success, RET_NODATA (-1) if no data is available, RET_NOMEM (-2) on internal memory error
*    Parameter data_out is Output of size float[1,4].
*    Parameter time_out is Output of size float[100].
* 
*/

#ifndef _IMAI_MODEL13_H_
#define _IMAI_MODEL13_H_
#ifdef _MSC_VER
#pragma once
#endif

#define IMAI_API_QUEUE

// DATA_IN [6] (24 bytes)
#define IMAI_DATA_IN_COUNT (6)
#define IMAI_DATA_IN_TYPE float

// TIME_IN [1] (4 bytes)
#define IMAI_TIME_IN_COUNT (1)
#define IMAI_TIME_IN_TYPE float

// DATA_OUT [1,4] (16 bytes)
#define IMAI_DATA_OUT_COUNT (4)
#define IMAI_DATA_OUT_TYPE float

// TIME_OUT [100] (400 bytes)
#define IMAI_TIME_OUT_COUNT (100)
#define IMAI_TIME_OUT_TYPE float

#define IMAI_RET_SUCCESS 0
#define IMAI_RET_NODATA -1
#define IMAI_RET_NOMEM -2

// Exported methods
void IMAI_init(void);
int IMAI_enqueue(const float *data_in, const float *time_in);
int IMAI_dequeue(float *data_out, float *time_out);

#endif /* _IMAI_MODEL13_H_ */
