Create a Trellis goal brief for a large P7 full-completion experiment.

Target file:
.trellis/tasks/p7-full-completion-experiment/goal.md

Do not implement yet. Only create the goal brief.

Title:
P7 Full Attribute System Completion Experiment

Purpose:
Run an aggressive but checkpointed experiment to determine whether Arborchive P7 can be completed end-to-end. The goal is to attempt remaining P7 work across attribute_arg_expr, attribute_arg_type, attribute_arg_name, generalized constants, and final DONE decision, while preserving CodeQL-shaped semantics and avoiding fake completion.

Important experiment framing:
- This is an experiment.
- The user accepts the risk of trying a large task chain.
- However, repository correctness still matters.
- If a subtask is unsafe, ambiguous, or likely to pollute the database, stop that subtask and document the blocker.
- Do not claim full DONE unless every full-DONE criterion is satisfied.
- Every implemented checkpoint must self-review before continuing.

Reference documents:
- docs/analysis/p7_full_done_criteria.md
- docs/analysis/p7f_attribute_expr_design.md
- docs/analysis/p7_remaining_attribute_scope_audit.md
- docs/analysis/p7_attribute_owner_links.md
- docs/roadmap.md
- docs/semmlecode.cpp.dbscheme

Current P7 state:
Implemented safe subsets:
- explicit supported attribute presence rows
- funcattributes owner links
- typeattributes owner links for stable record/enum/alias type ids
- varattributes owner links for field/global/local/param variable ids
- stmtattributes conservative supported inner stmt owner links
- canonical ParmVarDecl variable entity id reuse
- attribute_arg_value for DeprecatedAttr message/replacement, AnnotateAttr string, SectionAttr string
- attribute_arg_constant for direct and shallow-wrapped AlignedAttr integer literal constants
- ExprProcessor::getOrProcessExprId(const clang::Expr *) precondition exists
- P7 remains PARTIAL
- docs/datatable-list.txt is intentionally not marked DONE

Remaining targets:
- attribute_arg_expr
- attribute_arg_type
- attribute_arg_name
- generalized constant/APValue policy
- broader constant/value families
- stmtattributes wrapper-vs-inner parity
- final P7 DONE decision

Global hard constraints:
- Do not modify schema/model/table init unless a documented CodeQL alignment bug is proven.
- Do not stringify arbitrary attribute args.
- Do not map expr/type/name payloads into attribute_arg_value.
- Do not use Type::getAsString() as type payload.
- Do not evaluate arbitrary expressions.
- Do not process dependent/template attribute args unless a safe policy is proven.
- Do not move attribute argument logic into ASTVisitor.
- ASTVisitor must remain thin.
- AttributeProcessor owns attribute argument classification and typed payload insertion.
- ExprProcessor/TypeProcessor may provide stable ids.
- Do not mark docs/datatable-list.txt DONE until final parity review proves full DONE.
- Do not hide uncertainty.

Checkpoint rule:
After every checkpoint:
1. run focused validation,
2. run self-review,
3. report PASS / PARTIAL PASS / BLOCKED / NEEDS CHANGES,
4. continue only if the change is semantically safe,
5. if unsafe, revert or isolate the unsafe checkpoint and document blocker.

Checkpoint 0: Baseline
Run:
- git status --short
- git diff --check
- make LLVM_CONFIG=/opt/homebrew/opt/llvm@19/bin/llvm-config debug -j 8
- ./scripts/test_all.sh

Generate or inspect existing P7 focused DBs with ./build/demo.
Confirm:
- current P7 fixtures pass
- attribute_arg_expr is currently 0 unless prior experiment changed it
- attribute_arg_type is 0
- attribute_arg_name is 0
- duplicate ParmVarDecl query returns no rows
- P7 remains PARTIAL in docs

Checkpoint 1: attribute_arg_expr minimal implementation
Goal:
Implement only the safest first expression payload subset.

Primary candidate:
- AlignedAttr non-literal expression, for example aligned(8 + 8), using ExprProcessor::getOrProcessExprId.

Rules:
- aligned(16), aligned((32)), aligned(64u) remain attribute_arg_constant.
- aligned(8 + 8) should become attribute_arg_expr only if getOrProcessExprId returns stable id.
- Do not evaluate aligned(8 + 8) into 16.
- Do not populate both constant and expr for the same argument.
- dependent/template aligned expressions are skipped.
- Do not implement EnableIfAttr, DiagnoseIfAttr, AnnotateAttr expr variants, or AssumeAlignedAttr yet unless explicitly proven safe in this checkpoint.

Add fixture:
- tests/unit-tests/p7/attribute_expr_args_case.cc

Validate:
- attribute_arg_expr > 0 for non-literal aligned expression
- constant rows still exist for literal aligned args
- no arg appears in both attribute_arg_constant and attribute_arg_expr
- attribute_arg_type and attribute_arg_name remain 0
- duplicate expr query or smoke evidence shows no duplicated expr rows
- test_all.sh passes

Self-review:
- Did any expression get stringified?
- Did any dependent expr get processed?
- Are expr ids stable and joinable?
- Did constant behavior remain intact?

Checkpoint 2: attribute_arg_expr broader design gate
Audit after checkpoint 1 before adding more expr attrs.
Consider:
- EnableIfAttr
- DiagnoseIfAttr
- AnnotateAttr expression variants
- AssumeAlignedAttr expression forms

