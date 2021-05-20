/***************************************************************
*      scanner routine for Mini C language                    *
***************************************************************/

// 130, 369줄 

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "Scanner.h"

extern FILE *sourceFile;                       // miniC source program


int superLetter(char ch);
int superLetterOrDigit(char ch);
int getNumber(char firstCharacter);
int hexValue(char ch);
void lexicalError(int n);	
char fileName[30];	// file name 

const char *tokenName[] = {
	"!",        "!=",      "%",       "%=",     "%ident",   "%number",
	/* 0          1           2         3          4          5        */
	"&&",       "(",       ")",       "*",      "*=",       "+",
	/* 6          7           8         9         10         11        */
	"++",       "+=",      ",",       "-",      "--",	    "-=",
	/* 12         13         14        15         16         17        */
	"/",        "/=",      ";",       "<",      "<=",       "=",
	/* 18         19         20        21         22         23        */
	"==",       ">",       ">=",      "[",      "]",        "eof",
	/* 24         25         26        27         28         29        */
	//   ...........    word symbols ................................. //
	/* 30         31         32        33         34         35        */
	"const",    "else",     "if",      "int",     "return",  "void",
	/* 36         37          38        39           40         41     */
	"while",    "{",        "||",       "}",	  "char",    "double",
	/* 42         43          44        45           46         47     */
	"for",     "do",       "goto",     "switch",  "case",     "break",
	/* 48         49          50        51           52         53     */
	"default",  ":",        "%tlchar", "%tlstring", "%tldouble", "%tlcomment"
};

const char *keyword[NO_KEYWORD] = { 
	"const",  "else",    "if",    "int",    "return",  "void",    "while",
	"char",   "double",  "for",   "do",     "goto",    "switch",  "case",
	"break",  "default"
};

enum tsymbol tnum[NO_KEYWORD] = {	
	tconst,    telse,     tif,     tint,     treturn,   tvoid,     twhile,
	tchar,     tdouble,   tfor,    tdo,      tgoto,     tswitch,   tcase,
	tbreak,    tdefault
};

struct tokenType scanner(char file[])
{
	strcpy_s(fileName, file);

	struct tokenType token;
	int i, index;
	char ch, id[ID_LENGTH], chr;
	char str[MAX_LENGTH];

	int linenum = 0;
	int columnum = 0;
	int precolnum;	// ungetc할 때 columnumber 

	token.number = tnull;	

