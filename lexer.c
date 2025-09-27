/*
 *
 * Autoři: xpasekj00
 * Soubor: lexer.c
 * Popis:
 */
 #include "lexer.h"
#include <stdlib.h>
#include <string.h>
 /*
 * Proměna  která je potřebna byti viditelnou v celem souboru
 * Kdyz tak predelat a predavat pres argumenty
 */
 static LexerState *lexer = NULL;
/*
* Funkce inicilizuje lexer allokojume pamet pro prvotni vstup
*@param const char *source input ktery mame z main.c
* @return vraci -1 při fail a 0 při uspěchu
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
  *Funkce na uvolneni lexeru a uklidu
 */
 void lexer_free(void) {
     free(lexer);
     lexer = NULL;
 }
 /*
 * Funkce zajištuje tokenizaci vstupu pomoci konečneho automatu
 * @return struct Token
 */
 Token lexer_token_next(){
	 Token token;
	 if (lexer == NULL || lexer->pos >= lexer->length) {
	     token.type = TOKEN_EOF;
	     return token;
	 }
	int i = lexer->pos;
	while (lexer->input[i] != ' ') {
		switch (lexer->input[i]) {

			default:
		}
	}
}
