#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <windows.h>

#include <C:\Users\glauc\Documents\projetos_vscode\libs\get.h>

// OBS: ao longo deste codigo, a macro __DBL_MIN__ foi usada para indicar casos em que algum erro ocorreu, ao inves do convencional 0

#ifndef M_PI
  #define M_PI 3.14159265358979
#endif

#ifndef M_e
  #define M_e 2.7182818284
#endif

#ifndef EPSILON
  #define EPSILON 0.000001
#endif

#define NUM_X0 256

#define DIFERENCE_MODULE(x, y) fabs(fabs(x) - fabs(y))

#define SIZE_ARR(X) sizeof(X)/sizeof(X[0])

static bool isop(char ch);

static void skipspace(char **exp);

static void checkbrackets(char **exp);

static double select_angle_mode(void);

static char checkvar(char *exp);

double PI_(char **exp);

double isbracket(char **exp);

double euller_number(char **exp);

static double parse_tan(char **exp);

static double parse_sen(char **exp);

static double parse_cos(char **exp);

static double parse_ln(char **exp);

static double parse_log(char **exp);

static double parsepow(char **exp);

static double parsefactor(char **exp);

static double parseterm(char **exp);

static double parseexp(char **exp);

double eval_X(char *exp, double X);

double eval(char *exp);

char *func_composite(char *exp, const char *g_x);

void format_function(char **eq, char *left_side, char *right_side);

double solve(char *exp, double *root, uint16_t *count_root);

double newton_method(char *exp, double x0);

int64_t fat(int8_t N);

bool eval_bool(char *exp);

int filter_equal_root(double *arr, double *root);

size_t select_root(char *func, double *root);

typedef double (*func_ptr)(char **exp);

typedef enum
{
  DEGREE = 10,
  RAD
} angle_mode;

typedef enum
{
  MULTIPLY_VAR = -1,
  NOT_VAR
} status_var;

typedef enum
{
  LEFT,
  RIGHT
} side_type;

typedef enum
{
  NAN_ = -1,
  NUMBER,
  CRITICAL_POINT
} status_newton_method;

typedef enum
{
  EVAL = 9, BOOLE, EQUATION, FORMAT_ERROR, INVALID_CHAR 
}status_solve;

typedef struct
{
  const char *name;
  func_ptr function;
} func_map;

angle_mode angle = RAD;
status_newton_method except = NUMBER;
status_solve except_solve = EVAL;

signed char var = 'x';

char valid_operation[] = {
  '+', '-', '*', '/', '(', ')', '!', '^', '='
};

// a cada nova funcao ou constante adicionada, modificar isFunc();

func_map functions[] = { //mapa contendo as funcoes e suas respectivas strings
    {"tan(", parse_tan},
    {"sen(", parse_sen},
    {"cos(", parse_cos},
    {"log(", parse_log},
    {"ln(", parse_ln},
    {"pi", PI_},
    {"e", euller_number},
    {"(", isbracket}};

bool isop(char ch) {
  bool chisop = false;
  for(int i = 0; (size_t)i < SIZE_ARR(valid_operation); i++) {
    chisop = (ch == valid_operation[i]);
    if(chisop) break;
  }
  return chisop ? 1 : 0;
}

/**
 * @brief funcao que compara dois valores. usada na comparacao da funcao qsort (que usa quick sort para ordenar arrays)
 * 
 * @param a
 * @param b 
 * @return -1 se a < b, 1 se a > b e 0 se a == b 
 */
int comp(const void *a, const void *b)
{
  if (*(double *)a < *(double *)b)
    return -1;
  else if (*(double *)a > *(double *)b)
    return 1;

  return 0;
}

/**
 * @brief passa os espaços da expressao
 * 
 * @param exp 
 */
void skipspace(char **exp)
{
  while (isspace(**exp))
  {
    (*exp)++;
  }
}

  /**
   * @brief converte a expressao str em minusculo
   * 
   * @param str 
   */
