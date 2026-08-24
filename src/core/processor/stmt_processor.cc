#include "core/processor/stmt_processor.h"
#include "core/srcloc_recorder.h"
#include "db/dependency_manager.h"
#include "db/storage_facade.h"
#include "model/db/stmt.h"
#include "util/id_generator.h"
#include "util/key_generator/expr.h"
#include "util/key_generator/stmt.h"
#include "util/key_generator/variable.h"
#include "util/logger/macros.h"
#include <clang/AST/Decl.h>
#include <clang/AST/Expr.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/StmtCXX.h>
#include <clang/Basic/LLVM.h>

namespace {

template <typename RelationModel>
void insertStmtRelation(int ownerId, Stmt *child, ASTContext *astContext) {
  if (auto *expr = llvm::dyn_cast<Expr>(child)) {
    KeyType stmtKey = KeyGen::Stmt_::makeKey(child, astContext);
    LocIdPair *locIdPair = SrcLocRecorder::processStmt(child, astContext);

    auto materializeExprStmt = [ownerId, stmtKey,
                                locationId = locIdPair->spec_id](int exprId) {
      DbModel::Stmt stmtModel = {exprId, static_cast<int>(StmtKind::EXPR),
                                 locationId};
      INSERT_STMT_CACHE(stmtKey, exprId);
      STG.insertClassObj(stmtModel);

      RelationModel relation = {ownerId, exprId};
      STG.insertClassObj(relation);
    };

    KeyType exprKey = KeyGen::Expr_::makeKey(expr, astContext);
    if (auto cachedId = SEARCH_EXPR_CACHE(exprKey)) {
      materializeExprStmt(*cachedId);
      return;
    }

    RelationModel unresolvedRelation = {ownerId, -1};
    STG.insertClassObj(unresolvedRelation);
    PendingUpdate update{exprKey, CacheType::EXPR, materializeExprStmt};
    DependencyManager::instance().addDependency(update);
    return;
  }

  KeyType stmtKey = KeyGen::Stmt_::makeKey(child, astContext);
  if (auto cachedId = SEARCH_STMT_CACHE(stmtKey)) {
    RelationModel relation = {ownerId, *cachedId};
    STG.insertClassObj(relation);
    return;
  }

  RelationModel unresolvedRelation = {ownerId, -1};
  STG.insertClassObj(unresolvedRelation);
  PendingUpdate update{stmtKey, CacheType::STMT, [ownerId](int resolvedId) {
                         RelationModel resolvedRelation = {ownerId, resolvedId};
                         STG.insertClassObj(resolvedRelation);
                       }};
  DependencyManager::instance().addDependency(update);
}

} // namespace

int StmtProcessor::getStmtId(Stmt *stmt, StmtKind stmtKind) {
  KeyType stmtKey = KeyGen::Stmt_::makeKey(stmt, ast_context_);
  if (auto cachedId = SEARCH_STMT_CACHE(stmtKey))
    return *cachedId;

  LocIdPair *locIdPair = SrcLocRecorder::processStmt(stmt, ast_context_);

  DbModel::Stmt stmtModel = {GENID(Stmt), static_cast<int>(stmtKind),
                             locIdPair->spec_id};

  INSERT_STMT_CACHE(stmtKey, stmtModel.id);
  STG.insertClassObj(stmtModel);
  return stmtModel.id;
}

int StmtProcessor::getSupportedAttributedStmtOwnerId(
    AttributedStmt *attributedStmt) {
  if (!attributedStmt || !attributedStmt->getSubStmt())
    return -1;

  Stmt *subStmt = attributedStmt->getSubStmt();
  if (auto *returnStmt = dyn_cast<ReturnStmt>(subStmt))
    return processReturnStmt(returnStmt);
  if (auto *nullStmt = dyn_cast<NullStmt>(subStmt))
    return processNullStmt(nullStmt);
  if (auto *compoundStmt = dyn_cast<CompoundStmt>(subStmt))
    return processBlockStmt(compoundStmt);
  if (auto *ifStmt = dyn_cast<IfStmt>(subStmt))
    return processIfStmt(ifStmt);
  if (auto *forStmt = dyn_cast<ForStmt>(subStmt))
    return processForStmt(forStmt);
  if (auto *rangeForStmt = dyn_cast<CXXForRangeStmt>(subStmt))
    return processCXXForRangeStmt(rangeForStmt);
  if (auto *whileStmt = dyn_cast<WhileStmt>(subStmt))
    return processWhileStmt(whileStmt);
  if (auto *doStmt = dyn_cast<DoStmt>(subStmt))
    return processDoStmt(doStmt);
  if (auto *switchStmt = dyn_cast<SwitchStmt>(subStmt))
    return processSwitchStmt(switchStmt);
  if (auto *declStmt = dyn_cast<DeclStmt>(subStmt))
    return processDeclStmt(declStmt);

  return -1;
}

