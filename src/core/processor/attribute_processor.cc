#include "core/processor/attribute_processor.h"
#include "core/processor/expr_processor.h"
#include "core/srcloc_recorder.h"
#include "db/storage_facade.h"
#include "model/db/attribute.h"
#include "util/id_generator.h"
#include <clang/AST/Expr.h>
#include <clang/Basic/IdentifierTable.h>
#include <llvm/Support/Casting.h>

void AttributeProcessor::processFunctionAttributes(
    int func_id, const clang::FunctionDecl *decl) {
  if (func_id == -1 || !decl)
    return;

  for (const clang::Attr *attr : decl->attrs()) {
    const int attr_id = recordAttribute(attr);
    if (attr_id == -1)
      continue;

    recordAttributeArguments(attr_id, attr);

    DbModel::FuncAttribute func_attr = {func_id, attr_id};
    STG.insertClassObj(func_attr);
  }
}

void AttributeProcessor::processTypeAttributes(int type_id,
                                               const clang::TypeDecl *decl) {
  if (type_id == -1 || !decl)
    return;

  for (const clang::Attr *attr : decl->attrs()) {
    const int attr_id = recordAttribute(attr);
    if (attr_id == -1)
      continue;

    recordAttributeArguments(attr_id, attr);

    DbModel::TypeAttribute type_attr = {type_id, attr_id};
    STG.insertClassObj(type_attr);
  }
}

void AttributeProcessor::processVariableAttributes(int var_id,
                                                   const clang::Decl *decl) {
  if (var_id == -1 || !decl)
    return;

  for (const clang::Attr *attr : decl->attrs()) {
    const int attr_id = recordAttribute(attr);
    if (attr_id == -1)
      continue;

    recordAttributeArguments(attr_id, attr);

    DbModel::VarAttribute var_attr = {var_id, attr_id};
    STG.insertClassObj(var_attr);
  }
}

void AttributeProcessor::processStatementAttributes(
    int stmt_id, const clang::AttributedStmt *stmt) {
  if (stmt_id == -1 || !stmt)
    return;

  for (const clang::Attr *attr : stmt->getAttrs()) {
    const int attr_id = recordAttribute(attr);
    if (attr_id == -1)
      continue;

    recordAttributeArguments(attr_id, attr);

    DbModel::StmtAttribute stmt_attr = {stmt_id, attr_id};
    STG.insertClassObj(stmt_attr);
  }
}

int AttributeProcessor::recordAttribute(const clang::Attr *attr) {
  if (!attr || attr->isImplicit())
    return -1;

  const int kind = mapAttributeKind(attr);
  if (kind == -1)
    return -1;

  clang::SourceLocation loc = attr->getLocation();
  LocIdPair *loc_id_pair = PROC_DEFT(loc, loc, ast_context_);
  DbModel::Attribute attribute = {GENID(Attribute), kind,
                                  getAttributeName(attr),
                                  getAttributeNamespace(attr),
                                  loc_id_pair->spec_id};
  STG.insertClassObj(attribute);
  return attribute.id;
}

void AttributeProcessor::recordAttributeArguments(int attr_id,
                                                  const clang::Attr *attr) {
  if (attr_id == -1 || !attr)
    return;

  if (const auto *deprecated = llvm::dyn_cast<clang::DeprecatedAttr>(attr)) {
    if (deprecated->getMessageLength() > 0)
      recordStringArgument(attr_id, 0, deprecated->getMessage().str(),
                           attr->getLocation());
    if (deprecated->getReplacementLength() > 0)
      recordStringArgument(attr_id, 1, deprecated->getReplacement().str(),
                           attr->getLocation());
    return;
  }

  if (const auto *annotate = llvm::dyn_cast<clang::AnnotateAttr>(attr)) {
    if (annotate->getAnnotationLength() > 0)
      recordStringArgument(attr_id, 0, annotate->getAnnotation().str(),
                           attr->getLocation());
    return;
  }

  if (const auto *section = llvm::dyn_cast<clang::SectionAttr>(attr)) {
    if (section->getNameLength() > 0)
      recordStringArgument(attr_id, 0, section->getName().str(),
                           attr->getLocation());
    return;
  }

  if (const auto *warn_unused =
          llvm::dyn_cast<clang::WarnUnusedResultAttr>(attr)) {
    if (warn_unused->getMessageLength() > 0)
      recordStringArgument(attr_id, 0, warn_unused->getMessage().str(),
                           attr->getLocation());
    return;
  }

  if (const auto *aligned = llvm::dyn_cast<clang::AlignedAttr>(attr)) {
    if (aligned->isAlignmentExpr())
      recordAlignedArgument(attr_id, 0, aligned->getAlignmentExpr());
    return;
  }

  if (const auto *assume_aligned =
          llvm::dyn_cast<clang::AssumeAlignedAttr>(attr)) {
    recordNonLiteralExpressionArgument(attr_id, 0,
                                       assume_aligned->getAlignment());
    recordNonLiteralExpressionArgument(attr_id, 1, assume_aligned->getOffset());
    return;
  }
}

