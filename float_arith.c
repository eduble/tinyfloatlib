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

#include <stdio.h>
#include "float_arith.h"
//#define DEBUG 

#ifdef DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

static L_INT tmp_int_value;
static S_INT tmp_shifts;

static FLOAT float_1 = { 1<<(INT_SIZE-2), -(INT_SIZE-2) };
static FLOAT float_0 = { 0, 0 };
static FLOAT powers_of_e[] = {	{ 1<<(INT_SIZE-2), -(INT_SIZE-2+8) } /* 1/256 */,
				{ 1<<(INT_SIZE-2), -(INT_SIZE-2+6) } /* 1/64 */,
				{ 1<<(INT_SIZE-2), -(INT_SIZE-2+4) } /* 1/16 */,
				{ 1<<(INT_SIZE-2), -(INT_SIZE-2+2) } /* 1/4 */,
				{ 1<<(INT_SIZE-2), -(INT_SIZE-2+0) } /* 1 */,
				{ 1<<(INT_SIZE-2), -(INT_SIZE-2-2) } /* 4 */
				};
static FLOAT powers_of_e_values[] = { 
#if INT_SIZE == 16
	{ 16448, -14 }, /* e^(1/256) */
	{ 16642, -14 }, /* e^(1/64) */
	{ 17440, -14 }, /* e^(1/16) */
	{ 21037, -14 }, /* e^(1/4) */
	{ 22268, -13 }, /* e^(1) */
	{ 27954, -9 }, /* e^(4) */
#else
#if INT_SIZE == 32
	{ 1077944320, -30 }, /* e^(1/256) */
	{ 1090650752, -30 }, /* e^(1/64) */
	{ 1142992256, -30 }, /* e^(1/16) */
	{ 1378711808, -30 }, /* e^(1/4) */
	{ 1459366400, -29 }, /* e^(1) */
	{ 1832009856, -25 }  /* e^(4) */
#endif
#endif
};

static const int NUM_POWERS_OF_E = sizeof(powers_of_e) / sizeof(FLOAT);

#define FLOAT_0 (&float_0)
#define FLOAT_1 (&float_1)
#define SIGN(pf) (((pf)->int_value > 0)?1:(((pf)->int_value < 0)?-1:0)) 
#define IS_ALMOST_ZERO(pf) (((pf)->int_value == 0)||((pf)->shifts < -(2*INT_SIZE)))
#define IS_EVEN(i) ((i)%2==0)

void copy(FLOAT *a, FLOAT *b)
{
	b->shifts = a->shifts;
	b->int_value = a->int_value;
}

void normalize(FLOAT *a)
{
	if (a->int_value == 0)
	{
		a->shifts = 0;
	}
	else
	{
		while (ABS(a->int_value) < (1<<(INT_SIZE-2)))
		{
			a->int_value <<= 1;
			a->shifts--;
		}
	}
}

void normalize_tmp_and_copy_to(FLOAT *result)
{
	if (tmp_int_value == 0)
	{
		tmp_shifts = 0;
	}
	else
	{
		while (ABS(tmp_int_value) >= ((L_INT)1<<(INT_SIZE-1)))
		{
			tmp_int_value >>= 1;
			tmp_shifts++;
		}

		while (ABS(tmp_int_value) < ((L_INT)1<<(INT_SIZE-2)))
		{
			tmp_int_value <<= 1;
			tmp_shifts--;
		}
	}

	result->int_value = tmp_int_value;
	result->shifts = tmp_shifts;
}

void multiply(FLOAT *a, FLOAT *b, FLOAT *result)
{
	tmp_int_value = ((L_INT)a->int_value * (L_INT)b->int_value) >> (INT_SIZE-2);
	tmp_shifts = a->shifts + b->shifts + (INT_SIZE-2);
	normalize_tmp_and_copy_to(result);
}

void multiply_by_int(FLOAT *a, INT i, FLOAT *result)
{
	tmp_int_value = ((L_INT)a->int_value * i);
	tmp_shifts = a->shifts;
	normalize_tmp_and_copy_to(result);
}

void divide(FLOAT *a, FLOAT *b, FLOAT *result)
{
	tmp_int_value = ((L_INT)a->int_value) << INT_SIZE;
	tmp_int_value /= ((L_INT)b->int_value);
	tmp_shifts = a->shifts - b->shifts - INT_SIZE;
	normalize_tmp_and_copy_to(result);
}

void divide_by_int(FLOAT *a, INT i, FLOAT *result)
{
	tmp_int_value = ((L_INT)a->int_value) << INT_SIZE;
	tmp_int_value /= i;
	tmp_shifts = a->shifts - INT_SIZE;
	normalize_tmp_and_copy_to(result);
}

void add(FLOAT *a, FLOAT *b, FLOAT *result)
{
	if (a->int_value == 0)
	{
		copy(b, result);
	}
	else if (b->int_value == 0)
	{
		copy(a, result);
	}
	else if (b->shifts > a->shifts)
	{
		add(b, a, result);
	}
	else	/* a->shifts >= b->shifts */
	{
		tmp_int_value = ((L_INT)a->int_value) << (INT_SIZE-1);
		tmp_int_value += ((L_INT)b->int_value) << ((INT_SIZE-1) + b->shifts - a->shifts);
		tmp_shifts = a->shifts - (INT_SIZE-1);
		normalize_tmp_and_copy_to(result);
	}
}

