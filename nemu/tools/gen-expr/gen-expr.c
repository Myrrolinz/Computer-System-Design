#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#define NR_TOKEN 500
#define MAXNUM 1000

// this should be enough
static char buf[60000];
int len, nr;

static uint32_t choose(uint32_t n){
	return rand() % n;	
}

static void gen_num(){
	if(nr >= NR_TOKEN) return ;
	int x = choose(MAXNUM) + 1;
	len += sprintf(buf + len, "%d", x);
	++ nr;
}

static void gen(char ch){
	if(nr >= NR_TOKEN && ch != ')') return ;
	buf[len ++] = ch; 
	++ nr;
}

static void gen_rand_op(){
	if(nr >= NR_TOKEN) return ;
	static char ops[] = "+-*/";
	buf[len ++] = ops[choose(4)];
	++ nr;
}

/* PA1.5
 * Date: 2020/7/24 
 */
static inline void gen_rand_expr() {
	if(nr >= NR_TOKEN) return ;
	switch(choose(3)){
		case 0:	gen_num(); break;
		case 1:
			if(nr + 3 > NR_TOKEN) return ;	
			gen('('); gen_rand_expr(); gen(')');
			break;
		default:
			if(nr + 3 > NR_TOKEN) return ;	
			gen_rand_expr(); gen_rand_op(); gen_rand_expr();
			break;
	}
}

static char code_buf[65536];
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
	len = 0;
	nr = 0;
    gen_rand_expr();
	buf[len] = '\0';
    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
