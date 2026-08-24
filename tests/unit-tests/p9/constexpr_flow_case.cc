void p9_marker() {}

constexpr void p9_constexpr_marker() {}

int p9_ordinary_if(bool condition) {
  if (int value = 10; condition) {
    return value;
  } else {
    return 0;
  }
}

int p9_ordinary_if_with_expression_initializer(bool condition) {
  if (p9_marker(); condition) {
    return 11;
  } else {
    return 12;
  }
}

constexpr int p9_constexpr_if() {
  if constexpr (true) {
    return 1;
  } else {
    return 2;
  }
}

constexpr int p9_constexpr_if_with_initializer() {
  if constexpr (int value = 3; true) {
    return value;
  } else {
    return 4;
  }
}

constexpr int p9_constexpr_if_with_expression_initializer() {
  if constexpr (p9_constexpr_marker(); true) {
    return 13;
  } else {
    return 14;
  }
}

constexpr int p9_consteval_if(bool condition) {
  if consteval {
    return 5;
  } else {
    return condition ? 6 : 7;
  }
}

constexpr int p9_not_consteval_if(bool condition) {
  if !consteval {
    return condition ? 8 : 9;
  } else {
    return 10;
  }
}

int main() {
  return p9_ordinary_if(false) +
         p9_ordinary_if_with_expression_initializer(false) +
         p9_constexpr_if() + p9_constexpr_if_with_initializer() +
         p9_constexpr_if_with_expression_initializer() +
         p9_consteval_if(false) + p9_not_consteval_if(false);
}
