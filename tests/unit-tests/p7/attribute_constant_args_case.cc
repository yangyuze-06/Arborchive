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

int main() {
  return p7f_aligned_direct() + p7f_aligned_paren() +
         p7f_aligned_unsigned();
}
