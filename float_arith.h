/*******************************************************************************
 * Copyright (c) 2012, 2014 Etienne Dublé, CNRS
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *  
 * or see the LICENSE file joined with this program.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *******************************************************************************/

#ifndef __FLOAT_ARITH_H
#define __FLOAT_ARITH_H

#include <stdint.h>
#define INT_SIZE 16

#if INT_SIZE == 32
#define S_INT int16_t
#define INT int32_t
#define L_INT int64_t
#else
#if INT_SIZE == 16
#define S_INT int8_t
#define INT int16_t
#define L_INT int32_t
#endif
#endif

typedef struct st_float {
	INT int_value;
	S_INT shifts;
} FLOAT;

#define ABS(f) (((f)>0)?(f):-(f))

void copy(FLOAT *a, FLOAT *b);
void multiply(FLOAT *a, FLOAT *b, FLOAT *result);
void multiply_by_int(FLOAT *a, INT i, FLOAT *result);
void divide(FLOAT *a, FLOAT *b, FLOAT *result);
void divide_by_int(FLOAT *a, INT i, FLOAT *result);
void add(FLOAT *a, FLOAT *b, FLOAT *result);
void negate(FLOAT *a, FLOAT *result);
void substract(FLOAT *a, FLOAT *b, FLOAT *result);
short is_greater(FLOAT *a, FLOAT *b);
void ln(FLOAT *f, FLOAT *result);
INT int_value(FLOAT *f);
void copy_int_to_float(INT i, FLOAT *result);
void print_float(FLOAT *f, S_INT dec_num);

#endif

