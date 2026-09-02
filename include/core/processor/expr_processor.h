#ifndef _EXPR_PROCESSOR_H_
#define _EXPR_PROCESSOR_H_

#include "core/processor/base_processor.h"
#include "core/srcloc_recorder.h"
#include "model/db/expr.h"
#include <clang/AST/Decl.h>
#include <clang/AST/Expr.h>
#include <clang/AST/ExprConcepts.h>
#include <clang/AST/ExprCXX.h>
#include <clang/AST/Stmt.h>
#include <unordered_set>

using namespace clang;

class TypeProcessor;
class SpecifierProcessor;

class ExprProcessor : public BaseProcessor {
public:
  int getOrProcessExprId(const clang::Expr *expr);
  int getOrProcessMainTreeExprId(const clang::Expr *expr);

  // Record a CodeQL main-expression-tree edge. Parent ids are stable ids from
  // an existing @expr, @stmt, or @initialiser row.
  void recordExprParent(const clang::Expr *child, int childIndex,
                        int parentId);
  void recordExprParentId(int childId, int childIndex, int parentId);

  void processDeclRef(DeclRefExpr *expr);
  void processMemberExpr(const MemberExpr *expr);

  void processUnaryOperator(const UnaryOperator *op);
  void processBinaryOperator(const BinaryOperator *op);
  void processConditionalOperator(const ConditionalOperator *op);

  void processStringLiteral(const StringLiteral *literal);
  void processIntegerLiteral(const IntegerLiteral *literal);
  int processAttributeIntegerLiteral(const IntegerLiteral *literal);
  void processFloatingLiteral(const FloatingLiteral *literal);
  void processCharacterLiteral(const CharacterLiteral *literal);
  void processBoolLiteral(const CXXBoolLiteralExpr *literal);

  void processAssignArithExpr(const BinaryOperator *op);
  void processAssignBitwiseExpr(const BinaryOperator *op);
  void processAssignPointerExpr(const BinaryOperator *op);
  void processAssignOpExpr(const BinaryOperator *op);
  void processAssignExpr(const BinaryOperator *op);
  void processCallExpr(const CallExpr *expr);

  int processCastExpr(const CastExpr *castExpr);
  int processParenExpr(const ParenExpr *parenExpr);

  void processArraySubscriptExpr(const ArraySubscriptExpr *expr);
  void processInitListExpr(const InitListExpr *expr);
  void processUnaryExprOrTypeTraitExpr(const UnaryExprOrTypeTraitExpr *expr);
  int processConceptSpecializationExpr(
      const ConceptSpecializationExpr *expr, int conceptId);
  int processNonTypeTemplateParmDecl(const NonTypeTemplateParmDecl *decl);

  ExprProcessor(ASTContext *ast_context, const PrintingPolicy pp,
                TypeProcessor *tp = nullptr,
                SpecifierProcessor *sp = nullptr)
      : BaseProcessor(ast_context, pp), type_processor_(tp),
        specifier_processor_(sp) {};
  ~ExprProcessor() = default;

private:
  int _typeId;
  int _varId;
  int _varDeclId;
  std::string _name;

  int findCachedExprId(const Expr *expr) const;
  bool canProcessExprForReference(const Expr *expr) const;

  int processBaseExpr(Expr *expr, ExprKind exprKind);
  void recordExprType(const Expr *expr, int exprId);
  void processCastTypes(const CastExpr *castExpr);
  void recordExprConv(int convertedId, int conversionId);
  void recordExprIsLoad(int exprId);
  void recordCompGenerated(int exprId);
  void recordConversionKind(int exprId, int kind);
  int classifyConversionKind(const CastExpr *castExpr) const;
  ExprKind classifyCastExprKind(const CastExpr *castExpr) const;

  int processLiteralValue(const std::string &value, const std::string &text,
                          int exprId);
  void recordValueBindExpr(int valueId, int exprId);

  void recordAggregateArrayInit(int initListExprId, const InitListExpr *expr);
  void recordAggregateFieldInit(int initListExprId, const InitListExpr *expr);
  void recordSizeOfBind(int exprId, const UnaryExprOrTypeTraitExpr *expr);

  const clang::Expr *normalizeMainTreeExpr(const clang::Expr *expr) const;
  void recordCallChildren(const CallExpr *expr, int parentId);

  std::unordered_set<std::string> recorded_parent_edges_;
  std::unordered_set<int> recorded_loads_;

  TypeProcessor *type_processor_ = nullptr;
  SpecifierProcessor *specifier_processor_ = nullptr;
};

#endif // _EXPR_PROCESSOR_H_
