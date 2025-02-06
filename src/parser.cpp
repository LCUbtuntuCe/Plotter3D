#include "../include/parser.hpp"

parser::parser() {
  expr_ptr = NULL;
}

double parser::eval_expr(char* expr) {
  double result;
  expr_ptr = expr;
  get_token();
  if (!*token) {
    serror(2);
    return 0.0;
  }
  eval_AS(result);
  if (*token)
    serror(0);
  return result;
}

void parser::eval_AS(double& result) {
  char op;
  double temp;

  eval_MD(result);
  while ((op = *token) == '+' || op == '-') {
    get_token();
    eval_MD(temp);
    switch (op) {
    case '+':
      result = result + temp;
      break;
    case '-':
      result = result - temp;
      break;
    }
  }
}

void parser::eval_MD(double& result) {
  char op;
  double temp;

  eval_E(result);
  while ((op = *token) == '*' || op == '/') {
    get_token();
    eval_E(temp);
    switch (op) {
    case '*':
      result = result * temp;
      break;
    case '/':
      result = result / temp;
      break;
    }
  }
}

void parser::eval_E(double& result) {
  double temp, ex;

  eval_unary(result);
  if (*token == '^') {
    get_token();
    eval_E(temp);
    ex = result;
    if (temp == 0.0) {
      result = 1.0;
      return;
    }
    for (int t=(int)temp-1; t>0; t--)
      result = result * (double)ex;
  }
}

void parser::eval_unary(double& result) {
  char op = 0;
  if ((token_type == DELIMITER) && *token == '+' || *token == '-') {
    op = *token;
    get_token();
  }
  eval_P(result);
  if (op == '-')
    result = -result;
}

void parser::eval_P(double& result) {
  if (*token == '(') {
    get_token();
    eval_AS(result);
    if (*token != ')')
      serror(1);
    get_token();
  }
  else
    atom(result);
}

void parser::atom(double &result) {
  switch (token_type) {
  case NUMBER:
    result = atof(token);
    get_token();
    return;
  default:
    serror(0);
  }
}

void parser::get_token() {
  char* temp;
  token_type = 0;
  temp = token;
  *temp = '\0';

  if (!*expr_ptr) return;

  while (isspace(*expr_ptr)) ++expr_ptr;

  if (strchr("+-*/^()", *expr_ptr)) {
    token_type = DELIMITER;
    *temp++ = *expr_ptr++;
  }
  else if (isalpha(*expr_ptr)) {
    while (!isdelim(*expr_ptr))
      *temp++ = *expr_ptr++;
    token_type = VARIABLE;
  }
  else if (isdigit(*expr_ptr)) {
    while (!isdelim(*expr_ptr))
      *temp++ = *expr_ptr++;
    token_type = NUMBER;
  }
  *temp = '\0';
}

bool parser::isdelim(char c) {
  if (strchr(" +-*/^()", c) || c==9 || c == '\r' || c == 0)
    return true;
  return false;
}

void parser::serror(int error) {
  static const char* e[] = {
    "SyntaxError",
    "Unbalanced Parentheses",
    "No expression"
  };
  std::cout << e[error] << std::endl;
}

int main() {
  char expstr[80];
  parser ob;
  while (true) {
    std::cout << "Expression: ";
    std::cin.getline(expstr, 79);
    if (*expstr == '.')
      break;
    std::cout << "Result: " << ob.eval_expr(expstr) << std::endl;
  }
  return 0;
}
