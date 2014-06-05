#Relatório do Projeto

##Introdução
Foram submetidas todas as metas para o Mooshak com avaliação máxima, exceto a 3 (geração de código) que não foi realizada, sob o nome “pcaetano”. Da última meta entregue para a entrega final (no PosMeta2) houve apenas uma pequena limpeza de código não utilizado e refactorização de alguns nomes.
Estrutura geral do projeto
O projeto tenta, acima de tudo, manter uma estrutura lógica e extremamente modular. Foi dada prioridade a ter resultados com um mínimo de código desnecessário e não foi dada uma grande ênfase ao uso de estruturas de dados mais robustas. Como tal, foi feito um uso liberal de listas ligadas e procuras de complexidade O(n). 
Dada a intenção de modularizar ao máximo o código, tentou-se o mais possível separar as diferentes componentes do programa. Uma consequência infeliz deste facto é o (relativamente) grande número de ficheiros de código fonte:

ijscanner.y
Funções para impressão da árvore, tabela de símbolos e algumas outras funções usadas diretamente no “parsing”. Ponto de entrada do programa.
structures.c structures.h 
Estruturas de dados e funções relativas à sua manipulação direta ou conversão entre estruturas. Dado o facto de serem usadas na maioria das componentes do programa as estruturas não foram separadas por ficheiros diferentes.
ast.c ast.h 
Geração da Árvore de Sintaxe Abstrata
symbols.h symbols.c 
Geração da Tabela de Símbolos e deteção de erros diretamente relacionados com a construção da mesma.
semantics.h semantics.c 
Análise Semântica e exibição de Erros Semânticos.  Tradução de literais (de strings para o seu valor semântico).
codeGen.h codeGen.c
Geração de código llvm

Cada ficheiro contém apenas “#includes” para o que seja estritamente necessário no código contido, juntamente com comentários a explicar a razão para serem necessários. No código em si seguiu-se a filosofia amplamente aceite de ter nomes auto-explicativos e comentários a explicitar o que possa não ser claro, juntamente com pequenas justificações da razão para ser usada uma solução particularmente menos óbvia.
Foi também incluído um makefile capaz de compilar o programa de raiz, com as funções adicionais de clean (limpar a diretoria de ficheiros resultantes da compilação) e zip (criar o zip a enviar para o mooshak).
##Análise Lexical (Meta 1)
O analisador lexical funciona de maneira relativamente simples. A sua função é meramente a de detetar os Tokens da linguagem e fazer a deteção antecipada de dois tipos de erros: Lexicais e Comentários não-terminados. São também contabilizadas as linhas e o índice da coluna atual do input a ser analisado mediante o uso de duas variáveis globais: lineNumber e columnNumber para armazenar a linha e coluna  atual.
###Tokens lexicais
Lista de tokens lexicais
Palavras Reservadas
INT int
BOOL bool
NEW new
IF if
ELSE else 
WHILE while
PRINT print
PARSEINT Integer.parseInt
CLASS class
PUBLIC public
STATIC static
VOID void
STRING string
DOTLENGTH .length 
RETURN return
RESERVED (lista de palavras reservadas do Java não utilizadas)
Caracteres
OCURV (
CCURV )
OBRACE {
CBRACE }
OSQUARE [ 
CSQUARE ]
SEMIC ;
COMMA ,

Literais (valores)
ID {letter}({letter}|{decDigit})*
INTLIT 0x{hexDigit}+|{decDigit}+
BOOLLIT ("true"|"false") 
Operações
LOGAND  &
LOGOR |
LESSER <
GREATER > 
EQUALITY ==
DIFFERENT !=
EQLESS <=
EQGREAT >= 
PLUS +
MINUS -
MULT *
DIV /
MODULO %
NOT !
ASSIGN =

