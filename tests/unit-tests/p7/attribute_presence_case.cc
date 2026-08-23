[[nodiscard]] int p7a_nodiscard_function() { return 7; }

int p7a_plain_function() { return 3; }

int main() {
  return p7a_nodiscard_function() + p7a_plain_function();
}