void strlower(char *str)
{
  while (*str)
  {
    if (*str >= 'A' && *str <= 'Z')
    {
      *str += 'a' - 'A';
    }
    str++;
  }
}

/**
 * @brief analisa a variavel global "angle" que diz se a calculadora esta configurada 
 * em radiano ou grau e retorna a conversao correspondente
 * 
 * @return double 
 */
double select_angle_mode()
{
  return (angle == RAD ? 1 : M_PI / 180);
}

/**
 * @brief calcula o fatorial de um inteiro de 64 bits
 * 
 * @param N 
 * @return int64_t 
 */
int64_t fat(int8_t N)
{
  int64_t fatorial = 1;
  if (N == 0)
    return 1;

  if (N < 0)
  {
    for (int16_t i = -1; i >= N; i--)
    {
      fatorial *= i;
    }
    return fatorial;
  }
  for (int16_t i = 1; i <= N; i++)
  {
    fatorial *= i;
  }
  return fatorial;
}

/**
 * @brief calcula a derivada da funcao num ponto
 * 
 * @param exp 
 * @param value 
 * @return double 
 */
double derivate(char *exp, double value)
{
  angle = RAD;
  // metodo de derivada de aproximacao de ordem 2
  return ((eval_X(exp, (value + EPSILON)) - eval_X(exp, (value - EPSILON))) / (2 * EPSILON));
}

/**
 * @brief analisa se uma expressao que comeca com parenteses é devidamente fechada
 * 
 * @param exp 
 */
void checkbrackets(char **exp)
{
  if (**exp != ')')
  {
    printf("expressao mal formada: expera-se ')'\n");
    return;
  }
  (*exp)++;
}

/**
 * @brief analisa de antemao se existe algum caracter nao mapeado na string
 * 
 * @param iterator 
 */
void check_invalid_char(char* iterator) {
while(*iterator) {
    if(!isalnum(*iterator) && !isop(*iterator) && !isspace(*iterator)){
      printf("caracter invalido: '%c'\n", *iterator);
      except_solve = INVALID_CHAR;
    }
    iterator++;
  }
}

/**
 * @brief avalia se o ponteiro atual esta apontando para um numero e o retorne. caso nao, retorne __DBL_MIN__
 * 
 * @param exp 
 * @return double 
 */
double parse_number(char **exp)
{
  skipspace(exp);

  char *endptr;

  double result = strtod(*exp, &endptr);

  if (*exp == endptr && **exp != '-' && **exp != '+')
  {
    printf("espera-se um numero! '%c'\n", **exp);
    result = 0;
  }

  *exp = endptr;
  return result;
}

double parse_tan(char **exp)
{
  *exp += strlen("tan(");
  double result = tan(parseexp(exp) * select_angle_mode());

  checkbrackets(exp);
  return result;
}

double parse_sen(char **exp)
{
  *exp += strlen("sen(");
  double result = sin(parseexp(exp) * select_angle_mode());

  checkbrackets(exp);
  return result;
}

double parse_cos(char **exp)
{
  *exp += strlen("cos(");
  double result = cos(parseexp(exp) * select_angle_mode());

  checkbrackets(exp);
  return result;
}

double parse_log(char **exp)
{
  *exp += strlen("log(");
  double result = log10(parseexp(exp));

  checkbrackets(exp);
  return result;
}

double parse_ln(char **exp)
{
  *exp += strlen("ln(");
  double result = log(parseexp(exp));

  checkbrackets(exp);
  return result;
}

double euller_number(char **exp)
{
  *exp += strlen("e");
  return M_e;
}

double PI_(char **exp)
{
  *exp += strlen("pi");
  return M_PI;
}

double isbracket(char **exp)
{
  ++(*exp);
  double result = parseexp(exp);

  checkbrackets(exp);

  return result;
}

