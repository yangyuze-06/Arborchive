struct P8Point {
  int x;
  int y;
};

int p8_global_scalar = 7;
int p8_braced_scalar{9};
P8Point p8_global_point{1, 2};
int p8_global_array[3]{3, 4, 5};

int main() {
  int p8_local_scalar = p8_global_scalar;
  int p8_local_braced{11};
  P8Point p8_local_point{p8_local_scalar, p8_local_braced};
  int p8_local_array[2]{p8_local_point.x, p8_local_point.y};

  return p8_global_array[0] + p8_local_array[1];
}
