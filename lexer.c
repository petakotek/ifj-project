/*
 *
 * Autoři: xpasekj00
 * Soubor: lexer.c
 * Popis:
 */
 #include "lexer.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

     return TOKEN_IDENTIFIER;
 }
 /*
 * Funkce nakrajeni stringu na token interne NUZKY
 *@param int start startovaci pozice
 *@param int end koncici pozice
 * @return vraci char *vysledek
 */
 char *substring(int start, int end) {
     if (start < 0 || end > lexer->length || start >= end) {
         return NULL;
     }
     int length = end - start;
     char *vysledek = malloc((length + 1) * sizeof(char));
     if (vysledek == NULL) {
         return NULL;
     }
     int pos = 0;
     for (int i = start; i < end; i++) {
         vysledek[pos++] = lexer->input[i];
     }
     vysledek[pos] = '\0';
     return vysledek;
 }

 /*
  * Hlavní funkce konečného automatu pro získání dalšího tokenu.
  *
  * @return  další token ze vstupu; pokud není více tokenů,
  *          vrací TOKEN_EOF; při chybě může vrátit ERROR_TOKEN
  */
 Token lexer_token_next(){
	 Token token;
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
	int stav = 0;
	int radek = 0;
	while (1) {
		c = lexer->input[lexer->pos];
		switch (c) {
			/*
			*Prvni stav kde jsem uprostred niceho a cekame na zacatek tokenu a nebo jsme na konci
		 	*/
			case 0:{
				if (c == '\0') {
					token.type = EOF;
					return token;
				}else if (isspace(c)) {
					//Tady musi byt dodelano hodne toho jelikoz mezery maji v Wran jazyce velky vyznam
					lexer->pos++;
					zacatek = lexer->pos;
				}else if (isalpha(c)) {
					stav = 1;
					lexer->pos++;
				}else if (isdigit(c)) {
					stav = 2;
					lexer->pos++;
				}else if (c == '\n') {
					radek++;
					lexer->pos++;
				}else {
					//dalsi zatim tady dam error
					fprintf(stderr,"unknow letter in lexer_token_next %c on line : %lu",c,lexer->pos);
				}
			}
			break;
			case 1:{
			  if (isalnum(c)) {
	                lexer->pos++;
	            } else {
					token.value.string = substring(zacatek, lexer->pos);
     				token.type = match_identifikator(token.value.string);
         			token.line = radek;
	                return token;
	            }
            break;
			}
			case 2:
            if (isdigit(c)) {
                lexer->pos++;
            } else {
                token.type = TOKEN_NUMBER;
                char *tmp = substring(zacatek, lexer->pos);
                token.line = radek;
                token.value.number = strtod(tmp, NULL);
                free(tmp);
                return token;
            }
            break;
			default:
			fprintf(stderr, "chyba v konecnem automatu ve switch to spadlo do default \n");
			fprintf(stderr, "chyba nastala s tim to charakterem : %c na radku %i \n",c,radek);
			token.type = TOKEN_ERROR;
            return token;
		}
	}
}
