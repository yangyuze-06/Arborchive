[[gnu::assume_aligned(8 + 8, 2 + 2)]]
int *p7f_assume_aligned_expr(int *ptr) { return ptr; }

[[gnu::assume_aligned(16)]]
int *p7f_assume_aligned_literal(int *ptr) { return ptr; }

template <int N>
[[gnu::assume_aligned(N + 1)]]
int *p7f_assume_aligned_dependent(int *ptr) { return ptr; }

int main() {
  int value = 0;
  return p7f_assume_aligned_expr(&value) ==
         p7f_assume_aligned_literal(&value);
}
