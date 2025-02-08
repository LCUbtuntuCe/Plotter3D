#pragma once

#include <cstdlib>
#include <cctype>
#include <cstring>
#include <iostream>


enum types {
  DELIMITER = 1,
  NUMBER,
  VARIABLE,
  FUNCTION
};


class parser {
public:
  parser();
  double eval_expr(char* exp);
  void set_xy(double x_val, double y_val);
private:
  double x;
  double y;
  char* expr_ptr;
  char token[100];
  char token_type;
  void eval_AS(double& result);
  void eval_MD(double& result);
  void eval_E(double& result);
  void eval_unary(double& result);
  void eval_P(double& result);
  void eval_function(double& result, const char* token);
  void atom(double& result);
  void get_token();
  bool isdelim(char c);
  void serror(int error);
};

