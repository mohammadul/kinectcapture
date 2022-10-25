/* ---------------------------------------------------------------------------
** This software is furnished "as is", without technical support,
** and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** program.h
** Contains header declarations and definitions for progam.c.
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

#ifndef __PROGRAM__
#define __PROGRAM__

#include "strfuncs.h"
#include "datastructs.h"

#define NUM_CODEWORDS 12

typedef struct __pcordword
{
    int codenum;
    int codeval;
    char hasval;
    char iscomment;
} pcodeword;

typedef struct __pcode
{
    int nlines;
    int allocated;
    pcodeword *cwrds;
} pcode;

extern pcode codes;
int kc_read_word(FILEPOINTER fp, char *c_word);
int getcodenum(const char *str);
pcodeword parse_line_program_file(FILEPOINTER fp);
void load_program_code(const char *fname);
void unload_program_code();
#endif
