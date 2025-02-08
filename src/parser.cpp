// modified parser from C++: Complete Reference

#include <parser.hpp>
#include <cmath>

parser::parser()
  : expr_ptr(NULL),
    x(0.0),
    y(0.0) { }

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
    for (int t = (int)temp - 1; t > 0; t--)
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

void parser::eval_function(double& result, const char* token) {
  if (strcmp(token, "sin") == 0)
    result = sin(result);
  else if (strcmp(token, "cos") == 0)
    result = cos(result);
  else if (strcmp(token, "tan") == 0)
    result = tan(result);
  else if (strcmp(token, "arcsin") == 0)
    result = asin(result);
  else if (strcmp(token, "arccos") == 0)
    result = acos(result);
  else if (strcmp(token, "arctan") == 0)
    result = atan(result);
  else if (strcmp(token, "rad") == 0)
    result = result * M_PI / 180;
  else if (strcmp(token, "deg") == 0)
    result = result * 180 / M_PI;
  else if (strcmp(token, "sqrt") == 0)
    result = sqrt(result);
  else if (strcmp(token, "exp") == 0)
    result = exp(result);
  else if (strcmp(token, "ln") == 0)
    result = log(result);
  else if (strcmp(token, "log10") == 0)
    result = log10(result);
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
    while (!isdelim(*expr_ptr)) {
      *temp++ = *expr_ptr++;
    }
    *temp = '\0';

    if (strcmp(token, "sin")    == 0 ||
	strcmp(token, "cos")    == 0 ||
	strcmp(token, "tan")    == 0 ||
	strcmp(token, "arcsin") == 0 ||
	strcmp(token, "arccos") == 0 ||
	strcmp(token, "arctan") == 0 ||
	strcmp(token, "rad")    == 0 ||
	strcmp(token, "deg")    == 0 ||
	strcmp(token, "sqrt")   == 0 ||
	strcmp(token, "exp")    == 0 ||
	strcmp(token, "ln")     == 0 ||
	strcmp(token, "log10")  == 0) {
      token_type = FUNCTION;
    } else {
      token_type = VARIABLE;
    }
  }
  else if (isdigit(*expr_ptr)) {
    while (!isdelim(*expr_ptr)) {
      *temp++ = *expr_ptr++;
    }
    token_type = NUMBER;
  }
  *temp = '\0';
}

void parser::atom(double& result) {
  switch (token_type) {
  case FUNCTION:
    char token_temp[100];
    strcpy(token_temp, token);
    get_token(); // skip function name
    if (*token != '(') serror(1);
    get_token(); // skip (
    eval_AS(result);
    eval_function(result, token_temp);
    if (*token != ')') serror(1);
    get_token();
    break;
  case VARIABLE:
    if (*token == 'x')
      result = x;
    else if (*token == 'y')
      result = y;
    else
      serror(3);
    get_token();
    return;
  case NUMBER:
    result = atof(token);
    get_token();
    return;
  default:
    serror(0);
  }
}

void parser::set_xy(double x_val, double y_val) {
  x = x_val;
  y = y_val;
}

bool parser::isdelim(char c) {
  if (strchr(" +-*/^()", c) || c == 9 || c == '\r' || c == 0) {
    return true;
  }
  return false;
}

void parser::serror(int error) {
  static const char* e[] = {
    "SyntaxError",
    "UnbalancedParentheses",
    "NoExpression",
    "InvalidVariable"
  };
  std::cout << e[error] << std::endl;
}

// int main() {
//   char expstr[80];
//   parser ob;
//   double c = 0.0;
//   for (;;) {
//     std::cout << "Expression: ";
//     std::cin.getline(expstr, 79);
//     if (*expstr == '.')
//       break;
//     std::cout << "Result: " << ob.eval_expr(expstr) << std::endl;
//   }
//   return 0;
// }
