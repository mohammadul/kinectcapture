/* ---------------------------------------------------------------------------
** This software is furnished "as is", without technical support,
** and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** datastructs.c
** Contains main declarations and definitions for data structures.
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

#include "../include/datastructs.h"

DTYPE_STACK dtype_stack_creat(void)
{
    DTYPE_STACK s;
    if((s = (DTYPE_STACK)malloc(sizeof(struct dtype_stack)))==NULL) stack_error(STACK_MALLOC);
    s->p = 0;
    s->length = STACK_MAX;
    if((s->stack = (DTYPE)malloc(sizeof(dtype)*(STACK_MAX)))==NULL) stack_error(STACK_MALLOC);
    return s;
}

int dtype_stack_free(DTYPE_STACK s)
{
    if (s==NULL) return 0;
    free(s->stack);
    free(s);
    return 1;
}

void dtype_stack_push(DTYPE_STACK s, dtype value)
{
    if(s->p>=s->length)
    {
        if((s->stack = (dtype *)realloc(s->stack, sizeof(dtype)*(s->length+STACK_MAX)))==NULL) stack_error(STACK_MALLOC);
        s->length += STACK_MAX;
    }
    s->stack[s->p++] = value;
}

dtype dtype_stack_pop(DTYPE_STACK s)
{
    if(s->p<=0) stack_error(STACK_EMPTY);
    return s->stack[--(s->p)];
}

DTYPE dtype_stack_top(DTYPE_STACK s)
{
    if(s->p<=0) stack_error(STACK_EMPTY);
    return &(s->stack[(s->p-1)]);
}

int dtype_stack_is_empty(DTYPE_STACK s)
{
    return ((int)(s->p==0));
}

int dtype_stack_error(int err_)
{
    switch(err_)
    {
    case STACK_MALLOC:
        fprintf(stderr, "stack: malloc error\n");
        break;
    case STACK_EMPTY:
        fprintf(stderr, "stack: stack empty\n");
        break;
    }
    exit(1);
}

