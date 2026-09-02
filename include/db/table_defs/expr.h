#ifndef _TABLE_DEFS_EXPR_H_
#define _TABLE_DEFS_EXPR_H_

#include "../third_party/sqlite_orm.h"
#include "model/db/expr.h"

using namespace sqlite_orm;

namespace ExprTableFn {

// clang-format off
inline auto exprs() {
  return make_table(
      "exprs",
      make_column("id", &DbModel::Expr::id, primary_key()),
      make_column("kind", &DbModel::Expr::kind),
      make_column("location", &DbModel::Expr::location));
}

inline auto exprparents() {
  return make_table(
      "exprparents",
      make_column("expr_id", &DbModel::ExprParent::expr_id),
      make_column("child_index", &DbModel::ExprParent::child_index),
      make_column("parent_id", &DbModel::ExprParent::parent_id));
}

inline auto exprconv() {
  return make_table(
      "exprconv",
      make_column("converted", &DbModel::ExprConv::converted),
      make_column("conversion", &DbModel::ExprConv::conversion, primary_key()));
}

inline auto expr_types() {
  return make_table(
      "expr_types",
      make_column("id", &DbModel::ExprType::id),
      make_column("typeid", &DbModel::ExprType::typeid_),
      make_column("value_category", &DbModel::ExprType::value_category));
}

inline auto expr_isload() {
  return make_table(
      "expr_isload",
      make_column("expr_id", &DbModel::ExprIsLoad::expr_id, primary_key()));
}

inline auto compgenerated() {
  return make_table(
      "compgenerated",
      make_column("id", &DbModel::CompGenerated::id, primary_key()));
}

inline auto conversionkinds() {
  return make_table(
      "conversionkinds",
      make_column("expr_id", &DbModel::ConversionKind::expr_id, primary_key()),
      make_column("kind", &DbModel::ConversionKind::kind));
}

inline auto funbind() {
  return make_table(
      "funbind",
      make_column("expr", &DbModel::FunBind::expr, primary_key()),
      make_column("fun", &DbModel::FunBind::fun));
}

inline auto iscall() {
  return make_table(
      "iscall",
      make_column("caller", &DbModel::IsCall::caller, primary_key()),
      make_column("kind", &DbModel::IsCall::kind));
}

inline auto varbind() {
  return make_table(
      "varbind",
      make_column("expr", &DbModel::VarBind::expr, primary_key()),
      make_column("var", &DbModel::VarBind::var));
}

inline auto values() {
  return make_table(
      "values",
      make_column("id", &DbModel::Values::id, primary_key()),
      make_column("str", &DbModel::Values::str));
}

inline auto valuetext() {
  return make_table(
      "valuetext",
      make_column("id", &DbModel::ValueText::id, primary_key()),
      make_column("text", &DbModel::ValueText::text));
}

inline auto valuebind() {
  return make_table(
      "valuebind",
      make_column("val", &DbModel::ValueBind::val, primary_key()),
      make_column("expr", &DbModel::ValueBind::expr));
}

inline auto aggregatearrayinit() {
  return make_table(
      "aggregate_array_init",
      make_column("aggregate", &DbModel::AggregateArrayInit::aggregate),
      make_column("initializer", &DbModel::AggregateArrayInit::initializer),
      make_column("element_index", &DbModel::AggregateArrayInit::element_index),
      make_column("position", &DbModel::AggregateArrayInit::position),
      primary_key(&DbModel::AggregateArrayInit::aggregate,
                  &DbModel::AggregateArrayInit::position));
}

inline auto aggregatefieldinit() {
  return make_table(
      "aggregate_field_init",
      make_column("aggregate", &DbModel::AggregateFieldInit::aggregate),
      make_column("initializer", &DbModel::AggregateFieldInit::initializer),
      make_column("field", &DbModel::AggregateFieldInit::field),
      make_column("position", &DbModel::AggregateFieldInit::position),
      primary_key(&DbModel::AggregateFieldInit::aggregate,
                  &DbModel::AggregateFieldInit::position));
}

inline auto sizeofbind() {
  return make_table(
      "sizeof_bind",
      make_column("expr", &DbModel::SizeOfBind::expr, primary_key()),
      make_column("type_id", &DbModel::SizeOfBind::type_id));
}

// clang-format on

} // namespace ExprTableFn

#endif // _TABLE_DEFS_EXPR_H_
