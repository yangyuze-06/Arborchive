#ifndef _INITIALIZATION_PROCESSOR_H_
#define _INITIALIZATION_PROCESSOR_H_

#include "core/processor/base_processor.h"
#include <clang/AST/Decl.h>
#include <clang/AST/Expr.h>
#include <string>
#include <unordered_set>

class InitializationProcessor : public BaseProcessor {
public:
  InitializationProcessor(clang::ASTContext *ast_context,
                          const clang::PrintingPolicy pp)
      : BaseProcessor(ast_context, pp) {}
  ~InitializationProcessor() = default;

  void processVarDeclInitializer(const clang::VarDecl *decl);

private:
  const clang::Expr *getInitializerExpr(const clang::VarDecl *decl) const;
  bool isBracedInitializer(const clang::Expr *expr) const;
  int resolveAccessibleId(const clang::VarDecl *decl) const;
  int resolveExprIdOrDefer(const clang::Expr *expr, int initId, int varId,
                           int locationId) const;
  void recordInitialiser(int initId, int varId, int exprId, int locationId) const;
  void recordBracedInitialiser(int initId) const;

  std::unordered_set<std::string> processed_initializers_;
};

#endif // _INITIALIZATION_PROCESSOR_H_
