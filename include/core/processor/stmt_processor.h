#ifndef _STMT_PROCESSOR_H_
#define _STMT_PROCESSOR_H_

#include "core/processor/base_processor.h"
#include "core/srcloc_recorder.h"
#include "model/db/stmt.h"
#include <clang/AST/Decl.h>
#include <clang/AST/Stmt.h>

using namespace clang;

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

  StmtProcessor(ASTContext *ast_context, const PrintingPolicy pp)
      : BaseProcessor(ast_context, pp) {};
  ~StmtProcessor() = default;

private:
  int _typeId;
  int _varId;
  int _varDeclId;
  std::string _name;
};

#endif // _STMT_PROCESSOR_H_
