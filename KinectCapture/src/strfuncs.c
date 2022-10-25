/* ---------------------------------------------------------------------------
** This software is furnished "as is", without technical support,
** and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** strfuncs.c
** Contains main declarations and definitions for string operations.
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

#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "../include/strfuncs.h"

char *kc_go_next_word(char *fp)
{
    int flag = 0;
    char ch = 0;
    while((flag < 2) && ((ch = *(fp))!= '\0'))
    {
        fp++;
        if ((ch =='\v') || (ch =='\r') || (ch =='\n') || (ch=='\t') || isspace(ch) || (ch == ',') || (ch == '!') || (ch == '(') || (ch == ')') || (ch == '{') || (ch == '}') || (ch == '[') || (ch == ']'))
        {
            if(flag == 0) flag = 1;
        }
        else if(flag == 1) flag = 2;
    }
    if(ch != '\0')
    {
        --fp;
        return fp;
    }
    else return NULL;
}

int kc_count_words_in_line(const char *fp, int *count)
{
    int flag = -1;
    char ch = 0;
    *count = 0;
    while((flag < 3) && ((ch = *(fp))!= '\0'))
    {
        fp++;
        if((ch =='\v') || (ch =='\r') || (ch =='\n') || (ch == '\t'))
        {
            if (flag == 0)
            {
                (*count)++;
                flag = 3;
            }
            if(flag == -1) flag = 3;/* line included to handle empty line */
            else flag = 2;
        }
        else if (isspace(ch) || (ch == ',') || (ch == '!') || (ch == '(') || (ch == ')') || (ch == '{') || (ch == '}') || (ch == '[') || (ch == ']'))
        {
            if(flag == 0)/* changed from <=0 to ==0 to skip initial space */
            {
                flag = 1;
                (*count)++;
            }
        }
        else if(flag == 1) flag = 0;
        else if(flag == 2) flag = 3;
        else flag = 0;
    }
    if(flag !=-1) --fp;
    if (ch == '\0')
    {
        if (flag == 0) (*count)++;
        return 1;
    }
    else return 0;
}


