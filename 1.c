/************************************************************
 * compiler_complete.c – Custom Language Compiler (Lexer with DFA, LL(1) Parser)
 ************************************************************/

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXTOKENS 50000
#define MAXLINE 1024
#define TOKFILE "tokens.txt"

#define NUM_STATES 84
#define NUM_INPUTS 36
#define DEAD 83

// Token types
#define T_INCLUDE 'I'
#define T_TYPE 'T'
#define T_FUNC 'F'
#define T_VAR 'V'
#define T_NUM 'N'
#define T_PRINTF 'P'
#define T_WHILE 'W'
#define T_BREAK 'K'
#define T_RETURN 'R'
#define T_MAIN 'M'
#define T_LOOP 'L'
#define T_BRACKET 'B'
#define T_OP 'O'
#define T_STMT 'S'

/* DFA states */
enum {
  D0,
  D1,
  D2,
  D3,
  D4,
  D5,
  D6,
  D7,
  D8,
  D9,
  D10,
  D11,
  D12,
  D13,
  D14,
  D15,
  D16,
  D17,
  D18,
  D19,
  D20,
  D21,
  D22,
  D23,
  D24,
  D25,
  D26,
  D27,
  D28,
  D29,
  D30,
  D31,
  D32,
  D33,
  D34,
  D35,
  D36,
  D37,
  D38,
  D39,
  D40,
  D41,
  D42,
  D43,
  D44,
  D45,
  D46,
  D47,
  D48,
  D49,
  D50,
  D51,
  D52,
  D53,
  D54,
  D55,
  D56,
  D57,
  D58,
  D59,
  D60,
  D61,
  D62,
  D63,
  D64,
  D65,
  D66,
  D67,
  D68,
  D69,
  D70,
  D71,
  D72,
  D73,
  D74,
  D75,
  D76,
  D77,
  D78,
  D79,
  D80,
  D81,
  D82
};

/* input-symbol enum (must match next_state column order) */
enum {
  HASH,
  LETTER_i,
  LETTER_n,
  LETTER_c,
  LETTER_l,
  LETTER_u,
  LETTER_d,
  LETTER_e,
  LT_ANGLE,
  LETTER_s,
  LETTER_t,
  LETTER_o,
  DOT,
  LETTER_h,
  GT_ANGLE,
  LETTER_r,
  LETTER_b,
  LETTER_k,
  LETTER_w,
  LETTER_a,
  LETTER_f,
  LETTER_m,
  LETTER_p,
  LETTER_F,
  UNDERSCORE,
  DIGIT,
  LETTER_OTHER,
  PLUS,
  EQUAL,
  COMMA,
  COLON,
  LPAREN,
  RPAREN,
  LBRACE,
  RBRACE,
  OTHER_INPUT
};

static inline int is_digit_char(char c) { return c >= '0' && c <= '9'; }
static inline int is_lower_char(char c) { return c >= 'a' && c <= 'z'; }
static inline int is_upper_char(char c) { return c >= 'A' && c <= 'Z'; }

// input mapping
int get_input(char c) {
  if (c == '#')
    return HASH;
  if (c == 'i')
    return LETTER_i;
  if (c == 'n')
    return LETTER_n;
  if (c == 'c')
    return LETTER_c;
  if (c == 'l')
    return LETTER_l;
  if (c == 'u')
    return LETTER_u;
  if (c == 'd')
    return LETTER_d;
  if (c == 'e')
    return LETTER_e;
  if (c == '<')
    return LT_ANGLE;
  if (c == 's')
    return LETTER_s;
  if (c == 't')
    return LETTER_t;
  if (c == 'o')
    return LETTER_o;
  if (c == '.')
    return DOT;
  if (c == 'h')
    return LETTER_h;
  if (c == '>')
    return GT_ANGLE;
  if (c == 'r')
    return LETTER_r;
  if (c == 'b')
    return LETTER_b;
  if (c == 'k')
    return LETTER_k;
  if (c == 'w')
    return LETTER_w;
  if (c == 'a')
    return LETTER_a;
  if (c == 'f')
    return LETTER_f;
  if (c == 'm')
    return LETTER_m;
  if (c == 'p')
    return LETTER_p;
  if (c == 'F')
    return LETTER_F;
  if (c == '_')
    return UNDERSCORE;
  if (is_digit_char(c))
    return DIGIT;
  if (is_lower_char(c) || is_upper_char(c))
    return LETTER_OTHER;
  if (c == '+')
    return PLUS;
  if (c == '=')
    return EQUAL;
  if (c == ',')
    return COMMA;
  if (c == ':')
    return COLON;
  if (c == '(')
    return LPAREN;
  if (c == ')')
    return RPAREN;
  if (c == '{')
    return LBRACE;
  if (c == '}')
    return RBRACE;
  /* whitespace / unrecognized -> OTHER_INPUT (handled outside DFA) */
  return OTHER_INPUT;
}

