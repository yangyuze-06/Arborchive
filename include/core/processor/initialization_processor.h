#ifndef _INITIALIZATION_PROCESSOR_H_
#define _INITIALIZATION_PROCESSOR_H_

#include "core/processor/base_processor.h"
#include <clang/AST/Decl.h>
#include <clang/AST/Expr.h>
#include <string>
#include <unordered_set>

class ExprProcessor;

class InitializationProcessor : public BaseProcessor {
public:
  InitializationProcessor(clang::ASTContext *ast_context,
                          const clang::PrintingPolicy pp,
                          ExprProcessor *expr_processor = nullptr)
      : BaseProcessor(ast_context, pp), expr_processor_(expr_processor) {}
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
  ExprProcessor *expr_processor_ = nullptr;
};

#endif // _INITIALIZATION_PROCESSOR_H_
