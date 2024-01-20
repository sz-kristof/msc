#include "PushbackCheckerCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Lex/Lexer.h"


using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace performance {

void PushbackCheckerCheck::registerMatchers(MatchFinder *Finder) {
  // Match member expressions that are a call to push_back on a std::vector.
  Finder->addMatcher(
      forStmt(hasDescendant(
          cxxMemberCallExpr(
              on(hasType(hasUnqualifiedDesugaredType(recordType(
                  hasDeclaration(cxxRecordDecl(hasName("::std::vector"))))))),
              callee(cxxMethodDecl(hasName("push_back"))),
              hasArgument(0, cxxOperatorCallExpr(hasOverloadedOperatorName("*")))
          ).bind("expr"))),
      this);
}

void PushbackCheckerCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *MatchedExpr = Result.Nodes.getNodeAs<clang::Expr>("expr");
  if (!MatchedExpr)
    return;

  SourceLocation StartLoc = MatchedExpr->getBeginLoc();
  SourceManager &SM = *Result.SourceManager;

  // Adjust StartLoc to the beginning of 'push_back'
  StartLoc = Lexer::findLocationAfterToken(StartLoc, tok::period, SM, Result.Context->getLangOpts(), false);

  if (StartLoc.isValid()) {
    diag(StartLoc, "push_back called on a std::vector")
    << FixItHint::CreateReplacement(
        CharSourceRange::getTokenRange(SourceRange(StartLoc, MatchedExpr->getEndLoc())),
        "insert can be used. For example: x.insert(x.end(),y.begin(),y.end())");

  }
}

}  // namespace vector
}  // namespace tidy
}  // namespace clang
