#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>

uint32_t isa_reg_str2val(const char *s, bool *success);

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NUM, TK_PLUS, TK_SUB, TK_MUL, TK_DIV,
  TK_LBR, TK_RBR, TK_HEX, TK_REG, TK_AND, TK_OR, TK_DEREF
  /* TODO: Add more token types */
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {
  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  /* PA1.5 */
  // TODO: rename all the token_type to enum.

  {" +", TK_NOTYPE},				// spaces
  {"\\+", TK_PLUS},					// plus
  {"==", TK_EQ},					// equal
  {"-", TK_SUB},					// substract
  {"\\*", TK_MUL},					// multiply or derefrence
  {"/", TK_DIV},					// divide
  {"0[Xx][0-9a-fA-F]+", TK_HEX},	// hex (must before the TK_NUM)
  {"[0-9]+", TK_NUM},				// number(dec)
  {"\\(", TK_LBR},					// left bracket
  {"\\)", TK_RBR},					// right bracket
  {"\\$[a-zA-Z]+", TK_REG},			// register
  {"&&", TK_AND},					// AND
  {"\\|\\|", TK_OR}					// OR
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;
  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

/* TODO: Change the size of the Token array for test */
#define NR_TOKEN 32
static Token tokens[NR_TOKEN] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;
        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

		/* TODO: PA1.5 */
		
		if(substr_len >= 32){
			printf("%.*s  The length of the substring is too long.\n", substr_len, substr_start);
			return false;
		}

		if(nr_token >= NR_TOKEN) {
			printf("The count of tokens(nr_token) is out of the maximum count(32)\n");
			return false;
		}
        switch (rules[i].token_type) {
			case TK_NOTYPE:
				break;
			case TK_NUM:
			case TK_HEX:
			case TK_REG:
				strncpy(tokens[nr_token].str, substr_start, substr_len);
				tokens[nr_token].str[substr_len] = '\0';
			default: 
				tokens[nr_token].type = rules[i].token_type;
				++ nr_token;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

/* PA1.5
 * Date: 2020/7/24
 */

/*  return 1  if the parentheses is a valid expression with left and right bracket.
 *  return 0  if the parentheses is a valid expression but without left and right bracket.
 *  return -1 if the parenteses isn't a valid expression. 
 */
int check_parentheses(int p, int q)
{
	bool LR = false;
	if(tokens[p].type == TK_LBR && tokens[q].type ==TK_RBR) LR = true;
	int top = 0, i;
	for(i = p; i <= q; ++ i){
		if(tokens[i].type == TK_LBR) ++ top;
		else if(tokens[i].type == TK_RBR) -- top;
		if(top < 0) return -1;
	}
	if(top != 0) return -1;
	if(!LR) return 0;

	/* Beware of such case like: (4 + 5) - (6 - 1)  */
	
	for(i = p + 1; i <= q - 1; ++ i){
		if(tokens[i].type == TK_LBR) ++ top;
		else if(tokens[i].type == TK_RBR) -- top;
		if(top < 0) return 0;
	}
	return 1;	// top must be zero
}

bool is_op(int ch){
	return ch == TK_PLUS || ch == TK_SUB || ch == TK_MUL || ch == TK_DIV
			|| ch == TK_AND || ch == TK_OR || ch == TK_EQ || ch == TK_DEREF;
}
/* return the priority of the oprator*/
int op_priority(int op)
{
	int pri;
	switch(op){
		case TK_OR:
			pri = 0;
			break;
		case TK_AND:
			pri = 1;
			break;
		case TK_EQ:
			pri = 2;
			break;
		case TK_PLUS:
		case TK_SUB :
			pri = 3;
			break;
		case TK_MUL:
		case TK_DIV:
			pri = 4;
			break;
		case TK_DEREF:
			pri = 5;
			break;
		default:
			pri = 10;
	}
	return pri;
}

int compare(int i, int j){
	int p1 = op_priority(tokens[i].type);
	int p2 = op_priority(tokens[j].type);
	return p1 < p2 ?  -1 : (p1 == p2 ? 0 : 1);
}

/* return -1 if there is no main operator */
int get_main_op(int p, int q)
{
	int inBracket = 0, i, pos = -1;
	for(i = p; i <= q; ++ i) {
		int type = tokens[i].type;
		if( !inBracket && is_op(type)){
			if(pos == -1) pos = i;
			else if(compare(i, pos) <= 0 ) pos = i; 
		}
		else if(type == TK_LBR ) inBracket ++ ;
		else if(type == TK_RBR ) inBracket -- ;
	}
	return pos;
}

uint32_t myexit(int p, int q, bool *success){
	printf("Invalid expression: [%d, %d]\n", p, q);
	for(; p <= q; ++ p){
		int type = tokens[p].type;
		printf("%d: %d", p, type);
		if(type == TK_NUM || type == TK_HEX || type == TK_REG)
			printf("- %s",tokens[p].str);
		printf("\n");
	}	
	*success = false;
	return 0;
}

/*add a parameter to judge whether the eval is success. */
uint32_t eval(int p, int q, bool *success)
{
	if(p > q) {
		return myexit(p, q, success);
	}else if(p == q){
		uint32_t val = 0;
		int type = tokens[p].type;
		if(type == TK_NUM || type == TK_HEX) {
			return strtoul(tokens[p].str, NULL, 0);
		}
		else if(type == TK_REG) {
			val = isa_reg_str2val(tokens[p].str + 1, success);
			if(*success) return val;
			printf("Unknown register: %s\n", tokens[p].str);
			return 0; 
		}
		return myexit(p, q, success);
	}

	int ret = check_parentheses(p, q);
	if(ret == -1) {
		return myexit(p, q, success);
	}
	
	if(ret == 1) {
		return eval(p + 1, q - 1, success);
	}
	
	int pos = get_main_op(p, q);
	if(pos == -1){
		return myexit(p, q, success);
	}
		
	uint32_t val1 = 0, val2 = 0, val = 0;
	if(tokens[pos].type != TK_DEREF)
		val1 = eval(p, pos - 1, success);

	if(*success == false) return 0;
	val2 = eval(pos + 1, q, success);
	if(*success == false) return 0;

	switch(tokens[pos].type){
		case TK_PLUS:
			val = val1 + val2;
			break;
		case TK_SUB:
			val = val1 - val2;
			break;
		case TK_MUL:
			val = val1 * val2;
			break;
		case TK_DIV:
			if(val2 == 0) {
				printf("Divide 0 error at [%d, %d]", p, q);
				return *success = false;
			}
			val = val1 / val2;
			break;
		case TK_AND:
			val = val1 && val2;
			break;
		case TK_OR:
			val = val1 || val2;
			break;
		case TK_EQ:
			val = val1 == val2;
			break;
		case TK_DEREF:
			val = vaddr_read(val2, 4);
			break;	
		default:
			printf("Unknown token type: %d\n", tokens[pos].type);
			return *success = false;
	}

	return val;
}

/* erase the index of the tokens array between [p, p + cnt - 1] */
void erase(int p, int cnt){
	int i;
	for(i = p; i + cnt < nr_token; ++ i){
		tokens[i] = tokens[i + cnt];
	}
	nr_token -= cnt;
}

/* PA1.5 1.6
 * Date: 2020/7/24
 */ 
uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  /* TODO: Insert codes to evaluate the expression. */

  /* Merge +- ++ -- -+ */  
  int i, type;
  for(i = 0; i < nr_token; ++ i){	
	type = tokens[i].type; 
	if(type == TK_PLUS || type == TK_SUB)
	{
		int j = i;
		int flag = 1;
		while(j < nr_token && (type == TK_PLUS || type == TK_SUB)){
			flag *= (type == TK_PLUS ? 1 : -1);
			type = tokens[++ j].type;
		}
		if(j - i > 1){
			tokens[i].type = (flag == 1? TK_PLUS : TK_SUB) ;
			erase(i + 1, j - i - 1);
		}
	}
  } 
   
  for(i = 0; i < nr_token; ++ i){
	if(tokens[i].type == TK_MUL && (i == 0 || is_op(tokens[i - 1].type))){
		if(i == 0 || tokens[i - 1].type != TK_DEREF)	// Maybe this case: **
			tokens[i].type = TK_DEREF;
	}
  }

  uint32_t ret = eval(0, nr_token - 1, success);
  return ret;
}