double parsepow(char **exp)
{
  skipspace(exp);

  // checa se a posicao atual do ponteiro é uma checa se é uma das funcoes mapeadas (sen, cos, log, etc)
  for (size_t i = 0; i < sizeof(functions) / sizeof(func_map); i++)
  {
    if (strncmp(*exp, functions[i].name, strlen(functions[i].name)) == 0)
    {
      return functions[i].function(exp);
    }
  }
  // default : recebe o numero
  return parse_number(exp);
}

double parsefactor(char **exp)
{
  skipspace(exp);

  double factor = parsepow(exp);
  double result = factor;

  skipspace(exp);

  while (**exp == '^' || **exp == '!')
  {
    char op = **exp;
    (*exp)++;

    // usa if-else porque se for fatorial, nao deve chamar o proximo fator
    if (op == '!')
      result = fat(result);
    else
    {
      double nextfactor = parsepow(exp);
      result = pow(result, nextfactor);
    }

    skipspace(exp);
  }
  return result;
}

double parseterm(char **exp)
{
  skipspace(exp);

  double factor = parsefactor(exp);
  double result = factor;

  skipspace(exp);

  while (**exp == '*' || **exp == '/')
  {
    char op = **exp;

    (*exp)++; // lembre-se de incrementar o ponteiro para nao analisar de novo o sinal e dar erro no retorno

    double nextfactor = parsefactor(exp);
    result *= (op == '*') ? nextfactor : 1 / nextfactor;

    skipspace(exp);
  }
  return result;
}

double parseexp(char **exp)
{
  skipspace(exp);

  double term = parseterm(exp);
  double result = term;

  skipspace(exp);

  while (**exp == '+' || **exp == '-')
  {
    char op = **exp;

    (*exp)++;

    double nextterm = parseterm(exp);
    result += (op == '+') ? nextterm : -nextterm;

    skipspace(exp);
  }

  return result;
}

/**
 * @brief analisa uma expressao numerica e retorna seu resultado
 * 
 * @param exp 
 * @return double 
 */
double eval(char *exp)
{
  strlower(exp);
  return parseexp(&exp);
}

// analise de expressao

/**
 * @brief adiciona um valor X na funcao no lugar da variavel
 * 
 * @param exp funcao 
 * @param X valor a ser substituido por X (ou var) na funcao
 * @return resultado da f(x) 
 */
double eval_X(char *exp, double X)
{

  strlower(exp);
  // array que vai guardar a nova funcao com o conteudo de X
  char func_x[4 * strlen(exp)];

  for (size_t i = 0; i < strlen(exp); i++)
  {
    func_x[i] = 0;
  }

  char *iterator = func_x;

  while (*exp)
  {
    if (*exp == var)
    {
      char value[NUM_X0];

      sprintf(value, "%lf", X);

      strcat(func_x, value);
      iterator += strlen(value);
    }
    else
    {
      *iterator = *exp;
      *(++iterator) = '\0';
    }
    exp++;
  }

  char *ptr = func_x;
  double result = parseexp(&ptr);

  return result;
}

/**
 * @brief analisa se o ponteiro na posicao atual é uma funcao e retorna seu tamanho para que o programa possa pula-la
 * 
 * @param exp 
 * @return int 
 */
int isFunc(char *exp)
{
  // cada nova constante definida deve ser adicionada aqui
  for (size_t i = 0; i < sizeof(functions) / sizeof(func_map); i++)
  {
    if (strncmp(exp, functions[i].name, strlen(functions[i].name)) == 0)
    {
      return strlen(functions[i].name);
    }
  }

  return 0;
}

/**
 * @brief formata uma fequacao em uma funcao. adiciona o caracter '*' em casos como "2x", elimina espaços
 * passa tudo para um lado da equacao e checa se a equacao esta adequada
 * 
 * @param func 
 * @param left_side 
 * @param right_side 
 */
