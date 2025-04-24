#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include <llvm/ADT/StringRef.h>
// #include <llvm/IR/Analysis.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>

#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Type.h>
#include <llvm/Support/Alignment.h>
#include <llvm/Support/AtomicOrdering.h>
#include <unordered_set>

#include <string>

using namespace llvm;

static Instruction& get_first_non_phi(BasicBlock& BB) {
    for (auto& ins : BB) {
        if (!llvm::isa<llvm::PHINode>(ins) && !llvm::isa<llvm::LandingPadInst>(ins)) {
            return ins;
        }
    }
    return BB.front();
}

static GlobalVariable* createGlobalStringArray(Module& module, const std::vector<std::string>& stringVector) {
    LLVMContext& context = module.getContext();

    std::vector<GlobalVariable*> stringGlobals;
    IRBuilder<> builder(module.getContext());

    for (const auto& str : stringVector) {
        auto globalStr = builder.CreateGlobalString(str, "", 0, &module);
        stringGlobals.push_back(globalStr);
    }

    std::vector<Constant*> stringPointerArray;

    for (auto* global : stringGlobals) {
        stringPointerArray.push_back(ConstantExpr::getBitCast(global, PointerType::getUnqual(Type::getInt8Ty(context))));
    }

    ArrayType* arrayType = ArrayType::get(PointerType::getUnqual(Type::getInt8Ty(context)), stringPointerArray.size());
    GlobalVariable* globalArray = new GlobalVariable(
        module,                                            // Module
        arrayType,                                         // Type (Array of pointers)
        true,                                              // Is constant
        GlobalValue::InternalLinkage,                      // Linkage
        ConstantArray::get(arrayType, stringPointerArray), // Initializer
        "global_string_array"                              // Name
    );

    return globalArray;
}

