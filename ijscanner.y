%{
#include "ast.h"
#include "semantics.h"
#include "symbols.h"
#include <stdbool.h>
#include "structures.h"
#include "codeGen.h"

extern int tokenStartColumn,tokenStartLine;
extern char* yytext;

/*Avoid some warnings (not all, Bison sucks)*/
int yylex(void);

int yyerror (const char *s){
	printf ("Line %d, col %d: %s: %s\n", tokenStartLine, tokenStartColumn, s, yytext);
	return -1;
}

//Globals
TreeNode* program = NULL; /*AST root*/
ClassSymbols* classTable = NULL;
int semantic_errors = 0;
%}

%union {
	TreeNode* isNode;
	NodeList* isNodeList;
	
	char* string;
}

//Types
%type<isNode> Start Program MethodDecl functionType VarDecl FieldDecl 
%type<isNodeList> classBody varDeclList vars formalParamsList methodBody FormalParams statementList Args
%type<isNode> Type Statement Expr exprIndexable

//tokens
%token INT BOOL NEW IF ELSE WHILE PRINT PARSEINT CLASS PUBLIC STATIC VOID STRING DOTLENGTH RETURN
%token OCURV CCURV OBRACE CBRACE OSQUARE CSQUARE
%token LOGAND LOGOR LESSER GREATER EQUALITY DIFFERENT EQLESS EQGREAT PLUS MINUS MULT DIV MODULO
%token NOT ASSIGN SEMIC COMMA
%token RESERVED

//Literals
%token<string> ID
%token<string> INTLIT
%token<string> BOOLLIT

//precedences
%nonassoc THEN
%nonassoc ELSE

%left LOGOR
%left LOGAND

%left EQUALITY DIFFERENT
%left LESSER GREATER EQLESS EQGREAT

%left PLUS MINUS
%left MULT DIV MODULO

%right NOT
%left OSQUARE DOTLENGTH

%%

Start: Program {$$=$1;}
Program: 	  CLASS ID OBRACE classBody CBRACE {program=InsertClass(InsertTerminal($2,TREE_ID),$4); $$=program;}

classBody: 	  FieldDecl classBody {$$=InsertTreeNodeIntoList($1,$2);}
			| MethodDecl classBody {$$=InsertTreeNodeIntoList($1,$2);}
			| {$$ = NULL;}

FieldDecl: STATIC VarDecl {$$=$2;}

MethodDecl:   PUBLIC STATIC functionType ID OCURV FormalParams CCURV OBRACE methodBody CBRACE {$$=InsertMethod($3,InsertTerminal($4,TREE_ID),$6,$9);}

functionType:   Type {$$=$1;}
			  | VOID {$$=InsertTerminal(NULL,TREE_VOID);} 

methodBody: varDeclList statementList {$$=MergeLists($1,$2);}

FormalParams: formalParamsList {$$=$1;} 
			| STRING OSQUARE CSQUARE ID {$$=InsertMainArgs(InsertTerminal($4,TREE_ID));}
			| {$$=NULL;}

formalParamsList:   Type ID COMMA formalParamsList {$$ = InsertFormalParams($1,InsertTerminal($2,TREE_ID),$4);} 
				  | Type ID {$$ = InsertFormalParams($1,InsertTerminal($2,TREE_ID), NULL);} 
				  
varDeclList:  VarDecl varDeclList {$$=InsertTreeNodeIntoList($1,$2);} 
			| {$$ = NULL;} 

VarDecl: Type vars {$$=InsertVarDecl($1,$2);} 
vars:   ID COMMA vars {$$ = InsertTreeNodeIntoList(InsertTerminal($1,TREE_ID),$3);}
	  | ID SEMIC {$$ = InsertTreeNodeIntoList(InsertTerminal($1,TREE_ID),NULL);} 

Type: 	  INT {$$=InsertTerminal(NULL,TREE_INT);}
		| BOOL {$$=InsertTerminal(NULL,TREE_BOOL);}
		| INT OSQUARE CSQUARE {$$=InsertTerminal(NULL,TREE_INTARRAY);}
		| BOOL OSQUARE CSQUARE {$$=InsertTerminal(NULL,TREE_BOOLARRAY);}

statementList:  Statement statementList {$$=InsertTreeNodeIntoList($1,$2);}
			  | {$$=NULL;}

Statement: 	  OBRACE statementList CBRACE {$$=InsertBraces($2);}
			| IF OCURV Expr CCURV Statement %prec THEN {$$=InsertIfElse($3,$5,NULL);}
			| IF OCURV Expr CCURV Statement ELSE Statement {$$=InsertIfElse($3,$5,$7);}
			| WHILE OCURV Expr CCURV Statement {$$=InsertWhile($3,$5);}
			| PRINT OCURV Expr CCURV SEMIC {$$=InsertPrint($3);}
			| ID ASSIGN Expr SEMIC {$$=InsertStore(InsertTerminal($1,TREE_ID),$3);}
			| ID OSQUARE Expr CSQUARE ASSIGN Expr SEMIC {$$=InsertStoreArray(InsertTerminal($1,TREE_ID),$3,$6);}
			| RETURN SEMIC {$$=InitTreeNode(TREE_RETURN);}
			| RETURN Expr SEMIC {TreeNode* retNode=InitTreeNode(TREE_RETURN); retNode->sons=InitNodeList($2); $$=retNode;}
			