void format_function(char **func, char *left_side, char *right_side)
{
  strlower(*func);

  while (*left_side)
  {

    strncat(*func, left_side, 1);

    // caso um numero e uma letra estejam lado a lado, adiciona o caracter vezes
    if (isdigit(*left_side) && isalpha(*(left_side + 1)))
    {
      strcat(*func, "*");
    }
    left_side++;

    skipspace(&left_side);
  }

  // caso nao tenha sinal de igual, a funcao esta formatada enquanto aos lados
  if (right_side == NULL)
    return;

  skipspace(&right_side);
  // caso o primeiro elemento do lado direito apos passar espacos nao seja sinal, coloca um sinal de '-' no lado esquerdo
  if (*right_side != '+' && *right_side != '-')
  {
    strcat(*func, "-");
  }

  while (*right_side)
  {

    switch (*right_side)
    {
    case '+':
      strcat(*func, "-");
      break;
    case '-':
      strcat(*func, "+");
      break;
    case ' ':
      break;
    default:
      strncat(*func, right_side, 1);
      break;
    }
    if (isdigit(*right_side) && isalpha(*(right_side + 1)))
    {
      strcat(*func, "*");
    }

    right_side++;
  }
}

/**
 * @brief soluciona a equacao guardada na string exp
 * 
 * @param exp expressao a ser resolvida
 * @param root array que guarda as raizes
 * @param count_root ponteiro para a quantidade de raizes
 * @return int 
 */
double solve(char *exp, double *root, uint16_t *count_root)
{

  char *lado_esquerdo = exp;
  char *lado_direito = strchr(exp, '=');

  char *func = (char *)calloc(strlen(exp), sizeof(char));

  var = checkvar(exp);

  // caso nao tenha sinal de igual ou tenha mais de um, a expressao é invalida
  if (lado_direito == NULL && !var)
  {
    format_function(&func, exp, NULL);
    except_solve = EVAL;

    return eval(func);
  }
  else if (lado_direito == NULL)
  {
    printf("formato de equacao invalido: necessario sinal de igual '='\n");
    except_solve = FORMAT_ERROR;
    return DBL_MIN;
  }
  else if (!var)
  {
    except_solve = BOOLE;
    if (eval_bool(exp))
    {
      printf("true\n");
      return 1;
    }
    printf("false\n");
    return 0;
  }

  if (var == MULTIPLY_VAR)
  {
    printf("equacao com muitas variaveis\n");
    except_solve = FORMAT_ERROR;
    return DBL_MIN;
  }

  // se a primeira ocorrencia estiver em posicao diferente da segunda, entao existe mais de um sinal de igual
  if (lado_direito != strrchr(exp, '='))
  {
    printf("formato de equacao invalido: equacoes possuem apenas uma igualdade\n");
    except_solve = FORMAT_ERROR;
    return DBL_MIN;
  }

  check_invalid_char(exp);

  if(except_solve == INVALID_CHAR) {
    return DBL_MIN;
  }

  char* equal_sign = lado_direito;
  *lado_direito++ = '\0';

  except_solve = EQUATION;
  format_function(&func, lado_esquerdo, lado_direito); // remove espaços e trsnsforma 2x em 2*x

  *count_root = select_root(func, root);

  free(func);
  *equal_sign = '=';
  return *count_root;
}

size_t select_root(char *func, double *root)
{
  double teste[NUM_X0];

  // preenche teste com numeros "aleatorios" de -100 a 100
  for (int i = 0; i < NUM_X0; i++)
    teste[i] = (double)-100 + rand() / (double)RAND_MAX * (100 + 100);

  // adiciona cada raiz proveniente do metodo de newton
  for (int i = 0; i < NUM_X0; i++)
    teste[i] = newton_method(func, teste[i]);

  // ordena as raizes para armazenar apenas as distintas no array "root"
  qsort(teste, NUM_X0, sizeof(double), comp);
  int count = filter_equal_root(teste, root);

  return count;
}

