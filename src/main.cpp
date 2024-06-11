#include <cassert>
#include <iostream>
#include <memory>

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclGroup.h"
#include "clang/Basic/LLVM.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendPluginRegistry.h"


using namespace clang;


namespace {
class NamingASTConsumer: public ASTConsumer {
    CompilerInstance& CI;

public:
    NamingASTConsumer(CompilerInstance& CI):
        CI(CI) {}

    bool HandleTopLevelDecl(DeclGroupRef DG) override {
        for (DeclGroupRef::iterator I = DG.begin(), E = DG.end(); I != E; ++I) {
            const Decl* D = *I;
            if (const FunctionDecl* FD = dyn_cast<FunctionDecl>(D)) {
                std::string Name = FD->getNameInfo().getName().getAsString();
                assert(Name.length() > 0 && "Unexpect empty identifier");
                char& First = Name.at(0);
                if (!(First >= 'a' && First <= 'z')) {
                    DiagnosticsEngine& Diag = CI.getDiagnostics();
                    unsigned           ID   = Diag.getCustomDiagID(
                    DiagnosticsEngine::Warning,
                    "Function name should start with "
                                "lowercase letter");
                    Diag.Report(FD->getLocation(), ID);
                }
            }
        }
        return true;
    }
};


class PluginNamingAction: public PluginASTAction {
public:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance& CI, StringRef file) override {
        return std::make_unique<NamingASTConsumer>(CI);
    }
    PluginASTAction::ActionType getActionType() override {
        return AddAfterMainAction;
    }

    bool ParseArgs(const CompilerInstance& CI, const std::vector<std::string>& arg) override {
        return true;
    }
};

static FrontendPluginRegistry::Add<PluginNamingAction> X("naming-plugin", "naming plugin");
} // namespace
