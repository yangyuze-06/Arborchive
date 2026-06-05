[[gnu::aligned(16)]]
int p7f_expr_aligned_literal() {
  return 16;
}

[[gnu::aligned(8 + 8)]]
int p7f_expr_aligned_add() {
  return 8;
}

[[gnu::aligned((4 * 4))]]
int p7f_expr_aligned_wrapped_mul() {
  return 4;
}

template <int N>
struct P7fDependentAligned {
  [[gnu::aligned(N + 1)]] int value;
};

int main() {
  return p7f_expr_aligned_literal() + p7f_expr_aligned_add() +
         p7f_expr_aligned_wrapped_mul();
}
