/*
 *
 * Autoři: xpasekj00
 * Soubor: lexer.c
 * Popis:
 */
 #include "lexer.h"
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
 /*
 * Proměna  která je potřebna byti viditelnou v celem souboru
 * Kdyz tak predelat a predavat pres argumenty
 */
 static LexerState *lexer = NULL;
 /*
  * Funkce inicializuje lexer a alokuje paměť pro stav lexikálního analyzátoru.
  *
  * @param source  vstupní řetězec zpracovávaný lexerem (z main.c)
  * @return        0 při úspěchu, -1 při chybě alokace
  */
 int lexer_init(const char *source){
	 lexer = malloc(sizeof(LexerState));
	 if(lexer == NULL){
		 return -1;
	 }
	 lexer->length = strlen(source);
	 lexer->input = source;
	 lexer->pos = 0;
	 return 0;
 }

 /*
  * Uvolní paměť spojenou s lexerem a nastaví ukazatel na NULL.
  */
 void lexer_free(void) {
     free(lexer);
     lexer = NULL;
 }
 /*
 *	Funkce na identifikaci identifikator
 * @param char *string
 * @return TokenType
 */
 TokenType match_identifikator(const char *identifikator) {
     if (strcmp(identifikator, "if") == 0) {
         return TOKEN_KEYWORD_IF;
     }
     if (strcmp(identifikator, "while") == 0) {
         return TOKEN_KEYWORD_WHILE;
     }
     if (strcmp(identifikator, "return") == 0) {
         return TOKEN_KEYWORD_RETURN;
     }
     if (strcmp(identifikator,"else") == 0){
     	return TOKEN_ELSE;
     }
     if (strcmp(identifikator, "var") == 0){
     	return TOKEN_IDENTIFIER;
     }
     return TOKEN_IDENTIFIER;
 }
 /*
 * Funkce nakrajeni stringu na token interne NUZKY
 *@param int start startovaci pozice
 *@param int end koncici pozice
 * @return vraci char *vysledek
 */
 char *substring(size_t start, size_t end) {
     if (end > lexer->length || start >= end) {
         return NULL;
     }
     int length = end - start;
     char *vysledek = malloc((length + 1) * sizeof(char));
     if (vysledek == NULL) {
         return NULL;
     }
     int pos = 0;
     for (size_t i = start; i < end; i++) {
         vysledek[pos++] = lexer->input[i];
     }
     vysledek[pos] = '\0';
     return vysledek;
 }
