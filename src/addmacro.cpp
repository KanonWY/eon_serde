#include "clang/AST/AST.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/AST/Attr.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;

namespace {

class MyAttrVisitor : public RecursiveASTVisitor<MyAttrVisitor> {
public:
  explicit MyAttrVisitor(ASTContext *Context, Rewriter *R)
      : Context(Context), RewriterPtr(R) {}

  bool VisitCXXRecordDecl(CXXRecordDecl *D) {
    if (D->hasAttrs()) {
      for (const auto *Attr : D->getAttrs()) {
        if (const auto *Annotate = dyn_cast<AnnotateAttr>(Attr)) {
          if (Annotate->getAnnotation() == "MyAttr") {
            llvm::errs() << "Found class with [[MyAttr]]: " << D->getNameAsString() << "\n";

            // 构建宏定义字符串
            std::string macroDefinition = "EON_PARAM(" + D->getNameAsString();
            for (const auto *Field : D->fields()) {
              macroDefinition += ", " + Field->getNameAsString();
            }
            macroDefinition += ");";

            // 打印类的所有成员变量并插入宏定义
            llvm::errs() << macroDefinition << "\n";

            // 获取文件的开头位置
            SourceLocation StartLoc = Context->getSourceManager().getLocForStartOfFile(Context->getSourceManager().getMainFileID());

            // 打印 StartLoc 的行和列信息
            llvm::errs() << "StartLoc line: " << Context->getSourceManager().getSpellingLineNumber(StartLoc)
                         << ", column: " << Context->getSourceManager().getSpellingColumnNumber(StartLoc) << "\n";

            // 尝试在文件开头插入简单注释
            RewriterPtr->InsertTextAfter(StartLoc, "\n// Test insertion at start\n");

            // 如果简单注释不崩溃，再尝试插入宏定义
            // RewriterPtr->InsertTextAfter(StartLoc, "\n" + macroDefinition + "\n");
          }
        }
      }
    }
    return true;
  }

private:
  ASTContext *Context;
  Rewriter *RewriterPtr;
};

class MyAttrConsumer : public ASTConsumer {
public:
  explicit MyAttrConsumer(ASTContext *Context, Rewriter *R)
      : Visitor(Context, R), RewriterPtr(R) {}

  void HandleTranslationUnit(ASTContext &Context) override {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  MyAttrVisitor Visitor;
  Rewriter *RewriterPtr;
};

class MyAttrAction : public PluginASTAction {
public:
  MyAttrAction() {}

protected:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, llvm::StringRef) override {
    RewriterPtr.reset(new Rewriter);
    RewriterPtr->setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return std::make_unique<MyAttrConsumer>(&CI.getASTContext(), RewriterPtr.get());
  }

  ActionType getActionType() override {
    return PluginASTAction::AddBeforeMainAction;
  }

  bool ParseArgs(const CompilerInstance &CI, const std::vector<std::string> &args) override {
    // 解析插件的参数（如果有）。现在我们不做任何处理。
    return true;
  }

private:
  std::unique_ptr<Rewriter> RewriterPtr;
};

} // namespace

static FrontendPluginRegistry::Add<MyAttrAction>
X("my-attr-plugin", "Parse custom [[MyAttr]] attributes");
