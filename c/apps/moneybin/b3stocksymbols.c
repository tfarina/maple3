#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fstrutils.h"
#include "test.h"

/**
 * Descrição do Campo     | Conteúdo | Tipo e Tamanho | Posição Inicial | Posição Final
 * 09 - Codigo do Mercado |          |       N(3)     |       109       |       111
 */

/**
 * TABELA DE TPMERC - RELAÇÃO DOS VALORES PARA TIPO DE MERCADO
 */
typedef enum market_type_e {
  /**
   * 010 VISTA
   */
  MT_EQUITY = 10,

  /**
   * 012 EXERCÍCIO DE OPÇÕES DE COMPRA
   */
  MT_CALL_EXERCISE = 12,

  /**
   * 013 EXERCÍCIO DE OPÇÕES DE VENDA
   */
  MT_PUT_EXERCISE = 13,

  /**
   * 017 LEILÃO
   */
  MT_AUCTION = 17,

  /**
   * 020 FRACIONÁRIO
   */
  MT_FRACTIONARY = 20,

  /**
   * 030 TERMO
   */
  MT_TERM = 30,

  /**
   * 050 FUTURO COM RETENÇÃO DE GANHO
   */
  MT_FUTURE_GAIN_RETENTION = 50,

  /**
   * 060 FUTURO COM MOVIMENTAÇÃO CONTÍNUA
   */
  MT_FUTURE_CONTINUOS_MOVING = 60,

  /**
   * 070 OPÇÕES DE COMPRA
   */
  MT_CALL = 70,

  /**
   * 080 OPÇÕESDE VENDA
   */
  MT_PUT = 80
} market_type_t;

static int
str_startswith(char const *str, char const *prefix)
{
  return strncmp(str, prefix, strlen(prefix)) == 0;
}

static char *
str_substring(char const *str, int start, int end)
{
  int len;
  char *result;

  len = end - start;
  result = malloc(sizeof(char) * len + 1);
  if (result == NULL) {
    return NULL;
  }
  memcpy(result, str + start, len);
  result[len] = '\0';

  return result;
}

static int
dump_b3_stock_symbols(char const *filename)
{
  FILE *fp;
  char linebuf[BUFSIZ];
  char *symbol;
  char *market_type;

  fp = fopen(filename, "r");
  if (fp == NULL) {
    fprintf(stderr, "%s: %s\n", filename, strerror(errno));
    return -1;
  }

  symbol = NULL;
  market_type = NULL;

  /* Loops through the file reading line by line. */
  while (fgets(linebuf, sizeof(linebuf), fp)) {
    market_type = str_substring(linebuf, 108, 111);

    if (str_startswith(market_type, "010")) {
      symbol = str_substring(linebuf, 2, 14);
      printf("%s\n", symbol);
    }
  }

  return 0;
}

int
main(int argc, char **argv)
{
  int retval;
  char *filename;

  if (argc != 2) {
    fprintf(stderr, "usage: b3stocksymbols TITULOS_NEGOCIAVEIS.txt\n");
    return 1;
  }

  filename = f_strdup(argv[1]);

  if (!test_exists(filename)) {
    fprintf(stderr, "%s: %s\n", filename, strerror(errno));
    return 1;
  }

  retval = dump_b3_stock_symbols(filename);
  if (retval < 0) {
    return 1;
  }

  free(filename);

  return 0;
}
