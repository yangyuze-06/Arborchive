struct Base {
  virtual ~Base() = default;
  int member;
};

struct Derived : Base {
  int inherited_member() { return member; }
};
struct Other {};

int decay_target(int value) { return value; }
using FunctionPointer = int (*)(int);

double implicit_numeric(int value) { return value; }
bool implicit_bool(int *pointer) { return pointer; }
int *implicit_null_pointer() { return nullptr; }
Base *implicit_upcast(Derived *pointer) { return pointer; }
int array_decay(int (&values)[3]) { return *values; }
int parenthesized_load(int value) { return (value); }
FunctionPointer function_decay() { return decay_target; }

double paren_load_chain(int value) {
  return static_cast<double>((value));
}

double nested_conversions(short value) {
  return static_cast<double>((int(value)));
}

long explicit_static(int value) { return static_cast<long>(value); }
unsigned long explicit_reinterpret(int *pointer) {
  return reinterpret_cast<unsigned long>(pointer);
}
int *explicit_const(const int *pointer) { return const_cast<int *>(pointer); }
Derived *explicit_dynamic(Base *pointer) {
  return dynamic_cast<Derived *>(pointer);
}
Derived *explicit_downcast(Base *pointer) {
  return static_cast<Derived *>(pointer);
}
Other &explicit_glvalue_adjust(Base &value) {
  return reinterpret_cast<Other &>(value);
}
double explicit_c_style(int value) { return (double)value; }
double explicit_functional(int value) { return double(value); }
int &&explicit_xvalue(int &value) { return static_cast<int &&>(value); }

int Base::*member_base = &Base::member;
int Derived::*member_derived = member_base;
int Base::*member_roundtrip =
    static_cast<int Base::*>(member_derived);
