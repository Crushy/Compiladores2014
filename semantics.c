#include "semantics.h"

extern int semantic_errors;
extern ClassSymbols* classTable;

void PrintSemanticError(const char *fmt, ...) {
	if (semantic_errors<=0) {
		va_list args;
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
	}
	
	semantic_errors++;
	return;
}

enum VariableType CheckLitInt(char* string) {
	char *endptr;
	/*or we could have used sscanf %i%c */
	errno = 0;
	int result = strtol(string, &endptr, 0);
	if ( *endptr != '\0' || ((result == LONG_MAX || result == LONG_MIN) && errno == ERANGE) ) {
		PrintSemanticError("Invalid literal %s\n", string);
		return VARIABLE_INVALID;
	}
	return VARIABLE_INT;
	
}

enum VariableType CheckLitBool(char* string) {
	if (strcasecmp(string,"true")!=0 && strcasecmp(string,"false")!=0) {
		PrintSemanticError("Invalid literal %s\n", string);
		return VARIABLE_INVALID;
	}
	else {
		return VARIABLE_BOOL;
	}
}

enum VariableType CheckCall(TreeNode* node, MethodSymbols* methodTable) {
	char* functionName=(char*)(node->sons->node->args);
	
	MethodSymbols* calledMethod = LookupMethodInClass(functionName,classTable);
	if (calledMethod==NULL) {
		PrintSemanticError("Cannot find symbol %s\n",functionName);
		return VARIABLE_INVALID;
	}
	
	VariableList* paramList = calledMethod->variables;
	/*First numParams args are parameters*/
	int i;
	NodeList* callSon = node->sons->next; /*Start with first param*/
	for (i=0; i<(calledMethod->numberOfParams); paramList=paramList->next, i++) {
		enum VariableType expectedType = paramList->var->type;
		
		if (callSon==NULL) {
			PrintSemanticError("Incompatible type of argument %d in call to method %s (got %s, required %s)\n",i,calledMethod->name,VarTypeToString(VARIABLE_VOID),VarTypeToString(expectedType));
			continue;
		}
		else {
			enum VariableType argumentType = GetExpressionType(callSon->node,methodTable);		
			if (argumentType!=expectedType) {
				PrintSemanticError("Incompatible type of argument %d in call to method %s (got %s, required %s)\n",i,calledMethod->name,VarTypeToString(argumentType),VarTypeToString(expectedType));
			}
			
			callSon=callSon->next;
		}
	}
	
	//Check if we have more parameters than expected
	while(callSon!=NULL) {
		enum VariableType argumentType = GetExpressionType(callSon->node,methodTable);
		PrintSemanticError("Incompatible type of argument %d in call to method %s (got %s, required %s)\n",i,calledMethod->name,VarTypeToString(argumentType),VarTypeToString(VARIABLE_VOID));
		i++;
		callSon=callSon->next;
	}
	
	return calledMethod->returnValue;
}

enum VariableType CheckID(TreeNode* node, MethodSymbols* methTable) {
	/*Lookup variable in method*/
	Variable* idVariable = LookupMethodVariable((char*)(node->args),methTable);
	
	/*Lookup variable in class*/
	if (idVariable==NULL) {
		idVariable = LookupVariableClass((char*)(node->args),classTable);
	}
	
	if (idVariable!=NULL)
		return idVariable->type;
	else {
		PrintSemanticError("Cannot find symbol %s\n",(char*)(node->args));
		return VARIABLE_INVALID;
	}
}

enum VariableType CheckOperatorIntsOnly(char* operator, TreeNode* currentNode, MethodSymbols* methodTable) {
	enum VariableType temp1,temp2;
	
	temp1=GetExpressionType(currentNode->sons->node, methodTable);
	temp2=GetExpressionType(currentNode->sons->next->node, methodTable);
	if (temp1!=VARIABLE_INT || temp2!=VARIABLE_INT) {
		PrintSemanticError("Operator %s cannot be applied to types %s, %s\n",operator,VarTypeToString(temp1),VarTypeToString(temp2));
		return VARIABLE_INVALID;
	}
	return VARIABLE_INT;
}

enum VariableType CheckOperatorCompareIntsOnly(char* operator, TreeNode* currentNode, MethodSymbols* methodTable) {
	enum VariableType temp1,temp2;
	
	temp1=GetExpressionType(currentNode->sons->node, methodTable);
	temp2=GetExpressionType(currentNode->sons->next->node, methodTable);
	if (temp1!=VARIABLE_INT || temp2!=VARIABLE_INT) {
		PrintSemanticError("Operator %s cannot be applied to types %s, %s\n",operator,VarTypeToString(temp1),VarTypeToString(temp2));
		return VARIABLE_INVALID;
	}
	return VARIABLE_BOOL;
}

