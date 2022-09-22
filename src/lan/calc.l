%option noyywrap nounistd

%{
#include <stdio.h>
#include "calc.tab.hpp"

#define YY_DECL int yylex()


%}

%%

	/* Whitespaces */
[ \t\n\r]  ;

	/* Multiline comment */


	/* Singleline comment */
"//".* ;

"shader" { return T_SHADER_KEYWORD;}
"float"	{ return T_FLOAT_KEYWORD;}
"if"	{ return T_IF_KEYWORD;}
"else"	{ return T_ELSE_KEYWORD;}
"int"	{ return T_INT_KEYWORD;}
"color"	{ return T_COLOR_KEYWORD;}
"rgb"	{ return T_RGB_KEYWORD;}
"hsv"	{ return T_HSV_KEYWORD;}


	/* Identifier */
[a-zA-Z_][a-zA-Z0-9_]* {yylval.sval = strdup(yytext); return T_IDENTIFIER;}

	/* Float value */
[0-9]+\.[0-9]+ 	{yylval.fval = atof(yytext); return T_NUMBER;}




"+"		{return T_PLUS;}
","		{return T_COLON;}
";"		{return T_SEMICOLON;}
"="		{return T_EQUAL;}
"("		{return T_PAR_LEFT;}
")"		{return T_PAR_RIGHT;}
"{"		{return T_BRACE_LEFT;}
"}"		{return T_BRACE_RIGHT;}
"<"		{return T_LOWERTHAN;}




%%