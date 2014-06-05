#pragma once

#include <string.h> /*strcmp*/
#include <stdio.h> /*printf*/

#include <errno.h> /*for string to int conversions (the error detection part)*/
#include <limits.h> /*see above*/
#include <stdbool.h>

#include <stdarg.h> /*for custom printf wrapper on semantic errors*/

#include "structures.h" /*for tree nodes*/

void PrintSemanticError(const char *fmt, ...);
void CheckSemanticError(TreeNode* class);

/*Not "public" but needed here due to cross-dependant functions*/
enum VariableType GetExpressionType(TreeNode* currentNode, MethodSymbols* methodTable);
