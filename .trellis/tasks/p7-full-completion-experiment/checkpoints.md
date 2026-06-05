# P7 Full Completion Experiment Checkpoints

This file records checkpoint evidence for the P7 full completion experiment.
P7 remains `PARTIAL` unless every full-DONE criterion is proven.

## Checkpoint 0: Baseline

Status: PASS

Commands:

```bash
git status --short
git diff --check
make LLVM_CONFIG=/opt/homebrew/opt/llvm@19/bin/llvm-config debug -j 8
./scripts/test_all.sh
```

Focused evidence:

```text
existing P7 focused DBs: attribute_arg_expr|0, attribute_arg_type|0,
attribute_arg_name|0
duplicate expr smoke query: 0 for current P7 DBs
duplicate ParmVarDecl query: no rows
docs/roadmap.md: P7 remains PARTIAL
```

Self-review:

- Baseline behavior was stable before Checkpoint 1 edits.
- Existing P7 fixtures passed.
- No schema/model/table initialization changes were present.

## Checkpoint 1: attribute_arg_expr Minimal Implementation

Status: PASS

Implemented:

- `AlignedAttr` argument classification now splits literal and non-literal
  source expressions.
- Direct/shallow integer literals remain in `attribute_arg_constant`.
- Non-literal, non-dependent `AlignedAttr` expressions use
  `ExprProcessor::getOrProcessExprId()` and populate `attribute_arg_expr`.
- Dependent template aligned expressions produce no typed payload row.
- `ASTVisitor` remains unchanged.

Focused fixture:

```text
tests/unit-tests/p7/attribute_expr_args_case.cc
```

Focused SQL evidence:

```text
attribute_arg_constant|1
attribute_arg_expr|2
attribute_arg_type|0
attribute_arg_name|0
p7f_expr_aligned_add|aligned|5|0|expr-kind 25
p7f_expr_aligned_wrapped_mul|aligned|5|0|expr-kind 27
p7f_expr_aligned_literal|aligned|2|0|16
dependent template aligned attribute has no payload row
constant/expr overlap query returned no rows
duplicate expr smoke query returned no rows
```

Regression evidence:

```text
attribute_constant_args_case:
attribute_arg_constant|3
attribute_arg_expr|0
attribute_arg_type|0
attribute_arg_name|0
p7f_aligned_direct|aligned|2|0|16
p7f_aligned_paren|aligned|2|0|32
p7f_aligned_unsigned|aligned|2|0|64
```

Self-review:

- No expression was stringified.
- No arbitrary expression was evaluated or folded.
- No argument was inserted into both `attribute_arg_constant` and
  `attribute_arg_expr`.
- Dependent expressions remain skipped by the ExprProcessor resolver.
- `attribute_arg_type` and `attribute_arg_name` remain empty.
- This is a minimal safe subset, not full `attribute_arg_expr` completion.
