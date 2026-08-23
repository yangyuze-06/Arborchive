[[nodiscard("p7f_use_result")]]
int p7f_warn_unused_message() {
  return 11;
}

[[nodiscard]]
int p7f_warn_unused_empty() {
  return 13;
}

int main() {
  return p7f_warn_unused_message() + p7f_warn_unused_empty();
}