exprIndexable :	  ID							{$$ = InsertTerminal($1,TREE_ID);}
				| INTLIT						{$$ = InsertTerminal($1,TREE_INTLIT);}
				| BOOLLIT						{$$ = InsertTerminal($1,TREE_BOOLLIT);}
				| OCURV Expr CCURV				{$$ = $2;}
				| ID OCURV CCURV				{$$ = InsertCall(InsertTerminal($1,TREE_ID),NULL);}
				| ID OCURV Args CCURV			{$$ = InsertCall(InsertTerminal($1,TREE_ID),$3);}
				| Expr DOTLENGTH 				{$$ = InsertExpression($1,TREE_LENGTH,NULL);}
				| PARSEINT OCURV ID OSQUARE Expr CSQUARE CCURV {$$ = InsertParseArgs(InsertTerminal($3,TREE_ID),$5);}
				| exprIndexable OSQUARE Expr CSQUARE		{$$ = InsertExpression($1,TREE_LOADARRAY,$3);}
Expr: 	  
		  NEW BOOL OSQUARE Expr CSQUARE {$$ = InsertExpression($4,TREE_NEWBOOL,NULL);}
		| NEW INT OSQUARE Expr CSQUARE  {$$ = InsertExpression($4,TREE_NEWINT,NULL);}
		| Expr LOGAND Expr 				{$$ = InsertExpression($1,TREE_AND,$3);}
		| Expr LOGOR Expr 				{$$ = InsertExpression($1,TREE_OR,$3);}
		| Expr LESSER Expr 				{$$ = InsertExpression($1,TREE_LT,$3);}
		| Expr GREATER Expr 			{$$ = InsertExpression($1,TREE_GT,$3);}
		| Expr EQUALITY Expr 			{$$ = InsertExpression($1,TREE_EQ,$3);}
		| Expr DIFFERENT Expr 			{$$ = InsertExpression($1,TREE_NEQ,$3);}
		| Expr EQLESS Expr 				{$$ = InsertExpression($1,TREE_LEQ,$3);}
		| Expr EQGREAT Expr 			{$$ = InsertExpression($1,TREE_GEQ,$3);}
		| Expr PLUS Expr 				{$$ = InsertExpression($1,TREE_ADD,$3);}
		| Expr MINUS Expr 				{$$ = InsertExpression($1,TREE_SUB,$3);}
		| Expr MULT Expr 				{$$ = InsertExpression($1,TREE_MUL,$3);}
		| Expr DIV Expr 				{$$ = InsertExpression($1,TREE_DIV,$3);}
		| Expr MODULO Expr 				{$$ = InsertExpression($1,TREE_MOD,$3);}
		| PLUS Expr		%prec NOT 		{$$ = InsertExpression($2,TREE_PLUS,NULL);}
		| MINUS Expr	%prec NOT 		{$$ = InsertExpression($2,TREE_MINUS,NULL);}
		| NOT Expr						{$$ = InsertExpression($2,TREE_NOT,NULL);}
		| exprIndexable 				{$$ = $1;}
		
		
Args:   Expr COMMA Args {$$=InsertTreeNodeIntoList($1,$3);}
	  | Expr {$$=InsertTreeNodeIntoList($1,NULL);}

%%
/*handle indentation*/
void indentToLevel(int level) {
	int i;
	for (i=0; i<level;i++) {
		printf("  ");
	}
}

