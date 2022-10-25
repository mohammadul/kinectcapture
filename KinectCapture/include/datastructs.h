/* ---------------------------------------------------------------------------
** This software is furnished "as is", without technical support,
** and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** datastructs.h
** Contains header declarations and definitions for datastructs.c.
**
** Author: Sk. Mohammadul Haque
** Copyright (c) 2014 Sk. Mohammadul Haque
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
** Unless required by applicable law or agreed to in writing, software distributed
** under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
** CONDITIONS OF ANY KIND, either express or implied. See the License for the
** specific language governing permissions and limitations under the License.
**
** For more details and updates, visit http://mohammadulhaque.alotspace.com
** -------------------------------------------------------------------------*/

#ifndef __DATA_STRUCTS__
#define __DATA_STRUCTS__

#include "commontypes.h"
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#define STACK_MAX 128

#define STACK_MALLOC 0
#define STACK_EMPTY 1

typedef struct dtype
{
    int first;
    int second;
} dtype;
typedef dtype* DTYPE;

typedef struct dtype_stack
{
    int p;
    int length;
    dtype *stack;
} dtype_stack;
typedef struct dtype_stack *DTYPE_STACK;

DTYPE_STACK dtype_stack_creat(void);
int dtype_stack_free(DTYPE_STACK s);
void dtype_stack_push(DTYPE_STACK s, dtype value);
dtype dtype_stack_pop(DTYPE_STACK s);
DTYPE dtype_stack_top(DTYPE_STACK s);
int dtype_stack_is_empty(DTYPE_STACK s);
int dtype_stack_error(int err_);

#endif
