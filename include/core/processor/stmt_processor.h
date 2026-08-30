#ifndef _STMT_PROCESSOR_H_
#define _STMT_PROCESSOR_H_

#include "core/processor/base_processor.h"
#include "core/srcloc_recorder.h"
#include "model/db/stmt.h"
#include <clang/AST/Decl.h>
#include <clang/AST/Stmt.h>

using namespace clang;

class ExprProcessor;

class StmtProcessor : public BaseProcessor {
public:
  int getStmtId(Stmt *stmt, StmtKind stmtKind);

  int getSupportedAttributedStmtOwnerId(AttributedStmt *attributedStmt);

  int processIfStmt(IfStmt *ifStmt);
  int processForStmt(ForStmt *forStmt);
  int processCXXForRangeStmt(CXXForRangeStmt *rangeForStmt);
  int processWhileStmt(WhileStmt *whileStmt);
  int processDoStmt(DoStmt *doStmt);
  int processSwitchStmt(SwitchStmt *switchStmt);
  int processBlockStmt(CompoundStmt *blockStmt);
  int processReturnStmt(ReturnStmt *returnStmt);
  int processDeclStmt(DeclStmt *declStmt);
  int processNullStmt(NullStmt *nullStmt);

  StmtProcessor(ASTContext *ast_context, const PrintingPolicy pp,
                ExprProcessor *expr_processor = nullptr)
      : BaseProcessor(ast_context, pp), expr_processor_(expr_processor) {};
  ~StmtProcessor() = default;

private:
  int _typeId;
  int _varId;
  int _varDeclId;
  std::string _name;
  ExprProcessor *expr_processor_ = nullptr;
};

#endif // _STMT_PROCESSOR_H_
