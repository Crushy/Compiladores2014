#pragma once
#include "structures.h"
#include "semantics.h" /*so we can print semantic errors*/
#include <stdlib.h> /*mallocs et all*/

void BuildSymbolTable(TreeNode* classRoot);
