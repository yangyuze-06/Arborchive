#include "core/processor/attribute_processor.h"
#include "core/srcloc_recorder.h"
#include "db/storage_facade.h"
#include "model/db/attribute.h"
#include "util/id_generator.h"
#include <clang/Basic/IdentifierTable.h>

void AttributeProcessor::processFunctionAttributes(
    int func_id, const clang::FunctionDecl *decl) {
  if (func_id == -1 || !decl)
    return;

  for (const clang::Attr *attr : decl->attrs()) {
    const int attr_id = recordAttribute(attr);
    if (attr_id == -1)
      continue;

    DbModel::FuncAttribute func_attr = {func_id, attr_id};
    STG.insertClassObj(func_attr);
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
