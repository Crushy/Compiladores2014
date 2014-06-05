#include "codeGen.h"

extern ClassSymbols* classTable;

int tempvarNumber = 0;

char* VarTypeToLLVM(enum VariableType type) {
	switch(type) {
		case VARIABLE_BOOL :
			return "i1";
		case VARIABLE_INT :
			return "i32";
		case VARIABLE_VOID :
			return "void";
		default :
			return "NOT A VALID TYPE";
	}
}
/*
char* insertOp(TreeNode* node, char* buff) {
	
	char* op;
	
	switch (node->type) {
		case TREE_ADD :
			op = "add";
			break;
		case TREE_SUB :
			op = "sub";
			break;
		case TREE_MUL :
			op = "mul";
			break;
		case TREE_DIV :
			op = "div";
			break;
		case TREE_INTLIT :
		case TREE_BOOLLIT :
		default:
			break;
	}
	
	//insertOp(node,buf1);
	
	return NULL;
	
}

int buf1[50]

cg(env,method,n->child,buf1) ->   sprintf(buf,"%%k");
printf("add ... %s",buf1);




void GenClassCode(ClassSymbols* mainClass, TreeNode* root) {
	
	//switch
	
	
}
*/
void GenerateCode(ClassSymbols* mainClass, TreeNode* root) {
	//printf("%s",root->sons->node->args);
	//Ignore class ID
	
	//printf("declare i32 @atoi(i8* nocapture) nowind\n");
	
	NodeList* methodDeclOrVarDecl = root->sons->next;
	for (; methodDeclOrVarDecl!=NULL; methodDeclOrVarDecl=methodDeclOrVarDecl->next) {
		char* name;
		MethodSymbols* meth;
		
		switch(methodDeclOrVarDecl->node->type) {
			
			case TREE_METHODDECL :
				name = methodDeclOrVarDecl->node->sons->next->node->args;
				meth = LookupMethodInClass(name,mainClass);
				
				if (strcasecmp("main",name)==0) {
					printf("define i32 @main ( i32 %%argc, i8  ** %%args) {\n");
					
					/*Should always return 0 for Mooshak to play ball*/
					printf("\tret 0\n");
				}
				else {
					printf("define %s @%s (", VarTypeToLLVM(meth->returnValue),meth->name); //TODO: handle entry arguments
					
					int i=meth->numberOfParams;
					VariableList* varList = meth->variables;
					
					/*Handle first case due to commas being a pain*/
					if (i>0) {
						printf("%s %%%s",VarTypeToLLVM(varList->var->type), varList->var->name);
						varList=varList->next;
						i--;
					}
					for (; i>0; i--, varList=varList->next) {
						printf(", %s %%%s",VarTypeToLLVM(varList->var->type), varList->var->name);
					}
					printf(") {\n");
					//TODO: add a return if there is none
					printf("\tret 0\n");
				
				}
				printf("}\n	\n");
				
				break;
			case TREE_VARDECL :
				break;
			default:
				break;
		}
	}
	//GenClassCode(mainClass,);
}
