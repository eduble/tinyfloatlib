# tinyfloatlib
A tiny library for floating point calculations on highly constrained processors.

## Introduction
This page describes a tiny library allowing to handle floating point calculations on highly constrained processors. 
It has a low flash and RAM footprint.
It defines a structure called "FLOAT" representing a `float` number, and some operations on this type (see the file `float_arith.h` for a list of these operations).
It is defined in 2 files, `float_arith.h` and `float_arith.c`.

## Usage example
Let's imagine we have to calculate `ln(12.536)`:
```c
#include <stdio.h>
#include "float_arith.h"

int main(int argc, char **argv)
{
	FLOAT f, f1000;
	copy_int_to_float(1000, &f1000);

	copy_int_to_float(12536, &f);		// f = 12536

	// f/f1000 -> f
	divide(&f, &f1000, &f);			// f = 12.536
	// ln(f) -> f;
	ln(&f, &f);				// f = ln(12.536)	

	print_float(&f, 4);
	putchar('\n');
}
```
If you store this code in a file called `test_ln.c`, you could compile this example by:
```
 $ gcc test_ln.c float_arith.c -o test_ln
```
and run it: 
```
 $ ./test_ln 
 2.5289
 $
```
## Customizations
### Integer size for computations
The library can handle 2 different integer sizes: 
* 16bits (default: lower memory footprint)
* 32bits (better precision)

You can compile the library in 32bits-mode by modifying the INT_SIZE value in `float_arith.h`.

### Disabling functions
If the target cannot handle the library in full, you could remove or comment the implementation of some functions, if you don't use them. 
Probably you will start with the following functions, because they require more resources than the others:  
* print_float()    (1)
* ln()

(1): The other way to display FLOAT values is to use the `int_value()` function ; it will only display the integer part, but if you need more precision just multiply by 100 or 1000 before...

