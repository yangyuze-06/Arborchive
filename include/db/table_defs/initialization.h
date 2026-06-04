#ifndef _TABLE_DEFS_INITIALIZATION_H_
#define _TABLE_DEFS_INITIALIZATION_H_

#include "../third_party/sqlite_orm.h"
#include "model/db/initialization.h"

using namespace sqlite_orm;

namespace InitializationTableFn {

// clang-format off
inline auto initialisers() {
  return make_table(
      "initialisers",
      make_column("init", &DbModel::Initialiser::id, primary_key()),
      make_column("var", &DbModel::Initialiser::var),
      make_column("expr", &DbModel::Initialiser::expr),
      make_column("location", &DbModel::Initialiser::location));
}

inline auto braced_initialisers() {
  return make_table(
      "braced_initialisers",
      make_column("init", &DbModel::BracedInitialiser::id, primary_key()));
}
// clang-format on

} // namespace InitializationTableFn

#endif // _TABLE_DEFS_INITIALIZATION_H_