void ShowAST(TreeNode* node,int indentLevel) {
	indentToLevel(indentLevel);
	indentLevel+=1;
	switch(node->type) {
		case TREE_PROGRAM :
			printf("Program\n");
			break;
		case TREE_VARDECL :
			printf("VarDecl\n");
			break;
		case TREE_METHODDECL :
			printf("MethodDecl\n");
			break;
		case TREE_METHODPARAMS :
			printf("MethodParams\n");
			break;
		case TREE_METHODBODY :
			printf("MethodBody\n");
			break;
		case TREE_PARAMDECLARATION :
			printf("ParamDeclaration\n");
			break;
		case TREE_COMPOUNDSTAT :
			printf("CompoundStat\n");
			break;
		case TREE_IFELSE :
			printf("IfElse\n");
			break;
		case TREE_PRINT :
			printf("Print\n");
			break;
		case TREE_RETURN :
			printf("Return\n");
			break;
		case TREE_STORE :
			printf("Store\n");
			break;
		case TREE_STOREARRAY :
			printf("StoreArray\n");
			break;
		case TREE_WHILE :
			printf("While\n");
			break;
		case TREE_OR :
			printf("Or\n");
			break;
		case TREE_AND :
			printf("And\n");
			break;
		case TREE_EQ :
			printf("Eq\n");
			break;
		case TREE_NEQ :
			printf("Neq\n");
			break;
		case TREE_LT :
			printf("Lt\n");
			break;
		case TREE_GT :
			printf("Gt\n");
			break;
		case TREE_LEQ :
			printf("Leq\n");
			break;
		case TREE_GEQ :
			printf("Geq\n");
			break;
		case TREE_ADD :
			printf("Add\n");
			break;
		case TREE_SUB :
			printf("Sub\n");
			break;
		case TREE_MUL :
			printf("Mul\n");
			break;
		case TREE_DIV :
			printf("Div\n");
			break;
		case TREE_MOD :
			printf("Mod\n");
			break;
		case TREE_NOT :
			printf("Not\n");
			break;
		case TREE_MINUS :
			printf("Minus\n");
			break;
		case TREE_PLUS :
			printf("Plus\n");
			break;
		case TREE_LENGTH :
			printf("Length\n");
			break;
		case TREE_LOADARRAY :
			printf("LoadArray\n");
			break;
		case TREE_CALL :
			printf("Call\n");
			break;
		case TREE_NEWINT :
			printf("NewInt\n");
			break;
		case TREE_NEWBOOL :
			printf("NewBool\n");
			break;
		case TREE_PARSEARGS :
			printf("ParseArgs\n");
			break;
		case TREE_INT:
			printf("Int\n");
			break;
		case TREE_BOOL:
			printf("Bool\n");
			break;
		case TREE_INTARRAY:
			printf("IntArray\n");
			break;
		case TREE_BOOLARRAY:
			printf("BoolArray\n");
			break;
		case TREE_STRINGARRAY:
			printf("StringArray\n");
			break;
		case TREE_VOID:
			printf("Void\n");
			break;
		case TREE_ID :
			printf("Id(%s)\n",(char*)(node->args));
			break;
		case TREE_INTLIT:
			printf("IntLit(%s)\n",(char*)(node->args));
			break;
		case TREE_BOOLLIT:
			printf("BoolLit(%s)\n",(char*)(node->args));
			break;
		case TREE_NULL:
			printf("Null\n");
			break;
		default :
			printf("Unimplemented node\n");
			break;
	}
	
	NodeList* son = node->sons;
	while ( son!=NULL ) {
		if (son->node==NULL) {
			indentToLevel(indentLevel);
			printf("NodeList item with no node\n");
			return;
		}
		ShowAST(son->node,indentLevel);
		son=son->next;
	}
	
	return;
}

void ShowSymbols(ClassSymbols* class) {
	
	/*Show class table*/
	printf("===== Class %s Symbol Table =====\n",class->name);
	MethodAndVarsSymbolsList* item = class->methodsAndVars;
	while (item!=NULL) {
		if (item->variable!=NULL) {
			printf("%s\t%s\n",item->variable->name,VarTypeToString(item->variable->type));
		}
		if (item->method!=NULL) {
			printf("%s\tmethod\n",item->method->name);
		}
		item=item->next;
	}
	
	/*Show methods table*/
	for (item = class->methodsAndVars; item!=NULL; item=item->next) {
		if (item->method!=NULL) {
			MethodSymbols* method = item->method;
			printf("\n===== Method %s Symbol Table =====\n",method->name);
			printf("return\t%s\n",VarTypeToString(method->returnValue));
			
			VariableList* vars = method->variables;
			int varNumber = 0;
			for (; vars!=NULL; vars=vars->next,varNumber++) {
				if (varNumber < method->numberOfParams) { /*Is a parameter*/
					printf("%s\t%s\tparam\n",vars->var->name,VarTypeToString(vars->var->type));
				}
				else {
					printf("%s\t%s\n",vars->var->name,VarTypeToString(vars->var->type));
				}
			}
		}
		
	}
}

int main(int argc, char *argv[]){
	
	bool printAst = false, printSymbols = false;
	if ( argc >= 2 ) {
		int i;
		for (i = 1; i<argc; i++) {
			if(strcmp(argv[i],"-t")==0) {
				printAst = true;
			}
			/*Only show symbols if we have no semantic errors*/
			else if(strcmp(argv[i],"-s")==0) {
				printSymbols = true;
				
			}
		}
	}
	
	
	if ( yyparse()!=0 ) { /*lexical errors!*/
		return 0;
	}
	if (printAst) {
		ShowAST(program,0);
		if (!printSymbols)
			return 0;
	}
	
	BuildSymbolTable(program);
	CheckSemanticError(program);
	
	if(semantic_errors<=0) { /*Success, no semantic errors!*/
		if (printSymbols)
			ShowSymbols(classTable);
		
		//GenerateCode(classTable,program);
	}
	
	return 0;
}
