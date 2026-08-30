int p10_callee(int value) { return value; }

using P10Function = int (*)(int);

struct P10Box {
  int method(int value) { return value; }
};

int p10_expression_graph(int a, int b, int *values, P10Function indirect,
                         P10Box &box) {
  int nested = a + b * 2;
  int selected = nested ? a : static_cast<int>(b);
  int aggregate[2]{a, b};
  int measured = sizeof(nested);

  values[a + 1] = p10_callee(selected);
  p10_callee(nested + measured);
  indirect(selected);
  box.method(selected);
  (nested);

  if (nested > 3)
    nested = p10_callee(nested + 1);

  for (int i = 0; i < 2; ++i)
    nested += values[i];

  while (nested < 10)
    ++nested;

  do {
    --nested;
  } while (nested > 8);

  switch (nested) {
  case 8:
    break;
  default:
    break;
  }

  return nested + aggregate[0];
}
