#include "ast.h"

TreeNode* InitTreeNode(enum NodeType type) {
	TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
	node->type = type;
	node->args = NULL;
	node->sons = NULL;
	return node;
}

NodeList* InitNodeList(TreeNode* node) {
	NodeList* list = (NodeList*)malloc(sizeof(NodeList));
	list->next = NULL;
	list->node = node;
	return list;
}

TreeNode* InsertTerminal(char* terminalValue, enum NodeType terminalType) {
	TreeNode* node = InitTreeNode(terminalType);
	node->args=(void*)terminalValue;
	return node;
}

/*Tree generation*/
TreeNode* InsertClass(TreeNode* name,NodeList* sons) {
	TreeNode* node = InitTreeNode(TREE_PROGRAM);
	
	node->sons = InitNodeList(name);
	node->sons->next = sons;
	
	return node;
}


TreeNode* InsertMethod(TreeNode* Type, TreeNode* name, NodeList* paramSons, NodeList* bodySons) {
	TreeNode* node=InitTreeNode(TREE_METHODDECL);
	
	NodeList* param=InitNodeList(InitTreeNode(TREE_METHODPARAMS));
	param->node->sons=paramSons;
	
	NodeList* body=InitNodeList(InitTreeNode(TREE_METHODBODY));
	body->node->sons=bodySons;
	
	NodeList* typeSon = InitNodeList(Type);
	NodeList* IDSon = InitNodeList(name);
	
	node->sons=typeSon;
	typeSon->next=IDSon;
	IDSon->next=param;
	param->next=body;
	
	return node;
}

TreeNode* InsertVarDecl(TreeNode* type,NodeList* names) {
	TreeNode* node = InitTreeNode(TREE_VARDECL);
	
	node->sons = InitNodeList(type);
	node->sons->next=names;
	return node;
}

NodeList* InsertFormalParams(TreeNode* type,TreeNode* name,NodeList* existing) {
	
	NodeList* param = InitNodeList(InitTreeNode(TREE_PARAMDECLARATION));
	
	NodeList* newSon1 = InitNodeList(type);
	NodeList* newSon2 = InitNodeList(name);
	
	param->node->sons=newSon1;
	newSon1->next=newSon2;
	
	param->next=existing;
	return param;
}

TreeNode* InsertIfElse(TreeNode* condition, TreeNode* ifBody, TreeNode* elseBody) {
	TreeNode* node = InitTreeNode(TREE_IFELSE);
	node->sons=InitNodeList(condition);
	
	if(ifBody==NULL) /*We may not have an if statement*/
		node->sons->next=InitNodeList(InsertTerminal(NULL,TREE_NULL));
	else
		node->sons->next=InitNodeList(ifBody);
	
	if(elseBody==NULL) /*We may not have an else statement*/
		node->sons->next->next=InitNodeList(InsertTerminal(NULL,TREE_NULL));
	else
		node->sons->next->next=InitNodeList(elseBody);
	return node;
}

TreeNode* InsertWhile(TreeNode* condition, TreeNode* body) {
	TreeNode* node = InitTreeNode(TREE_WHILE);
	node->sons=InitNodeList(condition);
	if (body==NULL)
		node->sons->next=InitNodeList(InsertTerminal(NULL,TREE_NULL));
	else
		node->sons->next=InitNodeList(body);
	return node;
}

TreeNode* InsertPrint(TreeNode* expr) {
	TreeNode* node = InitTreeNode(TREE_PRINT);
	node->sons=InitNodeList(expr);
	return node;
}

TreeNode* InsertStore(TreeNode* varName, TreeNode* expr) {
	TreeNode* node = InitTreeNode(TREE_STORE);
	node->sons=InitNodeList(varName);
	node->sons->next=InitNodeList(expr);
	return node;
}

TreeNode* InsertStoreArray(TreeNode* arrayName, TreeNode* indexExpr, TreeNode* value) {
	TreeNode* node = InitTreeNode(TREE_STOREARRAY);
	
	node->sons=InitNodeList(arrayName); //ID
	node->sons->next=InitNodeList(indexExpr); //index
	node->sons->next->next=InitNodeList(value); //value
	return node;
}

TreeNode* InsertExpression(TreeNode* son1, enum NodeType exprType, TreeNode* son2) {
	TreeNode* node = InitTreeNode(exprType);
	
	NodeList* son2item = NULL;
	if (son2!=NULL)
		son2item=InitNodeList(son2);
		
	if (son1!=NULL) {
		node->sons=InitNodeList(son1);
		node->sons->next=son2item;
	}
	else {
		node->sons=son2item;
	}
	
	return node;
}
TreeNode* InsertParseArgs(TreeNode* name, TreeNode* index) {
	TreeNode* node = InitTreeNode(TREE_PARSEARGS);
	if (name!=NULL) {
		node->sons=InitNodeList(name);
	}
	else {
		node->sons=InitNodeList(InsertTerminal(NULL,TREE_NULL));
	}
	
	if (index!=NULL){
		node->sons->next=InitNodeList(index);
	}
	else {
		node->sons->next=InitNodeList(InsertTerminal(NULL,TREE_NULL));
	}
	return node;
}


TreeNode* InsertCall(TreeNode* name, NodeList* args) {
	TreeNode* node = InitTreeNode(TREE_CALL);
	
	node->sons=InitNodeList(name);
	
	
	node->sons->next=args;
	
	return node;
}

TreeNode* InsertBraces(NodeList* statements) {
	TreeNode* retVal;
	
	if ( statements==NULL ) {
		return NULL;
	} 
	else if(statements->next==NULL){
		retVal=statements->node;
		free(statements);
		return retVal;
	}
	else {
		TreeNode* comp=InsertTerminal(NULL,TREE_COMPOUNDSTAT);
		comp->sons=statements;
		return comp;
	}
	return NULL;
}

NodeList* InsertTreeNodeIntoList(TreeNode* newTreeNode, NodeList* existing) {
	if (newTreeNode==NULL)
		return existing;
	
	NodeList* newSon=InitNodeList(newTreeNode);
	newSon->next=existing;
	return newSon;
}

TreeNode* AddSonsToTreeNode(TreeNode* node, NodeList* sons) {
	/*Probably not needed but what the heck why not*/
	NodeList* currSon=node->sons;
	if(currSon!=NULL) {
		
		while (currSon->next!=NULL) {
			currSon=currSon->next;
		}
		
		currSon->next=sons;
	}
	else {
		node->sons=sons;
	}
	
	return node;
}

NodeList* MergeLists(NodeList* first, NodeList* second) {
	if(first==NULL)
		return second;
	
	NodeList* currSon=first;
	while (currSon->next!=NULL) {
		currSon=currSon->next;
	}
	currSon->next=second;
	
	return first;
}

NodeList* InsertMainArgs(TreeNode* argName) {
	NodeList* methodDecl=InitNodeList(InitTreeNode(TREE_PARAMDECLARATION));
	
	NodeList* type=InitNodeList(InitTreeNode(TREE_STRINGARRAY));
	NodeList* name = InitNodeList(argName);
	
	methodDecl->node->sons=type;
	type->next=name;
	
	return methodDecl;
}