int StmtProcessor::processIfStmt(IfStmt *ifStmt) {
  IfStatementKind statementKind = ifStmt->getStatementKind();
  StmtKind stmtKind = StmtKind::IF;
  switch (statementKind) {
  case IfStatementKind::Ordinary:
    stmtKind = StmtKind::IF;
    break;
  case IfStatementKind::Constexpr:
    stmtKind = StmtKind::CONSTEXPR_IF;
    break;
  case IfStatementKind::ConstevalNonNegated:
    stmtKind = StmtKind::CONSTEVAL_IF;
    break;
  case IfStatementKind::ConstevalNegated:
    stmtKind = StmtKind::NOT_CONSTEVAL_IF;
    break;
  }

  int if_stmt_id = getStmtId(ifStmt, stmtKind);

  if (statementKind == IfStatementKind::Ordinary) {
    if (Stmt *init = ifStmt->getInit())
      insertStmtRelation<DbModel::IfInit>(if_stmt_id, init, ast_context_);
    if (Stmt *then = ifStmt->getThen())
      insertStmtRelation<DbModel::IfThen>(if_stmt_id, then, ast_context_);
    if (Stmt *elseStmt = ifStmt->getElse())
      insertStmtRelation<DbModel::IfElse>(if_stmt_id, elseStmt, ast_context_);
  } else if (statementKind == IfStatementKind::Constexpr) {
    if (Stmt *init = ifStmt->getInit())
      insertStmtRelation<DbModel::ConstexprIfInit>(if_stmt_id, init,
                                                   ast_context_);
    if (Stmt *then = ifStmt->getThen())
      insertStmtRelation<DbModel::ConstexprIfThen>(if_stmt_id, then,
                                                   ast_context_);
    if (Stmt *elseStmt = ifStmt->getElse())
      insertStmtRelation<DbModel::ConstexprIfElse>(if_stmt_id, elseStmt,
                                                   ast_context_);
  } else {
    if (Stmt *then = ifStmt->getThen())
      insertStmtRelation<DbModel::ConstevalIfThen>(if_stmt_id, then,
                                                   ast_context_);
    if (Stmt *elseStmt = ifStmt->getElse())
      insertStmtRelation<DbModel::ConstevalIfElse>(if_stmt_id, elseStmt,
                                                   ast_context_);
  }

  return if_stmt_id;
}

int StmtProcessor::processForStmt(ForStmt *forStmt) {
  int for_stmt_id = getStmtId(forStmt, StmtKind::FOR);

  // int for_or_range_id = GENID(StmtForOrRangeBased);
  // DbModel::StmtForOrRangeBased stmtForOrRangeBasedModel = {
  //     for_or_range_id, static_cast<int>(ForType::FOR), for_stmt_id};
  // STG.insertClassObj(stmtForOrRangeBasedModel);

  // 1. 处理初始化部分
  if (Stmt *init = forStmt->getInit()) {
    KeyType stmtKey = KeyGen::Stmt_::makeKey(init, ast_context_);
    if (auto cachedId = SEARCH_STMT_CACHE(stmtKey)) {
      DbModel::ForInit forInitModel = {for_stmt_id, *cachedId};
      STG.insertClassObj(forInitModel);
    } else {
      DbModel::ForInit forInitModel = {for_stmt_id, -1};
      STG.insertClassObj(forInitModel);
      PendingUpdate update{
          stmtKey, CacheType::STMT, [for_stmt_id](int resolvedId) {
            DbModel::ForInit updated_record = {for_stmt_id, resolvedId};
            STG.insertClassObj(updated_record);
          }};
      DependencyManager::instance().addDependency(update);
    }
  }

  // 2. 处理条件部分
  if (Expr *cond = forStmt->getCond()) {
    KeyType exprKey = KeyGen::Expr_::makeKey(cond, ast_context_);
    if (auto cachedId = SEARCH_EXPR_CACHE(exprKey)) {
      DbModel::ForCond forCondModel = {for_stmt_id, *cachedId};
      STG.insertClassObj(forCondModel);
    } else {
      DbModel::ForCond forCondModel = {for_stmt_id, -1};
      STG.insertClassObj(forCondModel);
      PendingUpdate update{
          exprKey, CacheType::EXPR, [for_stmt_id](int resolvedId) {
            DbModel::ForCond updated_record = {for_stmt_id, resolvedId};
            STG.insertClassObj(updated_record);
          }};
      DependencyManager::instance().addDependency(update);
    }
  }

  // 3. 处理更新部分
  if (Expr *inc = forStmt->getInc()) {
    KeyType exprKey = KeyGen::Expr_::makeKey(inc, ast_context_);
    if (auto cachedId = SEARCH_EXPR_CACHE(exprKey)) {
      DbModel::ForUpdate forUpdateModel = {for_stmt_id, *cachedId};
      STG.insertClassObj(forUpdateModel);
    } else {
      DbModel::ForUpdate forUpdateModel = {for_stmt_id, -1};
      STG.insertClassObj(forUpdateModel);
      PendingUpdate update{
          exprKey, CacheType::EXPR, [for_stmt_id](int resolvedId) {
            DbModel::ForUpdate updated_record = {for_stmt_id, resolvedId};
            STG.insertClassObj(updated_record);
          }};
      DependencyManager::instance().addDependency(update);
    }
  }

  // 4. 处理循环体
  if (Stmt *body = forStmt->getBody()) {
    KeyType stmtKey = KeyGen::Stmt_::makeKey(body, ast_context_);
    if (auto cachedId = SEARCH_STMT_CACHE(stmtKey)) {
      DbModel::ForBody forBodyModel = {for_stmt_id, *cachedId};
      STG.insertClassObj(forBodyModel);
    } else {
      DbModel::ForBody forBodyModel = {for_stmt_id, -1};
      STG.insertClassObj(forBodyModel);
      PendingUpdate update{
          stmtKey, CacheType::STMT, [for_stmt_id](int resolvedId) {
            DbModel::ForBody updated_record = {for_stmt_id, resolvedId};
            STG.insertClassObj(updated_record);
          }};
      DependencyManager::instance().addDependency(update);
    }
  }

  return for_stmt_id;
}

