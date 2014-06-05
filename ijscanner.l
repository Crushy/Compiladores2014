%{
int yylex(void);

#include "structures.h"
#include <string.h> /*strdup*/
#include "y.tab.h"

int tokenStartLine=1,tokenStartColumn=1,lineNumber=1, columnNumber = 1;
int commentLine,commentColumn;

#define INCREMENTCOLUMN {tokenStartColumn=columnNumber; columnNumber+=yyleng;}
#define RESETCOLUMNN {columnNumber=1; tokenStartColumn=1;}
#define CHANGELINE {lineNumber++; tokenStartLine=lineNumber;}

%}

decDigit	[0-9]
letter		[a-zA-Z_$]
hexDigit	[a-fA-F0-9]
lineChange 	("\r\n"|"\n")

%X COMMENT

%%
[ \t]+ {/*Ignore Whitespace*/ INCREMENTCOLUMN tokenStartColumn=columnNumber; /*Whitespace isn't a token*/}
{lineChange} 				{CHANGELINE RESETCOLUMNN}


<INITIAL>"//".*	{INCREMENTCOLUMN RESETCOLUMNN}

<INITIAL>"/*" 			{commentLine=lineNumber; commentColumn=columnNumber; INCREMENTCOLUMN BEGIN(COMMENT);}
<COMMENT>"*/" 			{INCREMENTCOLUMN BEGIN(INITIAL); tokenStartColumn=columnNumber; /*Because comments aren't tokens*/}
<COMMENT><<EOF>> 		{printf("Line %d, col %d: unterminated comment\n",commentLine,commentColumn); BEGIN(INITIAL);}
<COMMENT>{lineChange} 	{CHANGELINE RESETCOLUMNN}
<COMMENT>. 				{/*"eat up" everything else*/ INCREMENTCOLUMN tokenStartColumn=columnNumber;}


"int"     			{ INCREMENTCOLUMN return INT;}
"boolean" 			{ INCREMENTCOLUMN return BOOL;}
"new" 				{ INCREMENTCOLUMN return NEW;}
"if" 				{ INCREMENTCOLUMN return IF;}
"else" 				{ INCREMENTCOLUMN return ELSE;}
"while" 			{ INCREMENTCOLUMN return WHILE;}
"System.out.println" { INCREMENTCOLUMN return PRINT;}
"Integer.parseInt" 	{ INCREMENTCOLUMN return PARSEINT;}
"class" 			{ INCREMENTCOLUMN return CLASS;}
"public" 			{ INCREMENTCOLUMN return PUBLIC;}
"static" 			{ INCREMENTCOLUMN return STATIC;}
"void" 				{ INCREMENTCOLUMN return VOID;}
"String" 			{ INCREMENTCOLUMN return STRING;}
".length" 			{ INCREMENTCOLUMN return DOTLENGTH;}
"return" 			{ INCREMENTCOLUMN return RETURN;}

"(" 					{ INCREMENTCOLUMN return OCURV;}
")" 					{ INCREMENTCOLUMN return CCURV;}
"{" 					{ INCREMENTCOLUMN return OBRACE;}
"}" 					{ INCREMENTCOLUMN return CBRACE;}
"[" 					{ INCREMENTCOLUMN return OSQUARE;}
"]" 					{ INCREMENTCOLUMN return CSQUARE;}

"&&" 					{ INCREMENTCOLUMN return LOGAND;}
"||"					{ INCREMENTCOLUMN return LOGOR;}
"<"						{ INCREMENTCOLUMN return LESSER;}
">"						{ INCREMENTCOLUMN return GREATER;}
"=="					{ INCREMENTCOLUMN return EQUALITY;}
"!="					{ INCREMENTCOLUMN return DIFFERENT;}
"<="					{ INCREMENTCOLUMN return EQLESS;}
">=" 					{ INCREMENTCOLUMN return EQGREAT;}
"+"						{ INCREMENTCOLUMN return PLUS;}
"-" 					{ INCREMENTCOLUMN return MINUS;}
"*"						{ INCREMENTCOLUMN return MULT;}
"/"						{ INCREMENTCOLUMN return DIV;}
"%" 					{ INCREMENTCOLUMN return MODULO;}

"!" 					{ INCREMENTCOLUMN return NOT;}
"=" 					{ INCREMENTCOLUMN return ASSIGN;}
";" 					{ INCREMENTCOLUMN return SEMIC;}
"," 					{ INCREMENTCOLUMN return COMMA;}

"enum"|"float"|"abstract"|"continue"|"for"|"new"|"switch"|"assert"|"default"|"goto"|"package"|"synchronized"|"do"|"private"|"this"|"break"|"double"|"implements"|"protected"|"throw"|"byte"|"import"|"public"|"throws"|"case"|"edigit"|"instanceof"|"transient"|"catch"|"extends"|"short"|"try"|"char"|"final"|"interface"|"static"|"void"|"finally"|"long"|"strictfp"|"volatile"|"const"|"native"|"super"|"null"|"--"|"++" {INCREMENTCOLUMN return RESERVED;}

0x{hexDigit}+|{decDigit}+		{yylval.string=strdup(yytext); INCREMENTCOLUMN return INTLIT;}
("true"|"false") 				{yylval.string=strdup(yytext); INCREMENTCOLUMN return BOOLLIT;}
{letter}({letter}|{decDigit})* 	{yylval.string=strdup(yytext); INCREMENTCOLUMN return ID;}

. {
	printf("Line %d, col %d: illegal character (\'%c\')\n",lineNumber,columnNumber,yytext[0]);
	INCREMENTCOLUMN
	tokenStartColumn=columnNumber;/*Illegal characters aren't tokens*/ 
} 

%%
int yywrap() {
	return 1;
}