/* accepting_tokens[state] -> token character (0 = non-accepting) */
char accepting_tokens[NUM_STATES] = {
    /* D0..D16 */ 0,  0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* D10..D16 */ 0, 0, 0, 0, 0, 0, 0,
    /* D17 */ 'I',
    /* D18..D19 */ 0, 0,
    /* D20 */ 'T',
    /* D21..D22 */ 0, 0,
    /* D23 */ 'T',
    /* D24..D28 */ 0, 0, 0, 0, 0,
    /* D29 */ 'R',
    /* D30..D33 */ 0, 0, 0, 0,
    /* D34 */ 'K',
    /* D35..D38 */ 0, 0, 0, 0,
    /* D39 */ 'W',
    /* D40..D42 */ 0, 0, 0,
    /* D43 */ 'M',
    /* D44..D48 */ 0, 0, 0, 0, 0,
    /* D49 */ 'P',
    /* D50 */ 0,
    /* D51..D54 */ 0, 0, 0, 0,
    /* D55 */ 'F',
    /* D56 */ 0,
    /* D57 */ 'F',
    /* D58..D59 */ 0, 0,
    /* D60..D63 */ 0, 0, 0, 0,
    /* D64..D66 */ 0, 0, 0,
    /* D67 */ 0,
    /* D68 */ 0,
    /* D69 */ 'V',
    /* D70 */ 0,
    /* D71 */ 'N',
    /* D72..D73 */ 0, 0,
    /* D74..D80 */ 0, 0, 0, 0, 0, 0, 0,
    /* D81 */ 0,
    /* D82 */ 'S',
    /* DEAD */ 0};