Adicionalmente existem tokens auxiliares usados para simplificar as regras de deteção de literais. Estes são:
decDigit	[0-9]
hexDigit	[a-fA-F0-9]
letter	[a-zA-Z_$]
As categorias servem apenas para indicar o tipo do token usado e não representam de maneira alguma a sua função na linguagem.
Valores
Além de identificar tokens, o Lex envia o valor de cada literal na forma de uma string (char*), recorrendo à union definida em “ijscanner.y”. Os restantes tokens lexicais cumprem apenas a função de keywords e, como tal, não estão explicitamente associados a valores.
###Erros Lexicais
A deteção de qualquer símbolo incrementa um número de coluna para a impressão do local de erros lexicais. Como tal, cada token está associado a uma macro INCREMENTCOLUMN cuja função é meramente incrementar o contador de acordo com o tamanho de cada token. Foi também inserida uma produção no início das regras, com o objetivo de contabilizar (e ignorar) caracteres ignoráveis, nomeadamente o “whitespace”:
[ \t]+ {INCREMENTCOLUMN}
A deteção de erros propriamente dita é unicamente feita por esta produção no final do ficheiro lex:
. {printf("Line %d, col %d: illegal character        (\'%c\')\n",yylineno,columnNumber,yytext[0]);} 
Que assegura a deteção correta de caracteres ilegais, partindo da assunção que tudo o que não é um token da linguagem ou “whitespace” pode ser considerado um erro lexical.
Comentários
Single Line
Comentários single-line são inteiramente tidos em conta recorrendo apenas à seguinte produção:
"//".* 	{INCREMENTCOLUMN RESETCOLUMNN ;}
Que assegura a absorção de todos os caracteres exceto os \n .
###Multi Line
O texto nesta secção foca-se na metodologia usada para a implementação de comentários clássicos de C ( /* ... */ ).
Para deteção adequada de comentários decidiu-se usar estados do Lex. Este método traz a clara vantagem de permitir-nos isolar a maneira como lidamos com o conteúdo dos comentários do resto das produções da linguagem. 
Dada a necessidade de, em caso de erros do tipo comentários não terminado, ser necessário imprimir o início do comentário1, foram criadas duas variáveis globais cuja única função é armazenar a linha e coluna do token “/*” que inicia o comentário atual.
<INITIAL>"/*" 			{ BEGIN(COMMENT); commentLine=lineNumber; commentColumn=columnNumber; INCREMENTCOLUMN}
<COMMENT>"*/" 			{ BEGIN(INITIAL); INCREMENTCOLUMN}
<COMMENT><<EOF>> 		{ printf("Line %d, col %d: unterminated comment\n",commentLine,commentColumn); BEGIN(INITIAL);}
<COMMENT>{lineChange} 	{lineNumber++; RESETCOLUMNN}
<COMMENT>. 			{/*"eat up" everything else*/ INCREMENTCOLUMN}
Foi explicitado o estado para a condição de começo (<INITIAL>) apenas para tornar mais claro as condições iniciais dos comentários. O estado <COMMENT> é usado em regime de exclusividade (não segue as regras de mais nenhum estado) e, portanto, tornou-se necessário duplicar o código para incrementação do número das linhas e colunas.
Análise Sintática (Meta 2)
Foram feitas algumas modificações ao ficheiro do Lex, nomeadamente a conversão dos prints para devolução de tokens e a criação de variáveis globais para armazenar o início de cada token visto a análise lexical considerar apenas o carácter atual. Foram também acrescentados os tokens "--" e "++", como pedido no enunciado.
Gramática
Apresenta-se a gramática tal e qual está representada no ficheiro ijscanner.y. A gramática segue uma convenção: regras originais estão representadas como no enunciado (com maiúsculas no início) e regras introduzidas começam com letras minúsculas. Na seguinte tabela as regras novas estão sublinhadas e cada nome entre “<>” indica o tipo que a produção devolve (explicado em maior detalhe na secção sobre a construção da Árvore Sintática).
Regra
Produção
Start <TreeNode>
Program
Program <TreeNode>
CLASS ID OBRACE classBody CBRACE
classBody <NodeList>
FieldDecl classBody 
MethodDecl classBody
ε
FieldDecl <TreeNode>
STATIC VarDecl
MethodDecl <TreeNode>
PUBLIC STATIC functionType ID OCURV FormalParams CCURV OBRACE methodBody CBRACE
functionType <TreeNode> 
Type
VOID 
methodBody <NodeList>
varDeclList statementList 
FormalParams <NodeList>
FormalParamsList
STRING OSQUARE CSQUARE ID 
ε
formalParamsList <NodeList> 
Type ID COMMA formalParamsList
Type ID  
varDeclList <NodeList>
VarDecl varDeclList
ε
VarDecl <TreeNode>
Type vars
vars <NodeList>
ID COMMA vars
ID SEMIC 
Type <TreeNode>
INT 
BOOL
INT OSQUARE CSQUARE
BOOL OSQUARE CSQUARE 
statementList <NodeList> 
 Statement statementList