int StmtProcessor::processCXXForRangeStmt(CXXForRangeStmt *rangeForStmt) {
  int for_stmt_id = getStmtId((Stmt *)rangeForStmt, StmtKind::FOR);

  // int for_or_range_id = GENID(StmtForOrRangeBased);
  // DbModel::StmtForOrRangeBased stmtForOrRangeBasedModel = {
  //     for_or_range_id, static_cast<int>(ForType::RANGE_BASED_FOR),
  //     for_stmt_id};
  // STG.insertClassObj(stmtForOrRangeBasedModel);

  // 1. 处理范围声明（相当于初始化）
  if (Stmt *init = rangeForStmt->getInit()) {
    KeyType stmtKey = KeyGen::Stmt_::makeKey(init, ast_context_);
    if (auto cachedId = SEARCH_STMT_CACHE(stmtKey)) {
      DbModel::ForInit forInitModel = {for_stmt_id, *cachedId};
      STG.insertClassObj(forInitModel);
    } else {
      DbModel::ForInit forInitModel = {for_stmt_id, -1};
      STG.insertClassObj(forInitModel);
      PendingUpdate update{
          stmtKey, CacheType::STMT, [for_stmt_id](int resolvedId) {
            DbModel::ForInit updated_record = {for_stmt_id, resolvedId};
            STG.insertClassObj(updated_record);
          }};
      DependencyManager::instance().addDependency(update);
    }
  }

  return for_stmt_id;
}

int StmtProcessor::processWhileStmt(WhileStmt *whileStmt) {
  int while_stmt_id = getStmtId(whileStmt, StmtKind::WHILE);

  // 处理循环体
  if (Stmt *body = whileStmt->getBody()) {
    KeyType stmtKey = KeyGen::Stmt_::makeKey(body, ast_context_);
    if (auto cachedId = SEARCH_STMT_CACHE(stmtKey)) {
      DbModel::WhileBody whileBodyModel = {while_stmt_id, *cachedId};
      STG.insertClassObj(whileBodyModel);
    } else {
      DbModel::WhileBody whileBodyModel = {while_stmt_id, -1};
      STG.insertClassObj(whileBodyModel);
      PendingUpdate update{
          stmtKey, CacheType::STMT, [while_stmt_id](int resolvedId) {
            DbModel::WhileBody updated_record = {while_stmt_id, resolvedId};
            STG.insertClassObj(updated_record);
          }};
      DependencyManager::instance().addDependency(update);
    }
  }

  return while_stmt_id;
}

