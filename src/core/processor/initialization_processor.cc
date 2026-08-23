#include "core/processor/initialization_processor.h"
#include "core/srcloc_recorder.h"
#include "db/dependency_manager.h"
#include "db/storage_facade.h"
#include "model/db/initialization.h"
#include "util/id_generator.h"
#include "util/key_generator/expr.h"
#include "util/key_generator/variable.h"
#include "util/logger/macros.h"
#include <clang/AST/Expr.h>
#include <llvm/Support/Casting.h>

void InitializationProcessor::processVarDeclInitializer(
    const clang::VarDecl *decl) {
  if (!decl || decl->isImplicit())
    return;

  const clang::Expr *initExpr = getInitializerExpr(decl);
  if (!initExpr)
    return;

  const KeyType varKey = KeyGen::Var::makeKey(decl, ast_context_);
  const KeyType exprKey = KeyGen::Expr_::makeKey(initExpr, ast_context_);
  if (varKey.empty() || exprKey.empty())
    return;

  const std::string initializerKey = varKey + "::init::" + exprKey;
  if (!processed_initializers_.insert(initializerKey).second)
    return;

  const int varId = resolveAccessibleId(decl);
  if (varId == -1) {
    LOG_WARNING << "Skipping initialiser without accessible variable id"
                << std::endl;
    return;
  }

  LocIdPair *locIdPair = SrcLocRecorder::processExpr(initExpr, ast_context_);
  const int initId = GENID(Initialiser);
  const int exprId =
      resolveExprIdOrDefer(initExpr, initId, varId, locIdPair->spec_id);

  recordInitialiser(initId, varId, exprId, locIdPair->spec_id);

  if (isBracedInitializer(initExpr))
    recordBracedInitialiser(initId);
}

const clang::Expr *
InitializationProcessor::getInitializerExpr(const clang::VarDecl *decl) const {
  if (!decl)
    return nullptr;

  const clang::Expr *init = decl->getInit();
  if (!init)
    return nullptr;

  const clang::Expr *ignored = init->IgnoreImplicit();
  return ignored ? ignored : init;
}

bool InitializationProcessor::isBracedInitializer(
    const clang::Expr *expr) const {
  return expr && llvm::isa<clang::InitListExpr>(expr);
}

int InitializationProcessor::resolveAccessibleId(
    const clang::VarDecl *decl) const {
  if (!decl)
    return -1;

  const KeyType varKey = KeyGen::Var::makeKey(decl, ast_context_);
  return SEARCH_VARIABLE_CACHE(varKey).value_or(-1);
}

int InitializationProcessor::resolveExprIdOrDefer(const clang::Expr *expr,
                                                   int initId, int varId,
                                                   int locationId) const {
  const KeyType exprKey = KeyGen::Expr_::makeKey(expr, ast_context_);
  if (auto cachedId = SEARCH_EXPR_CACHE(exprKey))
    return *cachedId;

  PendingUpdate update{
      exprKey, CacheType::EXPR,
      [initId, varId, locationId](int resolvedExprId) {
        DbModel::Initialiser updatedInitialiser = {initId, varId,
                                                   resolvedExprId, locationId};
        STG.insertClassObj(updatedInitialiser);
      }};
  DependencyManager::instance().addDependency(update);

  return -1;
}

void InitializationProcessor::recordInitialiser(int initId, int varId,
                                                int exprId,
                                                int locationId) const {
  DbModel::Initialiser initialiser = {initId, varId, exprId, locationId};
  STG.insertClassObj(initialiser);
}

void InitializationProcessor::recordBracedInitialiser(int initId) const {
  DbModel::BracedInitialiser bracedInitialiser = {initId};
  STG.insertClassObj(bracedInitialiser);
}
