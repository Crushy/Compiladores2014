#pragma once

#include "structures.h"

#include <stdio.h> /*printf and friends*/
#include <string.h> /*for detecting main() using strcmp*/

void GenerateCode(ClassSymbols* mainClass, TreeNode* root);