enum VariableType CheckOperatorBoolsOnly(char* operator, TreeNode* currentNode, MethodSymbols* methodTable) {
	enum VariableType temp1,temp2;
	
	temp1=GetExpressionType(currentNode->sons->node, methodTable);
	temp2=GetExpressionType(currentNode->sons->next->node, methodTable);
	if (temp1!=VARIABLE_BOOL || temp2!=VARIABLE_BOOL) {
		PrintSemanticError("Operator %s cannot be applied to types %s, %s\n",operator,VarTypeToString(temp1),VarTypeToString(temp2));
		return VARIABLE_INVALID;
	}
	return VARIABLE_BOOL;
}

enum VariableType CheckOperatorComparison(char* operator, TreeNode* currentNode, MethodSymbols* methodTable) {
	enum VariableType temp1,temp2;
	
	temp1=GetExpressionType(currentNode->sons->node, methodTable);
	temp2=GetExpressionType(currentNode->sons->next->node, methodTable);
	if (temp1!=temp2) {
		PrintSemanticError("Operator %s cannot be applied to types %s, %s\n",operator,VarTypeToString(temp1),VarTypeToString(temp2));
	}
	return VARIABLE_BOOL;
}

enum VariableType GetExpressionType(TreeNode* currentNode, MethodSymbols* methodTable) {
	enum VariableType temp1,temp2;
	
	switch(currentNode->type) {
		case TREE_OR :
			return CheckOperatorBoolsOnly("||",currentNode,methodTable);
		case TREE_AND :
			return CheckOperatorBoolsOnly("&&",currentNode,methodTable);
		case TREE_EQ :
			return CheckOperatorComparison("==",currentNode,methodTable);
		case TREE_NEQ :
			return CheckOperatorComparison("!=",currentNode,methodTable);
		case TREE_LT :
			return CheckOperatorCompareIntsOnly("<",currentNode,methodTable);
		case TREE_GT :
			return CheckOperatorCompareIntsOnly(">",currentNode,methodTable);
		case TREE_LEQ :
			return CheckOperatorCompareIntsOnly("<=",currentNode,methodTable);
		case TREE_GEQ :
			return CheckOperatorCompareIntsOnly(">=",currentNode,methodTable);
		case TREE_ADD :
			return CheckOperatorIntsOnly("+",currentNode,methodTable);
		case TREE_SUB :
			return CheckOperatorIntsOnly("-",currentNode,methodTable);
		case TREE_MUL :
			return CheckOperatorIntsOnly("*",currentNode,methodTable);
		case TREE_DIV :
			return CheckOperatorIntsOnly("/",currentNode,methodTable);
		case TREE_MOD :
			return CheckOperatorIntsOnly("%",currentNode,methodTable);
		case TREE_NOT :
			if((temp1=GetExpressionType(currentNode->sons->node,methodTable))==VARIABLE_BOOL)
				return VARIABLE_BOOL;
			else {
				PrintSemanticError("Operator %s cannot be applied to type %s\n","!",VarTypeToString(temp1));
				return VARIABLE_INVALID;
			}
		case TREE_PLUS :
			if((temp1=GetExpressionType(currentNode->sons->node,methodTable))==VARIABLE_INT)
				return VARIABLE_INT;
			else {
				PrintSemanticError("Operator %s cannot be applied to type %s\n","+",VarTypeToString(temp1));
				return VARIABLE_INVALID;
			}
		case TREE_MINUS :
			if((temp1=GetExpressionType(currentNode->sons->node,methodTable))==VARIABLE_INT)
				return VARIABLE_INT;
			else {
				PrintSemanticError("Operator %s cannot be applied to type %s\n","-",VarTypeToString(temp1));
				return VARIABLE_INVALID;
			}
		case TREE_LENGTH :
			temp1=GetExpressionType(currentNode->sons->node,methodTable);
			switch (temp1) {
				case VARIABLE_BOOLARRAY :
				case VARIABLE_INTARRAY :
				case VARIABLE_STRINGARRAY :
					return VARIABLE_INT;
				default :
					PrintSemanticError("Operator %s cannot be applied to type %s\n",".length",VarTypeToString(temp1));
					return VARIABLE_INVALID;
			}
			return VARIABLE_INT;
			
		case TREE_LOADARRAY :
			temp1=GetExpressionType(currentNode->sons->node,methodTable); /*look up variable type*/
			temp2=GetExpressionType(currentNode->sons->next->node,methodTable); /*Look up indexer type*/
			
			if (temp2!=VARIABLE_INT) {
				/*indexer value is not an int*/
				PrintSemanticError("Operator %s cannot be applied to types %s, %s\n","[",VarTypeToString(temp1),VarTypeToString(temp2));
				return VARIABLE_INVALID;
			}
			
			/*Check if the indexed variable is a valid array type*/
			switch (temp1) {
				case VARIABLE_BOOLARRAY :
					return VARIABLE_BOOL;
				case VARIABLE_INTARRAY :
					return VARIABLE_INT;
				case VARIABLE_STRINGARRAY : /*Can't do anything with this particular array type*/
				default :
					/*operator [] being used in something not indexable*/
					PrintSemanticError("Operator %s cannot be applied to types %s, %s\n","[",VarTypeToString(temp1),VarTypeToString(temp2));
					return VARIABLE_INVALID;
			}
			
		case TREE_CALL :
			return CheckCall(currentNode,methodTable);
			
		case TREE_NEWINT :
			temp1 = GetExpressionType(currentNode->sons->node,methodTable);
			
			if (temp1!=VARIABLE_INT) { /*Size must be an int*/
				PrintSemanticError("Operator %s cannot be applied to type %s\n","new int",VarTypeToString(temp1));
				return VARIABLE_INVALID;
			}
			return VARIABLE_INTARRAY;
			
		case TREE_NEWBOOL :
			temp1 = GetExpressionType(currentNode->sons->node,methodTable);
			
			if (temp1!=VARIABLE_INT) { /*Size must be an int*/
				PrintSemanticError("Operator %s cannot be applied to type %s\n","new boolean",VarTypeToString(temp1));
				return VARIABLE_INVALID;
			}
			return VARIABLE_BOOLARRAY;
			
		case TREE_PARSEARGS : 
			temp1=GetExpressionType(currentNode->sons->node,methodTable); /*look up variable type*/
			temp2=GetExpressionType(currentNode->sons->next->node,methodTable); /*Look up indexer type*/
			if (temp2!=VARIABLE_INT) {
				/*indexer value is not an int*/
				PrintSemanticError("Operator %s cannot be applied to types %s, %s\n","Integer.parseInt",VarTypeToString(temp1),VarTypeToString(temp2));
				return VARIABLE_INVALID;
			}
			switch (temp1) { /*parseArgs only takes string arrays*/
				case VARIABLE_STRINGARRAY :
					return VARIABLE_INT; /*Parseargs always returns an int since it's Integer.parseInt*/
					break;
				default :
					/*operator [] being used in something not indexable*/
					PrintSemanticError("Operator %s cannot be applied to types %s, %s\n","Integer.parseInt",VarTypeToString(temp1),VarTypeToString(temp2));
					return VARIABLE_INVALID;
			}
			return VARIABLE_INVALID;
		case TREE_ID :
			return CheckID(currentNode,methodTable);
		case TREE_INTLIT :
			return CheckLitInt((char*)currentNode->args);
		case TREE_BOOLLIT :
			return CheckLitBool((char*)currentNode->args);
		default :
			return VARIABLE_INVALID;
	}
}

