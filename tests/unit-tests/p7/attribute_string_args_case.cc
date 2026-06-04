__attribute__((section("__DATA,p7f_section"))) int p7f_section_var = 0;

__attribute__((annotate("p7f_annotation"))) int p7f_annotated_var = 0;

__attribute__((
    deprecated("use p7f_replacement_target", "p7f_replacement_target")))
int p7f_deprecated_with_replacement() {
  return 2;
}

int p7f_replacement_target() { return 3; }

int main() {
  return p7f_deprecated_with_replacement() + p7f_section_var +
         p7f_annotated_var;
}