/*
* Funkce na proces zpracovani cisel
*/


 /*
  * Hlavní funkce konečného automatu pro získání dalšího tokenu.
  *
  * @return  další token ze vstupu; pokud není více tokenů,
  *          vrací TOKEN_EOF; při chybě může vrátit ERROR_TOKEN
  */
  const char *token_type_to_string(TokenType type) {
      switch (type) {
          case TOKEN_EOF:           return "EOF";
          case TOKEN_IDENTIFIER:    return "IDENTIFIER";
          case TOKEN_INTEGER:       return "INTEGER";
          case TOKEN_FLOAT:         return "FLOAT";
          case TOKEN_HEXA:          return "HEXA";
          case TOKEN_INTEGER_EXP:   return "INTEGER_EXP";
          case TOKEN_FLOAT_EXP:     return "FLOAT_EXP";
          case TOKEN_KEYWORD_IF:    return "KEYWORD_IF";
          case TOKEN_KEYWORD_WHILE: return "KEYWORD_WHILE";
          case TOKEN_ELSE:			return "KEYWORD_ELSE";
          case TOKEN_PLUS:          return "PLUS";
          case TOKEN_MINUS:         return "MINUS";
          case TOKEN_STAR:          return "STAR";
          case TOKEN_SLASH:         return "SLASH";
          case TOKEN_ASSIGN:        return "ASSIGN";
          case TOKEN_EQ:            return "EQ";
          case TOKEN_LT:            return "LT";
          case TOKEN_GT:            return "GT";
          case TOKEN_LPAREN:        return "LPAREN";
          case TOKEN_RPAREN:        return "RPAREN";
          case TOKEN_DOT:     return "DOT";
          case TOKEN_ERROR:         return "ERROR";
          case TOKEN_KEYWORD_RETURN:return "KEYWORD_RETURN";
          case TOKEN_STRING:		return "STRING";
          case TOKEN_GLOBAL_ID: 	return "GLOBAL_ID";
          case TOKEN_NEW_LINE:		return "TOKEN_NEW_LINE";
          default:                  return "UNKNOWN_TOKEN";
      }
}
 Token lexer_token_next(){
			Token token = {0};
	 if (lexer == NULL || lexer->pos >= lexer->length) {
	     token.type = TOKEN_EOF;
	     return token;
	 }
	char c;
	/*
	* Promena zacatek slozi k urceni zacatku :) tokenu
	*/
	int zacatek = lexer->pos;
	/*
	* Promena ktera urcuje v jakem stavu(slovo,cislo atd)
	*/
	StateType stav = START_LEX;
	int radek = 0;
	while (1) {
		if(lexer->pos < lexer->length){
		c = lexer->input[lexer->pos];
		}else {
			token.type = TOKEN_EOF;
			return token;
			break;
		}
		if(c == '\n'){
			radek++;
		}
		switch (stav) {
			/*
			*Prvni stav kde jsem uprostred niceho a cekame na zacatek tokenu a nebo jsme na konci
		 	*/
				/*
				* Dodelat stav symbolu ve kterem porovname symboly pridat token
			 */
			case START_LEX:{
				if (c == '\0') {
					token.type = TOKEN_EOF;
					return token;
				}else if (isalpha(c)) {
					stav = ID;
					zacatek = lexer->pos;
					lexer->pos++;
				}else if (isdigit(c)) {
					stav = NUMBER;
					zacatek = lexer->pos;
					lexer->pos++;
				}else if (c == '"') {
					stav = STRING;
					zacatek = lexer->pos;
					lexer->pos++;
				}else if (c == '/') {
					stav = BACKSLASH;
					lexer->pos++;
				}else if (c == '_') {
					stav = UNDERSCORE;
					zacatek = lexer->pos;
					lexer->pos++;
				}else if (c == '0') {
					stav = ZERO;
					zacatek = lexer->pos;
					lexer->pos++;
				}else if (c == '\n') {
					token.type = TOKEN_NEW_LINE;
					lexer->pos++;
					return token;
				}
				else if(c == ' '){
					lexer->pos++;
				}else {
					//dalsi zatim tady dam error
					fprintf(stderr,"unknow letter in lexer_token_next %c on line : %lu",c,lexer->pos);
				}
				break;
			}
			case ID:{
				if (isalnum(c) || c == '_') {
	                lexer->pos++;
	            }else{
					token.value.string = substring(zacatek, lexer->pos);
     				token.type = match_identifikator(token.value.string);
         			token.line = radek;
	                return token;
	            }
				break;
			}
			case UNDERSCORE:{
				if (c == '_') {
					stav = GLOBAL_ID;
	                lexer->pos++;
	            }else {
					token.type = TOKEN_ERROR;
					fprintf(stderr, "Chybny znak v identifikatoru na radku %i",radek);
					return token;
					}
            break;
			}
			case GLOBAL_ID:{
				if (isalnum(c) || c == '_') {
	                lexer->pos++;
	            }else{
					token.value.string = substring(zacatek, lexer->pos);
     				token.type = match_identifikator(token.value.string);
         			token.line = radek;
	                return token;
				}
				break;
			}
			case NUMBER:{
	            if (isdigit(c)) {
	                lexer->pos++;
	            }else if(c == '.'){
	            	stav= FLOAT_NUMBER;
	             	lexer->pos++;
	            }else if (c == ' ' || c == '\n') {
		            token.type = TOKEN_INTEGER;
		            char *tmp = substring(zacatek, lexer->pos);
		            char *endptr;
		            token.value.number = strtod(tmp, &endptr);
	            if (endptr == tmp) {
	                fprintf(stderr, "Chyba: neočekávaný řetězec místo čísla: %s\n", tmp);
	                token.type = TOKEN_ERROR;
	            }
	            	token.line = radek;
		            free(tmp);
		            return token;
	            }else if (c == 'e') {
	            	stav=DIGIT_EXP;
	             	lexer->pos++;
	            } else {
		            token.type = TOKEN_ERROR;
					fprintf(stderr, "Chybny znak v čisle %i",radek);
	            }
	            break;
			}
			case FLOAT_NUMBER:{
				if(c == 'e'){
					stav = FLOAT_EXP;
				}else if (isdigit(c)) {
					lexer->pos++;
				}else if (c == ' ' || c == '\n') {
		            token.type = TOKEN_FLOAT;
		            char *tmp = substring(zacatek, lexer->pos);
		            char *endptr;
		            token.value.number = strtod(tmp, &endptr);
		            if (endptr == tmp) {
		                fprintf(stderr, "Chyba: neočekávaný řetězec místo čísla: %s\n", tmp);
		                token.type = TOKEN_ERROR;
		            }
	            	token.line = radek;
		            free(tmp);
		            return token;
				}else{
				    token.type = TOKEN_ERROR;
					fprintf(stderr, "Chybny znak v čisle %i",radek);
				}
				break;
			}
			case ZERO:{
				if(c == 'x'){
					stav = HEXA_NUM;
					lexer->pos++;
				}else if(c == '.'){
					stav = FLOAT_NUMBER;
					lexer->pos++;
				}else if(isdigit(c)){
					stav = NUMBER;
				}else {
				    token.type = TOKEN_ERROR;
					fprintf(stderr, "Chybny znak v čisle %i",radek);
				}
				break;
			}
			case HEXA_NUM:{
				if(isxdigit(c)){
					lexer->pos++;
				}else if(c == ' ' || c == '\n'){
				     token.type = TOKEN_HEXA;
				     char *tmp = substring(zacatek, lexer->pos);
				     char *endptr;
				     token.value.number = strtod(tmp, &endptr);
				     if (endptr == tmp) {
				         fprintf(stderr, "Chyba: neočekávaný řetězec místo čísla: %s\n", tmp);
				         token.type = TOKEN_ERROR;
				     }
				    token.line = radek;
				     token.value.number = strtod(tmp, NULL);
				     free(tmp);
				     return token;
				}else{
			    token.type = TOKEN_ERROR;
				fprintf(stderr, "Chybny znak v čisle %i",radek);
				}
				break;
			}
			case DIGIT_EXP:{
				if(isdigit(c)){
					lexer->pos++;
				}else if (c == ' ' || c == '\n' || c == '\0') {
				    token.type = TOKEN_INTEGER_EXP;
				    char *tmp = substring(zacatek, lexer->pos);
				    char *endptr;
				    token.value.number = strtod(tmp, &endptr);
			    if (endptr == tmp) {
			        fprintf(stderr, "Chyba: neočekávaný řetězec místo čísla: %s\n", tmp);
			        token.type = TOKEN_ERROR;
			    }
				    token.line = radek;
				    free(tmp);
				    return token;
				}else{
					token.type = TOKEN_ERROR;
					fprintf(stderr, "Chybny znak v čisle %i",radek);
				}
				break;
			}
			case FLOAT_EXP:{
				if(isdigit(c)){
					lexer->pos++;
				}else if (c == ' ') {
				    token.type = TOKEN_FLOAT_EXP;
				    char *tmp = substring(zacatek, lexer->pos);
				    char *endptr;
				    token.value.number = strtod(tmp, &endptr);
			    if (endptr == tmp) {
			        fprintf(stderr, "Chyba: neočekávaný řetězec místo čísla: %s\n", tmp);
			        token.type = TOKEN_ERROR;
			    }
				    token.line = radek;
				    free(tmp);
				    return token;
				}else{
					token.type = TOKEN_ERROR;
					fprintf(stderr, "Chybny znak v čisle %i",radek);
				}
				break;
			}
			case BACKSLASH:{
				if(c == '/'){
					stav = LINE_COMMENT;
					lexer->pos++;
				}else if (c == '*') {
					stav = BLOCK_COMMENT;
					lexer->pos++;
				}
				break;
			}
			case LINE_COMMENT:{
				if(c == '\n'){
					stav = START_LEX;
				}
			lexer->pos++;
			break;
			}
			case BLOCK_COMMENT:{
				if(c == '*'){
					stav = STAR;
			}
				lexer->pos++;
			break;
			}
			case STAR:{
				if(c == '/'){
					stav = START_LEX;
				}
				lexer->pos++;
				break;
			}
			case STRING:{
				if (c == '"') {
					if(lexer->pos + 1 < lexer->length && lexer->input[lexer->pos+ 1] == '"'){
						stav=STRING_BLOCK;
						lexer->pos++;
						lexer->pos++;
					}else {
					    token.type = TOKEN_STRING;
					    token.line = radek;
				    	token.value.string = substring(zacatek+1, lexer->pos);
						lexer->pos++;
					    return token;
					}
				}else if (c == '\n' || c == '\0') {
					fprintf(stderr, "Chyba: neočekávaný konec řádku v řetězci na řádku %i with char %c \n", radek,c);
        			token.type = TOKEN_ERROR;
           			return token;
				}else if(c == '\\'){
			     lexer->pos++;
			        char next = lexer->input[lexer->pos];
			        switch (next) {
			            case 'n': case 'r': case 't': case '"': case '\\':
			                lexer->pos++;
			                break;
						case 'x':
							stav = STRING_HEX_N;
							lexer->pos++;
							break;
			            default:
			                fprintf(stderr, "Chyba: neznámá escape sekvence \\%c na řádku %i\n",
			                        next, radek);
			                token.type = TOKEN_ERROR;
			                return token;
			        }
				break;
				}else if(c >= 32){
					lexer->pos++;
				}
				break;
			}
			case STRING_BLOCK:{
				while (isspace(lexer->input[lexer->pos++])) {}
				if(c == '"' && lexer->input[lexer->pos] + 1 == '"' && lexer->input[lexer->pos] + 1 == '"'){
					lexer->pos = lexer->pos + 2;
				    token.type = TOKEN_STRING_BLOCK;
				    token.line = radek;
					token.value.string = substring(zacatek, lexer->pos);
				    return token;
				}else {
					lexer->pos++;
				}
				break;
			}
			default:
			fprintf(stderr, "chyba v konecnem automatu ve switch to spadlo do default \n");
			fprintf(stderr, "chyba nastala s tim to charakterem : %c na radku %i \n",c,radek);
			token.type = TOKEN_ERROR;
            return token;
		}
	}
	token.type = TOKEN_ERROR;
    return token;
}
