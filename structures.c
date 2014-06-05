#include "structures.h"

char* VarTypeToString(enum VariableType type) {
	switch(type) {
		case VARIABLE_BOOL :
			return "boolean";
		case VARIABLE_INT :
			return "int";
		case VARIABLE_INTARRAY :
			return "int[]";
		case VARIABLE_BOOLARRAY :
			return "boolean[]";
		case VARIABLE_VOID :
			return "void";
		case VARIABLE_STRINGARRAY :
			return "String[]";
		case VARIABLE_INVALID :
			return "INVALID";
		default :
			return "NOT A VALID TYPE";
	}
}

enum VariableType ProcessType(enum NodeType type) {
	switch (type) {
		case TREE_INT:
			return VARIABLE_INT;
		case TREE_BOOL:
			return VARIABLE_BOOL;
		case TREE_INTARRAY:
			return VARIABLE_INTARRAY;
		case TREE_BOOLARRAY:
			return VARIABLE_BOOLARRAY;
		case TREE_STRINGARRAY:
			return VARIABLE_STRINGARRAY;
		case TREE_VOID:
			return VARIABLE_VOID;
		default:
			return VARIABLE_INVALID;
	}
}


MethodAndVarsSymbolsList* CreateMethodSymbolsAndVarsSymbolsList() {
	MethodAndVarsSymbolsList* object = (MethodAndVarsSymbolsList*)malloc(sizeof(MethodAndVarsSymbolsList));
	object->method=NULL;
	object->variable=NULL;
	object->next=NULL;
	return object;
}

VariableList* CreateVariableList() {
	VariableList* object = (VariableList*)malloc(sizeof(VariableList));
	object->var=NULL;
	object->next=NULL;
	return object;
}

Variable* CreateVariable(char* name,enum VariableType type) {
	Variable* var = (Variable*)malloc(sizeof(Variable));
	/*TODO Maybe copy this string instead of keeping a reference 
	 * so we can delete the AST yet keep the symbol table*/
	var->name = name; 
	var->type = type;
	return var;
}

MethodSymbols* CreateMethodSymbols(enum VariableType type,char* name) {
	MethodSymbols* meth = (MethodSymbols*)malloc(sizeof(MethodSymbols));
	/*TODO Maybe copy this string instead of keeping a reference 
	 * so we can delete the AST yet keep the symbol table*/
	meth->name = name;
	meth->returnValue = type;
	meth->variables=NULL;
	meth->numberOfParams=0;
	return meth;
}

ClassSymbols* CreateClassSymbols(char* name) {
	ClassSymbols* class = (ClassSymbols*)malloc(sizeof(ClassSymbols));
	class->name=name;
	class->methodsAndVars=NULL;
	return class;
}

/*Lookups*/
MethodSymbols* LookupMethodInClass( char* name, ClassSymbols* classTable ) {
	MethodAndVarsSymbolsList* current;
	
	for (current=classTable->methodsAndVars; current!=NULL; current=current->next) {
		if ( current->method!=NULL ) {
			if (strcmp(current->method->name,name)==0) {
				return current->method;
			}
		}
	}
	return NULL;
}

Variable* LookupVariableClass( char* name, ClassSymbols* classTable ) {
	MethodAndVarsSymbolsList* current;
	
	for (current=classTable->methodsAndVars; current!=NULL; current=current->next) {
		if ( current->variable!=NULL ) {
			if (strcmp(current->variable->name,name)==0) {
				return current->variable;
			}
		}
	}
	return NULL;
}

Variable* LookupMethodVariable(char* name, MethodSymbols* methodSymb) {
	VariableList* current;
	
	for (current=methodSymb->variables; current!=NULL; current=current->next) {
		if (strcmp(current->var->name,name)==0) {
			return current->var;
		}
	}
	return NULL;
}