ε
Statement <TreeNode>
OBRACE statementList CBRACE
IF OCURV Expr CCURV Statement %prec THEN
IF OCURV Expr CCURV Statement ELSE Statement
WHILE OCURV Expr CCURV Statement
PRINT OCURV Expr CCURV SEMIC
ID ASSIGN Expr SEMIC
ID OSQUARE Expr CSQUARE ASSIGN Expr SEMIC
RETURN SEMIC
RETURN Expr SEMIC
exprIndexable <TreeNode>
ID
INTLIT
BOOLLIT
OCURV Expr CCURV 
ID OCURV CCURV
ID OCURV Args CCURV
Expr DOTLENGTH %prec NEWARRAY
PARSEINT OCURV ID OSQUARE Expr CSQUARE CCURV
exprIndexable OSQUARE Expr CSQUARE
Expr <TreeNode>
NEW BOOL OSQUARE Expr CSQUARE
NEW INT OSQUARE Expr CSQUARE
Expr LOGAND Expr
Expr LOGOR Expr
Expr LESSER Expr
Expr GREATER Expr
Expr EQUALITY Expr
Expr DIFFERENT Expr
Expr EQLESS Expr
Expr EQGREAT Expr
Expr PLUS Expr
Expr MINUS Expr
Expr MULT Expr
Expr DIV Expr
Expr MODULO Expr
PLUS Expr	   %prec NOT
MINUS Expr %prec NOT
NOT Expr
exprIndexable 	
Args <NodeList>
Expr COMMA Args
Expr

As novas produções com o sufixo “List” destinam-se unicamente a permitir a construção de listas.
Arrays Multi-dimensionais
O problema da deteção de indexação foi resolvido mediante o uso de uma produção extra, exprIndexable. Esta produção contém apenas expressões que podem ser sujeitas a indexação (juntamente com literais inteiros e booleanos para apresentar mensagens de erro coerentes com o esperado pelo Mooshak).
###Precedências
Para a análise sintática foi necessário, para além da transcrição da gramática do enunciado para o formato do YACC, a definição de prioridades, obtidas mediante a consulta da documentação do java relativa à prioridade de operações. Houve também alguns casos específicos que necessitaram de tratamento especial.
Gostaria de chamar à atenção que as produções PLUS Expr, MINUS Expr e NOT Expr usam a mesma precedência por representarem, respetivamente, a deteção do sinal positivo, negativo, e a operação de negação aplicadas a uma expressão (ao invés da adição ou subtração que possuem prioridades menores).
##Árvore de Sintaxe Abstrata
###Estrutura
A estrutura de dados para a árvore é bastante simples e consiste, acima de tudo, no uso de estruturas do tipo TreeNode e NodeList.
Regras diferentes podem ser do tipo de simples nós (TreeNode) ou os seus containers (NodeList). Isto facilita o debug do modo que garante que certas regras, que esperam apenas um nó, não recebam listas destes.
Em muitas produções (as mais notáveis são as 'expr's), não há código que garanta o número mínimo de nós-filho especificado no enunciado, pois a própria gramática assegura a presença desses nós.
Em termos de código, TreeNode conta com os seguintes atributos:
typedef struct _Node {
	struct _NodeList* sons;
	enum NodeType type;
	char* args;
}TreeNode;
Sendo NodeType uma enumeração dos tipos de nós indicados no enunciado. Cada um destes está declarado no código com o prefixo “TREE_” e o seu nome em maiúsculas (i.e. Add ficaria “TREE_ADD”). De seguida apresenta-se a lista de tipos de nós possíveis para a Árvore de Sintaxe:

 _NodeList é apenas uma lista ligada de TreeNodes:
typedef struct _NodeList {
	struct _Node* node;
	struct _NodeList* next;
}NodeList;
Sempre que é mencionado o uso de listas ligadas neste projeto, estas seguem uma estrutura análoga.
Construção
A construção da AST foi feita recorrendo (sempre que conveniente) a funções modulares, sendo as mais comuns :
TreeNode* InsertTerminal(char* terminalValue, enum NodeType terminalType)
Usada (acima de tudo) para criar nós que não contêm mais descendentes. Devolve um nó do tipo terminalType com o valor terminalValue, e
NodeList* InsertTreeNodeIntoList(TreeNode* newTreeNode, NodeList* existing) 
que insere um novo nó numa lista de nós. Lida corretamente com o caso de uma produção poder não originar nós válidos – por exemplo, em varDeclList, o último elemento de uma lista de declarações de variáveis tem de assumir o valor NULL.