int AttributeProcessor::recordAttributeArg(int attr_id, AttributeArgKind kind,
                                           int index,
                                           clang::SourceLocation loc) {
  if (attr_id == -1 || loc.isInvalid())
    return -1;

  LocIdPair *loc_id_pair = PROC_DEFT(loc, loc, ast_context_);
  DbModel::AttributeArg arg = {GENID(AttributeArg), static_cast<int>(kind),
                               attr_id, index, loc_id_pair->spec_id};
  STG.insertClassObj(arg);
  return arg.id;
}

void AttributeProcessor::recordStringArgument(int attr_id, int index,
                                              const std::string &value,
                                              clang::SourceLocation loc) {
  int arg_id = recordAttributeArg(attr_id, AttributeArgKind::TOKEN, index, loc);
  if (arg_id == -1)
    return;

  DbModel::AttributeArgValue arg_value = {arg_id, value};
  STG.insertClassObj(arg_value);
}

void AttributeProcessor::recordAlignedArgument(int attr_id, int index,
                                               const clang::Expr *expr) {
  if (!expr_processor_)
    return;

  if (getStableIntegerLiteral(expr)) {
    recordIntegerConstantArgument(attr_id, index, expr);
    return;
  }

  const clang::Expr *value_expr = getStableNonLiteralExpr(expr);
  if (!value_expr)
    return;

  recordExpressionArgument(attr_id, index, value_expr);
}

void AttributeProcessor::recordNonLiteralExpressionArgument(
    int attr_id, int index, const clang::Expr *expr) {
  const clang::Expr *value_expr = getStableNonLiteralExpr(expr);
  if (!value_expr)
    return;

  recordExpressionArgument(attr_id, index, value_expr);
}

void AttributeProcessor::recordIntegerConstantArgument(int attr_id, int index,
                                                       const clang::Expr *expr) {
  if (!expr_processor_)
    return;

  const clang::IntegerLiteral *literal = getStableIntegerLiteral(expr);
  if (!literal)
    return;

  int expr_id = expr_processor_->processAttributeIntegerLiteral(literal);
  if (expr_id == -1)
    return;

  int arg_id = recordAttributeArg(attr_id, AttributeArgKind::CONSTANT, index,
                                  literal->getBeginLoc());
  if (arg_id == -1)
    return;

  DbModel::AttributeArgConstant arg_constant = {arg_id, expr_id};
  STG.insertClassObj(arg_constant);
}

void AttributeProcessor::recordExpressionArgument(int attr_id, int index,
                                                  const clang::Expr *expr) {
  if (!expr_processor_ || !expr)
    return;

  int expr_id = expr_processor_->getOrProcessExprId(expr);
  if (expr_id == -1)
    return;

  int arg_id = recordAttributeArg(attr_id, AttributeArgKind::EXPR, index,
                                  expr->getBeginLoc());
  if (arg_id == -1)
    return;

  DbModel::AttributeArgExpr arg_expr = {arg_id, expr_id};
  STG.insertClassObj(arg_expr);
}

const clang::IntegerLiteral *
AttributeProcessor::getStableIntegerLiteral(const clang::Expr *expr) const {
  const clang::Expr *current = expr;
  for (int depth = 0; current && depth < 4; ++depth) {
    current = current->IgnoreParenImpCasts();

    if (const auto *constant_expr =
            llvm::dyn_cast<clang::ConstantExpr>(current)) {
      current = constant_expr->getSubExpr();
      continue;
    }

    return llvm::dyn_cast<clang::IntegerLiteral>(current);
  }

  return nullptr;
}

const clang::Expr *
AttributeProcessor::getStableNonLiteralExpr(const clang::Expr *expr) const {
  const clang::Expr *current = expr;
  for (int depth = 0; current && depth < 4; ++depth) {
    current = current->IgnoreParenImpCasts();

    if (const auto *constant_expr =
            llvm::dyn_cast<clang::ConstantExpr>(current)) {
      current = constant_expr->getSubExpr();
      continue;
    }

    if (llvm::isa<clang::IntegerLiteral>(current))
      return nullptr;

    return current;
  }

  return nullptr;
}

int AttributeProcessor::mapAttributeKind(const clang::Attr *attr) const {
  if (!attr)
    return -1;

  if (attr->isGNUAttribute())
    return static_cast<int>(AttributeKind::GNU);
  if (attr->isStandardAttributeSyntax())
    return attr->isAlignas() ? static_cast<int>(AttributeKind::ALIGNAS)
                             : static_cast<int>(AttributeKind::STD);
  if (attr->isDeclspecAttribute())
    return static_cast<int>(AttributeKind::DECLSPEC);
  if (attr->isMicrosoftAttribute())
    return static_cast<int>(AttributeKind::MS);

  return -1;
}

std::string AttributeProcessor::getAttributeName(const clang::Attr *attr) const {
  if (!attr)
    return "";

  if (const clang::IdentifierInfo *name = attr->getAttrName())
    return name->getName().str();

  return attr->getNormalizedFullName();
}

std::string
AttributeProcessor::getAttributeNamespace(const clang::Attr *attr) const {
  if (!attr || !attr->hasScope())
    return "";

  if (const clang::IdentifierInfo *scope = attr->getScopeName())
    return scope->getName().str();

  return "";
}