Only implement additional expr attrs if all are true:
- Clang API is stable in this checkout
- ExprProcessor can produce stable id
- no duplicate rows
- dependent skip is clear
- positive and negative fixture is possible

If not safe, document blocker in docs/analysis/p7f_attribute_expr_design.md and do not implement.

Checkpoint 3: attribute_arg_type design and minimal implementation
Before implementation, audit:
- TypeSourceInfo / QualType source availability
- TypeProcessor stable @type id policy
- @type/@usertype compatibility
- canonical vs spelling type
- dependent type skip
- no getAsString fallback

Candidate:
- only one very stable Clang Attr type payload if available.
- AnnotateTypeAttr only if local Clang API and CodeQL semantics are clear.
- If no safe candidate exists, document blocker and leave attribute_arg_type deferred.

Add fixture only if implemented:
- tests/unit-tests/p7/attribute_type_args_case.cc

Validate:
- attribute_arg_type rows join to stable type ids
- no string fallback
- dependent types skipped
- expr/name behavior unaffected
- test_all.sh passes

Self-review:
- Are type ids truly stable?
- Is @type/@usertype policy documented?
- Was spelling/canonical behavior chosen deliberately?

Checkpoint 4: attribute_arg_name Microsoft named args
Before implementation, audit:
- CodeQL says named arguments are Microsoft feature.
- Determine if local Clang and fixture syntax can produce Microsoft named arguments.
- Do not use ModeAttr, FormatAttr, AvailabilityAttr, CleanupAttr as name unless CodeQL parity is proven.

Implement only if true Microsoft named argument semantics are available and testable.
Otherwise document blocker.

Add fixture only if implemented:
- tests/unit-tests/p7/attribute_name_args_case.cc

Validate:
- attribute_arg_name rows only for true named args
- string payloads are not misclassified
- decl refs are not misclassified
- expr/type behavior unaffected
- test_all.sh passes

Self-review:
- Did any identifier-like token get guessed as name?
- Is this aligned with CodeQL named argument semantics?

Checkpoint 5: constant/value family expansion
Goal:
Expand constant/value families only for stable Clang APIs.

Candidates:
- NonNullAttr integer indexes
- FormatAttr integer indexes and stable text/name-like kind only if target table is clear
- AllocSizeAttr / AllocAlignAttr integer positions
- AvailabilityAttr message/replacement strings only if indexing semantics clear
- AliasAttr only if target/compiler validation stable

Rules:
- Do not implement generalized APValue unless designed and tested.
- Do not evaluate arbitrary expressions.
- Do not turn expr payloads into constants unless CodeQL parity says so.
- Keep argument indexes deterministic.

Add focused fixtures:
- tests/unit-tests/p7/attribute_extra_constant_args_case.cc
- tests/unit-tests/p7/attribute_extra_value_args_case.cc
or one combined fixture if cleaner.

Validate:
- expected value/constant rows
- no accidental expr/type/name rows
- positive and negative cases
- test_all.sh passes

Checkpoint 6: stmtattributes wrapper-vs-inner parity review
Goal:
Review whether current stmtattributes inner-stmt ownership is enough for full DONE.

Tasks:
- inspect local statement schema and CodeQL dbscheme
- inspect StmtProcessor AttributedStmt handling
- determine whether wrapper stmt ids should exist
- if wrapper support is needed and safe, implement minimal stable wrapper strategy
- if not safe, document why stmtattributes remains PARTIAL

Do not force wrapper ids if it would destabilize existing statement extraction.

Validate:
- existing fallthrough/likely rows still correct
- unsupported wrappers skipped or modeled deliberately
- no duplicate wrapper/inner owner rows
- test_all.sh passes

Checkpoint 7: docs and final parity decision
Update docs:
- docs/roadmap.md
- docs/analysis/p7_full_done_criteria.md
- docs/analysis/p7f_attribute_expr_design.md
- docs/analysis/p7_remaining_attribute_scope_audit.md
- optional docs/analysis/p7_full_completion_experiment.md

Decide:
- Can P7 be full DONE?
- If yes, list exact evidence for every P7 table.
- If no, keep P7 PARTIAL and list remaining blockers.

Only modify docs/datatable-list.txt if:
- all P7 full-DONE criteria are met,
- expr/type/name/constant/value/owner semantics have positive and negative tests,
- unsupported/dependent cases have documented policy,
- SQLite evidence supports every table,
- no remaining partial owner-link blocker exists.

Final validation:
- git diff --check
- make LLVM_CONFIG=/opt/homebrew/opt/llvm@19/bin/llvm-config debug -j 8
- ./scripts/test_all.sh
- focused ./build/demo generation for every new fixture
- db_summary.py for focused DBs
- SQLite evidence for every P7 table
- duplicate ParmVarDecl query remains empty
- duplicate expr smoke query remains empty if applicable
- P7a/P7b compatibility queries
- owner-link fixture queries

Final report:
1. Verdict: PASS / PARTIAL PASS / BLOCKED / NEEDS CHANGES
2. Whether P7 can honestly be marked full DONE
3. Changed files grouped by code/tests/docs
4. Implemented payload families
5. Deferred payload families and blockers
6. Architecture boundary review
7. SQLite evidence
8. Validation results
9. Risk of schema pollution
10. Recommended commit message

Recommended commit names:
- If full P7 truly passes: feat: complete P7 attribute system
- If several safe subsets are added but P7 remains partial: feat: expand P7 attribute argument coverage
- If mainly blockers/docs: docs: document P7 full completion blockers