int filter_equal_root(double *arr, double *root)
{
  int i = 0, count = 0;

  // percorre todo o array
  while (i < NUM_X0)
  {
    if ( ((DIFERENCE_MODULE(arr[i], arr[i+1]) < EPSILON ) && i < NUM_X0 - 1) )
    {
      i++;
      continue;
    }
    // caso o conteudo do array secounta dbl_min, significa que um erro ocorreu
    if (arr[i] != __DBL_MIN__)
      root[count++] = arr[i];
    i++;
  }
  return count;
}

double newton_method(char *exp, double x0)
{

  double x = x0;
  for (unsigned i = 0; i < 1000; i++)
  {

    double f_x = eval_X(exp, x0);
    if (f_x == __DBL_MIN__)
      return f_x;

    double f_x_derivate = derivate(exp, x0);

    x = x0 - f_x / f_x_derivate;

    if (fabs(f_x) < EPSILON)
    {
      except = NUMBER;
      return x;
    }

    // tratamento de erros

    // caso um erro ocorrer, _DBL_MIN__ é retornado;
    if (isnan(x))
    {
      except = NAN_;
      return __DBL_MIN__;
    }

    if (fabs(f_x_derivate) < EPSILON)
    {
      except = CRITICAL_POINT;
      return __DBL_MIN__;
    }
    x0 = x;
  }
  return x;
}

char checkvar(char *exp)
{
  int i = 0;
  char var[64];

  // analisa a expressao
  while (*exp)
  {
    int len_func = isFunc(exp);

    if (len_func)
    {
      exp += len_func;
    }
    else if (isalpha(*exp))
    {
      var[i++] = *exp;
      exp++;
    }
    else
    {
      exp++;
    }
  }
  int count = i;

  if (count == 0)
    return NOT_VAR;

  i = 0;

  while (i < count)
  {
    if (var[i] != var[0])
    {
      return MULTIPLY_VAR;
    }
    i++;
  }
  return var[0];
}

/**
 * @brief essa analisa uma expressao matematica e retorna o valor logico da igualdade. ex: 1 = 1 retorna true
 * 
 * @param exp ponteiro para a expressao a ser analisada
 * @return true 
 * @return false 
 */
bool eval_bool(char *exp)
{
  char *lado_esquerdo = exp;
  char *lado_direito = strchr(exp, '=');

  if (lado_direito != strrchr(exp, '='))
  {
    printf("formato invalido para equacoes booleanas!\n");
    return 0;
  }
  *lado_direito++ = '\0';

  return fabs(fabs(eval(lado_esquerdo)) - fabs(eval(lado_direito))) < EPSILON;
}

/**
 * @brief compoe duas funcoes
 * 
 * @param exp 
 * @param g_x 
 * @return ponteiro para a funcao composta
 */
char *func_composite(char *exp, const char *g_x)
{
  strlower(exp);

  // string que vai guardar a nova funcao com o conteudo de X
  char *func_x = malloc(16 * strlen(exp));

  for (size_t i = 0; i < strlen(exp); i++)
  {
    func_x[i] = 0;
  }

  char *iterator = func_x;

  while (*exp)
  {
    if (*exp == var)
    {
      strcat(func_x, g_x);
      iterator += strlen(g_x);
    }
    else
    {
      *iterator = *exp;
      *(++iterator) = '\0';
    }
    exp++;
  }

  return func_x;
}

int main()
{
SetConsoleOutputCP(CP_UTF8);

srand(time(NULL));

angle = RAD;
double root[64] = {};
uint16_t count;

char exp[512];

printf("\nescreva uma expressao(equacao, numerica ou booleana):\n\n");

fgets(exp, 512, stdin);
exp[strcspn(exp, "\n")] = '\0';

  double result = solve(exp, root, &count);

if(except_solve == EVAL) {
  printf("%s = %lf", exp, result);
}
else if(except_solve == EQUATION){
  printf("as solucoes reais de %s são: \n", exp);
  for(int i = 0; i < count; i++) {
    printf("%c = %lf\n", var, root[i]);
  }
}
  
return 0;
}