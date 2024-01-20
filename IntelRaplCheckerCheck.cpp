#include "IntelRaplCheckerCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include <iostream>
#include "clang/Lex/Lexer.h"

using namespace clang::ast_matchers;
using namespace clang;
using namespace std;

namespace clang {
namespace tidy {
namespace performance {

void IntelRaplCheckerCheck::registerMatchers(MatchFinder *Finder) {
    Finder->addMatcher(cxxMemberCallExpr().bind("insertCall"), this);
    Finder->addMatcher(forStmt().bind("forLoop"), this);
}

bool areExprsEqual(const Expr *E1, const Expr *E2, ASTContext &Context) {
    SourceManager &SM = Context.getSourceManager();
    LangOptions LO;
    StringRef Text1 = Lexer::getSourceText(CharSourceRange::getTokenRange(E1->getSourceRange()), SM, LO);
    StringRef Text2 = Lexer::getSourceText(CharSourceRange::getTokenRange(E2->getSourceRange()), SM, LO);
    return Text1.equals(Text2);
}

void IntelRaplCheckerCheck::check(const MatchFinder::MatchResult &Result) {
    const auto *InsertCall = Result.Nodes.getNodeAs<CXXMemberCallExpr>("insertCall");
    if (!InsertCall) return;

    // Check the number of arguments in the insert call
    if (InsertCall->getNumArgs() != 2) return;

    const auto *MethodDecl = InsertCall->getMethodDecl();
    if (MethodDecl->getNameAsString() != "insert") return;

    const auto *ReceiverType = MethodDecl->getThisType()->getPointeeType()->getAsCXXRecordDecl();
    if (ReceiverType->getNameAsString() != "vector" && ReceiverType->getNameAsString() != "deque" && ReceiverType->getNameAsString() != "list") return;

    findEndCalls(InsertCall->getArg(0), InsertCall->getImplicitObjectArgument(), InsertCall, *Result.Context);
}


void IntelRaplCheckerCheck::findEndCalls(const Expr *E, const Expr *ContainerExpr, const CXXMemberCallExpr *InsertCall, ASTContext &Context) {
    if (!E || !ContainerExpr) return;

    if (const auto *Call = dyn_cast<CXXMemberCallExpr>(E)) {
        if (const auto *MethodDecl = Call->getMethodDecl()) {
            if (MethodDecl->getNameAsString() == "end") {
                const auto *Member = dyn_cast<MemberExpr>(Call->getCallee());
                const auto *ContainerEnd = Member ? Member->getBase() : nullptr;
                const auto *MemberInsert = dyn_cast<MemberExpr>(ContainerExpr);
                const auto *ContainerInsert = MemberInsert ? MemberInsert->getBase() : nullptr;
                llvm::errs() << "ContainerEnd: " << ContainerEnd << "\n";
                llvm::errs() << "ContainerInsert: " << ContainerInsert << "\n";
                llvm::errs() << "ContainerExpr type: " << ContainerExpr->getType().getAsString() << "\n";
                ContainerExpr->dump();
                if (ContainerEnd) {
                    SourceLocation InsertMethodNameLoc = InsertCall->getExprLoc();
                    SourceLocation InsertMethodEndLoc = Lexer::getLocForEndOfToken(InsertMethodNameLoc, 0, Context.getSourceManager(), 
                    Context.getLangOpts());
                    SourceRange InsertMethodRange(InsertMethodNameLoc, InsertMethodEndLoc);
                    // Compare the expressions to see if they refer to the same container object
                    if (areExprsEqual(ContainerEnd, ContainerExpr, Context)) {
                    diag(InsertMethodNameLoc, "Insert at end can be replaced by push_back")
                    << FixItHint::CreateReplacement(InsertMethodRange, "push_back");
                    }
                }
            }
        }
    }

    for (const auto *ChildStmt : E->children()) {
        if (const auto *ChildExpr = dyn_cast_or_null<Expr>(ChildStmt)) {
            findEndCalls(ChildExpr, ContainerExpr, InsertCall, Context);
        }
    }
}



} // namespace performance
} // namespace tidy
} // namespace clang
