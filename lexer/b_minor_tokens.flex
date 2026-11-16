%option noyywrap
%{
#include "tokens.h"
%}

/* Defining a goddamn integer */
NZDIGIT [1-9]
DIGIT 0|{NZDIGIT}
UNDERSCORES _+
DIGITORUND \_|{DIGIT}
DIGITSANDUND {DIGITORUND}+
DIGITS {DIGIT}|{DIGIT}{DIGITSANDUND}?{DIGIT}
DECIMALNUMERAL 0|{NZDIGIT}{DIGITS}?
/*                                  |{NZDIGIT}{UNDERSCORES}{DIGITS}*/

INTEGERLITERAL {DECIMALNUMERAL}

/*Input char means no \n or \r*/
LINETERM [\n\r]|\r\n
SINGLECHAR [^\'\\\n\r]
ESCAPESEQ \\b|\\t|\\n|\\f|\\r|\\\"|\\\'|\\\\|\\[0-7]|\\[0-7][0-7]|\\[0-3][0-7][0-7]
CHARLITERAL \'{SINGLECHAR}\'|\'{ESCAPESEQ}\'
/* Defining a goddamn string */
STRINGCHAR [^\"\\\n\r]|{ESCAPESEQ}
STRINGCHARS {STRINGCHAR}+
STRINGLITERAL \"{STRINGCHARS}?\"

/* Defining a goddamn identifier */
LETTER [a-zA-Z_]
LETTERORDIGIGT {LETTER}|{DIGIT}
IDENTIFIERCHARS {LETTER}{LETTERORDIGIGT}*
IDENTIFIER {IDENTIFIERCHARS}

/* Defining comments */

NOTSTAR [^\n\r\*]|{LINETERM}
NOTSTARNOTSLASH [^\n\r\*\/]|{LINETERM}
CHARSINLINE [^\n\r]+
COMMTAIL ({NOTSTAR}*\*+{NOTSTARNOTSLASH})*{NOTSTAR}*\*+\/
TRADCOMM \/\*{COMMTAIL}
EOLCOMM \/\/{CHARSINLINE}?
COMMENT {TRADCOMM}|{EOLCOMM}

/* Operators (to make life easier) */
OPFIRSTLINE \=|\>|\<|\!|\~|\?
OPSECONDLINE \=\=|\<\=|\>\=|\!\=|\&\&|\|\||\+\+|\-\-
OPTHIRDLINE \+|\-|\*|\/|\&|\||\^|\%|\<\<|\>\>|\>\>\>
OPFOURTHLINE \+\=|\-\=|\/\=|\&\=|\|\=|\^\=|\%\=|\<\<\=|\>\>\=|\>\>\>\=
OPERATOR {OPFIRSTLINE}|{OPSECONDLINE}|{OPTHIRDLINE}|{OPFOURTHLINE}

%%
{COMMENT} {}
array {return TOKEN_KW_ARRAY;}
bool {return TOKEN_KW_BOOL;}
char {return TOKEN_KW_CHAR;}
else {return TOKEN_KW_ELSE;}
false {return TOKEN_LIT_FALSE;}
for {return TOKEN_KW_FOR;}
function {return TOKEN_KW_FUNCTION;}
if {return TOKEN_KW_IF;}
int {return TOKEN_KW_INT;}
print {return TOKEN_KW_PRINT;}
return {return TOKEN_KW_RETURN;}
string {return TOKEN_KW_STRING;}
true {return TOKEN_LIT_TRUE;}
void {return TOKEN_KW_VOID;}
while {return TOKEN_KW_WHILE;}
{INTEGERLITERAL} {return TOKEN_LIT_INT;}
{STRINGLITERAL} {return TOKEN_LIT_STRING;}
{CHARLITERAL} {return TOKEN_LIT_CHAR;}
{IDENTIFIER} {return TOKEN_ID;}
[ \t\f\n\r]|\r\n {}
\( {return TOKEN_LPAREN;}
\) {return TOKEN_RPAREN;}
\{ {return TOKEN_LBRACE;}
\} {return TOKEN_RBRACE;}
\[ {return TOKEN_LBRACK;}
\] {return TOKEN_RBRACK;}
\; {return TOKEN_SEMICOLON;}
\, {return TOKEN_COMMA;}
\: {return TOKEN_COLON;}
\+ {return TOKEN_PLUS;}
\- {return TOKEN_MINUS;}
\* {return TOKEN_MULTIPLY;}
\/ {return TOKEN_DIVIDE;}
\& {return TOKEN_BITAND;}
\^ {return TOKEN_BITXOR;}
\| {return TOKEN_BITOR;}
\~ {return TOKEN_BITNOT;}
\&\& {return TOKEN_LOGAND;}
\|\| {return TOKEN_LOGOR;}
\! {return TOKEN_LOGNOT;}
\% {return TOKEN_MOD;}
\< {return TOKEN_LESS;}
\<\= {return TOKEN_LEQ;}
\> {return TOKEN_GREATER;}
\>\= {return TOKEN_GEQ;}
\+\+ {return TOKEN_INCREMENT;}
\-\- {return TOKEN_DECREMENT;}
\= {return TOKEN_EQUALS;}
\=\= {return TOKEN_EQUALITY;}
\!\= {return TOKEN_INEQUALITY;}
[^\n\r] {}
