#pragma once

#include <stdlib.h> /*malloc*/
#include <string.h> /* for strcmp */

/*AST*/
enum NodeType{
	TREE_PROGRAM,
	
	TREE_VARDECL, TREE_METHODDECL, TREE_METHODPARAMS, TREE_METHODBODY, TREE_PARAMDECLARATION,
	
	TREE_COMPOUNDSTAT,TREE_IFELSE,TREE_PRINT,TREE_RETURN,TREE_STORE,TREE_STOREARRAY,TREE_WHILE,
	TREE_OR,TREE_AND,TREE_EQ,TREE_NEQ,TREE_LT,TREE_GT,TREE_LEQ,TREE_GEQ,TREE_ADD,TREE_SUB,TREE_MUL,TREE_DIV,TREE_MOD,TREE_NOT,TREE_MINUS,TREE_PLUS,TREE_LENGTH,TREE_LOADARRAY,TREE_CALL,TREE_NEWINT,TREE_NEWBOOL,TREE_PARSEARGS,
	
	TREE_INT,TREE_BOOL,TREE_INTARRAY,TREE_BOOLARRAY,TREE_STRINGARRAY,TREE_VOID,TREE_ID,TREE_INTLIT,TREE_BOOLLIT,
	
	TREE_NULL
};

typedef struct _Node {
	struct _NodeList* sons;
	enum NodeType type;
	char* args;
}TreeNode;

typedef struct _NodeList {
	struct _Node* node;
	struct _NodeList* next;
}NodeList;

/*Symbol table*/
enum VariableType{
	VARIABLE_BOOL,
	VARIABLE_INT,
	VARIABLE_INTARRAY,
	VARIABLE_BOOLARRAY,
	VARIABLE_VOID,
	VARIABLE_STRINGARRAY,
	VARIABLE_INVALID
};

typedef struct _Variable{
	enum VariableType type;
	char *name;
}Variable;

typedef struct _VariableList{
	struct _Variable* var;
	struct _VariableList* next;
}VariableList;

typedef struct _MethodSymbols {
	char* name;
	int numberOfParams; /*first numberOfParams Variables are method parameters*/
	enum VariableType returnValue;
	struct _VariableList* variables;
}MethodSymbols;

typedef struct _MethodAndVarsSymbolsList {
	struct _MethodSymbols* method;
	struct _Variable* variable;
	struct _MethodAndVarsSymbolsList* next;
}MethodAndVarsSymbolsList;

typedef struct _ClassSymbols {
	char* name;
	struct _MethodAndVarsSymbolsList* methodsAndVars;
}ClassSymbols;

char* VarTypeToString(enum VariableType type);
enum VariableType ProcessType(enum NodeType type);

MethodAndVarsSymbolsList* CreateMethodSymbolsAndVarsSymbolsList();
VariableList* CreateVariableList();
Variable* CreateVariable(char* name,enum VariableType type);
MethodSymbols* CreateMethodSymbols(enum VariableType type,char* name);
ClassSymbols* CreateClassSymbols(char* name);
MethodSymbols* LookupMethodInClass( char* name, ClassSymbols* classTable );
Variable* LookupVariableClass( char* name, ClassSymbols* classTable );
Variable* LookupMethodVariable(char* name, MethodSymbols* methodSymb );
