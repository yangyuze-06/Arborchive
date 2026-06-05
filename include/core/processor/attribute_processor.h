#ifndef _ATTRIBUTE_PROCESSOR_H_
#define _ATTRIBUTE_PROCESSOR_H_

#include "core/processor/base_processor.h"
#include "model/db/attribute.h"
#include <clang/AST/Attr.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Expr.h>
#include <clang/AST/Stmt.h>

class ExprProcessor;

class AttributeProcessor : public BaseProcessor {
public:
  void processFunctionAttributes(int func_id, const clang::FunctionDecl *decl);
  void processTypeAttributes(int type_id, const clang::TypeDecl *decl);
  void processVariableAttributes(int var_id, const clang::Decl *decl);
  void processStatementAttributes(int stmt_id,
                                  const clang::AttributedStmt *stmt);

  AttributeProcessor(clang::ASTContext *ast_context,
                     const clang::PrintingPolicy pp,
                     ExprProcessor *expr_processor = nullptr)
      : BaseProcessor(ast_context, pp), expr_processor_(expr_processor) {};
  ~AttributeProcessor() = default;

private:
  ExprProcessor *expr_processor_;

  int recordAttribute(const clang::Attr *attr);
  void recordAttributeArguments(int attr_id, const clang::Attr *attr);
  int recordAttributeArg(int attr_id, AttributeArgKind kind, int index,
                         clang::SourceLocation loc);
  void recordStringArgument(int attr_id, int index, const std::string &value,
                            clang::SourceLocation loc);
  void recordAlignedArgument(int attr_id, int index, const clang::Expr *expr);
  void recordNonLiteralExpressionArgument(int attr_id, int index,
                                          const clang::Expr *expr);
  void recordIntegerConstantArgument(int attr_id, int index,
                                     const clang::Expr *expr);
  void recordExpressionArgument(int attr_id, int index,
                                const clang::Expr *expr);
  const clang::IntegerLiteral *
  getStableIntegerLiteral(const clang::Expr *expr) const;
  const clang::Expr *getStableNonLiteralExpr(const clang::Expr *expr) const;

  int mapAttributeKind(const clang::Attr *attr) const;
  std::string getAttributeName(const clang::Attr *attr) const;
  std::string getAttributeNamespace(const clang::Attr *attr) const;
};

#endif // _ATTRIBUTE_PROCESSOR_H_