void CheckPrintStatement(TreeNode* printNode, MethodSymbols* methodTable) {
	enum VariableType varType = GetExpressionType(printNode->sons->node, methodTable);
	
	switch (varType) {
		case VARIABLE_BOOL :
		case VARIABLE_INT :
			return;
		default :
			PrintSemanticError("Incompatible type in %s statement (got %s, required %s or %s)\n","System.out.println",VarTypeToString(varType),VarTypeToString(VARIABLE_BOOL),VarTypeToString(VARIABLE_INT));
			return;
	}
	
}

void CheckNormalAssignment(char* variableName, TreeNode* expression, MethodSymbols* methodTable) {
	/*First check the method context*/
	Variable* var = LookupMethodVariable(variableName, methodTable);
	
	/*if we haven't found anything, look in the class*/
	if (var==NULL)
		var=LookupVariableClass(variableName,classTable);
	
	if (var==NULL) {
		PrintSemanticError("Cannot find symbol %s\n",variableName);
		return;
	}
	
	enum VariableType expressionType = GetExpressionType(expression,methodTable);
	if(var->type!=expressionType){
		PrintSemanticError("Incompatible type in assignment to %s (got %s, required %s)\n",variableName,VarTypeToString(expressionType),VarTypeToString(var->type));
		return;
	}
	return;
}

