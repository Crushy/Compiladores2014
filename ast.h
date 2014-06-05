/*
* Functions used to build the ast 
*/
#pragma once

#include "structures.h" /* TreeNode, NodeList and such*/
#include <stdio.h> /*printfs*/
#include <stdlib.h> /*mallocs et all*/
#include <string.h> /*strcmp*/

TreeNode* InitTreeNode(enum NodeType type);
NodeList* InitNodeList(TreeNode* node);
TreeNode* InsertTerminal(char* terminalValue, enum NodeType terminalType);

/*Tree generation*/
TreeNode* InsertClass(TreeNode* name,NodeList* sons);
TreeNode* InsertMethod(TreeNode* Type, TreeNode* name, NodeList* paramSons, NodeList* bodySons);
TreeNode* InsertVarDecl(TreeNode* type,NodeList* names);
NodeList* InsertFormalParams(TreeNode* type,TreeNode* name,NodeList* existing);
TreeNode* InsertIfElse(TreeNode* condition, TreeNode* ifBody, TreeNode* elseBody);
TreeNode* InsertWhile(TreeNode* condition, TreeNode* body);
TreeNode* InsertPrint(TreeNode* expr);
TreeNode* InsertStore(TreeNode* varName, TreeNode* expr);
TreeNode* InsertStoreArray(TreeNode* arrayName, TreeNode* indexExpr, TreeNode* value);
/*TreeNode* insertInitArray(TreeNode* sizeExpr, enum NodeType type);*/
TreeNode* InsertExpression(TreeNode* son1, enum NodeType exprType, TreeNode* son2);
TreeNode* InsertParseArgs(TreeNode* name, TreeNode* index);
TreeNode* InsertCall(TreeNode* name, NodeList* args);
TreeNode* InsertBraces(NodeList* statements);
NodeList* InsertTreeNodeIntoList(TreeNode* newTreeNode, NodeList* existing);
TreeNode* AddSonsToTreeNode(TreeNode* node, NodeList* sons);
NodeList* MergeLists(NodeList* first, NodeList* second);

NodeList* InsertMainArgs(TreeNode* argName);
