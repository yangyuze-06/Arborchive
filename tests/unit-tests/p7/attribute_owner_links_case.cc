struct [[deprecated]] P7cDeprecatedRecord {
  [[no_unique_address]] int field;
};

enum [[deprecated]] P7cDeprecatedEnum { P7cEnumValue = 1 };

using P7cDeprecatedAlias [[deprecated]] = int;

[[maybe_unused]] int p7d_global = P7cEnumValue;

int p7_owner_links([[maybe_unused]] int param) {
  [[maybe_unused]] P7cDeprecatedAlias local = param;

  switch (local) {
  case 0:
    [[fallthrough]];
  default:
    break;
  }

  if (local) [[likely]] return local;
  return 0;
}

int main() {
  P7cDeprecatedRecord record{p7d_global};
  return p7_owner_links(record.field);
}