/* next_state[state][input] (36 columns) */
int next_state[NUM_STATES][NUM_INPUTS] = {

    /* D0 */ {D1,  D18, D56,  D56, D60, D56, D21, D56, D74, D56, D56, D56,
              D81, D56, DEAD, D24, D30, D56, D35, D56, D56, D40, D44, D56,
              D51, D70, D56,  D72, D73, D75, D76, D77, D78, D79, D80, DEAD},
    /* D1 */ {DEAD, D2,   DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
              DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
              DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
              DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D2 */ {DEAD, DEAD, D3,   DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
              DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
              DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
              DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D3 */ {DEAD, DEAD, DEAD, D4,   DEAD, DEAD, DEAD, DEAD, DEAD,
              DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
              DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
              DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D4 */ {DEAD, DEAD, DEAD, DEAD, D5,   DEAD, DEAD, DEAD, DEAD,
              DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
              DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
              DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D5 */ {DEAD, DEAD, DEAD, DEAD, DEAD, D6,   DEAD, DEAD, DEAD,
              DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
              DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
              DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D6 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, D7,   DEAD, DEAD,
              DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
              DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
              DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D7 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, D8,   DEAD,
              DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
              DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
              DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D8 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, D9,
              DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
              DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
              DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D9 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
              D10,  DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
              DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
              DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D10 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, D11,  DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D11 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, D12,  DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D12 */ {DEAD, D13,  DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D13 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, D14,  DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D14 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, D15,  DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D15 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, D16,  DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D16 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, D17,  DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D17 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D18 */ {DEAD, DEAD, D19,  DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D19 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, D20,  DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D20 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D21 */ {DEAD, DEAD, DEAD, D22,  DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D22 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, D23,  DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D23 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D24 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, D25,  DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D25 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, D26,  DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D26 */ {DEAD, DEAD, DEAD, DEAD, DEAD, D27,  DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D27 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, D28,  DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D28 */ {DEAD, DEAD, D29,  DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D29 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D30 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, D31,  DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D31 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, D32,  DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D32 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, D33,  DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D33 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, D34,  DEAD, DEAD, D34,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D34 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               D35,  DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D35 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, D36,  DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D36 */ {DEAD, D37,  DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D37 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, D38,  DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D38 */ {DEAD, DEAD, DEAD, DEAD, D39,  DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D39 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D40 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, D41,  DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D41 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, D42,  DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D42 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, D43,  DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D43 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D44 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, D45,  DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D45 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, D46,  DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D46 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, D47,  DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D47 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, D48,  DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D48 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, D49,  DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D49 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D50 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D51 */ {DEAD, D52,  D52,  D52,  D52,  D52,  D52,  D52,  DEAD,
               D52,  D52,  D52,  DEAD, D52,  DEAD, D52,  D52,  D52,
               D52,  D52,  D52,  D52,  D52,  D52,  DEAD, DEAD, D52,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D52 */ {DEAD, D53,  D53,  D53,  D53,  D53,  D53,  D53,  DEAD,
               D53,  D53,  D53,  DEAD, D53,  DEAD, D53,  D53,  D53,
               D53,  D53,  D53,  D53,  D53,  D53,  DEAD, D54,  D53,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D53 */ {DEAD, D53,  D53,  D53,  D53,  D53,  D53,  D53,  DEAD,
               D53,  D53,  D53,  DEAD, D53,  DEAD, D53,  D53,  D53,
               D53,  D53,  D53,  D53,  D53,  D53,  DEAD, D54,  D53,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D54 */ {DEAD, D55,  D55,  D55,  D55,  D55,  D55,  D55,  DEAD,
               D55,  D55,  D55,  DEAD, D55,  DEAD, D55,  D55,  D55,
               D55,  D55,  D55,  D55,  D55,  D55,  DEAD, DEAD, D55,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D55 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D56 */ {DEAD, D57,  D57,  D57,  D57,  D57,  D57,  D57,  DEAD,
               D57,  D57,  D57,  DEAD, D57,  DEAD, D57,  D57,  D57,
               D57,  D57,  D57,  D57,  D57,  D58,  DEAD, DEAD, D57,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D57 */ {DEAD, D57,  D57,  D57,  D57,  D57,  D57,  D57,  DEAD,
               D57,  D57,  D57,  DEAD, D57,  DEAD, D57,  D57,  D57,
               D57,  D57,  D57,  D57,  D57,  D58,  DEAD, DEAD, D57,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D58 */ {DEAD, DEAD, D59,  DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D59 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D60 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D61 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D62 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D63 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D64 */ {DEAD, D65,  D65,  D65,  D65,  D65,  D65,  D65,  DEAD,
               D65,  D65,  D65,  DEAD, D65,  DEAD, D65,  D65,  D65,
               D65,  D65,  D65,  D65,  D65,  D65,  DEAD, DEAD, D65,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D65 */ {DEAD, D66,  D66,  D66,  D66,  D66,  D66,  D66,  DEAD,
               D66,  D66,  D66,  DEAD, D66,  DEAD, D66,  D66,  D66,
               D66,  D66,  D66,  D66,  D66,  D66,  DEAD, D67,  D66,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D66 */ {DEAD, D66,  D66,  D66,  D66,  D66,  D66,  D66,  DEAD,
               D66,  D66,  D66,  DEAD, D66,  DEAD, D66,  D66,  D66,
               D66,  D66,  D66,  D66,  D66,  D66,  DEAD, D67,  D66,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D67 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, D68,  DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D68 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, D69,  DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D69 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D70 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, D71,  DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D71 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, D71,  DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D72 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D73 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D74 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D75 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D76 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D77 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D78 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D79 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D80 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D81 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, D82,  DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* D82 */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
               DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
    /* DEAD */ {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
                DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
                DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD,
                DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
};

/* init_dfa - no dynamic init required since DFA is static */
void init_dfa() { /* nothing to do */ }

/* Helper functions to avoid using strncmp (explicit char-wise comparisons) */

/* returns true if s starts with prefix */
static bool starts_with(const char *s, const char *prefix) {
  while (*prefix) {
    if (*s == '\0' || *s != *prefix)
      return false;
    s++;
    prefix++;
  }
  return true;
}

/* classify keywords by direct char checks (no strncmp) */
char classify_keyword_or_identifier(const char *word, int len) {
  if (len == 3 && word[0] == 'i' && word[1] == 'n' && word[2] == 't')
    return T_TYPE;
  if (len == 3 && word[0] == 'd' && word[1] == 'e' && word[2] == 'c')
    return T_TYPE;
  if (len == 6 && word[0] == 'p' && word[1] == 'r' && word[2] == 'i' &&
      word[3] == 'n' && word[4] == 't' && word[5] == 'f')
    return T_PRINTF;
  if (len == 5 && word[0] == 'w' && word[1] == 'h' && word[2] == 'i' &&
      word[3] == 'l' && word[4] == 'e')
    return T_WHILE;
  if (len == 5 && word[0] == 'b' && word[1] == 'r' && word[2] == 'e' &&
      word[3] == 'a' && word[4] == 'k')
    return T_BREAK;
  if (len == 6 && word[0] == 'r' && word[1] == 'e' && word[2] == 't' &&
      word[3] == 'u' && word[4] == 'r' && word[5] == 'n')
    return T_RETURN;
  if (len == 4 && word[0] == 'm' && word[1] == 'a' && word[2] == 'i' &&
      word[3] == 'n')
    return T_MAIN;
  return 'O'; /* 'O' used in this DFA as generic identifier token */
}

/* Function name detection (end with 'Fn' and only alphabets) */
bool is_function_name(const char *word, int len) {
  if (len < 3)
    return false;
  if (word[len - 2] != 'F' || word[len - 1] != 'n')
    return false;
  for (int i = 0; i < len; ++i) {
    if (!isalpha((unsigned char)word[i]))
      return false;
  }
  return true;
}

/* Loop label detection using starts_with instead of strncmp */
bool is_loop_label(const char *word, int len) {
  /* pattern: loop_<letters><2digits>:  (e.g. loop_main01:) */
  if (len < 8)
    return false;
  if (!starts_with(word, "loop_"))
    return false;
  if (word[len - 1] != ':')
    return false;
  if (!isdigit((unsigned char)word[len - 3]) ||
      !isdigit((unsigned char)word[len - 2]))
    return false;
  for (int i = 5; i < len - 3; i++) {
    if (!isalpha((unsigned char)word[i]) && word[i] != '_')
      return false;
  }
  return true;
}

/* DFA-based classification of a token string into a single-character token
 * symbol */
char dfa_classify(const char *word, int len, bool is_first_line) {
  if (is_first_line)
    return T_INCLUDE;
  if (len == 2 && word[0] == '.' && word[1] == '.')
    return T_STMT;
  if (is_loop_label(word, len))
    return T_LOOP;

  int state = 0;
  int last_accepting_state = -1;
  char last_token = 0;

  for (int i = 0; i < len; ++i) {
    int idx = get_input(word[i]);
    if (idx < 0 || idx >= NUM_INPUTS)
      idx = OTHER_INPUT;
    state = next_state[state][idx];
    if (state == DEAD)
      break;
    if (accepting_tokens[state] != 0) {
      last_accepting_state = state;
      last_token = accepting_tokens[state];
    }
  }

  if (last_accepting_state != -1 && last_token != 0) {
    // Check for keywords
    if (last_token == 'T')
      return T_TYPE;
    if (last_token == 'R')
      return T_RETURN;
    if (last_token == 'K')
      return T_BREAK;
    if (last_token == 'W')
      return T_WHILE;
    if (last_token == 'M')
      return T_MAIN;
    if (last_token == 'P')
      return T_PRINTF;
    if (last_token == 'F')
      return T_FUNC;
    if (last_token == 'V')
      return T_VAR;
    if (last_token == 'N')
      return T_NUM;
    if (last_token == 'S')
      return T_STMT;
    if (last_token == 'I')
      return T_INCLUDE;
    return T_OP;
  }

  // Check manually for keywords using helper
  char k = classify_keyword_or_identifier(word, len);
  if (k != 'O')
    return k;
  if (is_function_name(word, len))
    return T_FUNC;

  return T_OP;
}

/* --- LEXER --- */
int run_lexer(const char *input_filename) {
  FILE *fin = fopen(input_filename, "r");
  if (!fin) {
    fprintf(stderr, "Cannot open input file '%s'\n", input_filename);
    return 1;
  }

  FILE *ftok = fopen(TOKFILE, "w");
  if (!ftok) {
    fclose(fin);
    fprintf(stderr, "Cannot open token output file '%s'\n", TOKFILE);
    return 1;
  }

  char line[MAXLINE];
  int lineno = 0;
  bool in_comment = false;

  printf("Lexer DFA Output:\n");
  printf("=================\n");

  while (fgets(line, sizeof(line), fin)) {
    line[strcspn(line, "\n\r")] = '\0';
    char *trim = line;
    while (*trim && isspace((unsigned char)*trim))
      trim++;
    if (*trim == '\0')
      continue;

    lineno++;

    /* Remove comments: // and /* ... *\/ */
    if (!in_comment) {
      char *comment_start = strstr(trim, "//");
      if (comment_start)
        *comment_start = '\0';
      char *block_start = strstr(trim, "/*");
      if (block_start) {
        *block_start = '\0';
        in_comment = true;
      }
    }

    if (in_comment) {
      char *block_end = strstr(trim, "*/");
      if (block_end) {
        memmove(trim, block_end + 2, strlen(block_end + 2) + 1);
        in_comment = false;
      } else
        continue; /* skip until comment closes */
    }

    if (lineno == 1) {
      fprintf(ftok, "%c ", T_INCLUDE);
      printf("%-20s -> %c\n", "#include <stdio.h>", T_INCLUDE);
      continue;
    }

    int i = 0, len = strlen(trim);
    while (i < len) {
      if (isspace((unsigned char)trim[i])) {
        i++;
        continue;
      }

      /* Handle two-dot statement terminator ".." explicitly */
      if (i + 1 < len && trim[i] == '.' && trim[i + 1] == '.') {
        fprintf(ftok, "%c ", T_STMT);
        printf("%-20s -> %c\n", "..", T_STMT);
        i += 2;
        continue;
      }

      /* Single-character brackets */
      if (strchr("(){}", trim[i])) {
        char s[2] = {trim[i], '\0'};
        fprintf(ftok, "%c ", T_BRACKET);
        printf("%-20s -> %c\n", s, T_BRACKET);
        i++;
        continue;
      }

      /* Single-character operators/punctuators (including ';', ',', '+', '-',
       * '<', '=', '*', '/', ':') */
      if (strchr(";,=+<-*/:", trim[i])) {
        char s[2] = {trim[i], '\0'};
        fprintf(ftok, "%c ", T_OP);
        printf("%-20s -> %c\n", s, T_OP);
        i++;
        continue;
      }

      /* Otherwise collect a multi-char token (identifier, numbers, keywords,
       * labels, function names) */
      int start = i;
      while (i < len && !isspace((unsigned char)trim[i]) &&
             !strchr("(){};,=+<-*/.:", trim[i])) {
        i++;
      }
      if (i == start) {
        /* If we didn't advance, this char was unrecognized; consume as operator
         */
        char c = trim[i++];
        char s[2] = {c, '\0'};
        fprintf(ftok, "%c ", T_OP);
        printf("%-20s -> %c\n", s, T_OP);
        continue;
      }

      int token_len = i - start;
      char token[512];
      if (token_len >= (int)sizeof(token))
        token_len = sizeof(token) - 1;
      strncpy(token, trim + start, token_len);
      token[token_len] = '\0';

      /* Check if this looks like loop label pattern and next char is ':' */
      if (starts_with(token, "loop_") && trim[i] == ':') {
        int newlen = token_len + 1;
        if (newlen < (int)sizeof(token)) {
          token[token_len] = ':';
          token[newlen] = '\0';
          token_len = newlen;
          i++;
        }
      }

      /* Also check for whitespace before colon for loop labels */
      if (starts_with(token, "loop_")) {
        int j = i;
        while (j < len && isspace((unsigned char)trim[j]))
          j++;
        if (j < len && trim[j] == ':') {
          /* Found colon after whitespace, include it */
          int newlen = token_len + 1;
          if (newlen < (int)sizeof(token)) {
            token[token_len] = ':';
            token[newlen] = '\0';
            token_len = newlen;
            i = j + 1; /* Skip past the colon */
          }
        }
      }

      char token_type = dfa_classify(token, token_len, false);

      /* Translate DFA outputs to parser token chars */
      if (token_type == T_VAR) {
        fprintf(ftok, "%c ", T_VAR);
        printf("%-20s -> %c\n", token, T_VAR);
      } else if (token_type == T_NUM) {
        fprintf(ftok, "%c ", T_NUM);
        printf("%-20s -> %c\n", token, T_NUM);
      } else if (token_type == T_PRINTF) {
        fprintf(ftok, "%c ", T_PRINTF);
        printf("%-20s -> %c\n", token, T_PRINTF);
      } else if (token_type == T_TYPE) {
        fprintf(ftok, "%c ", T_TYPE);
        printf("%-20s -> %c\n", token, T_TYPE);
      } else if (token_type == T_FUNC) {
        fprintf(ftok, "%c ", T_FUNC);
        printf("%-20s -> %c\n", token, T_FUNC);
      } else if (token_type == T_LOOP) {
        fprintf(ftok, "%c ", T_LOOP);
        printf("%-20s -> %c\n", token, T_LOOP);
      } else {
        /* default single-character token mappings */
        fprintf(ftok, "%c ", token_type);
        printf("%-20s -> %c\n", token, token_type);
      }
    }
  }

  fclose(fin);
  fclose(ftok);

  /* Print compact token stream */
  printf("\nCompact Token Stream:\n");
  printf("=====================\n");
  FILE *ftok_read = fopen(TOKFILE, "r");
  if (ftok_read) {
    char token[4];
    while (fscanf(ftok_read, "%s", token) == 1)
      printf("%s", token);
    printf("\n");
    fclose(ftok_read);
  }

  return 0;
}

/* --- PARSER WITH VISUALIZATION --- */

#define MAX_STACK 100
#define MAX_PROD 22

typedef struct {
  char lhs;    // Non-terminal
  char *rhs;   // Right-hand side (string)
  int prod_id; // Production number
} Production;

/* Grammar for your language */
Production grammar[] = {
    // S -> I Q A
    {'S', "I Q A", 1}, // Q = OptFuncs, A = Main

    // Q -> U Q | epsilon
    {'Q', "U Q", 2},
    {'Q', "", 3},

    // U -> T F B T V B B C B
    {'U', "T F B T V B B C B", 4},

    // A -> T M B B B C B
    {'A', "T M B B B C B", 5},

    // C -> D C | epsilon
    {'C', "D C", 6},
    {'C', "", 7},

    // Statements
    {'D', "T V O E S", 8},
    {'D', "V O E S", 9},
    {'D', "R E S", 10},
    {'D', "P B V B S", 11}, // printf ( var ) ..
    {'D', "K S", 12},

    // New: loop label + while structure as a statement
    // D -> L W B T V O N S B B C B (loop_xxx : while ...)
    {'D', "L W B T V O N S B B C B", 20},

    // Expressions and terms
    {'E', "G H", 13},
    {'H', "O G H", 14},
    {'H', "", 15},
    {'G', "V", 16},
    {'G', "N", 17},
    {'G', "F B E B", 18},
    {'G', "B E B", 19}};

#define NUM_PRODUCTIONS (sizeof(grammar) / sizeof(grammar[0]))

// Updated LL(1) Parsing Table - W added for while keyword
// Terminals order: I, T, F, V, N, P, R, K, M, B, O, S, W, L, $
int parsing_table[9][15] = {
    // S row
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},

    // Q row (OptFuncs)
    {0, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3},

    // U row (Func)
    {0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},

    // A row (Main)
    {0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},

    // C row (Stmts) - allow O to start (for loop labels) L is 13
    {0, 6, 0, 6, 0, 6, 6, 6, 0, 7, 6, 0, 0, 6, 0},

    // D row (Stmt) - O can start loop (O O W pattern) -> prod 20
    {0, 8, 0, 9, 0, 11, 10, 12, 0, 0, 20, 0, 0, 20, 0},

    // E row (Exp)
    {0, 0, 18, 13, 13, 0, 0, 0, 0, 13, 0, 0, 0, 0, 0},

    // H row (Exp') - allow epsilon on B, S, L, $ ; O leads to 14
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 14, 15, 15, 0, 15},

    // G row (Term)
    {0, 0, 18, 16, 17, 0, 0, 0, 0, 19, 0, 0, 0, 0, 0}};

// Map characters to table indices (updated with L and $ at end)
int get_nonterm_index(char nt) {
  switch (nt) {
  case 'S':
    return 0;
  case 'Q':
    return 1;
  case 'U':
    return 2;
  case 'A':
    return 3;
  case 'C':
    return 4;
  case 'D':
    return 5;
  case 'E':
    return 6;
  case 'H':
    return 7;
  case 'G':
    return 8;
  default:
    return -1;
  }
}

int get_term_index(char term) {
  switch (term) {
  case 'I':
    return 0;
  case 'T':
    return 1;
  case 'F':
    return 2;
  case 'V':
    return 3;
  case 'N':
    return 4;
  case 'P':
    return 5;
  case 'R':
    return 6;
  case 'K':
    return 7;
  case 'M':
    return 8;
  case 'B':
    return 9;
  case 'O':
    return 10;
  case 'S':
    return 11;
  case 'W':
    return 12;
  case 'L':
    return 13;
  case '$':
    return 14;
  default:
    return -1;
  }
}

// Stack for parsing
char stack[MAX_STACK];
int stack_top = -1;

void push(char c) {
  if (stack_top < MAX_STACK - 1) {
    stack[++stack_top] = c;
  }
}

char pop() {
  if (stack_top >= 0) {
    return stack[stack_top--];
  }
  return '\0';
}

char peek_stack() {
  if (stack_top >= 0) {
    return stack[stack_top];
  }
  return '\0';
}

// Token management
char tokens[MAXTOKENS];
int tcount = 0;
int tpos = 0;

void load_tokens(const char *fname) {
  FILE *f = fopen(fname, "r");
  if (!f) {
    perror("open tokens");
    exit(1);
  }
  char buf[4096];
  int idx = 0;
  while (fscanf(f, " %1s", buf) == 1) {
    tokens[idx++] = buf[0];
    if (idx >= MAXTOKENS)
      break;
  }
  tokens[idx] = '\0';
  tcount = idx;
  fclose(f);
}

char peek_token() {
  if (tpos >= tcount)
    return '$';
  return tokens[tpos];
}

char next_token() {
  if (tpos >= tcount)
    return '$';
  return tokens[tpos++];
}

// Look ahead to see next token
char peek_next_token() {
  if (tpos + 1 >= tcount)
    return '$';
  return tokens[tpos + 1];
}

// LL(1) Parser with visualization
int parse_with_visualization() {
  // Initialize stack
  stack_top = -1;
  push('$');
  push('S'); // Start symbol

  printf("\n=== LL(1) PARSING TABLE VISUALIZATION ===\n");
  printf("Tokens: ");
  for (int i = 0; i < tcount; i++)
    printf("%c ", tokens[i]);
  printf("$\n\n");

  printf("%-20s %-15s %-8s %-25s %-10s\n", "Stack", "Lookahead", "Top",
         "Production", "Action");
  printf("-------------------- --------------- -------- "
         "------------------------- ----------\n");

  int step = 0;

  while (stack_top >= 0) {
    char top = peek_stack();
    char lookahead = peek_token();

    // Create stack string
    char stack_str[200] = "";
    for (int i = 0; i <= stack_top; i++) {
      char temp[3] = {stack[i], '\0'};
      strcat(stack_str, temp);
      if (i < stack_top)
        strcat(stack_str, " ");
    }

    printf("%-20s %-15c %-8c", stack_str, lookahead, top);

    if (top == '$' && lookahead == '$') {
      printf("%-25s %-10s\n", "", "ACCEPT");
      return 1;
    }

    if (top == lookahead) {
      pop();
      next_token();
      printf("%-25s %-10s\n", "", "match");
      continue;
    }

    // Check if top is a non-terminal
    int nt_idx = get_nonterm_index(top);
    int t_idx = get_term_index(lookahead);

    if (nt_idx >= 0 && t_idx >= 0) {
      int prod_id = parsing_table[nt_idx][t_idx];

      // Special handling for OptFuncs (Q) when we see T
      if (prod_id == -1 && top == 'Q' && lookahead == 'T') {
        // Look ahead to see if next is F (function) or M (main)
        char next = peek_next_token();
        if (next == 'F') {
          prod_id = 2; // Func OptFuncs
        } else if (next == 'M') {
          prod_id = 3; // epsilon
        } else {
          printf("\nERROR: After T, expected F or M, got %c\n", next);
          return 0;
        }
      }

      if (prod_id == 0) {
        printf("\nERROR: No production for %c on %c\n", top, lookahead);
        return 0;
      }

      // Find the production
      Production *prod = NULL;
      for (int i = 0; i < NUM_PRODUCTIONS; i++) {
        if (grammar[i].prod_id == prod_id) {
          prod = &grammar[i];
          break;
        }
      }

      if (!prod) {
        printf("\nERROR: Production %d not found\n", prod_id);
        return 0;
      }

      // Print production
      char prod_str[200];
      if (strlen(prod->rhs) == 0) {
        strcpy(prod_str, "epsilon");
        printf("%-25s", prod_str);
      } else {
        sprintf(prod_str, "%c -> %s", top, prod->rhs);
        printf("%-25s", prod_str);
      }

      // Apply production
      pop(); // Remove non-terminal

      if (strlen(prod->rhs) > 0) {
        // Push RHS in reverse order
        char rhs_copy[200];
        strcpy(rhs_copy, prod->rhs);

        // Remove spaces and push in reverse
        char clean_rhs[200] = "";
        for (int i = 0; rhs_copy[i] != '\0'; i++) {
          if (rhs_copy[i] != ' ') {
            char temp[2] = {rhs_copy[i], '\0'};
            strcat(clean_rhs, temp);
          }
        }

        // Push in reverse order
        for (int i = strlen(clean_rhs) - 1; i >= 0; i--) {
          push(clean_rhs[i]);
        }
      }

      printf(" %-10s\n", "apply");
    } else {
      printf("\nERROR: Invalid parse (top=%c, lookahead=%c, nt_idx=%d, "
             "t_idx=%d)\n",
             top, lookahead, nt_idx, t_idx);
      return 0;
    }

    step++;
    if (step > 5000) {
      printf("\nERROR: Too many steps (possible infinite loop)\n");
      return 0;
    }
  }

  return 0;
}

// --- DISPLAY FUNCTIONS ---

void display_nfa_rules() {
  printf("\n=== NFA PATTERN RULES ===\n");
  printf("Token Type    | Pattern\n");
  printf("------------- | -------\n");
  printf("VARIABLE      | _[a-zA-Z]+[0-9][a-zA-Z]\n");
  printf("FUNCTION      | [a-zA-Z]+Fn\n");
  printf("LOOP_LABEL    | loop_[a-zA-Z]+[0-9][0-9]:\n");
  printf("NUMBER        | [0-9]+\n");
  printf("KEYWORD       | int|dec|printf|while|break|return|main\n");
  printf("STATEMENT_END | ..\n");
  printf("BRACKET       | ( | ) | { | }\n");
  printf("OPERATOR      | ; | , | = | + | < | - | * | / | :\n");
  printf("\n");
}

void display_dfa_matrix() {
  printf("\n=== UNIFIED DFA TRANSITION MATRIX ===\n");
  printf("Input Categories: 0=alpha, 1=digit, 2=_, 3=\", 4=., 5=bracket, 6=:, "
         "7=other\n\n");
  printf("State | alpha digit  _    \"    .    brkt :    othr | Accept\n");
  printf("------+-------------------------------------------+-------\n");

  const char *state_desc[] = {"START",      "alpha",   "digit",    "underscore",
                              "dot1",       "bracket", "operator", "..",
                              "unused",     "alpha+",  "id+digit", "_+alpha",
                              "_alpha+dig", "VARIABLE"};

  for (int i = 0; i < 14; i++) {
    printf("%2d %-10s|", i, state_desc[i]);
    for (int j = 0; j < NUM_INPUTS; j++) {
      int next = next_state[i][j];
      if (next == DEAD)
        printf("  --  ");
      else
        printf("  %2d  ", next);
    }
    printf("| ");
    if (accepting_tokens[i] != '\0')
      printf("%c", accepting_tokens[i]);
    printf("\n");
  }
  printf("\n");
}

void display_first_follow_sets() {
  printf("\n=== FIRST AND FOLLOW SETS ===\n");
  printf("Non-Terminal | FIRST Set           | FOLLOW Set\n");
  printf("------------ | ------------------- | -------------------\n");
  printf("S            | {I}                 | {$}\n");
  printf("Q            | {T, epsilon}        | {T}\n");
  printf("U            | {T}                 | {T}\n");
  printf("A            | {T}                 | {$}\n");
  printf("C            | {T,V,R,P,K,O,eps}   | {B}\n");
  printf("D            | {T,V,R,P,K,O}       | {T,V,R,P,K,O,B}\n");
  printf("E            | {V,N,F,B}           | {S,B,O}\n");
  printf("H            | {O, epsilon}        | {S,B}\n");
  printf("G            | {V,N,F,B}           | {S,B,O}\n");
  printf("\n");
}

void display_parsing_table() {
  printf("\n=== LL(1) PARSING TABLE ===\n");
  printf("Rows: Non-terminals (S,Q,U,A,C,D,E,H,G)\n");
  printf("Cols: Terminals (I,T,F,V,N,P,R,K,M,B,O,S,W,$)\n\n");

  const char *nonterms[] = {"S", "Q", "U", "A", "C", "D", "E", "H", "G"};
  const char *terms[] = {"I", "T", "F", "V", "N", "P", "R", "K",
                         "M", "B", "O", "S", "W", "L", "$"};

  printf("   |");
  for (int j = 0; j < 15; j++) {
    printf(" %2s |", terms[j]);
  }
  printf("\n---+");
  for (int j = 0; j < 15; j++) {
    printf("----+");
  }
  printf("\n");

  for (int i = 0; i < 9; i++) {
    printf(" %s |", nonterms[i]);
    for (int j = 0; j < 15; j++) {
      int prod = parsing_table[i][j];
      if (prod == 0)
        printf("  - |");
      else if (prod == -1)
        printf(" LA |");
      else
        printf(" %2d |", prod);
    }
    printf("\n");
  }
  printf("\n");
}

// --- MAIN ---
int main() {
  init_dfa();

  printf("\n");
  printf("############################################################\n");
  printf("###   CUSTOM LANGUAGE COMPILER - CSE332 LAB PROJECT     ###\n");
  printf("###   Lexer (DFA) + Parser (LL1)                        ###\n");
  printf("############################################################\n");

  // Display compiler construction theory
  display_nfa_rules();
  display_dfa_matrix();

  printf("\n=== GRAMMAR PRODUCTIONS ===\n");
  printf("1.  S -> I Q A\n");
  printf("2.  Q -> U Q\n");
  printf("3.  Q -> epsilon\n");
  printf("4.  U -> T F B T V B B C B\n");
  printf("5.  A -> T M B B B C B\n");
  printf("6.  C -> D C\n");
  printf("7.  C -> epsilon\n");
  printf("8.  D -> T V O E S\n");
  printf("9.  D -> V O E S\n");
  printf("10. D -> R E S\n");
  printf("11. D -> P B V B S\n"); // printf ( var ) ..
  printf("12. D -> K S\n");
  printf("13. E -> G H\n");
  printf("14. H -> O G H\n");
  printf("15. H -> epsilon\n");
  printf("16. G -> V\n");
  printf("17. G -> N\n");
  printf("18. G -> F B E B\n");
  printf("19. G -> B E B\n");
  printf("20. D -> L W B T V O N S B B C B\n");
  printf("\n");

  display_parsing_table();

  // User input mode loop
  while (1) {
    printf("\n############################################################\n");
    printf("###   INTERACTIVE MODE                                  ###\n");
    printf("############################################################\n");
    printf("Enter your code below (type 'END' on a new line to finish, 'EXIT' "
           "to quit):\n");
    printf("----------------------------------------------------\n");

    FILE *fout = fopen("user_input.c", "w");
    if (!fout) {
      printf("Error creating temporary file.\n");
      return 1;
    }

    char line[MAXLINE];
    int stop_flag = 0;
    while (fgets(line, sizeof(line), stdin)) {
      if (strncmp(line, "END", 3) == 0)
        break;
      if (strncmp(line, "EXIT", 4) == 0) {
        stop_flag = 1;
        break;
      }
      fputs(line, fout);
    }
    fclose(fout);

    if (stop_flag) {
      printf("Exiting... Goodbye!\n");
      break;
    }

    const char *input_file = "user_input.c";
    printf("----------------------------------------------------\n");

    printf("\n############################################################\n");
    printf("###   LEXICAL ANALYSIS (DFA-BASED TOKENIZATION)        ###\n");
    printf("############################################################\n");

    int lexerr = run_lexer(input_file);
    if (lexerr != 0) {
      printf("\n[ERROR] Lexer failed for %s.\n", input_file);
      continue;
    }

    printf("\n############################################################\n");
    printf("###   SYNTAX ANALYSIS (LL(1) PARSING)                  ###\n");
    printf("############################################################\n");

    printf("\n=== LOADING TOKENS FOR PARSING ===\n");
    load_tokens(TOKFILE);
    // Reset tpos for safety
    tpos = 0;

    printf("Loaded %d tokens: ", tcount);
    for (int i = 0; i < tcount; i++)
      printf("%c ", tokens[i]);
    printf("\n");

    // Display FIRST/FOLLOW right before parsing
    display_first_follow_sets();

    printf("\n=== RUNNING LL(1) PARSER ===\n");
    int ok = parse_with_visualization();

    printf("\n############################################################\n");
    if (ok) {
      printf("###   RESULT: ACCEPTED ✓                                ###\n");
    } else {
      printf("###   RESULT: REJECTED ✗                                ###\n");
    }
    printf("############################################################\n");
  }

  return 0;
}
