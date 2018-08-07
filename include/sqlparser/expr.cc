// Copyright (c) 2018 Baidu, Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "expr.h"
#include "unordered_map"

namespace parser {
static std::unordered_map<int, std::string> FUNC_STR_MAP = {
    {FT_COMMON, ""},
    {FT_AGG, ""},
    // ~ ! -
    {FT_BIT_NOT, " ~ "},
    {FT_LOGIC_NOT, " ! "},
    {FT_UMINUS, " - "},
    // + - * /
    {FT_ADD, " + "},
    {FT_MINUS, " - "},
    {FT_MULTIPLIES, " * "},
    {FT_DIVIDES, " / "},
    // % << >> & | ^
    {FT_MOD, " % "},
    {FT_LS, " << "},
    {FT_RS, " >> "},
    {FT_BIT_AND, " & "},
    {FT_BIT_OR, " | "},
    {FT_BIT_XOR, " ^ "},
    // == != > >= < <=
    {FT_EQ, " = "},
    {FT_NE, " != "},
    {FT_GT, " > "},
    {FT_GE, " >= "},
    {FT_LT, " < "},
    {FT_LE, " <= "},
    // && || xor
    {FT_LOGIC_AND, " && "},
    {FT_LOGIC_OR, " || "},
    {FT_LOGIC_XOR, " XOR "},
    // is like in null ture
    {FT_IS_NULL, "IS NULL"},
    {FT_IS_TRUE, "IS TRUE"},
    {FT_IS_UNKNOWN, "IS UNKNOWN"},
    {FT_IN, "IN"},
    {FT_LIKE, "LIKE"},
    {FT_BETWEEN, "BETWEEN"},
    {FT_VALUES, "values"}
};

static std::unordered_map<int, const char*> FUNC_FN_NAME_MAP = {
    {FT_COMMON, ""},
    {FT_AGG, ""},
    // ~ ! -
    {FT_BIT_NOT, "bit_not"},
    {FT_LOGIC_NOT, "logic_not"},
    {FT_UMINUS, "minus"},
    // + - * /
    {FT_ADD, "add"},
    {FT_MINUS, "minus"},
    {FT_MULTIPLIES, "multiplies"},
    {FT_DIVIDES, "divides"},
    // % << >> & | ^
    {FT_MOD, "mod"},
    {FT_LS, "left_shift"},
    {FT_RS, "right_shift"},
    {FT_BIT_AND, "bit_and"},
    {FT_BIT_OR, "bit_or"},
    {FT_BIT_XOR, "bit_xor"},
    // == != > >= < <=
    {FT_EQ, "eq"},
    {FT_NE, "ne"},
    {FT_GT, "gt"},
    {FT_GE, "ge"},
    {FT_LT, "lt"},
    {FT_LE, "le"},
    // && || xor
    {FT_LOGIC_AND, "logic_and"},
    {FT_LOGIC_OR, "logic_or"},
    {FT_LOGIC_XOR, "logic_xor"},
    // is like in null ture
    {FT_IS_NULL, "is_null"},
    {FT_IS_TRUE, "is_true"},
    {FT_IS_UNKNOWN, "is_unknown"},
    {FT_IN, "in"},
    {FT_LIKE, "like"},
    {FT_BETWEEN, "between"}
};

inline const char* type_to_name(FuncType t) {
    if (FUNC_FN_NAME_MAP.count(t) == 0) {
        return "unkown";
    }
    return FUNC_FN_NAME_MAP[t];
}

FuncExpr* FuncExpr::new_unary_op_node(FuncType t, Node* arg1, butil::Arena& arena) {
    FuncExpr* fun = new(arena.allocate(sizeof(FuncExpr)))FuncExpr();
    fun->func_type = t;
    fun->fn_name = type_to_name(t);
    fun->children.push_back(arg1, arena);
    return fun;
}
FuncExpr* FuncExpr::new_binary_op_node(FuncType t, Node* arg1, Node* arg2, butil::Arena& arena) {
    FuncExpr* fun = new(arena.allocate(sizeof(FuncExpr)))FuncExpr();
    fun->func_type = t;
    fun->fn_name = type_to_name(t);
    fun->children.push_back(arg1, arena);
    fun->children.push_back(arg2, arena);
    return fun;
}
FuncExpr* FuncExpr::new_ternary_op_node(
        FuncType t, Node* arg1, Node* arg2, Node* arg3, butil::Arena& arena) {
    FuncExpr* fun = new(arena.allocate(sizeof(FuncExpr)))FuncExpr();
    fun->func_type = t;
    fun->fn_name = type_to_name(t);
    fun->children.reserve(3, arena);
    fun->children.push_back(arg1, arena);
    fun->children.push_back(arg2, arena);
    fun->children.push_back(arg3, arena);
    return fun;
}

void FuncExpr::to_stream(std::ostream& os) const {
    static const char* not_str[] = {"", " NOT"};
    static const char* true_str[] = {"TRUE", "FALSE"};
    os << "(";
    switch (func_type) {
        case FT_COMMON: {
        case FT_AGG:
            os << fn_name << "(";
            if (distinct) {
                os << "DISTINCT ";
            }
            for (int i = 0; i < children.size(); i++) {
                os << children[i];
                if (i != children.size() - 1) {
                    os << ",";
                }
            }
            if (is_star) {
                os << "*";
            }
            os << ")";
            break;
        }
        case FT_BIT_NOT:
        case FT_LOGIC_NOT:
        case FT_UMINUS:
            os << FUNC_STR_MAP[func_type] << children[0];
            break;
        case FT_ADD:
        case FT_MINUS:
        case FT_MULTIPLIES:
        case FT_DIVIDES:
        case FT_MOD:
        case FT_LS:
        case FT_RS:
        case FT_BIT_AND:
        case FT_BIT_OR:
        case FT_BIT_XOR:
        case FT_EQ:
        case FT_NE:
        case FT_GT:
        case FT_GE:
        case FT_LT:
        case FT_LE:
        case FT_LOGIC_AND:
        case FT_LOGIC_OR:
        case FT_LOGIC_XOR:
            os << children[0] << FUNC_STR_MAP[func_type] << children[1];
            break;
        case FT_IS_NULL:
            os << children[0] << " IS" << not_str[is_not] << " NULL";
            break;
        case FT_IS_TRUE:
            os << children[0] << " IS " << true_str[is_not];
            break;
        case FT_IS_UNKNOWN:
            os << children[0] << " IS" << not_str[is_not] << " UNKNOWN";
            break;
        case FT_IN:
            os << children[0] << not_str[is_not] << " IN " << children[1]; 
            break;
        case FT_LIKE:
            os << children[0] << not_str[is_not] << " LIKE " << children[1]; 
            break;
        case FT_BETWEEN:
            os << children[0] << not_str[is_not] << 
                " BETWEEN " << children[1] << " AND " << children[2]; 
            break;
        case FT_VALUES:
            os << "VALUES(" << children[0] << ")";
        default:
            break;
    }
    os << ")";
};

void ColumnName::to_stream(std::ostream& os) const {
    if (db.value != nullptr) {
        os << db << ".";
    } 
    if (table.value != nullptr) {
        os << table << ".";
    }
    os << name;
}

void LiteralExpr::to_stream(std::ostream& os) const {
    switch (literal_type) {
        case LT_INT:
            os << _u.int64_val;
            break;
        case LT_DOUBLE:
            os << _u.double_val;
            break;
        case LT_STRING:
            os << _u.str_val.value;
            break;
        case LT_BOOL:
            os << _u.bool_val;
            break;
        case LT_NULL:
            os << "NULL";
            break;
        default:
            break;
    }
}

}

/* vim: set ts=4 sw=4 sts=4 tw=100 */