	do {
		while (isspace(ch = fgetc(sourceFile))) { // state 1: skip blanks
			if (ch == '\n') {
				linenum++;
				columnum = 1;
			}
			else columnum++;
		}
		if (superLetter(ch)) { // 1. identifier or keyword 
			i = 0;
			token.linenum = linenum;	
			token.columnum = columnum;
			do {
				if (i < ID_LENGTH) id[i++] = ch;
				ch = fgetc(sourceFile);
			} while (superLetterOrDigit(ch));
			if (i >= ID_LENGTH) lexicalError(1);
			id[i] = '\0'; // null
			precolnum = columnum;
			ungetc(ch, sourceFile); linenum--; columnum = precolnum; // retract
									 // find the identifier in the keyword table
			for (index = 0; index < NO_KEYWORD; index++)
				if (!strcmp(id, keyword[index])) break;	
			if (index < NO_KEYWORD)    // found, keyword exit
				token.number = tnum[index];
			else {                     // not found, identifier exit
				token.number = tident; 
				strcpy_s(token.value.id, id);
			}
		}  // end of identifier or keyword
		else if (isdigit(ch)) {  // 2. number
			token.linenum = linenum;
			token.columnum = columnum;
			token.number = tnumber;
			token.value.num = getNumber(ch); 
		}
		else switch (ch) {  // 3. special character
		case '\'': // char
			ch = fgetc(sourceFile);
			columnum++;
			token.number = tlchar;
			token.linenum = linenum;
			token.columnum = columnum;
			token.value.chr = ch;
			ch = fgetc(sourceFile);
			if (ch != '\'') lexicalError(5);
		case '"': // string 
			ch = fgetc(sourceFile);
			columnum++;
			token.number = tlstring;
			token.linenum = linenum;
			token.columnum = columnum;
			int idx = 0;
			while (ch != '"') {
				if (ch == '\n') {
					ch == ' ';
				}
				token.value.str[idx++] = ch;
				ch = fgetc(sourceFile);
			}
		case '.':	// 여기도 double 
			ch = fgetc(sourceFile); 
		case '/':	
			ch = fgetc(sourceFile);
			columnum++;
			if (ch == '*') {			// text comment
				token.number = tlcomment;
				columnum++;
				do {
					ch = fgetc(sourceFile);
					if (ch == '*') { // /** ~ */
						columnum++;
						ch = fgetc(sourceFile);
					}
					token.linenum = linenum;
					token.columnum = columnum;
					int idx = 0;
					while (ch != '*') {
						token.value.comment[idx++] = ch;
						ch = fgetc(sourceFile);
					}
					ch = fgetc(sourceFile);
				} while (ch != '/');
			}
			else if (ch == '/') {		// line comment
				token.number = tlcomment;
				columnum++;
				ch = fgetc(sourceFile);
				if (ch == '/') { // multi line comment 
					columnum++;
					ch = fgetc(sourceFile);
				}
				token.linenum = linenum;
				token.columnum = columnum;
				int idx = 0;
				while (ch != '\n') {
					token.value.comment[idx++] = ch;
					ch = fgetc(sourceFile);
				}
			}
			else if (ch == '=') {
				token.number = tdivAssign;
				token.linenum = linenum;
				token.columnum = columnum;
			}
			else {
				token.linenum = linenum;
				token.columnum = columnum;
				token.number = tdiv;
				precolnum = columnum;
				precolnum = columnum;
				ungetc(ch, sourceFile); linenum--; columnum = precolnum; // retract
			}
			break;
		case '!': // != , !
			token.linenum = linenum;
			token.columnum = columnum;
			ch = fgetc(sourceFile);
			if (ch == '=')  token.number = tnotequ;
			else {
				token.number = tnot;
				precolnum = columnum;
				ungetc(ch, sourceFile); linenum--; columnum = precolnum; // retract
			}
			break;
		case '%': // %= , %
			token.linenum = linenum;
			token.columnum = columnum;
			ch = fgetc(sourceFile);
			if (ch == '=') {
				token.number = tremAssign;
			}
			else {
				token.number = tremainder;
				precolnum = columnum;
				ungetc(ch, sourceFile); linenum--; columnum = precolnum; // retract
			}
			break;
		case '&': // &&
			token.linenum = linenum;
			token.columnum = columnum;
			ch = fgetc(sourceFile);
			if (ch == '&')  token.number = tand;
			else {
				lexicalError(2); 
				precolnum = columnum;
				ungetc(ch, sourceFile);  linenum--; columnum = precolnum; // retract
			}
			break;
		case '*': // *= , *
			token.linenum = linenum;
			token.columnum = columnum;
			ch = fgetc(sourceFile);
			if (ch == '=')  token.number = tmulAssign;
			else {
				token.number = tmul;
				precolnum = columnum;
				ungetc(ch, sourceFile);  linenum--; columnum = precolnum; // retract
			}
			break;
		case '+': // += , +
			token.linenum = linenum;
			token.columnum = columnum;
			ch = fgetc(sourceFile);
			if (ch == '+')  token.number = tinc;
			else if (ch == '=') token.number = taddAssign;
			else {
				token.number = tplus;
				precolnum = columnum;
				ungetc(ch, sourceFile);  linenum--; columnum = precolnum; // retract
			}
			break;
		case '-': // -= , -
			token.linenum = linenum;
			token.columnum = columnum;
			ch = fgetc(sourceFile);
			if (ch == '-')  token.number = tdec;
			else if (ch == '=') token.number = tsubAssign;
			else {
				token.number = tminus;
				precolnum = columnum;
				ungetc(ch, sourceFile);  linenum--; columnum = precolnum; // retract
			}
			break;
		case '<': // <= , <
			token.linenum = linenum;
			token.columnum = columnum;
			ch = fgetc(sourceFile);
			if (ch == '=') token.number = tlesse;
			else {
				token.number = tless;
				precolnum = columnum;
				ungetc(ch, sourceFile);  linenum--; columnum = precolnum; // retract
			}
			break;
		case '=': // == , =
			token.linenum = linenum;
			token.columnum = columnum;
			ch = fgetc(sourceFile);
			if (ch == '=')  token.number = tequal;
			else {
				token.number = tassign;
				precolnum = columnum;
				ungetc(ch, sourceFile);  linenum--; columnum = precolnum; // retract
			}
			break;
		case '>': // >= , >
			token.linenum = linenum;
			token.columnum = columnum;
			ch = fgetc(sourceFile);
			if (ch == '=') token.number = tgreate;
			else {
				token.number = tgreat;
				precolnum = columnum;
				ungetc(ch, sourceFile);  linenum--; columnum = precolnum; // retract
			}
			break;
		case '|': // || 
			token.linenum = linenum;
			token.columnum = columnum;
			ch = fgetc(sourceFile);
			if (ch == '|')  token.number = tor;
			else {
				lexicalError(3); 
				precolnum = columnum;
				ungetc(ch, sourceFile);  linenum--; columnum = precolnum; // retract
			}
			break;
		case '(': token.number = tlparen; token.linenum = linenum; token.columnum = columnum;      break;
		case ')': token.number = trparen; token.linenum = linenum; token.columnum = columnum;      break;
		case ',': token.number = tcomma; token.linenum = linenum; token.columnum = columnum;      break;
		case ';': token.number = tsemicolon; token.linenum = linenum; token.columnum = columnum;      break;
		case '[': token.number = tlbracket; token.linenum = linenum; token.columnum = columnum;      break;
		case ']': token.number = trbracket; token.linenum = linenum; token.columnum = columnum;      break;
		case '{': token.number = tlbrace; token.linenum = linenum; token.columnum = columnum;      break;
		case '}': token.number = trbrace; token.linenum = linenum; token.columnum = columnum;      break;
		case ':': token.number = tcolon; token.linenum = linenum; token.columnum = columnum;      break;
		case EOF: token.number = teof; token.linenum = linenum; token.columnum = columnum;      break;
		default: {
			printf("Current character : %c", ch);
			lexicalError(4); 
			break;
		}

		} // switch end
	} while (token.number == tnull);
	return token;
} // end of scanner