Muitas vezes são usadas funções especializadas para lidar com casos específico necessários para gerar a árvore tal como o Mooshak o espera.
Tratamento de Erros
A deteção de erros gramaticais no input precisou apenas da adição da função indicada no enunciado. 

##Análise Semântica (Meta 2)
A análise Semântica foi feita em 2 fases distintas: uma primeira, onde é feita a construção da tabela de símbolos e uma segunda onde é feita uma análise mais profunda. Em ambas as fases a árvore é percorrida em (pseudo2) pós-ordem.
##Tabela de Símbolos
Estrutura
Decidiu-se utilizar uma enumeração para distinguir os diversos tipos que uma variável pode assumir:
```
enum VariableType{
	VARIABLE_BOOL,
	VARIABLE_INT,
	VARIABLE_INTARRAY,
	VARIABLE_BOOLARRAY,
	VARIABLE_VOID,
	VARIABLE_STRINGARRAY,
	VARIABLE_INVALID
};
```
Nós terminais (sem descendentes) irão gerar símbolos com um destes tipos mediante uma conversão do tipo do seu nó (mediante a função ProcessType(enum NodeType type), presente em structures.c).
Este método facilita o debug e mantém o código o mais modular possível, tendo em conta que limita a possibilidade de erros inerente à reutilização dos tipos de nós da árvore abstrata, por exemplo. Tem também a vantagem de podermos definir um tipo inválido para poder quebrar em cadeia uma sequência longa de operadores após ser detetado um erro semântico no seu interior.

A tabela em si é armazenada tendo uma estrutura para conter a classe como raiz:
```
typedef struct _ClassSymbols {
	char* name;
	struct _MethodAndVarsSymbolsList* methodsAndVars;
}ClassSymbols;
```
Que contém uma lista de Variáveis (da classe) e os seus Métodos. Foi necessário condensar os dois pois o mooshak necessita que as variáveis e métodos sejam apresentados por ordem de declaração. Num contexto real estes seriam formalmente separados.
A lista de métodos segue a já mencionada estrutura de uma lista ligada, com a particularidade de poder conter um método ou uma variável. Caso esteja presente um, o outro terá o valor NULL. São tomadas diversas medidas no código para assegurar este facto.
typedef struct _MethodAndVarsSymbolsList {
	struct _MethodSymbols* method;
	struct _Variable* variable;
	struct _MethodAndVarsSymbolsList* next;
}MethodAndVarsSymbolsList;

Cada método (MethodSymbols) tem a sua própria tabela de símbolos:
```
typedef struct _MethodSymbols {
	char* name;
	int numberOfParams; /*first numberOfParams Variables are method parameters*/
	enum VariableType returnValue;
	struct _VariableList* variables;
}MethodSymbols;
```
numberOfParams é apenas um contador que indica quantas das primeiras variáveis são argumentos de entrada. Poderia ter sido usada uma nova lista para armazenar apenas argumentos, mas isto elimina a necessidade de percorrer duas listas e é perfeitamente válido uma vez que os argumentos de entrada são sempre as primeiras variáveis locais de um método.