void negate(FLOAT *a, FLOAT *result)
{
	result->int_value = -(a->int_value);
	result->shifts = a->shifts;
}

void substract(FLOAT *a, FLOAT *b, FLOAT *result)
{
	FLOAT negated_b;
	negate(b, &negated_b);
	add(a, &negated_b, result);
}

short is_greater(FLOAT *a, FLOAT *b)
{
	if (SIGN(a) > SIGN(b))
		return 1;
	else if (SIGN(b) > SIGN(a))
		return 0;
	else {
		// same sign
		if (a->shifts > b->shifts)
			return SIGN(a) > 0;
		else if (b->shifts > a->shifts)
			return SIGN(a) < 0;
		else {	// same shifts
			return (a->int_value > b->int_value);
		}	
	}	
}


void ln(FLOAT *f, FLOAT *result)
{
	FLOAT x, x_power_i, inc, tmp_f, tmp_result;
	short i;
	PRINTF("calculating ln(%f).\n", printed_float(f));

	// we will calculate either ln(f)
	// or -ln(1/f), which are equivalent.
	// if f > 1, the following would not converge.
	if (is_greater(f, FLOAT_1))
	{
		PRINTF("f -> 1/f optimization.\n");
		divide(FLOAT_1, f, f);
		ln(f, &tmp_result);
		negate(&tmp_result, result);
		return;
	}

	// at this step 0 < f <= 1 
	// we want f to be as close to 1 as possible
	// to improve the convergence times of the 
	// following processings.
	//
	// so we will apply several times the following:
	// log(f) = log(f*e^a) - a
	// with f*e^a closer to 1 than f.
	// 
	// e^a and a are given in the tables
	// powers_of_e_values and powers_of_e above.
	copy(FLOAT_0, &tmp_result);
	for (i = NUM_POWERS_OF_E-1; i >= 0; i--)
	{
		while (1)
		{
			multiply(f, &powers_of_e_values[i], &tmp_f);
			if (!is_greater(FLOAT_1, &tmp_f))
			 	break;

			PRINTF("f -> f*%f optimization.\n", printed_float(&powers_of_e_values[i]));
			substract(&tmp_result, &powers_of_e[i], &tmp_result);
			copy(&tmp_f, f);
		}
	}

	// finally we apply the following formula:
	// ln(1 + x) = x - x2/2 + x3/3 - x4/4 + x5/5
	// f = 1+x should be very close to 1 at this step.
	// Therefore x should be very close to 0
	// and the formula should converge fast enough.
	PRINTF("f = %f before the final loop.\n", printed_float(f)); // should be close to 1
	substract(f, FLOAT_1, &x);
	add(&tmp_result, &x, &tmp_result);
	
	multiply(&x, &x, &x_power_i); // with i=2, see below
	for (i = 2; ; i++)
	{
		divide_by_int(&x_power_i, i, &inc);
		if (IS_EVEN(i))
			negate(&inc, &inc);
		if (IS_ALMOST_ZERO(&inc))
			break;
		add(&tmp_result, &inc, &tmp_result);

		multiply(&x_power_i, &x, &x_power_i); // for next loop
	}
	PRINTF("The loop converged after %i iterations.\n", i-2);
	copy(&tmp_result, result);
}

INT int_value(FLOAT *f)
{
	INT result;
	if (f->shifts > 0)
	{
		result = (f->int_value)<<(f->shifts);
	}
	else
	{
		if (-(f->shifts) > (INT_SIZE-1))
		{
			result = 0;
		}
		else
		{
			result = (f->int_value)>>(-(f->shifts));
		}
	}
	return result;
}

void print_float(FLOAT *f, S_INT dec_num)
{
	FLOAT cur, cur2, f10, fdigit;
	INT digit;
	S_INT dec_index = 0;
	
	if (SIGN(f) < 0)
	{
		putchar('-');
		negate(f, &cur);
	}
	else
	{
		copy(f, &cur);
	}

	copy_int_to_float(10, &f10);

	copy(FLOAT_1, &fdigit);
	while (is_greater(&cur, &fdigit))
		multiply(&fdigit, &f10, &fdigit);

	while (1)
	{
		divide(&fdigit, &f10, &fdigit);
		if (dec_index == 0)
		{
		     	if (is_greater(FLOAT_1, &fdigit))
			{
				putchar('.');
				dec_index++;
			}
		}
		else
		{
			dec_index++;
		}
		divide(&cur, &fdigit, &cur2);
		digit = int_value(&cur2);
		putchar('0' + digit);
		if (dec_index == dec_num)
		{
			break;
		}
		multiply_by_int(&fdigit, digit, &cur2);
		substract(&cur, &cur2, &cur);
	}
}

void copy_int_to_float(INT i, FLOAT *result)
{
	result->int_value = i;
	result->shifts = 0;
	normalize(result);
}


