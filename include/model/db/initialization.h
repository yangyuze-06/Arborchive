#ifndef _MODEL_INITIALIZATION_H_
#define _MODEL_INITIALIZATION_H_

namespace DbModel {

struct Initialiser {
  int id;
  int var;
  int expr;
  int location;
};

struct BracedInitialiser {
  int id;
};

} // namespace DbModel

#endif // _MODEL_INITIALIZATION_H_
