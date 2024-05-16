//#include "expr_debug.h"
#include "expr.h"

//https://github.com/zserge/expr

/**

 float gauss_distrib( float proj ){
 //static int  first = 1;
 //if( first ){
 //  printf( "gaussian distribution roughness: %f\n", gauss_m );
 //  first = 0;
 //}
 CLAMP( proj, 0.0f, 1.0f );
 float  w = acosf( proj ) / gauss_m;
 return exp( -(w*w) );
 }

 **/
float my_acos(struct expr_func *f, vec_expr_t *args, void *c ){
  float a = expr_eval (&vec_nth(args, 0));
  return acosf(a);
}

float my_exp(struct expr_func *f, vec_expr_t *args, void *c ){
  float a = expr_eval (&vec_nth(args, 0));
  return exp(a);
}

float x = 0.0;
float my_in(){
  return x;
}

static struct expr_func user_funcs[] = {
  { "in", my_in, NULL, 0 },
  { "acos", my_acos, NULL, 0 },
  { "exp",  my_exp,  NULL, 0 },
  { NULL,   NULL,    NULL, 0 },
};

const char *s =
    "x = in()\n"
    "pi = 3.14159265\n"
    "r = acos( x ) * 180.0 / pi";

typedef struct expr_var expr_var_t;

int main() {
  struct expr_var_list vars = {0};
  struct expr *e = expr_create( s, strlen(s), &vars, user_funcs );
  if( e == NULL ){
    printf ("Syntax error");
    return 1;
  }

  x = 0.5f; // passed to evaluator via my_in()
  float result = expr_eval( e );
  printf( "*********\n");
  printf("result: %f\n", result);
  printf( "*********\n");
  expr_var_t *v = vars.head;
  while(v){
    printf( "%s: %f\n", v->name, v->value );
    v = v->next;
  }
  printf( "*********\n");
  expr_var_t *r = expr_var( &vars, "r", 1 );
  x = cosf( 45.0f * 3.14159265 / 180.0f );
  expr_eval( e );
  printf( "r value: %f\n", r->value );
  printf( "*********\n");

  expr_destroy( e, &vars );
  return 0;
}
