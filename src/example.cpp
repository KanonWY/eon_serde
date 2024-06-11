#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

using namespace clang;
using namespace clang::tooling;

class AddClassVisitor : public RecursiveASTVisitor<AddClassVisitor> {
public:
    explicit AddClassVisitor(ASTContext &Context, Rewriter &R)
        : Context(Context), TheRewriter(R) {}

    bool VisitFunctionDecl(FunctionDecl *Declaration) {
        if (Context.getSourceManager().isInMainFile(Declaration->getLocation())) {
            TheRewriter.InsertText(Declaration->getBeginLoc(), "// Function: ", true, true);
        }
        return true;
    }

private:
    ASTContext &Context;
    Rewriter &TheRewriter;
};

class AddClassASTConsumer : public ASTConsumer {
public:
    explicit AddClassASTConsumer(ASTContext &Context, Rewriter &R)
        : Visitor(Context, R) {}

    void HandleTranslationUnit(ASTContext &Context) override {
        Visitor.TraverseDecl(Context.getTranslationUnitDecl());
    }

private:
    AddClassVisitor Visitor;
};

class AddClassAction : public ASTFrontendAction {
public:
    AddClassAction() {}

    void EndSourceFileAction() override {
        SourceManager &SM = TheRewriter.getSourceMgr();
        llvm::errs() << "** EndSourceFileAction for: "
                     << SM.getFileEntryForID(SM.getMainFileID())->getName() << "\n";
        TheRewriter.getEditBuffer(SM.getMainFileID()).write(llvm::outs());
    }

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override {
        TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
        return std::make_unique<AddClassASTConsumer>(CI.getASTContext(), TheRewriter);
    }

private:
    Rewriter TheRewriter;
};

static llvm::cl::OptionCategory MyToolCategory("my-tool options");

int main(int argc, const char **argv) {
    auto ExpectedParser = CommonOptionsParser::create(argc, argv, MyToolCategory);
    if (!ExpectedParser) {
        llvm::errs() << ExpectedParser.takeError();
        return 1;
    }

    CommonOptionsParser &OptionsParser = ExpectedParser.get();
    ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());
    return Tool.run(newFrontendActionFactory<AddClassAction>().get());
}