Para terminar, cada variável consiste simplesmente no seu tipo e num nome.
typedef struct _Variable{
	enum VariableType type;
	char *name;
}Variable;
####Código
A construção da tabela de símbolos limita-se a percorrer os nós constituintes da classe (filhos do nó “Program”), acrescentando as variáveis e os métodos à lista de símbolos da classe. Ao encontrar um método são introduzidos sequencialmente os argumentos de entrada e as variáveis locais.
É verificado se um símbolo está a ser redefinido ou não mediante as seguintes funções presentes em “symbols.h”:
MethodAndVarsSymbolsList* CheckExistingClassAttribute( char* name, ClassSymbols* classSymb )
VariableList* CheckExistingMethodAttribute(char* name, MethodSymbols* methodSymb)
Ambas as funções verificam apenas a existência de um símbolo e devolvem o último elemento na tabela (para evitar ter de a percorrer novamente na subsequente inserção). Chamo a atenção que estes métodos destinam-se apenas a serem usados na construção da tabela de símbolos e não no lookup de variáveis na verificação semântica.
Deteção de erros
A deteção de erros semânticos resume-se acima de tudo a verificar a coerência de tipos para operações.
Os “statement”s são verificados do seguinte modo: a árvore é percorrida até um nó terminal; de seguida obtém-se o seu tipo através de conversão do literal ou procura na tabela de símbolos. Este tipo é propagado pelos seus nós pai e comparado com os tipos que espera. 
Mais concretamente, é verificado a validade de cada “statement” individualmente e, na presença de expressões é chamada uma função “GetExpressionType” que irá avaliar cada expressão detetada e chamar funções como “CheckOperatorIntsOnly” (usada na adição por exemplo) que irão chamar novamente “GetExpressionType” para confirmar se os objetos da adição são de facto apenas inteiros ou se deve ser declarado um erro semântico.
Não irei entrar em mais detalhes sobre a deteção de erros devido à grande especificidade do código relevante a cada tipo de operador e “statement”. No entanto, o código usado é bastante simples de compreender.

São usadas estas funções para procurar o tipo de um símbolo (definidas em “structures.c”):
MethodSymbols* LookupMethodInClass( char* name, ClassSymbols* classTable )
Variable* LookupVariableClass( char* name, ClassSymbols* classTable )
Variable* LookupMethodVariable(char* name, MethodSymbols* methodSymb )
Estas funções procuram, respetivamente, a tabela de símbolos de um método dada uma classe, uma variável dada uma classe e uma variável dado um método. Todas usam o nome como identificador e procuram linearmente a tabela de símbolos respetiva. No lookup da tabela de símbolos na maioria dos casos procura-se uma variável na tabela do método atual e, caso isto devolva NULL (não encontrado), procura-se na tabela de símbolos da classe.
Geração de código (Meta 3)
Não foi implementada devido a falta de tempo.
Outras notas
Trabalho futuro
Teria sido útil armazenar o número de linha na estrutura da árvore de sintaxe. Esta simples adição e a eliminação de algumas restrições do método de avaliação teriam levado a uma deteção de erros semânticos muito mais útil, sendo necessário apenas mudar uma linha de código para exibir todos os erros semânticos detetados no programa de input.
Do mesmo modo, a inferência de tipo para variáveis e métodos seria extremamente simples de implementar, bastando apenas eliminar este requisito na construção da tabela de símbolos: a análise semântica distingue a procura de variáveis da procura de métodos explicitamente.
Otimizações
Memória usada
O projeto não apresenta nenhum memory leak. De facto, a execução do programa no Valgrind, dado o ficheiro “gcd.ijava” (fornecido na meta 2 do enunciado),  resulta no seguinte output:
```
==2830== Memcheck, a memory error detector
==2830== Copyright (C) 2002-2013, and GNU GPL'd, by Julian Seward et al.
==2830== Using Valgrind-3.10.0.SVN and LibVEX; rerun with -h for copyright info
==2830== Command: ./ijscanner
==2830== 
==2830== 
==2830== HEAP SUMMARY:
==2830==     in use at exit: 18,132 bytes in 203 blocks
==2830==   total heap usage: 204 allocs, 1 frees, 18,140 bytes allocated
==2830== 
==2830== LEAK SUMMARY:
==2830==    definitely lost: 0 bytes in 0 blocks
==2830==    indirectly lost: 0 bytes in 0 blocks
==2830==      possibly lost: 0 bytes in 0 blocks
==2830==    still reachable: 18,132 bytes in 203 blocks
==2830==         suppressed: 0 bytes in 0 blocks
==2830== Rerun with --leak-check=full to see details of leaked memory
==2830== 
==2830== For counts of detected and suppressed errors, rerun with: -v
==2830== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```
A restante memória acessível mas não alocada poderia ser libertada mediante uma função (omitida por não fazer parte dos objetivos diretos do projeto) que percorre a árvore de sintaxe abstrata e faz free de cada nó de acordo com um algoritmo depth-first. 
Estruturas de dados
Como mencionado na introdução o projeto beneficiaria do uso de estruturas de dados mais adequadas à procura rápida de símbolos. No entanto, não foram incluídas em nome da simplicidade e natureza puramente académica deste projeto.
