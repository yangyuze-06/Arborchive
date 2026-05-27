#ifndef _TABLE_DEFS_ATTRIBUTE_H_
#define _TABLE_DEFS_ATTRIBUTE_H_

#include "../third_party/sqlite_orm.h"
#include "model/db/attribute.h"

using namespace sqlite_orm;

namespace AttributeTableFn {

// clang-format off
inline auto attributes() {
  return make_table(
      "attributes",
      make_column("id", &DbModel::Attribute::id, primary_key()),
      make_column("kind", &DbModel::Attribute::kind),
      make_column("name", &DbModel::Attribute::name),
      make_column("name_space", &DbModel::Attribute::name_space),
      make_column("location", &DbModel::Attribute::location));
}

inline auto attribute_args() {
  return make_table(
      "attribute_args",
      make_column("id", &DbModel::AttributeArg::id, primary_key()),
      make_column("kind", &DbModel::AttributeArg::kind),
      make_column("attribute", &DbModel::AttributeArg::attribute),
      make_column("index", &DbModel::AttributeArg::index),
      make_column("location", &DbModel::AttributeArg::location));
}

inline auto attribute_arg_value() {
  return make_table(
      "attribute_arg_value",
      make_column("arg", &DbModel::AttributeArgValue::arg, primary_key()),
      make_column("value", &DbModel::AttributeArgValue::value));
}

inline auto attribute_arg_type() {
  return make_table(
      "attribute_arg_type",
      make_column("arg", &DbModel::AttributeArgType::arg, primary_key()),
      make_column("type_id", &DbModel::AttributeArgType::type_id));
}

inline auto attribute_arg_constant() {
  return make_table(
      "attribute_arg_constant",
      make_column("arg", &DbModel::AttributeArgConstant::arg, primary_key()),
      make_column("constant", &DbModel::AttributeArgConstant::constant));
}

inline auto attribute_arg_expr() {
  return make_table(
      "attribute_arg_expr",
      make_column("arg", &DbModel::AttributeArgExpr::arg, primary_key()),
      make_column("expr", &DbModel::AttributeArgExpr::expr));
}

inline auto attribute_arg_name() {
  return make_table(
      "attribute_arg_name",
      make_column("arg", &DbModel::AttributeArgName::arg, primary_key()),
      make_column("name", &DbModel::AttributeArgName::name));
}

inline auto typeattributes() {
  return make_table(
      "typeattributes",
      make_column("type_id", &DbModel::TypeAttribute::type_id),
      make_column("spec_id", &DbModel::TypeAttribute::spec_id),
      primary_key(&DbModel::TypeAttribute::type_id,
                  &DbModel::TypeAttribute::spec_id));
}

inline auto funcattributes() {
  return make_table(
      "funcattributes",
      make_column("func_id", &DbModel::FuncAttribute::func_id),
      make_column("spec_id", &DbModel::FuncAttribute::spec_id),
      primary_key(&DbModel::FuncAttribute::func_id,
                  &DbModel::FuncAttribute::spec_id));
}

inline auto varattributes() {
  return make_table(
      "varattributes",
      make_column("var_id", &DbModel::VarAttribute::var_id),
      make_column("spec_id", &DbModel::VarAttribute::spec_id),
      primary_key(&DbModel::VarAttribute::var_id,
                  &DbModel::VarAttribute::spec_id));
}

inline auto stmtattributes() {
  return make_table(
      "stmtattributes",
      make_column("stmt_id", &DbModel::StmtAttribute::stmt_id),
      make_column("spec_id", &DbModel::StmtAttribute::spec_id),
      primary_key(&DbModel::StmtAttribute::stmt_id,
                  &DbModel::StmtAttribute::spec_id));
}
// clang-format on

} // namespace AttributeTableFn

#endif // _TABLE_DEFS_ATTRIBUTE_H_
