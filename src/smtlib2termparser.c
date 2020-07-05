/* -*- C -*-
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

#include "smtparser/smtlib2termparser.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>


static std::string smtlib2_term_parser_format_error(const char *fmt, ...);



smtlib2_term_parser::smtlib2_term_parser(smtlib2_context ctx) : ctx_(ctx),
    function_term_handler_(NULL), number_term_handler_(NULL) {

}


smtlib2_term smtlib2_term_parser::smtlib2_term_parser_make_term(
                                           const std::string & symbol,
                                           smtlib2_sort sort,
                                           const int_vec & index,
                                           const term_vec & args)
{

    smtlib2_term ret = NULL;
    if (index.empty()) {
        smtlib2_term def = smtlib2_term_parser_get_binding(symbol);
        if (def) {
            auto const & params = smtlib2_term_parser_get_params(def);
            if (!params.emtpy()) {
                errmsg_ = smtlib2_term_parser_format_error(
                    "macros with parameters not supported yet");
                return NULL;
            } else {
                if (!args.empty()) {
                    errmsg_ = smtlib2_term_parser_format_error(
                        "wrong number of arguments for symbol `%s'",
                        symbol.c_str());
                    return NULL;
                }
                ret = def;
            } // if param else 

            if (!ret) {
                errmsg_ = smtlib2_term_parser_format_error(
                    "error in parsing symbol `%s'", symbol.c_str());
            }
            return ret;
        } // if def found
    } // index empty

    auto pos = symbol_handlers_.find(symbol);
    if (pos != symbol_handlers_.end()) {
        smtlib2_term_parser_symbolhandler handler = pos->second;
        handler(ctx_, symbol, sort, index, args);
        if (!ret) {
            errmsg_ = smtlib2_term_parser_format_error(
                "handler for symbol `%s' returned error", symbol.c_str());
        }
        return ret;
    } else {
        /* if we are here, this must be an uninterpreted function, otherwise
         * it is a parsing error */
        assert(function_term_handler_);
        ret = function_term_handler_(ctx_, symbol, sort, index, args);
        if (!ret) {
            errmsg_ = smtlib2_term_parser_format_error("error in parsing symbol `%s'",
                                             symbol.c_str());
        }
        return ret;
    }
} // smtlib2_term_parser_make_term


smtlib2_term smtlib2_term_parser::smtlib2_term_parser_make_number_term(
                                                  const std::string & rep,
                                                  unsigned int width,
                                                  unsigned int base)
{
    if (!tp->number_term_handler_) {
        errmsg_ = smtlib2_term_parser_format_error( "no handler for numbers");
        return NULL;
    } else {
        smtlib2_term ret = number_term_handler_(ctx_, rep, width, base);
        if (!ret) {
            errmsg_ = smtlib2_term_parser_format_error(
                "handler for numbers returned error");
        }
        return ret;
    }
}


void smtlib2_term_parser::smtlib2_term_parser_push_let_scope()
{
    let_levels_.push_back("");
}


void smtlib2_term_parser::smtlib2_term_parser_pop_let_scope()
{
    assert (let_levels_.size() > 0);
    auto let_levels_pos = let_levels_.rbegin();
    while(*let_levels_ != "") {
        assert (let_levels_pos != let_levels_.rend());

        auto let_bindings_pos = let_bindings_.find(*let_levels_pos);
        assert (let_bindings_pos != let_bindings_.end());
        const auto & vv = let_bindings_pos->second;
        assert (vv.size() > 0);
        vv.pop_back();
        if (vv.empty()) {
            let_bindings_.erase(let_bindings_pos);
        }
        let_levels_.pop_back();
        let_levels_pos = let_levels_.rbegin();
    }
    let_levels_.pop_back(); // remove ""
} // smtlib2_term_parser_pop_let_scope


void  smtlib2_term_parser::smtlib2_term_parser_define_let_binding(
                                            const std::string & symbol,
                                            smtlib2_term term)
{
    if (let_levels_.size() == 0) {
        errmsg_ = smtlib2_term_parser_format_error("parse error");
    } else {
        auto pos = bindings_.find(symbol);
        if (pos != bindings_.end()) {
            errmsg_ = smtlib2_term_parser_format_error("symbol `%s' already defined",
                                             symbol.c_str());
        } else {
            int i;
            for (i = let_levels_.size() -1 ; i >= 0 ; --i) {
                const std::string & s2 = let_levels_.at(i);
                if(s2 == "")
                    break;
                if (s2 == symbol) {
                    errmsg_ = smtlib2_term_parser_format_error(
                        "symbol `%s' already defined", symbol.c_str());
                    return;
                }
            } // end for
            let_levels_.push_back(symbol);
            let_bindings_[symbol].push_back(term);
        }
    }
} // smtlib2_term_parser_define_let_binding


void  smtlib2_term_parser::smtlib2_term_parser_define_binding(
                                        const std::string & symbol,
                                        const int_vec & params,
                                        smtlib2_term term)
{
    if (smtlib2_term_parser_get_binding(symbol)) {
        errmsg_ = smtlib2_term_parser_format_error("symbol `%s' already defined",
                                         symbol.c_str());
    } else {
        bindings_[symbol] = term;
        if (!params.empty()) {
            term_params_[term] = params;
        }
    }
}


void  smtlib2_term_parser::smtlib2_term_parser_undefine_binding(
                                          const std::string & symbol)
{
    auto pos = bindings_.find(symbol);
    if (pos == bindings_.end()) {
        errmsg_ = smtlib2_term_parser_format_error("symbol `%s' is not defined",
                                         symbol.c_str());
    } else {
        smtlib2_term t = pos->second;
        auto param_pos = term_params_.find(t);
        if (param_pos != term_params_.end()) {
            term_params_.erase(param_pos);
        }
        bindings_.erase(pos);
    }
}


void  smtlib2_term_parser::smtlib2_term_parser_set_handler(
                                     const std::string & symbol,
                                     smtlib2_term_parser_symbolhandler handler)
{
    symbol_handlers_[symbol] = handler;
}


void  smtlib2_term_parser::smtlib2_term_parser_set_function_handler(
    smtlib2_term_parser_functionhandler handler)
{
    function_term_handler_ = handler;
}


void  smtlib2_term_parser::smtlib2_term_parser_set_number_handler(
    smtlib2_term_parser_numberhandler handler)
{
    number_term_handler_ = handler;
}


bool  smtlib2_term_parser::smtlib2_term_parser_error() const
{
    return !(errmsg_.empty());
}


std::string smtlib2_term_parser::smtlib2_term_parser_get_error_msg() const
{
    return errmsg_;
}


static std::string smtlib2_term_parser_format_error(const char *fmt, ...)
{   
    std::string ret;
    va_list args;
    va_start(args, fmt);
    ret = smtlib2_vsprintf(fmt, args);
    va_end(args);
    return ret;
}

// make these private functions

smtlib2_term smtlib2_term_parser::smtlib2_term_parser_get_binding(const std::string & symbol) const
{
    auto pos = let_bindings_.find(symbol);
    if (pos != let_bindings_.end()) {
        assert (!(pos->second.emtpy()));
        return (pos->second.back())
    }
    auto pos_b = bindings_.find(symbol);
    if (pos_b != bindings_.end())
        return pos_b->second;
    return NULL;
}


const int_vec & 
smtlib2_term_parser::smtlib2_term_parser_get_params(smtlib2_term term) const
{
    auto pos = term_params_.find(term);
    if (pos != term_params_.end())
        return pos->second;
    return empty_inv_vec;
}
