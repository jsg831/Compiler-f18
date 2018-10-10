%{
#define LIST                { strncat(buf, yytext, sizeof(buf) - strlen(buf) - 1); }
#define append(t)           { strncat(str, t, sizeof(str) - strlen(str) - 1); }
#define token(t)            { LIST; if (Opt_Token) printf("<%s>\n", t); }
#define tokenOperator(t)    { LIST; if (Opt_Token) printf("<\"%s\">\n", t); }
#define tokenString(t, s)   { LIST; if (Opt_Token) printf("<%s:%s>\n", t, s); }
#define MAX_LINE_LENGTH 257
#define MAX_ID_LENGTH 257
int Opt_Source = 1;
int Opt_Token = 1;
int Opt_Statistic = 1;
unsigned int linenum = 1;
char buf[MAX_LINE_LENGTH];
char str[MAX_LINE_LENGTH];
typedef struct ID {
    char name[MAX_LINE_LENGTH];
    unsigned int freq;
} ID;
ID *identifiers;
unsigned int id_size, i;
%}

%Start STRING COMMENT

/* Definitions */
letter      [_A-Za-z]
digit       [0-9]
comment     \/\/[^\n]*
delim       [,;\(\)\[\]\{\}]
operator    \+|\-|\*|\/|\%|=|\<|\<=|!=|\>=|\>|==|&&|\|\||!
keyword     while|do|if|else|true|false|for|int|print|const|read|boolean|bool|void|float|double|string|continue|break|return
id          {letter}({letter}|{digit})*
ws          [ \t]
integer     (0|[1-9][0-9]*)
float       {digit}+\.{digit}+
scientific  ({digit}+(\.{digit}+)?)[Ee][\+\-]?{digit}+
string      \"([^\"\n]|\\\"|\\\\)*\"
%%
 /* pragmatics */
<INITIAL>^(#pragma{ws}+source{ws}+on) {
    LIST;
    Opt_Source = 1;
}

<INITIAL>^(#pragma{ws}+source{ws}+off) {
    LIST;
    Opt_Source = 0;
}

<INITIAL>^(#pragma{ws}+token{ws}+on) {
    LIST;
    Opt_Token = 1;
}

<INITIAL>^(#pragma{ws}+token{ws}+off) {
    LIST;
    Opt_Token = 0;
}

<INITIAL>^(#pragma{ws}+statistic{ws}+on) {
    LIST;
    Opt_Statistic = 1;
}

<INITIAL>^(#pragma{ws}+statistic{ws}+off) {
    LIST;
    Opt_Statistic = 0;
}

 /* comments and whitespaces */
<INITIAL>"/*" {
    LIST;
    BEGIN COMMENT;
}

<COMMENT>"*/" {
    LIST;
    BEGIN INITIAL;
}

{comment} |
<COMMENT>. |
<INITIAL>{ws} {
    LIST;
}

 /* delimiters */
<INITIAL>{delim} {
    tokenString("delim", yytext);
}

 /* operators */
<INITIAL>{operator} {
    tokenOperator(yytext);
}

 /* keywords */
<INITIAL>{keyword} {
    tokenString("KW", yytext);
}

 /* identifiers */
<INITIAL>{id} {
    for (i = 0; i < id_size; ++i) {
        if (strcmp(yytext, identifiers[i].name) == 0) {
            identifiers[i].freq += 1;
            break;
        }
    }
    if (i == id_size) {
        id_size += 1;
        if (identifiers == NULL)
            identifiers = (ID*)malloc(sizeof(ID)*id_size);
        else
            identifiers = (ID*)realloc(identifiers, sizeof(ID)*id_size);
        if (identifiers == NULL) {
            fprintf(stderr, "Insufficient memory.\n");
            exit(1);
        }
        strcpy(identifiers[i].name, yytext);
        identifiers[i].freq = 1;
    }
    tokenString("id", yytext);
}

 /* numbers */
<INITIAL>{integer} {
    tokenString("integer", yytext);
}

<INITIAL>{float} {
    tokenString("float", yytext);
}

<INITIAL>{scientific} {
    tokenString("scientific", yytext);
}

 /* string literals */
<INITIAL>\" {
    LIST;
    str[0] = '\0';
    BEGIN STRING;
}

<STRING>\\\\ {
    LIST;
    append("\\");
}

<STRING>\\\" {
    LIST;
    append("\"");
}

<STRING>\\ {
    LIST;
    append("\\");
}

<STRING>[^\\\"\n]* {
    LIST;
    append(yytext);
}

<STRING>\" {
    append("\0");
    tokenString("string", str);
    BEGIN INITIAL;
}

 /* newlines */
<INITIAL>\n |
<COMMENT>\n {
    LIST;
    if (Opt_Source)
        printf("%d:%s\n", linenum, buf);
    ++linenum;
    buf[0] = '\0';
}

. { // Unrecognized character
    fprintf(stderr, "Error at line %d: %s\n", linenum, yytext);
    exit(1);
}

%%

int main( int argc, char **argv )
{
    if (argc >= 2)
    {
        yyin = fopen( argv[1], "r" );
        if ( NULL == yyin ) {
            fprintf( stderr, "Cannot open: %s\n", argv[1] );
            exit(-1);
        }
    }
    else
    {
        yyin = stdin;
    }

    identifiers = NULL;
    id_size = 0;
    yylex();

    if (Opt_Statistic)
    {
        printf("frequencies of identifiers:\n");
        for (i = 0; i < id_size; ++i) {
            printf("%s\t%d\n", identifiers[i].name, identifiers[i].freq);
        }
    }

    exit(0);
}
