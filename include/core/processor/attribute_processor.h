#ifndef _ATTRIBUTE_PROCESSOR_H_
#define _ATTRIBUTE_PROCESSOR_H_

#include "core/processor/base_processor.h"
#include <clang/AST/Attr.h>
#include <clang/AST/Decl.h>

class AttributeProcessor : public BaseProcessor {
public:
  void processFunctionAttributes(int func_id, const clang::FunctionDecl *decl);

  AttributeProcessor(clang::ASTContext *ast_context,
                     const clang::PrintingPolicy pp)
      : BaseProcessor(ast_context, pp) {};
  ~AttributeProcessor() = default;

private:
  int recordAttribute(const clang::Attr *attr);
  int mapAttributeKind(const clang::Attr *attr) const;
  std::string getAttributeName(const clang::Attr *attr) const;
  std::string getAttributeNamespace(const clang::Attr *attr) const;
};

#endif // _ATTRIBUTE_PROCESSOR_H_
