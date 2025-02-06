#ifndef H_PARSER
#define H_PARSER

#include <cstdlib>
#include <cctype>
#include <cstring>
#include <iostream>

enum types {
  DELIMITER = 1,
  NUMBER,
  VARIABLE
};


class parser {
public:
  parser();
  double eval_expr(char* exp);
private:
  char* expr_ptr;
  char token[100];
  char token_type;
  void eval_AS(double& result);
  void eval_MD(double& result);
  void eval_E(double& result);
  void eval_unary(double& result);
  void eval_P(double& result);
  void atom(double& result);
  void get_token();
  bool isdelim(char c);
  void serror(int error);
};


#endif
