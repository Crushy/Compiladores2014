#include "symbols.h"

extern ClassSymbols* classTable;

/*
* Returns null if symbol exists in class table
* returns last symbol so we can piggyback this lookup instead
* of doing a new one
*/
MethodAndVarsSymbolsList* CheckExistingClassAttribute( char* name, ClassSymbols* classSymb ) {
	MethodAndVarsSymbolsList* last = NULL;
	MethodAndVarsSymbolsList* current;
	
	for (current=classSymb->methodsAndVars; current!=NULL; last=current, current=current->next) {
		if ( current->method!=NULL ) {
			if (strcmp(current->method->name,name)==0) {
				PrintSemanticError("Symbol %s already defined\n",name);
			}
		}
		else if ( current->variable!=NULL) {
			if (strcmp(current->variable->name,name)==0) {
				PrintSemanticError("Symbol %s already defined\n",name);
			}
		}
	}
	return last;
}

/*
* Returns null if symbol exists in method table
* returns last symbol so we can piggyback this lookup instead
* of doing a new one
*/
VariableList* CheckExistingMethodAttribute(char* name, MethodSymbols* methodSymb) {
	VariableList* last = NULL;
	VariableList* current;
	
	/*Lookup in own method*/
	for (current=methodSymb->variables; current!=NULL; last=current, current=current->next) {
		if (strcmp(current->var->name,name)==0) {
			PrintSemanticError("Symbol %s already defined\n",name);
		}
	}
	
	return last;
}

void AddNewClassVariables(ClassSymbols* classTable, TreeNode* vardecl) {
	/*Process TreeNode type into variable type*/
	enum VariableType variablesType = ProcessType(vardecl->sons->node->type); 
	NodeList* ids = vardecl->sons->next;
	while ( ids!=NULL ) {
		MethodAndVarsSymbolsList* lastNode=CheckExistingClassAttribute((char*)(ids->node->args),classTable);
		
		MethodAndVarsSymbolsList* newVariableListItem = CreateMethodSymbolsAndVarsSymbolsList();
		
		newVariableListItem->variable = CreateVariable((char*)(ids->node->args),variablesType);
		
		/*Handle first node in list case*/
		if (lastNode!=NULL)
			lastNode->next=newVariableListItem;
		else { /*We're the first node*/
			classTable->methodsAndVars=newVariableListItem;
		}
		
		ids = ids->next;
	}
	return;
}

void AddNewMethodTable(ClassSymbols* classTable, TreeNode* methodDecl) {
	MethodAndVarsSymbolsList* newMethodListItem = CreateMethodSymbolsAndVarsSymbolsList();
	
	/*Process return type and name*/
	MethodSymbols* newMethod = CreateMethodSymbols(ProcessType(methodDecl->sons->node->type),(char*)(methodDecl->sons->next->node->args));
	newMethodListItem->method=newMethod;
	
	MethodAndVarsSymbolsList* lastNode = CheckExistingClassAttribute((char*)(methodDecl->sons->next->node->args),classTable);
	
	/*Process method params*/
	TreeNode* methodParams = methodDecl->sons->next->next->node;
	NodeList* paramDeclarations = methodParams->sons;
	VariableList* lastItem=NULL;
	int numParams=0;
	
	/*Handle method parameter*/
	while( paramDeclarations!=NULL ) {
		/*ParamDeclaration->(Type,ID)*/
		char* name=(char*)paramDeclarations->node->sons->next->node->args;
		lastItem=CheckExistingMethodAttribute(name,newMethod);
		
		/*ParamDeclaration->(Type)*/
		enum VariableType type = ProcessType(paramDeclarations->node->sons->node->type);
		
		VariableList* varList=CreateVariableList();
		varList->var=CreateVariable(name,type);
		
		if (newMethod->variables==NULL) {
			newMethod->variables=varList;
		}
		else {
			lastItem->next = varList;
		}
		
		lastItem = varList;
		
		paramDeclarations=paramDeclarations->next;
		numParams++;
	}
	newMethod->numberOfParams = numParams;
	
	
	/*Handle local variables*/
	TreeNode* methodBody = methodDecl->sons->next->next->next->node;
	NodeList* varDecls = methodBody->sons;
	
	/*
	* variables are at the beggining of the method 
	* Therefore, stop if we are no longer getting variables
	*/
	while(varDecls!=NULL && (varDecls->node->type==TREE_VARDECL)) {
		enum VariableType variablesType = ProcessType(varDecls->node->sons->node->type); //Process TreeNode into variable type
		
		NodeList* ids = varDecls->node->sons->next; //get first ID
		while ( ids!=NULL ) {
			lastItem=CheckExistingMethodAttribute((char*)(ids->node->args),newMethod);
			VariableList* object = CreateVariableList();
			
			object->var = CreateVariable((char*)(ids->node->args),variablesType);
			
			if (newMethod->variables==NULL) {
				newMethod->variables=object;
			}
			else {
				lastItem->next = object;
			}
			
			ids = ids->next;
		}
		
		varDecls=varDecls->next;
	}
	
	/*Init methodsTable if it hasn't been initiated already*/
	if (lastNode!=NULL)
		lastNode->next=newMethodListItem;
	else { /*We're the first node*/
		classTable->methodsAndVars=newMethodListItem;
	}
	
	return;
}

void BuildSymbolTable(TreeNode* classRoot) {
	/*Build up lists of variable and method declarations*/
	classTable=CreateClassSymbols((char*)(classRoot->sons->node->args));  /*class's ID node*/
	
	NodeList* temp; /*start after ID*/
	for ( temp= classRoot->sons->next; temp!=NULL; temp=temp->next) {
		/* Class variables */
		if (temp->node->type==TREE_VARDECL) {
			AddNewClassVariables(classTable,temp->node);
		}
		else if ( temp->node->type == TREE_METHODDECL ) {
			AddNewMethodTable(classTable,temp->node);
		}
		
		else {
			printf("Critical failure, AST has invalid nodes\n");
			exit(EXIT_FAILURE);//irrelevant but also definitively invalid, let's be picky
		}
	}
	return;
}
