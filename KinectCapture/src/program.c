/* ---------------------------------------------------------------------------
** This software is furnished "as is", without technical support,
** and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** program.c
** Contains main declarations and definitions for the program.
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

#include "../include/program.h"

/* command look up */
const char commandstring[NUM_CODEWORDS][24] =
{
    "", /* 0 */
    "MODE", /* 1 */
    "START", /* 2 */
    "STOP", /* 3 */
    "CAPTURE", /* 4 */
    "SAVE", /* 5 */
    "PROJECTOR", /* 6 */
    "END", /* 7 */
    "PAUSE", /* 8 */
    "BRIGHTNESS", /* 9 */
    "SWITCH", /* 10 */
    "LOOP", /* 11 */
};

pcode codes;

int kc_read_word(FILEPOINTER fp, char *c_word)
{
    int flag = 0, t = 0, comment_flag = 0, ch = 0;
    while((flag<3) && ((ch = getc(fp))!= EOF)) /*no need for state 3 to be corrected*/
    {
        if((ch =='\v') || (ch =='\r') || (ch =='\n') || (ch =='\t') || isspace(ch) || (ch == ',') || (ch == '!') || (ch == '(') || (ch == ')') || (ch == '{') || (ch == '}') || (ch == '[') || (ch == ']'))
        {
            if(flag != 0) flag = 2;
        }
        else if(flag<2)
        {
            flag = 1;
            if(ch=='%') /* comment line to skip */
            {
                flag = 4;
                comment_flag = 1;
            }
            c_word[t++] = ch;
        }
        else if(flag == 2) flag = 3; /* reached next word */ /*to be corrected for deleting state 3*/
    }
    c_word[t] = '\0';
    if(flag==4) /* skip remaining part of the line */
    {
         while((comment_flag==1) && ((ch = (char)getc(fp))!= EOF))/*no need for state 3 to be corrected*/
        {
            if (ch =='\r' || ch =='\n')
            {
                comment_flag = 0;
            }
        }
            if(ch != EOF)
        {
            ungetc(ch, fp);
            return 2;
        }
        else return 2;
    }
    if(ch!=EOF)
    {
        ungetc(ch, fp);
        return 1;
    }
    else return 0;
}

int getcodenum(const char *str)
{
    int c = 0, i;

    for(i=0; i<NUM_CODEWORDS; i++)
    {
        if(strcmp(str, commandstring[i])==0)
        {
            c = i;
            break;
        }
    }
    return c;
}

pcodeword parse_line_program_file(FILEPOINTER fp)
{
    int c = 0, flag = 0;
    char c_word[32];
    pcodeword pc;
    pc.codenum = 0;
    pc.hasval = 0;
    pc.iscomment = 0;
    flag = kc_read_word(fp, c_word);
    if(flag==2)
    {
        pc.iscomment = 1;
        return pc;
    }
    c = getcodenum(c_word);
    if(c==0) return pc;
    else
    {
        if(flag==0 && c!=3) /* except stop reached already*/
        {
            pc.codenum = 0;
            return pc;
        }
        switch(c)
        {
        case 1: /* mode */
            flag = kc_read_word(fp, c_word);
            c = strtol(c_word, NULL, 0);
            if(c<1 || c>6)
            {
                pc.codenum = 0;
                return pc;
            }
            pc.hasval = 0;
            pc.codenum = c;
            break;
        case 8: /* pause */
            pc.codenum = c+6;
            flag = kc_read_word(fp, c_word);
            c = strtol(c_word, NULL, 0);
            if(c<0)
            {
                pc.codenum = 0;
                return pc;
            }
            pc.hasval = 1;
            pc.codeval = c;
            break;
        case 9: /* brightness */
            pc.codenum = c+6;

            flag = kc_read_word(fp, c_word);
            c = strtol(c_word, NULL, 0);
            if(c<0 || c>50)
            {
                pc.codenum = 0;
                return pc;
            }
            pc.hasval = 1;
            pc.codeval = c;
            break;
        case 10: /* switch */
            pc.codenum = c+6;

            flag = kc_read_word(fp, c_word);
            c = strtol(c_word, NULL, 0);
            if(c<-1)
            {
                pc.codenum = 0;
                return pc;
            }
            pc.hasval = 1;
            pc.codeval = c;
            break;
        case 11: /* loop */
            pc.codenum = c+6;

            flag = kc_read_word(fp, c_word);
            c = strtol(c_word, NULL, 0);
            if(c<0)
            {
                pc.codenum = 0;
                return pc;
            }
            pc.hasval = 1;
            pc.codeval = c;
            break;
        default:
            pc.codenum = c+5;
            pc.codeval = 1;
        }
    }
    return pc;
}

void load_program_code(const char *fname)
{
    int flag = 0, k = 0, ok = 0;
    pcodeword curr_cwd;
    FILEPOINTER fp = NULL;
    DTYPE_STACK stck = NULL;
    codes.allocated = 0;
    codes.cwrds = NULL;
    codes.nlines = 0;
    stck = dtype_stack_creat();

    if((fp = fopen(fname, "r"))!=NULL)
    {
        while(!flag)
        {
            curr_cwd = parse_line_program_file(fp);
            if(curr_cwd.iscomment==1) continue;
            if(curr_cwd.codenum!=0)
            {
                if(ok==0 && k==0 && curr_cwd.codenum==7) ok = 1;
                if(ok==1 && curr_cwd.codenum==17)
                {
                    dtype tmp;
                    tmp.first = codes.nlines;
                    tmp.second = curr_cwd.codeval;
                    dtype_stack_push(stck, tmp);
                }
                if(ok==1 && curr_cwd.codenum==12)
                {
                    if(dtype_stack_is_empty(stck))
                    {
                        /* bad looping */
                        flag = 1;
                        ok = 3;
                    }
                    else dtype_stack_pop(stck);
                }

                if(ok==1 && curr_cwd.codenum==8)
                {
                    ok = 2;
                    flag = 1;
                }
                k++;
                codes.cwrds = (pcodeword *)realloc(codes.cwrds, k*sizeof(pcodeword));
                codes.cwrds[k-1] = curr_cwd;
                codes.allocated = 1;
                codes.nlines = k;
            }
            else
            {
                /* error */
                flag = 1;
                if(codes.allocated) free(codes.cwrds);
                codes.allocated = 0;
                codes.nlines = 0;
            }
        }
        if(codes.allocated && ok!=2) /* incomplete code */
        {
            free(codes.cwrds);
            codes.allocated = 0;
            codes.nlines = 0;
        }
        else if(!dtype_stack_is_empty(stck) || ok==3) /* bad looping */
        {
            if(codes.allocated) free(codes.cwrds);
            codes.allocated = 0;
            codes.nlines = 0;
        }
        fclose(fp);
    }
    dtype_stack_free(stck);
}

void unload_program_code()
{
    free(codes.cwrds);
    codes.allocated = 0;
    codes.nlines = 0;
}

