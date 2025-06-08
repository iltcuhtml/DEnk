#pragma once

#include <variant>

#include "tokenizer.hpp"

/* Integer Literal Node */
struct NodeTermIntLit
{
    Token int_lit;
};

/* Identifier (variable) Node */
struct NodeTermIdent
{
    Token ident;
};

struct NodeExpr; // Forward Declaration

/* Parenthesized Expression Node */
struct NodeTermParen
{
    NodeExpr* expr;
};

/* Binary Operators */
enum class BinOp
{
    Add, Sub, Mul, Div
};

/* Binary Expression Node */
struct NodeBinExpr
{
    BinOp op;
    NodeExpr* lhs;
    NodeExpr* rhs;
};

/* Logic Operators */
enum class LogicOp
{
    NotEqual, Equal, Less, LessEqual, Greater, GreaterEqual
};

/* Logic Expression Node */
struct NodeLogicExpr
{
    LogicOp op;
    NodeExpr* lhs;
    NodeExpr* rhs;
};

/* Term Node */
struct NodeTerm
{
    std::variant<NodeTermIntLit*, NodeTermIdent*, NodeTermParen*> var;
};

/* Expression Node */
struct NodeExpr
{
    std::variant<NodeTerm*, NodeBinExpr*, NodeLogicExpr*> var;
};

/* Return Statement Node */
struct NodeStmtBeende
{
    NodeExpr* expr;
};

/* Assignment Statement Node */
struct NodeStmtBestimme
{
    Token ident;
    NodeExpr* expr;
};

struct NodeStmt; // Forward Declaration

/* Block Scope Node */
struct NodeScope
{
    std::vector<NodeStmt*> stmts;
};

/* If Statement Node */
struct NodeStmtFalls
{
    NodeExpr* expr;
    NodeScope* scope;
    NodeStmtFalls* falls;
};

/* Statement Node */
struct NodeStmt
{
    std::variant<NodeStmtBeende*, NodeStmtBestimme*, NodeScope*, NodeStmtFalls*> var;
};

/* Program Root Node */
struct NodeProg
{
    std::vector<NodeStmt*> stmts;
};