/* -*- C++ -*-
 *
 * Adapted to C++ by Hongce Zhang (hongcez@princeton.edu)
 * ------------------------------------------------
 *           Original Header Below
 * ------------------------------------------------
 * 
 * Parser for Terms in the SMT-LIB v2 format
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

#ifndef SMTLIB2TERMPARSER_H_INCLUDED
#define SMTLIB2TERMPARSER_H_INCLUDED

#include "smtparser/smtlib2types.h"
#include "smtparser/smtlib2utils.h"


typedef struct smtlib2_term_parser smtlib2_term_parser;

typedef smtlib2_term (*smtlib2_term_parser_symbolhandler)(smtlib2_context ctx,
                                                          const std::string & symbol,
                                                          smtlib2_sort sort,
                                                          const int_vec & idx,
                                                          const term_vec & args);

typedef smtlib2_term (*smtlib2_term_parser_numberhandler)(smtlib2_context ctx,
                                                          const std::string & rep,
                                                          unsigned int width,
                                                          unsigned int base);
typedef smtlib2_term (*smtlib2_term_parser_functionhandler)(
                                                          smtlib2_context ctx,
                                                          const std::string & symbol,
                                                          smtlib2_sort sort,
                                                          const int_vec & index,
                                                          const term_vec & args);

struct smtlib2_term_parser {
    smtlib2_context ctx_;
    std::unordered_map<std::string, smtlib2_term_parser_symbolhandler> symbol_handlers_;
    // smtlib2_hashtable *symbol_handlers_;

    smtlib2_term_parser_functionhandler function_term_handler_;
    smtlib2_term_parser_numberhandler number_term_handler_;

    std::unordered_map<std::string, std::vector<smtlib2_term>> let_bindings_;
    //smtlib2_hashtable *let_bindings_;
    std::vector<std::string> let_levels_;
    //vec let_levels_;

    std::unordered_map<std::string, smtlib2_term> bindings_;
    //smtlib2_hashtable *bindings_;
    
    std::unordered_map<std::string, int_vec> term_params_;
    //smtlib2_hashtable *term_params_;
    std::string errmsg_;

    smtlib2_term_parser(smtlib2_context ctx);

    smtlib2_term smtlib2_term_parser_make_term(
                                            const std::string & symbol,
                                            smtlib2_sort sort,
                                            const int_vec & index,
                                            const term_vec & args);
    smtlib2_term smtlib2_term_parser_make_number_term(
                                                    const std::string & rep,
                                                    unsigned int width,
                                                    unsigned int base);

    void smtlib2_term_parser_push_let_scope();
    void smtlib2_term_parser_pop_let_scope();
    void smtlib2_term_parser_define_let_binding(
                                                const std::string & symbol,
                                                smtlib2_term term);
    void smtlib2_term_parser_define_binding(
                                            const std::string & symbol,
                                            const int_vec & params,
                                            smtlib2_term term);
    void smtlib2_term_parser_undefine_binding(
                                            const std::string & symbol);

    void smtlib2_term_parser_set_handler(
                                        const std::string & symbol,
                                        smtlib2_term_parser_symbolhandler handler);
    void smtlib2_term_parser_set_function_handler(
        
        smtlib2_term_parser_functionhandler handler);
    void smtlib2_term_parser_set_number_handler(
        
        smtlib2_term_parser_numberhandler handler);

    bool smtlib2_term_parser_error() const;
    std::string smtlib2_term_parser_get_error_msg() const;

private:
    smtlib2_term smtlib2_term_parser_get_binding(const std::string & symbol) const;
    const int_vec & smtlib2_term_parser_get_params(smtlib2_term term) const;
private:
    int_vec empty_inv_vec;
};




#endif /* SMTLIBTERMPARSER_H_INCLUDED */