void lexicalError(int n)
{
	printf(" *** Lexical Error : ");
	switch (n) {
	case 1: printf("an identifier length must be less than 12.\n");	
		break;
	case 2: printf("next character must be &\n"); 
		break;
	case 3: printf("next character must be |\n"); 
		break;
	case 4: printf("invalid character\n");	
		break;
	case 5: printf("next character must be \'\n");
		break;
	}
}

int superLetter(char ch)	
{
	if (isalpha(ch) || ch == '_') return 1;
	else return 0;
}

int superLetterOrDigit(char ch)
{
	if (isalnum(ch) || ch == '_') return 1;	
	else return 0;
}

int getNumber(char firstCharacter)
{
	int num = 0;
	int value;
	char ch;

	if (firstCharacter == '0') {
		ch = fgetc(sourceFile);
		if ((ch == 'X') || (ch == 'x')) {		// hexa decimal
			while ((value = hexValue(ch = fgetc(sourceFile))) != -1)
				num = 16 * num + value;
		}
		else if ((ch >= '0') && (ch <= '7'))	// octal
			do {
				num = 8 * num + (int)(ch - '0');
				ch = fgetc(sourceFile);
			} while ((ch >= '0') && (ch <= '7'));
		else num = 0;						// zero
	}
	else if (ch == '.') { // double 
		//~ 
	}
	else {									// decimal
		ch = firstCharacter;
		do {
			num = 10 * num + (int)(ch - '0');
			ch = fgetc(sourceFile);
		} while (isdigit(ch));
	}
	ungetc(ch, sourceFile);  /*  retract  */
	return num;
}

int hexValue(char ch)
{
	switch (ch) {
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		return (ch - '0');
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		return (ch - 'A' + 10);
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		return (ch - 'a' + 10);
	default: return -1;
	}
}

void printToken(struct tokenType token)
{
	printf("number: %d, value: %s\n", token.number, token.value.id);
	if (token.number == tident)
		printf("%s (%d, %s, %s, %d, %d)", token.value.id, token.number, token.value.id, fileName, token.linenum, token.columnum);
	else if (token.number == tnumber)
		printf("%s (%d, %s, %s, %d, %d)", token.value.num, token.number, token.value.num, fileName, token.linenum, token.columnum);
	else if (token.number == tlcomment)
		printf("%s (%d, %s, %s, %d, %d)", token.value.comment, token.number, token.value.comment, fileName, token.linenum, token.columnum);
	else
		printf("number: %d(%s)\n", token.number, tokenName[token.number]);
}
