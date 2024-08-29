#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

#include <inttypes.h>
#include <float.h>
#include <windows.h>
#include <conio.h>
#include <string.h>

#define STACK_BUFFER_SIZE 1024
float get_uf32_limited(float min,float max);
int64_t get_i64_limited(int64_t min,int64_t max);
uint64_t get_u64_limited(uint64_t min,uint64_t max);
float  get_f32_limited(float min,float max);
double get_f64_limited(double min,double max);

static inline float get_uf32(){
    return(get_uf32_limited(0, FLT_MAX));
}

static inline uint8_t get_u8(){
    return (uint8_t)get_u64_limited(0,UINT8_MAX);
}

static inline uint16_t get_u16(){
    return (uint16_t)get_u64_limited(0,UINT16_MAX);
}

static inline uint32_t get_u32(){
    return (uint32_t)get_u64_limited(0,UINT32_MAX);
}

static inline uint64_t get_u64(){
    return get_u64_limited(0,UINT64_MAX);
}

static inline int8_t get_i8(){
    return (int8_t)get_i64_limited(INT8_MIN, INT8_MAX);
}

static inline int16_t get_i16(){
    return (int16_t)get_i64_limited(INT16_MIN,INT16_MAX);
}

static inline int32_t get_i32(){
    return (int32_t)get_i64_limited(INT32_MIN,INT32_MAX);
}
static inline int64_t get_i64(){
    return get_i64_limited(INT64_MIN,INT64_MAX);
}

static inline float get_f32(){
    return get_f32_limited(-FLT_MAX,FLT_MAX);
}

static inline double get_f64(){
    return get_f64_limited(-DBL_MAX,DBL_MAX);
}

float get_f32_limited(float min,float max){
    char rbuf[STACK_BUFFER_SIZE];
    char* endptr = NULL;
    float result;
    while(1){
        fgets(rbuf,STACK_BUFFER_SIZE,stdin);
        result = strtof(rbuf,&endptr);
        errno = 0;
        if(endptr > rbuf && errno == 0 && 
           result >= min && result <= max) break;
        printf("\033[A"); //sobe uma linha
        printf("\033[K"); //apaga a linha atual
        rbuf[0]='\0';
    }
    return result;
}

double get_f64_limited(double min,double max){
    char rbuf[STACK_BUFFER_SIZE];
    char* endptr = NULL;
    double result;
    while(1){
        fgets(rbuf,STACK_BUFFER_SIZE,stdin);
        errno = 0;
        result = strtod(rbuf,&endptr);
        if(endptr > rbuf && errno == 0 && 
           result >= min && result <= max) break;
        printf("\033[A"); //sobe uma linha
        printf("\033[K"); //apaga a linha atual
        rbuf[0]='\0';
    }
    return result;
}


uint64_t get_u64_limited(uint64_t min,uint64_t max){
    char rbuf[STACK_BUFFER_SIZE];
    char* endptr = NULL;
    uint64_t result;
    while(1){
        // if(GetAsyncKeyState(VK_ESCAPE)) return -1;
        fgets(rbuf,STACK_BUFFER_SIZE,stdin);
        errno = 0;
        result = (uint64_t)strtoull(rbuf,&endptr,10);
        if(endptr > rbuf && errno == 0 && 
           result >= min && result <= max) break;
        printf("\033[A"); //sobe uma linha
        printf("\033[K"); //apaga a linha atual
        rbuf[0]='\0';
    }
    return result;
}

int64_t get_i64_limited(int64_t min,int64_t max){
    char rbuf[STACK_BUFFER_SIZE];
    char* endptr = NULL;
    int64_t result;
    while(1){
        fgets(rbuf,STACK_BUFFER_SIZE,stdin);
        errno = 0;
        result = (int64_t) strtoll(rbuf,&endptr,10);
        if(endptr > rbuf && errno == 0 && 
           result >= min && result <= max) break;
        printf("\a");
        printf("\033[A"); //sobe uma linha
        printf("\033[K"); //apaga a linha atual
        rbuf[0]='\0';
    }
    return result;
}

#define TAMANHO_BUFFER 256

int64_t get_i64_specif(int16_t* especif, int16_t tamanho) {
    char rbuf[TAMANHO_BUFFER];
    char* endptr = NULL;
    int64_t resultado;

    while(1) {
        errno = 0;
        fgets(rbuf, TAMANHO_BUFFER, stdin);
        resultado = strtoll(rbuf, &endptr, 10);

        if (endptr == rbuf || errno != 0) {
            printf("\033[A"); //sobe uma linha
            printf("\033[K"); //apaga a linha atual
            rbuf[0]='\0';
            continue;
        }

        int match = 0;
        for(int i = 0; i < tamanho; i++) {
            if (resultado == especif[i]) {
                match = 1;
                break;
            }
        }

        if (match) {
            break;
        } else {
            printf("\tO numero digitado nao corresponde a nenhum valor especificado. Tente novamente!\n\t");
        }
    }

    return resultado;
}
float get_uf32_limited(float min,float max){
    char rbuf[STACK_BUFFER_SIZE];
    char* endptr = NULL;
    float result;
    while(1){
        fgets(rbuf,STACK_BUFFER_SIZE,stdin);
        result = strtof(rbuf,&endptr);
        errno = 0;
        if(endptr > rbuf && errno == 0 && 
           result >= min && result <= max && result>0) break;
        printf("\033[A"); //sobe uma linha
        printf("\033[K"); //apaga a linha atual
        rbuf[0]='\0';
    }
    return result;
}

char *s_gets(char *str, int size){
    fgets(str, size, stdin);
    str[strcspn(str, "\n")]='\0';
}