void CheckArrayAssignment(char* arrayName, TreeNode* indexExpression, TreeNode* assignedExpression, MethodSymbols* methodTable) {
	/*First check the method context*/
	Variable* var = LookupMethodVariable(arrayName, methodTable);
	
	/*if you haven't found anything, look in the class*/
	if (var==NULL)	
		var=LookupVariableClass(arrayName,classTable);
		
	if (var==NULL) {
		PrintSemanticError("Cannot find symbol %s\n",arrayName);
		return;
	}
	
	enum VariableType indexingType = GetExpressionType(indexExpression,methodTable);
	enum VariableType expressionType = GetExpressionType(assignedExpression,methodTable);
	if (indexingType!=VARIABLE_INT) {
		PrintSemanticError("Operator %s cannot be applied to types %s, %s\n","[",VarTypeToString(var->type),VarTypeToString(indexingType));
		return;
	}
	
	enum VariableType arrayExpectedType;
	switch(var->type) {
		case VARIABLE_INTARRAY :
			arrayExpectedType=VARIABLE_INT;
			break;
		case VARIABLE_BOOLARRAY :
			arrayExpectedType=VARIABLE_BOOL;
			break;
		default :
			PrintSemanticError("Operator %s cannot be applied to types %s, %s\n","[",VarTypeToString(var->type),VarTypeToString(indexingType));
			return;
	}
	if (arrayExpectedType!=expressionType) {
		PrintSemanticError("Incompatible type in assignment to %s[] (got %s, required %s)\n",arrayName,VarTypeToString(expressionType),VarTypeToString(arrayExpectedType));
		return;
	}
	return;
}


void CheckStatement(TreeNode* node, MethodSymbols* methodTable) {
	
	NodeList* temp;
	enum VariableType tempVarType;
	
	switch (node->type) {
		/*For compound statements check every statement inside them*/
		case TREE_COMPOUNDSTAT :
			for (temp=node->sons;temp!=NULL;temp=temp->next)
				CheckStatement(temp->node, methodTable);
			break;
		case TREE_STORE :
			/*Sends variable name and respective expression*/
			CheckNormalAssignment((char*)node->sons->node->args,node->sons->next->node, methodTable);
			break;
		case TREE_STOREARRAY :
			/*Sends variable name, index expression and assigned expression*/
			CheckArrayAssignment((char*)node->sons->node->args, node->sons->next->node, node->sons->next->next->node, methodTable);
			break;
		case TREE_RETURN :
			if (node->sons!=NULL) {
				tempVarType = GetExpressionType(node->sons->node, methodTable);
			}
			else {
				tempVarType = VARIABLE_VOID;
			}
			
			if (tempVarType!=methodTable->returnValue) {
				PrintSemanticError("Incompatible type in %s statement (got %s, required %s)\n","return",VarTypeToString(tempVarType),VarTypeToString(methodTable->returnValue));
			}
			break;
		case TREE_IFELSE :
			/*Check if condition*/
			tempVarType = GetExpressionType(node->sons->node, methodTable);
			if (tempVarType!=VARIABLE_BOOL) {
				PrintSemanticError("Incompatible type in %s statement (got %s, required %s)\n","if",VarTypeToString(tempVarType),"boolean");
			}
			
			/*Check if statement(s)*/
			CheckStatement(node->sons->next->node, methodTable);
			
			/*Check else statement(s)*/
			CheckStatement(node->sons->next->next->node, methodTable);
			break;
		case TREE_PRINT :
			CheckPrintStatement(node,methodTable);
			break;
		case TREE_WHILE :
			/*Check while condition*/
			tempVarType = GetExpressionType(node->sons->node, methodTable);
			if (tempVarType!=VARIABLE_BOOL) {
				PrintSemanticError("Incompatible type in %s statement (got %s, required %s)\n","while",VarTypeToString(tempVarType),"boolean");
			}
			
			/*Check while statement(s)*/
			CheckStatement(node->sons->next->node, methodTable);
			break;
		case TREE_NULL :
			return;
		default:
			break;
	}
	
	return;
}

void CheckSemanticError(TreeNode* class) {
	NodeList* currentMethod;
	/*Start after ID*/
	for (currentMethod=class->sons->next; currentMethod!=NULL; currentMethod=currentMethod->next) {
		if (currentMethod->node->type==TREE_METHODDECL) {
			/*bypass method type, name and params, directly to methodbody*/
			NodeList* methodBody=currentMethod->node->sons->next->next->next;
			MethodSymbols* methodTable = LookupMethodInClass((char*)currentMethod->node->sons->next->node->args,classTable);
			NodeList* currentStatement;
			for (currentStatement = methodBody->node->sons; currentStatement!=NULL; currentStatement=currentStatement->next) {
				/*No use checking variable declarations*/
				if (currentStatement->node->type!=TREE_VARDECL) {
					CheckStatement(currentStatement->node,methodTable);
				}
			}
			
		}
	}
	
	return;
}
