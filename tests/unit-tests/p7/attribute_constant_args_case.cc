[[gnu::aligned(16)]]
int p7f_aligned_direct() {
  return 16;
}

[[gnu::aligned((32))]]
int p7f_aligned_paren() {
  return 32;
}

[[gnu::aligned(64U)]]
int p7f_aligned_unsigned() {
  return 64;
}

[[gnu::aligned(8 + 8)]]
int p7f_aligned_expr_skipped() {
  return 8;
}

int main() {
  return p7f_aligned_direct() + p7f_aligned_paren() +
         p7f_aligned_unsigned() + p7f_aligned_expr_skipped();
}