int StmtProcessor::processDoStmt(DoStmt *doStmt) {
  int do_stmt_id = getStmtId(doStmt, StmtKind::END_TEST_WHILE);

  // 处理循环体
  if (Stmt *body = doStmt->getBody()) {
    KeyType stmtKey = KeyGen::Stmt_::makeKey(body, ast_context_);
    if (auto cachedId = SEARCH_STMT_CACHE(stmtKey)) {
      DbModel::DoBody doBodyModel = {do_stmt_id, *cachedId};
      STG.insertClassObj(doBodyModel);
    } else {
      DbModel::DoBody doBodyModel = {do_stmt_id, -1};
      STG.insertClassObj(doBodyModel);
      PendingUpdate update{
          stmtKey, CacheType::STMT, [do_stmt_id](int resolvedId) {
            DbModel::DoBody updated_record = {do_stmt_id, resolvedId};
            STG.insertClassObj(updated_record);
          }};
      DependencyManager::instance().addDependency(update);
    }
  }

  return do_stmt_id;
}

int StmtProcessor::processSwitchStmt(SwitchStmt *switchStmt) {
  int switch_stmt_id = getStmtId(switchStmt, StmtKind::SWITCH);

  // 1. 处理初始化部分
  if (Stmt *init = switchStmt->getInit()) {
    KeyType stmtKey = KeyGen::Stmt_::makeKey(init, ast_context_);
    if (auto cachedId = SEARCH_STMT_CACHE(stmtKey)) {
      DbModel::SwitchInit switchInitModel = {switch_stmt_id, *cachedId};
      STG.insertClassObj(switchInitModel);
    } else {
      DbModel::SwitchInit switchInitModel = {switch_stmt_id, -1};
      STG.insertClassObj(switchInitModel);
      PendingUpdate update{
          stmtKey, CacheType::STMT, [switch_stmt_id](int resolvedId) {
            DbModel::SwitchInit updated_record = {switch_stmt_id, resolvedId};
            STG.insertClassObj(updated_record);
          }};
      DependencyManager::instance().addDependency(update);
    }
  }

  // 2. 处理主体部分
  if (Stmt *body = switchStmt->getBody()) {
    KeyType stmtKey = KeyGen::Stmt_::makeKey(body, ast_context_);
    if (auto cachedId = SEARCH_STMT_CACHE(stmtKey)) {
      DbModel::SwitchBody switchBodyModel = {switch_stmt_id, *cachedId};
      STG.insertClassObj(switchBodyModel);
    } else {
      DbModel::SwitchBody switchBodyModel = {switch_stmt_id, -1};
      STG.insertClassObj(switchBodyModel);
      PendingUpdate update{
          stmtKey, CacheType::STMT, [switch_stmt_id](int resolvedId) {
            DbModel::SwitchBody updated_record = {switch_stmt_id, resolvedId};
            STG.insertClassObj(updated_record);
          }};
      DependencyManager::instance().addDependency(update);
    }

    // 3. 处理 case 部分 - 遍历 body 中的 case 语句
    if (CompoundStmt *compoundBody = dyn_cast<CompoundStmt>(body)) {
      int case_index = 0;
      for (Stmt *child : compoundBody->children()) {
        if (SwitchCase *switchCase = dyn_cast<SwitchCase>(child)) {
          int case_id = getStmtId(switchCase, StmtKind::SWITCH_CASE);
          KeyType caseKey = KeyGen::Stmt_::makeKey(switchCase, ast_context_);
          if (auto cachedId = SEARCH_STMT_CACHE(caseKey)) {
            DbModel::SwitchCase switchCaseModel = {switch_stmt_id, case_index,
                                                   *cachedId};
            STG.insertClassObj(switchCaseModel);
          } else {
            DbModel::SwitchCase switchCaseModel = {switch_stmt_id, case_index,
                                                   -1};
            STG.insertClassObj(switchCaseModel);
            PendingUpdate update{caseKey, CacheType::STMT,
                                 [switch_stmt_id, case_index](int resolvedId) {
                                   DbModel::SwitchCase updated_record = {
                                       switch_stmt_id, case_index, resolvedId};
                                   STG.insertClassObj(updated_record);
                                 }};
            DependencyManager::instance().addDependency(update);
          }
          case_index++;
        }
      }
    }
  }

  return switch_stmt_id;
}

int StmtProcessor::processBlockStmt(CompoundStmt *blockStmt) {
  return getStmtId(blockStmt, StmtKind::BLOCK);
}

int StmtProcessor::processReturnStmt(ReturnStmt *returnStmt) {
  return getStmtId(returnStmt, StmtKind::RETURN);
}

int StmtProcessor::processDeclStmt(DeclStmt *declStmt) {
  return getStmtId(declStmt, StmtKind::DECL);
}

int StmtProcessor::processNullStmt(NullStmt *nullStmt) {
  return getStmtId(nullStmt, StmtKind::EMPTY);
}
