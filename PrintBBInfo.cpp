#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"


using namespace llvm;

struct PrintBBInfoPass : public PassInfoMixin<PrintBBInfoPass> {
    PreservedAnalyses run(Function& F, FunctionAnalysisManager& FAM) {
        errs() << "Function: " << F.getName() << "\n";

        int bbIndex = 0;
        for (auto& BB : F) {
            errs() << "  BasicBlock #" << bbIndex++ << ":\n";

            for (auto& I : BB) {
                if (const DebugLoc& DL = I.getDebugLoc()) {
                    unsigned Line = DL.getLine();
                    unsigned Col = DL.getCol();
                    StringRef File = DL->getFilename();
                    errs() << "    " << I.getOpcodeName() << " at " << File
                           << ":" << Line << ":" << Col << "\n";
                } else {
                    errs() << "    " << I.getOpcodeName() << " (no debug info)\n";
                }
            }
        }

        return PreservedAnalyses::all();
    }
};

// Register the pass with LLVM's pass manager
llvm::PassPluginLibraryInfo getPrintBBInfoPassPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "PrintBBInfoPass", LLVM_VERSION_STRING,
            [](PassBuilder& PB) {
                PB.registerPipelineParsingCallback(
                    [](StringRef Name, FunctionPassManager& FPM,
                       ArrayRef<PassBuilder::PipelineElement>) {
                        if (Name == "print-bb-info") {
                            FPM.addPass(PrintBBInfoPass());
                            return true;
                        }
                        return false;
                    });
            }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return getPrintBBInfoPassPluginInfo();
}
