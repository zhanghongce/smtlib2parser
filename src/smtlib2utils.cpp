/* -*- C++ -*-
 *
 * Adapted to C++ by Hongce Zhang (hongcez@princeton.edu)
 * ------------------------------------------------
 *           Original Header Below
 * ------------------------------------------------
 * 
 * Utility functions and data structures for the SMT-LIB v2 parser
 *
 * Author: Alberto Griggio <griggio@fbk.eu>
 *
 * Copyright (C) 2010 Alberto Griggio
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#include "smtparser/smtlib2utils.h"
#include <stdio.h>
#include <stdarg.h>


std::string smtlib2_vsprintf(const char *fmt, va_list args)
{
    size_t size = 256;
    char *ret = (char *)malloc(size);
    while (ret) {
        int res = vsnprintf(ret, size, fmt, args);
        if (res > -1 && res < size) {
            break;
        } else if (res > -1) {
            size = res+1;
        } else {
            size *= 2;
        }
        ret = (char *)realloc(ret, size);
    }
    std::string ret_str = ret;
    free(ret);
    return ret_str;
}


std::string smtlib2_sprintf(const char *fmt, ...)
{
    va_list args;
    std::string ret;
    va_start(args, fmt);
    ret = smtlib2_vsprintf(fmt, args);
    va_end(args);
    return ret;
}