namespace {
struct MyFunctionPass : PassInfoMixin<MyFunctionPass> {
    PreservedAnalyses run(Function& F, FunctionAnalysisManager& FAM) {

        if (F.hasFnAttribute("jacob_malloy_created_by_code_coverage")) {
            return PreservedAnalyses::all();
        }

        errs() << "Running on function: " << F.getName() << "\n";
        LLVMContext& context = F.getContext();
        Module* M = F.getParent();
        const DataLayout& DL = M->getDataLayout(); // Get the DataLayout
        IRBuilder<> builder(&F.getEntryBlock().front());

        int count = 0;

        int basicblockcount = F.size();
        int intsneeded = ((basicblockcount - 1) / 32) + 1;

        ArrayType* ArrayTy = ArrayType::get(Type::getInt32Ty(context), intsneeded);
        Constant* ZeroInit = ConstantAggregateZero::get(ArrayTy);

        GlobalVariable* bitfield_array = new GlobalVariable(*M, ArrayTy, false, GlobalValue::PrivateLinkage, ZeroInit);

        errs() << F.getName() << " has " << F.size() << " basic blocks\n";
        std::vector<std::string> block_strings;
        for (auto& BB : F) {
            std::unordered_set<std::string> bbstrings;
            std::vector<Value*> indices = {
                ConstantInt::get(Type::getInt32Ty(context), 0),
                ConstantInt::get(Type::getInt32Ty(context), count / 32),
            };
            IRBuilder<> BB_Builder(&get_first_non_phi(BB));
            Value* array_item = BB_Builder.CreateGEP(bitfield_array->getValueType(), bitfield_array, indices);

            auto or_value = ConstantInt::get(Type::getInt32Ty(context), 1 << (count % 32));

            BB_Builder.CreateAtomicRMW(AtomicRMWInst::Or, array_item, or_value, MaybeAlign(4), AtomicOrdering::Monotonic);

            errs() << F.getName() << " Basic Block number " << count << "\n";
            std::string bb_string = "";
            for (auto& ins : BB) {
                const DebugLoc& location = ins.getDebugLoc();
                if (location) {
                    bbstrings.insert((F.getName() + "," + std::to_string(count) + "," + location->getDirectory() + "/" + location->getFilename() + "," + std::to_string(location.getLine()) + "," + std::to_string(location.getCol()) + "\n").str());
                } else {
                }
            }
            for (auto& s : bbstrings) {
                bb_string += s;
            }
            block_strings.push_back(bb_string);
            count += 1;
        }

        auto string_array = createGlobalStringArray(*M, block_strings);

        // destructor setup
        //
        auto dtor_func_type = FunctionType::get(Type::getVoidTy(context), {}, false);
        auto dtor_func = Function::Create(dtor_func_type, Function::InternalLinkage, "", M);
        dtor_func->addFnAttr("jacob_malloy_created_by_code_coverage");

        // Create basic blocks for the function
        BasicBlock* entry = BasicBlock::Create(context, "", dtor_func);
        BasicBlock* block5 = BasicBlock::Create(context, "", dtor_func);
        BasicBlock* block7 = BasicBlock::Create(context, "", dtor_func);
        BasicBlock* block8 = BasicBlock::Create(context, "", dtor_func);
        BasicBlock* block19 = BasicBlock::Create(context, "", dtor_func);
        BasicBlock* block23 = BasicBlock::Create(context, "", dtor_func);

        auto arg0 = ConstantInt::get(IntegerType::getInt32Ty(context), basicblockcount);

        // Simulate some instructions (icmp, br, phi, etc.)
        builder.SetInsertPoint(entry);
        builder.CreateBr(block5);

        // Block 5
        builder.SetInsertPoint(block5);
        Value* zext = arg0;
        // Value* zext = builder.CreateZExt(builder.CreateNeg(arg0), Type::getInt64Ty(context), "zext");
        builder.CreateBr(block8);

        // Block 7
        builder.SetInsertPoint(block7);
        builder.CreateRetVoid();

        // Block 8 (phi node and further instructions)
        builder.SetInsertPoint(block8);
        PHINode* phi = builder.CreatePHI(Type::getInt32Ty(context), 2, "phi");
        phi->addIncoming(ConstantInt::get(Type::getInt32Ty(context), 0), block5);

        // Value* trunc = builder.CreateTrunc(phi, Type::getInt32Ty(context), "trunc");
        Value* lshr = builder.CreateLShr(phi, ConstantInt::get(Type::getInt32Ty(context), 5), "lshr");
        Value* and64 = builder.CreateAnd(lshr, ConstantInt::get(Type::getInt32Ty(context), 134217727), "and64");
        Value* gep2 = builder.CreateGEP(Type::getInt32Ty(context), bitfield_array, and64, "gep2");
        Value* load2 = builder.CreateLoad(Type::getInt32Ty(context), gep2, "load2");
        Value* and31 = builder.CreateAnd(phi, ConstantInt::get(Type::getInt32Ty(context), 31), "and31");
        Value* shl = builder.CreateShl(ConstantInt::get(Type::getInt32Ty(context), 1), and31, "shl");
        Value* mask = builder.CreateAnd(load2, shl, "mask");
        Value* icmpZero = builder.CreateICmpEQ(mask, ConstantInt::get(Type::getInt32Ty(context), 0), "icmp_zero");
        builder.CreateCondBr(icmpZero, block19, block23);

        // Block 19: call puts
        auto zero = ConstantInt::get(IntegerType::getInt32Ty(context), 0);
        builder.SetInsertPoint(block19);

        Value* gep1 = builder.CreateGEP(PointerType::get(context, 0), string_array, {phi}, "gep1");
        Value* load1 = builder.CreateLoad(PointerType::get(context, 0), gep1, "load1");

        FunctionType* putsType = FunctionType::get(Type::getInt32Ty(context), {PointerType::get(context, 0)}, false);
        FunctionCallee putsFunc = M->getOrInsertFunction("puts", putsType);
        builder.CreateCall(putsFunc, {load1});
        builder.CreateBr(block23);

        // Block 23: increment loop and condition check
        builder.SetInsertPoint(block23);
        Value* next = builder.CreateAdd(phi, ConstantInt::get(Type::getInt32Ty(context), 1), "next");
        phi->addIncoming(next, block23);
        Value* cmp = builder.CreateICmpEQ(next, zext, "cmp");
        builder.CreateCondBr(cmp, block7, block8);
        appendToGlobalDtors(*M, dtor_func, 0);
        return PreservedAnalyses::all();
    }
};
} // namespace

extern "C" ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, "MyPass", "v0.1",
        [](PassBuilder& PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager& FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "my-func-pass") {
                        FPM.addPass(MyFunctionPass());
                        return true;
                    }
                    return false;
                });
        }};
}
