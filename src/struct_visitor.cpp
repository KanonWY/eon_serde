#include "clang/AST/AST.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/AST/Attr.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;

namespace {

class MyAttrVisitor: public RecursiveASTVisitor<MyAttrVisitor> {
public:
    explicit MyAttrVisitor(ASTContext* Context):
        Context(Context) {}

    bool VisitCXXRecordDecl(CXXRecordDecl* D) {
        if (D->hasAttrs()) {
            for (const auto* Attr: D->getAttrs()) {
                if (const auto* Annotate = dyn_cast<AnnotateAttr>(Attr)) {
                    if (Annotate->getAnnotation() == "EonSer") {
                        llvm::errs() << "Found class with [[EonSer]]: " << D->getNameAsString() << "\n";

                        for (const auto* Field: D->fields()) {
                            llvm::errs() << " Member: " << Field->getNameAsString()
                                         << " of type " << Field->getType().getAsString() << "\n";
                        }
                    }
                }
            }
        }
        return true;
    }

private:
    ASTContext* Context;
};

class MyAttrConsumer: public ASTConsumer {
public:
    explicit MyAttrConsumer(ASTContext* Context):
        Visitor(Context) {}

    void HandleTranslationUnit(ASTContext& Context) override {
        Visitor.TraverseDecl(Context.getTranslationUnitDecl());
    }

private:
    MyAttrVisitor Visitor;
};

class MyAttrAction: public PluginASTAction {
protected:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance& CI, llvm::StringRef) override {
        return std::make_unique<MyAttrConsumer>(&CI.getASTContext());
    }

    bool ParseArgs(const CompilerInstance& CI, const std::vector<std::string>& args) override {
        // 解析插件的参数（如果有）。现在我们不做任何处理。
        return true;
    }

    PluginASTAction::ActionType getActionType() override {
        return PluginASTAction::AddBeforeMainAction;
    }
};

} // namespace

static FrontendPluginRegistry::Add<MyAttrAction>
X("my-attr-plugin", "Parse custom [[MyAttr]] attributes");
