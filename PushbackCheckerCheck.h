#ifndef PUSHBACK_CHECKER_CHECK_H
#define PUSHBACK_CHECKER_CHECK_H

//#include "clang/ASTMatchers/ASTMatchFinder.h"
//#include "clang/Frontend/CompilerInstance.h"
//#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
//#include "clang/StaticAnalyzer/Core/Checker.h"
//#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "../ClangTidyCheck.h"

namespace clang::tidy::performance {

class PushbackCheckerCheck : public ClangTidyCheck {
public:
  PushbackCheckerCheck(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}

  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
};

} 

#endif  // PUSHBACK_CHECKER_CHECK_H
