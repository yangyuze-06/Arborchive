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

namespace DbModel {

struct Attribute {
  int id;
  int kind;
  std::string name;
  std::string name_space;
  int location;
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
