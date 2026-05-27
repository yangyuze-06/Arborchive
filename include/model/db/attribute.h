#ifndef _MODEL_ATTRIBUTE_H_
#define _MODEL_ATTRIBUTE_H_

#include <string>

enum class AttributeKind {
  GNU = 0,
  STD = 1,
  DECLSPEC = 2,
  MS = 3,
  ALIGNAS = 4,
};

enum class AttributeArgKind {
  EMPTY = 0,
  TOKEN = 1,
  CONSTANT = 2,
  TYPE = 3,
  CONSTANT_EXPR = 4,
  EXPR = 5,
};

namespace DbModel {

struct Attribute {
  int id;
  int kind;
  std::string name;
  std::string name_space;
  int location;
};

struct AttributeArg {
  int id;
  int kind;
  int attribute;
  int index;
  int location;
};

struct AttributeArgValue {
  int arg;
  std::string value;
};

struct AttributeArgType {
  int arg;
  int type_id;
};

struct AttributeArgConstant {
  int arg;
  int constant;
};

struct AttributeArgExpr {
  int arg;
  int expr;
};

struct AttributeArgName {
  int arg;
  std::string name;
};

struct TypeAttribute {
  int type_id;
  int spec_id;
};

struct FuncAttribute {
  int func_id;
  int spec_id;
};

struct VarAttribute {
  int var_id;
  int spec_id;
};

struct StmtAttribute {
  int stmt_id;
  int spec_id;
};

} // namespace DbModel

#endif // _MODEL_ATTRIBUTE_H_
