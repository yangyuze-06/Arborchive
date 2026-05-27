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
