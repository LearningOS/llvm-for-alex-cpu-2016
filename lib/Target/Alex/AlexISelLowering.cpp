#include "AlexTargetMachine.h"
#include "AlexISelLowering.h"
#include "AlexISelDAGToDAG.h"
#include "AlexRegisterInfo.h"
#include "AlexMachineFunction.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "Alex-lower"
#include "AlexGenCallingConv.inc"
#include "AlexTargetObjectFile.h"

static const MCPhysReg O32IntRegs[] = {
        Alex::T1, Alex::T2
};

static unsigned
addLiveIn(MachineFunction &MF, unsigned PReg, const TargetRegisterClass *RC)
{
    unsigned VReg = MF.getRegInfo().createVirtualRegister(RC);
    MF.getRegInfo().addLiveIn(PReg, VReg);
    return VReg;
}


const char *AlexTargetLowering::getTargetNodeName(unsigned Opcode) const {
    switch (Opcode) {
        case AlexISD::JmpLink:           return "AlexISD::JmpLink";
        case AlexISD::TailCall:          return "AlexISD::TailCall";
        case AlexISD::Hi:                return "AlexISD::Hi";
        case AlexISD::Lo:                return "AlexISD::Lo";
        case AlexISD::GPRel:             return "AlexISD::GPRel";
        case AlexISD::Ret:               return "AlexISD::Ret";
        case AlexISD::EH_RETURN:         return "AlexISD::EH_RETURN";
        case AlexISD::DivRem:            return "AlexISD::DivRem";
        case AlexISD::DivRemU:           return "AlexISD::DivRemU";
        case AlexISD::Wrapper:           return "AlexISD::Wrapper";
        case AlexISD::Push:              return "AlexISD::Push";
        case AlexISD::Pop:               return "AlexISD::Pop";
        case AlexISD::LI32:              return "AlexISD::LI32";
        default:                         return NULL;
    }
}

AlexTargetLowering::AlexTargetLowering(const AlexTargetMachine *targetMachine,
                                         const AlexSubtarget *subtarget,
                                       const AlexRegisterInfo* registerInfo)
        : TargetLowering(*targetMachine), subtarget(subtarget) {
    // disable dag nodes here

    setOperationAction(ISD::SELECT_CC,         MVT::i32,   Expand);
    setOperationAction(ISD::SELECT_CC,         MVT::Other, Expand);
    //setOperationAction(ISD::BR_CC,             MVT::Other, Expand);
    //setOperationAction(ISD::BR_CC,             MVT::iAny, Expand);

    //setOperationAction(ISD::SELECT,            MVT::i32,   Expand);
    setOperationAction(ISD::VASTART,           MVT::Other, Custom);
    setOperationAction(ISD::GlobalAddress,     MVT::i32,   Custom);
    setOperationAction(ISD::BlockAddress,      MVT::i32,   Custom);
    setOperationAction(ISD::JumpTable,         MVT::i32,   Custom);
    setOperationAction(ISD::JumpTable,         MVT::Other, Custom);

    // Support va_arg(): variable numbers (not fixed numbers) of arguments
    //  (parameters) for function all
    setOperationAction(ISD::VAARG,             MVT::Other, Expand);
    setOperationAction(ISD::VACOPY,            MVT::Other, Expand);
    setOperationAction(ISD::VAEND,             MVT::Other, Expand);
    setOperationAction(ISD::BR_JT,             MVT::Other, Expand);

    //@llvm.stacksave
    // Use the default for now
    setOperationAction(ISD::STACKSAVE,         MVT::Other, Expand);
    setOperationAction(ISD::STACKRESTORE,      MVT::Other, Expand);
    addRegisterClass(MVT::i32, &Alex::Int32RegsRegClass);
    computeRegisterProperties(registerInfo);
}
SDValue AlexTargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const
{
    switch (Op.getOpcode())
    {
    case ISD::VASTART:            return lowerVASTART(Op, DAG);
      //  case ISD::BRCOND:             return lowerBRCOND(Op, DAG);
    case ISD::SELECT:             return lowerSELECT(Op, DAG);
    case ISD::GlobalAddress:      return lowerGlobalAddress(Op, DAG);
    case ISD::BlockAddress:       return lowerBlockAddress(Op, DAG);
    case ISD::JumpTable:          return lowerJumpTable(Op, DAG);
    }
    return SDValue();
}

SDValue AlexTargetLowering::lowerSELECT(SDValue Op, SelectionDAG &DAG) const  {
    MachineFunction &MF = DAG.getMachineFunction();
    AlexFunctionInfo *FuncInfo = MF.getInfo<AlexFunctionInfo>();

    SDLoc DL = SDLoc(Op);
    SDValue FI = DAG.getFrameIndex(FuncInfo->getVarArgsFrameIndex(),
                                   getPointerTy(MF.getDataLayout()));

    return DAG.getConstant(0x1234, DL, MVT::i16);
    //return DAG.getStore(Op.getOperand(0), DL, FI,
    //             Op.getOperand(1), nullptr, false, false, 0);
}

SDValue AlexTargetLowering::lowerVASTART(SDValue Op, SelectionDAG &DAG) const {
    MachineFunction &MF = DAG.getMachineFunction();
    AlexFunctionInfo *FuncInfo = MF.getInfo<AlexFunctionInfo>();

    SDLoc DL = SDLoc(Op);
    SDValue FI = DAG.getFrameIndex(FuncInfo->getVarArgsFrameIndex(),
                                   getPointerTy(MF.getDataLayout()));

    // vastart just stores the address of the VarArgsFrameIndex slot into the
    // memory location argument.
    const Value *SV = cast<SrcValueSDNode>(Op.getOperand(2))->getValue();
    return DAG.getStore(Op.getOperand(0), DL, FI, Op.getOperand(1),
                        MachinePointerInfo(SV), false, false, 0);
}
void AlexTargetLowering::writeVarArgRegs(std::vector<SDValue> &OutChains,
                                         const AlexCC &CC, SDValue Chain,
                                         SDLoc DL, SelectionDAG &DAG) const {
    unsigned NumRegs = CC.numIntArgRegs();
    const ArrayRef<MCPhysReg> ArgRegs = CC.intArgRegs();
    const CCState &CCInfo = CC.getCCInfo();
    unsigned Idx = CCInfo.getFirstUnallocated(ArgRegs);
    unsigned RegSize = CC.regSize();
    MVT RegTy = MVT::getIntegerVT(RegSize * 8);
    const TargetRegisterClass *RC = getRegClassFor(RegTy);
    MachineFunction &MF = DAG.getMachineFunction();
    MachineFrameInfo *MFI = MF.getFrameInfo();
    AlexFunctionInfo *AlexFI = MF.getInfo<AlexFunctionInfo>();

    // Offset of the first variable argument from stack pointer.
    int VaArgOffset;

    if (NumRegs == Idx)
        VaArgOffset = CCInfo.getNextStackOffset(); //RoundUpToAlignment(CCInfo.getNextStackOffset(), RegSize);
    else
        VaArgOffset = (int)CC.reservedArgArea() - (int)(RegSize * (NumRegs - Idx));

    // Record the frame index of the first variable argument
    // which is a value necessary to VASTART.
    int FI = MFI->CreateFixedObject(RegSize, VaArgOffset, true);
    AlexFI->setVarArgsFrameIndex(FI);

    // Copy the integer registers that have not been used for argument passing
    // to the argument register save area. For O32, the save area is allocated
    // in the caller's stack frame, while for N32/64, it is allocated in the
    // callee's stack frame.
    for (unsigned I = Idx; I < NumRegs; ++I, VaArgOffset += RegSize) {
        unsigned Reg = addLiveIn(MF, ArgRegs[I], RC);
        SDValue ArgValue = DAG.getCopyFromReg(Chain, DL, Reg, RegTy);
        FI = MFI->CreateFixedObject(RegSize, VaArgOffset, true);
        SDValue PtrOff = DAG.getFrameIndex(FI, getPointerTy(DAG.getDataLayout()));
        SDValue Store = DAG.getStore(Chain, DL, ArgValue, PtrOff,
                                     MachinePointerInfo(), false, false, 0);
        cast<StoreSDNode>(Store.getNode())->getMemOperand()->setValue(
                (Value *)nullptr);
        OutChains.push_back(Store);
    }
}

SDValue AlexTargetLowering::getGlobalReg(SelectionDAG &DAG, EVT Ty) const {
    AlexFunctionInfo *FI = DAG.getMachineFunction().getInfo<AlexFunctionInfo>();
    return DAG.getRegister(Alex::GP, Ty);
}

//@getTargetNode(GlobalAddressSDNode
SDValue AlexTargetLowering::getTargetNode(GlobalAddressSDNode *N, EVT Ty,
                                          SelectionDAG &DAG,
                                          unsigned Flag) const {
    return DAG.getTargetGlobalAddress(N->getGlobal(), SDLoc(N), Ty, 0, Flag);
}

//@getTargetNode(ExternalSymbolSDNode
SDValue AlexTargetLowering::getTargetNode(ExternalSymbolSDNode *N, EVT Ty,
                                          SelectionDAG &DAG,
                                          unsigned Flag) const {
    return DAG.getTargetExternalSymbol(N->getSymbol(), Ty);
}
SDValue AlexTargetLowering::getTargetNode(BlockAddressSDNode *N, EVT Ty,
                                          SelectionDAG &DAG,
                                          unsigned Flag) const {
    return DAG.getTargetBlockAddress(N->getBlockAddress(), Ty, 0, Flag);
}
SDValue AlexTargetLowering::lowerBlockAddress(SDValue Op,
                                              SelectionDAG &DAG) const {
    BlockAddressSDNode *N = cast<BlockAddressSDNode>(Op);
    EVT Ty = Op.getValueType();

    return getAddrNonPIC(N, Ty, DAG);
}

template<class NodeTy>
SDValue AlexTargetLowering::getAddrGlobal(NodeTy *N, EVT Ty, SelectionDAG &DAG,
                      unsigned Flag, SDValue Chain,
                      const MachinePointerInfo &PtrInfo) const {
    SDLoc DL(N);
    return DAG.getNode(AlexISD::Wrapper, DL, Ty,
                              getTargetNode(N, Ty, DAG, Flag), Chain /*??*/);
    //return DAG.getLoad(Ty, DL, Chain, Tgt, PtrInfo, false, false, false, 0);
}
SDValue AlexTargetLowering::lowerGlobalAddress(SDValue Op,
                                               SelectionDAG &DAG) const {
    //@lowerGlobalAddress }
    SDLoc DL(Op);
    const AlexTargetObjectFile *TLOF =
            static_cast<const AlexTargetObjectFile *>(getTargetMachine().getObjFileLowering());
    //@lga 1 {
    EVT Ty = Op.getValueType();
    GlobalAddressSDNode *N = cast<GlobalAddressSDNode>(Op);
    const GlobalValue *GV = N->getGlobal();

    return getAddrNonPIC(N, Ty, DAG);
}


SDValue AlexTargetLowering::LowerFormalArguments(SDValue chain, CallingConv::ID CallConv, bool IsVarArg,
                                                 const SmallVectorImpl<ISD::InputArg> &Ins, SDLoc dl, SelectionDAG &dag,
                                                 SmallVectorImpl<SDValue> &InVals) const {
    MachineFunction &MF = dag.getMachineFunction();
    MachineFrameInfo *MFI = MF.getFrameInfo();
    AlexFunctionInfo *AlexFI = MF.getInfo<AlexFunctionInfo>();

    AlexFI->setVarArgsFrameIndex(0);

    // Assign locations to all of the incoming arguments.
    SmallVector<CCValAssign, 16> ArgLocs;
    CCState CCInfo(CallConv, IsVarArg, dag.getMachineFunction(),
                   ArgLocs, *dag.getContext());
    AlexCC AlexCCInfo(CallConv, false,
                      CCInfo);
    AlexFI->setFormalArgInfo(CCInfo.getNextStackOffset(),
                             AlexCCInfo.hasByValArg());

    Function::const_arg_iterator FuncArg =
            dag.getMachineFunction().getFunction()->arg_begin();
    bool UseSoftFloat = false;

    AlexCCInfo.analyzeFormalArguments(Ins, UseSoftFloat, FuncArg);

    // Used with vargs to acumulate store chains.
    std::vector<SDValue> OutChains;

    unsigned CurArgIdx = 0;
    AlexCC::byval_iterator ByValArg = AlexCCInfo.byval_begin();

    //@2 {
    for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
        //@2 }
        CCValAssign &VA = ArgLocs[i];
        std::advance(FuncArg, Ins[i].OrigArgIndex - CurArgIdx);
        CurArgIdx = Ins[i].OrigArgIndex;
        EVT ValVT = VA.getValVT();
        ISD::ArgFlagsTy Flags = Ins[i].Flags;
        bool IsRegLoc = VA.isRegLoc();

        //@byval pass {
        if (Flags.isByVal()) {
            assert(Flags.getByValSize() &&
                   "ByVal args of size 0 should have been ignored by front-end.");
            assert(ByValArg != AlexCCInfo.byval_end());
            copyByValRegs(chain, dl, OutChains, dag, Flags, InVals, &*FuncArg,
                          AlexCCInfo, *ByValArg);
            ++ByValArg;
            continue;
        }
        //@byval pass }
        // Arguments stored on registers
        // sanity check
        assert(VA.isMemLoc());

        // The stack pointer offset is relative to the caller stack frame.
        int FI = MFI->CreateFixedObject(ValVT.getSizeInBits()/8,
                                        VA.getLocMemOffset(), true);

        // Create load nodes to retrieve arguments from the stack
        SDValue FIN = dag.getFrameIndex(FI, getPointerTy(dag.getDataLayout()));
        SDValue Load = dag.getLoad(ValVT, dl, chain, FIN,
                                   MachinePointerInfo::getFixedStack(MF, FI, 0),
                                   false, false, false, 0);
        InVals.push_back(Load);
        OutChains.push_back(Load.getValue(1));
    }

    for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
        // The Alex ABIs for returning structs by value requires that we copy
        // the sret argument into $v0 for the return. Save the argument into
        // a virtual register so that we can access it from the return points.
        if (Ins[i].Flags.isSRet()) {
            unsigned Reg = AlexFI->getSRetReturnReg();
            if (!Reg) {
                Reg = MF.getRegInfo().createVirtualRegister(
                        getRegClassFor(MVT::i32));
                AlexFI->setSRetReturnReg(Reg);
            }
            SDValue Copy = dag.getCopyToReg(dag.getEntryNode(), dl, Reg, InVals[i]);
            chain = dag.getNode(ISD::TokenFactor, dl, MVT::Other, Copy, chain);
            break;
        }
    }

    if (IsVarArg)
        writeVarArgRegs(OutChains, AlexCCInfo, chain, dl, dag);

    // All stores are grouped in one node to allow the matching between
    // the size of Ins and InVals. This only happens when on varg functions
    if (!OutChains.empty()) {
        OutChains.push_back(chain);
        chain = dag.getNode(ISD::TokenFactor, dl, MVT::Other, OutChains);
    }
    return chain;
}
void AlexTargetLowering::AlexCC::handleByValArg(unsigned ValNo, MVT ValVT,
                                                MVT LocVT,
                                                CCValAssign::LocInfo LocInfo,
                                                ISD::ArgFlagsTy ArgFlags) {
    assert(ArgFlags.getByValSize() && "Byval argument's size shouldn't be 0.");

    struct ByValArgInfo ByVal;
    unsigned RegSize = regSize();
    unsigned ByValSize = ArgFlags.getByValSize();//RoundUpToAlignment(, RegSize);
    unsigned Align = std::min(std::max(ArgFlags.getByValAlign(), RegSize),
                              RegSize * 2);

    // Allocate space on caller's stack.
    ByVal.Address = CCInfo.AllocateStack(ByValSize - RegSize * ByVal.NumRegs,
                                         Align);
    CCInfo.addLoc(CCValAssign::getMem(ValNo, ValVT, ByVal.Address, LocVT,
                                      LocInfo));
    ByValArgs.push_back(ByVal);
}

void AlexTargetLowering::AlexCC::analyzeFormalArguments(const SmallVectorImpl<ISD::InputArg> &Args,
                       bool IsSoftFloat, Function::const_arg_iterator FuncArg) {
    unsigned NumArgs = Args.size();
    llvm::CCAssignFn *FixedFn = CC_Alex;
    unsigned CurArgIdx = 0;

    for (unsigned I = 0; I != NumArgs; ++I) {
        MVT ArgVT = Args[I].VT;
        ISD::ArgFlagsTy ArgFlags = Args[I].Flags;
        std::advance(FuncArg, Args[I].OrigArgIndex - CurArgIdx);
        CurArgIdx = Args[I].OrigArgIndex;

        if (ArgFlags.isByVal()) {
            handleByValArg(I, ArgVT, ArgVT, CCValAssign::Full, ArgFlags);
            continue;
        }

        MVT RegVT = getRegVT(ArgVT, FuncArg->getType(), nullptr, IsSoftFloat);

        if (!FixedFn(I, ArgVT, RegVT, CCValAssign::Full, ArgFlags, CCInfo))
            continue;

        llvm_unreachable(nullptr);
    }
}

template<typename Ty>
void AlexTargetLowering::AlexCC::
analyzeReturn(const SmallVectorImpl<Ty> &RetVals, bool IsSoftFloat,
              const SDNode *CallNode, const Type *RetTy) const {
    CCAssignFn *Fn;

    Fn = RetCC_Alex;

    for (unsigned I = 0, E = RetVals.size(); I < E; ++I) {
        MVT VT = RetVals[I].VT;
        ISD::ArgFlagsTy Flags = RetVals[I].Flags;
        MVT RegVT = this->getRegVT(VT, RetTy, CallNode, IsSoftFloat);

        if (Fn(I, VT, RegVT, CCValAssign::Full, Flags, this->CCInfo)) {
#ifndef NDEBUG
            dbgs() << "Call result #" << I << " has unhandled type "
            << EVT(VT).getEVTString() << '\n';
#endif
            llvm_unreachable(nullptr);
        }
    }
}
AlexTargetLowering::AlexCC::AlexCC(
        CallingConv::ID CC, bool IsO32_, CCState &Info,
        AlexCC::SpecialCallingConvType SpecialCallingConv_)
        : CCInfo(Info), CallConv(CC), IsO32(IsO32_) {
    // Pre-allocate reserved argument area.
    CCInfo.AllocateStack(reservedArgArea(), 1);
}
void AlexTargetLowering::AlexCC::
analyzeCallResult(const SmallVectorImpl<ISD::InputArg> &Ins, bool IsSoftFloat,
                  const SDNode *CallNode, const Type *RetTy) const {
    analyzeReturn(Ins, IsSoftFloat, CallNode, RetTy);
}

void AlexTargetLowering::AlexCC::
analyzeReturn(const SmallVectorImpl<ISD::OutputArg> &Outs, bool IsSoftFloat,
              const Type *RetTy) const {
    analyzeReturn(Outs, IsSoftFloat, nullptr, RetTy);
}

SDValue AlexTargetLowering::LowerReturn(SDValue chain, CallingConv::ID CallConv, bool isVarArg,
                                        const SmallVectorImpl<ISD::OutputArg> &outs,
                                        const SmallVectorImpl<SDValue> &outVals,
                                        SDLoc dl,
                                        SelectionDAG &dag) const {
    SmallVector<CCValAssign, 16> RVLocs;
    MachineFunction &MF = dag.getMachineFunction();

    // CCState - Info about the registers and stack slot.
    CCState CCInfo(CallConv, isVarArg, MF, RVLocs,
                   *dag.getContext());
    AlexCC AlexCCInfo(CallConv, true,
                        CCInfo);

    // Analyze return values.
    AlexCCInfo.analyzeReturn(outs, false,
                              MF.getFunction()->getReturnType());

    SDValue Flag;
    SmallVector<SDValue, 4> RetOps(1, chain);

    // Copy the result values into the output registers.
    for (unsigned i = 0; i != RVLocs.size(); ++i) {
        SDValue Val = outVals[i];
        CCValAssign &VA = RVLocs[i];
        assert(VA.isRegLoc() && "Can only return in registers!");

        if (RVLocs[i].getValVT() != RVLocs[i].getLocVT())
            Val = dag.getNode(ISD::BITCAST, dl, RVLocs[i].getLocVT(), Val);

        chain = dag.getCopyToReg(chain, dl, VA.getLocReg(), Val, Flag);

        // Guarantee that all emitted copies are stuck together with flags.
        Flag = chain.getValue(1);
        RetOps.push_back(dag.getRegister(VA.getLocReg(), VA.getLocVT()));
    }

//@Ordinary struct type: 2 {
    // The Alex ABIs for returning structs by value requires that we copy
    // the sret argument into $v0 for the return. We saved the argument into
    // a virtual register in the entry block, so now we copy the value out
    // and into $v0.
    if (MF.getFunction()->hasStructRetAttr()) {
        AlexFunctionInfo *AlexFI = MF.getInfo<AlexFunctionInfo>();
        unsigned Reg = AlexFI->getSRetReturnReg();

        if (!Reg)
            llvm_unreachable("sret virtual register not created in the entry block");
        SDValue Val =
                dag.getCopyFromReg(chain, dl, Reg, getPointerTy(dag.getDataLayout()));
        unsigned RETVAL = Alex::T0;

        chain = dag.getCopyToReg(chain, dl, RETVAL, Val, Flag);
        Flag = chain.getValue(1);
        RetOps.push_back(dag.getRegister(RETVAL, getPointerTy(dag.getDataLayout())));
    }
//@Ordinary struct type: 2 }

    RetOps[0] = chain;  // Update chain.

    // Add the flag if we have it.
    if (Flag.getNode())
        RetOps.push_back(Flag);

    // Return on Alex is always a "ret $lr"
    return dag.getNode(AlexISD::Ret, dl, MVT::Other, RetOps);
}

SDValue AlexTargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI, SmallVectorImpl<SDValue> &InVals) const {
    SelectionDAG &DAG                     = CLI.DAG;
    SDLoc DL                              = CLI.DL;
    SmallVectorImpl<ISD::OutputArg> &Outs = CLI.Outs;
    SmallVectorImpl<SDValue> &OutVals     = CLI.OutVals;
    SmallVectorImpl<ISD::InputArg> &Ins   = CLI.Ins;
    SDValue Chain                         = CLI.Chain;
    SDValue Callee                        = CLI.Callee;
    bool &IsTailCall                      = CLI.IsTailCall;
    CallingConv::ID CallConv              = CLI.CallConv;
    bool IsVarArg                         = CLI.IsVarArg;

    MachineFunction &MF = DAG.getMachineFunction();
    MachineFrameInfo *MFI = MF.getFrameInfo();
    const TargetFrameLowering *TFL = MF.getSubtarget().getFrameLowering();
    AlexFunctionInfo *FuncInfo = MF.getInfo<AlexFunctionInfo>();
    bool IsPIC = false;//getTargetMachine().getRelocationModel() == Reloc::PIC_;
    AlexFunctionInfo *AlexFI = MF.getInfo<AlexFunctionInfo>();

    // Analyze operands of the call, assigning locations to each operand.
    SmallVector<CCValAssign, 16> ArgLocs;
    CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(),
                   ArgLocs, *DAG.getContext());
    //AlexCC::SpecialCallingConvType SpecialCallingConv =
   //         getSpecialCallingConv(Callee);
    AlexCC AlexCCInfo(CallConv, false,
                      CCInfo, AlexCC::SpecialCallingConvType::NoSpecialCallingConv);

    AlexCCInfo.analyzeCallOperands(Outs, IsVarArg,
                                   false,
                                   Callee.getNode(), CLI.getArgs());

    // Get a count of how many bytes are to be pushed on the stack.
    unsigned NextStackOffset = CCInfo.getNextStackOffset();

    // Chain is the output chain of the last Load/Store or CopyToReg node.
    // ByValChain is the output chain of the last Memcpy node created for copying
    // byval arguments to the stack.
    //unsigned StackAlignment = TFL->getStackAlignment();
    //NextStackOffset = RoundUpToAlignment(NextStackOffset, StackAlignment);
    SDValue NextStackOffsetVal = DAG.getIntPtrConstant(NextStackOffset, DL, true);

    //@TailCall 2 {
    //if (!IsTailCall)
    Chain = DAG.getCALLSEQ_START(Chain, NextStackOffsetVal, DL);
    //@TailCall 2 }

    SDValue StackPtr =
            DAG.getCopyFromReg(Chain, DL, Alex::SP,
                               getPointerTy(DAG.getDataLayout()));

    // With EABI is it possible to have 16 args on registers.
    std::deque< std::pair<unsigned, SDValue> > RegsToPass;
    SmallVector<SDValue, 8> MemOpChains;
    AlexCC::byval_iterator ByValArg = AlexCCInfo.byval_begin();

    //@1 {
    // Walk the register/memloc assignments, inserting copies/loads.
    for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
        //@1 }
        SDValue Arg = OutVals[i];
        CCValAssign &VA = ArgLocs[i];
        MVT LocVT = VA.getLocVT();
        ISD::ArgFlagsTy Flags = Outs[i].Flags;

        //@ByVal Arg {
        /*if (Flags.isByVal()) {
            assert(Flags.getByValSize() &&
                   "ByVal args of size 0 should have been ignored by front-end.");
            assert(ByValArg != AlexCCInfo.byval_end());
            assert(!IsTailCall &&
                   "Do not tail-call optimize if there is a byval argument.");
            passByValArg(Chain, DL, RegsToPass, MemOpChains, StackPtr, MFI, DAG, Arg,
                         AlexCCInfo, *ByValArg, Flags, Subtarget.isLittle());
            ++ByValArg;
            continue;
        }*/
        //@ByVal Arg }

        // Promote the value if needed.
        switch (VA.getLocInfo()) {
            default: llvm_unreachable("Unknown loc info!");
            case CCValAssign::Full:
                break;
            case CCValAssign::SExt:
                Arg = DAG.getNode(ISD::SIGN_EXTEND, DL, LocVT, Arg);
                break;
            case CCValAssign::ZExt:
                Arg = DAG.getNode(ISD::ZERO_EXTEND, DL, LocVT, Arg);
                break;
            case CCValAssign::AExt:
                Arg = DAG.getNode(ISD::ANY_EXTEND, DL, LocVT, Arg);
                break;
        }

        // Arguments that can be passed on register must be kept at
        // RegsToPass vector
        //if (VA.isRegLoc()) {
        //    RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
        //    continue;
        //}

        // Register can't get to this point...
        assert(VA.isMemLoc());

        // emit ISD::STORE whichs stores the
        // parameter value to a stack Location
        MemOpChains.push_back(passArgOnStack(StackPtr, VA.getLocMemOffset(),
                                             Chain, Arg, DL, IsTailCall, DAG));
    }

    // Transform all store nodes into one single node because all store
    // nodes are independent of each other.
    if (!MemOpChains.empty())
        Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, MemOpChains);

    // If the callee is a GlobalAddress/ExternalSymbol node (quite common, every
    // direct call is) turn it into a TargetGlobalAddress/TargetExternalSymbol
    // node so that legalize doesn't hack it.
    bool IsPICCall = false;//IsPIC; // true if calls are translated to
    // jalr $t9
    bool GlobalOrExternal = false, InternalLinkage = false;
    SDValue CalleeLo;
    EVT Ty = Callee.getValueType();

    if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee)) {
        //getAddrNonPIC()

        Callee = getAddrNonPIC(G, Ty, DAG);/*DAG.getTargetGlobalAddress(G->getGlobal(),
                                            DL,
                                            getPointerTy(DAG.getDataLayout()),
                                            0,
                                            AlexII::MO_NO_FLAG);*/

        GlobalOrExternal = true;
    }
    else if (ExternalSymbolSDNode *S = dyn_cast<ExternalSymbolSDNode>(Callee)) {
        const char *Sym = S->getSymbol();
        Callee = DAG.getTargetExternalSymbol(Sym,
                                                 getPointerTy(DAG.getDataLayout()),
                                                 0);
        GlobalOrExternal = true;
    }

    SmallVector<SDValue, 8> Ops(1, Chain);
    SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);

    getOpndList(Ops, RegsToPass, IsPICCall, GlobalOrExternal, InternalLinkage,
                CLI, Callee, Chain);

    //@TailCall 3 {
    //if (IsTailCall)
    //    return DAG.getNode(AlexISD::TailCall, DL, MVT::Other, Ops);
    //@TailCall 3 }

    Chain = DAG.getNode(AlexISD::JmpLink, DL, NodeTys, Ops);
    SDValue InFlag = Chain.getValue(1);

    // Create the CALLSEQ_END node.
    Chain = DAG.getCALLSEQ_END(Chain, NextStackOffsetVal,
                               DAG.getIntPtrConstant(0, DL, true), InFlag, DL);
    InFlag = Chain.getValue(1);

    // Handle result values, copying them out of physregs into vregs that we
    // return.
    return LowerCallResult(Chain, InFlag, CallConv, IsVarArg,
                           Ins, DL, DAG, InVals, CLI.Callee.getNode(), CLI.RetTy);
}

void AlexTargetLowering::AlexCC::
analyzeCallOperands(const SmallVectorImpl<ISD::OutputArg> &Args,
                    bool IsVarArg, bool IsSoftFloat, const SDNode *CallNode,
                    std::vector<ArgListEntry> &FuncArgs) {
//@analyzeCallOperands body {
    assert((CallConv != CallingConv::Fast || !IsVarArg) &&
           "CallingConv::Fast shouldn't be used for vararg functions.");

    unsigned NumOpnds = Args.size();
    llvm::CCAssignFn *FixedFn = CC_Alex;

    //@3 {
    for (unsigned I = 0; I != NumOpnds; ++I) {
        //@3 }
        MVT ArgVT = Args[I].VT;
        ISD::ArgFlagsTy ArgFlags = Args[I].Flags;
        bool R;

        if (ArgFlags.isByVal()) {
            handleByValArg(I, ArgVT, ArgVT, CCValAssign::Full, ArgFlags);
            continue;
        }

        {
            MVT RegVT = getRegVT(ArgVT, FuncArgs[Args[I].OrigArgIndex].Ty, CallNode,
                                 IsSoftFloat);
            R = FixedFn(I, ArgVT, RegVT, CCValAssign::Full, ArgFlags, CCInfo);
        }

        if (R) {
#ifndef NDEBUG
            dbgs() << "Call operand #" << I << " has unhandled type "
            << EVT(ArgVT).getEVTString();
#endif
            llvm_unreachable(nullptr);
        }
    }
}

SDValue
AlexTargetLowering::passArgOnStack(SDValue StackPtr, unsigned Offset,
                                   SDValue Chain, SDValue Arg, SDLoc DL,
                                   bool IsTailCall, SelectionDAG &DAG) const {
    if (!IsTailCall) {
        SDValue PtrOff = DAG.getNode(ISD::ADD,
                                     DL,
                                     getPointerTy(DAG.getDataLayout()),
                                     StackPtr,
                                     DAG.getIntPtrConstant(Offset, DL));
        return DAG.getStore(Chain, DL, Arg, PtrOff, MachinePointerInfo(), false,
                            false, 0);
    }

    MachineFrameInfo *MFI = DAG.getMachineFunction().getFrameInfo();
    int FI = MFI->CreateFixedObject(Arg.getValueSizeInBits() / 8, Offset, false);
    SDValue FIN = DAG.getFrameIndex(FI, getPointerTy(DAG.getDataLayout()));
    return DAG.getStore(Chain, DL, Arg, FIN, MachinePointerInfo(),
            /*isVolatile=*/ true, false, 0);
}


void AlexTargetLowering::
getOpndList(SmallVectorImpl<SDValue> &Ops,
            std::deque< std::pair<unsigned, SDValue> > &RegsToPass,
            bool IsPICCall, bool GlobalOrExternal, bool InternalLinkage,
            CallLoweringInfo &CLI, SDValue Callee, SDValue Chain) const {
    // T9 should contain the address of the callee function if
    // -reloction-model=pic or it is an indirect call.
    //if (IsPICCall || !GlobalOrExternal) {
    //    unsigned T9Reg = Alex::T9;
    //    RegsToPass.push_front(std::make_pair(T9Reg, Callee));
    //} else
        Ops.push_back(Callee);

    // Insert node "GP copy globalreg" before call to function.
    //
    // R_Alex_CALL* operators (emitted when non-internal functions are called
    // in PIC mode) allow symbols to be resolved via lazy binding.
    // The lazy binding stub requires GP to point to the GOT.
    //if (IsPICCall && !InternalLinkage) {
    //    unsigned GPReg = Alex::GP;
    //    EVT Ty = MVT::i32;
    //    RegsToPass.push_back(std::make_pair(GPReg, getGlobalReg(CLI.DAG, Ty)));
    //}

    // Build a sequence of copy-to-reg nodes chained together with token
    // chain and flag operands which copy the outgoing args into registers.
    // The InFlag in necessary since all emitted instructions must be
    // stuck together.
    SDValue InFlag;

    for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i) {
        Chain = CLI.DAG.getCopyToReg(Chain, CLI.DL, RegsToPass[i].first,
                                     RegsToPass[i].second, InFlag);
        InFlag = Chain.getValue(1);
    }

    // Add argument registers to the end of the list so that they are
    // known live into the call.
    for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i)
        Ops.push_back(CLI.DAG.getRegister(RegsToPass[i].first,
                                          RegsToPass[i].second.getValueType()));

    // Add a register mask operand representing the call-preserved registers.
    const TargetRegisterInfo *TRI = subtarget->getRegisterInfo();
    const uint32_t *Mask =
            TRI->getCallPreservedMask(CLI.DAG.getMachineFunction(), CLI.CallConv);
    assert(Mask && "Missing call preserved mask for calling convention");
    Ops.push_back(CLI.DAG.getRegisterMask(Mask));

    if (InFlag.getNode())
        Ops.push_back(InFlag);
}
SDValue
AlexTargetLowering::LowerCallResult(SDValue Chain, SDValue InFlag,
                                    CallingConv::ID CallConv, bool IsVarArg,
                                    const SmallVectorImpl<ISD::InputArg> &Ins,
                                    SDLoc DL, SelectionDAG &DAG,
                                    SmallVectorImpl<SDValue> &InVals,
                                    const SDNode *CallNode,
                                    const Type *RetTy) const {
    // Assign locations to each value returned by this call.
    SmallVector<CCValAssign, 16> RVLocs;
    CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(),
                   RVLocs, *DAG.getContext());

    AlexCC AlexCCInfo(CallConv, false, CCInfo);

    AlexCCInfo.analyzeCallResult(Ins, false,
                                 CallNode, RetTy);

    // Copy all of the result registers out of their specified physreg.
    for (unsigned i = 0; i != RVLocs.size(); ++i) {
        SDValue Val = DAG.getCopyFromReg(Chain, DL, RVLocs[i].getLocReg(),
                                         RVLocs[i].getLocVT(), InFlag);
        Chain = Val.getValue(1);
        InFlag = Val.getValue(2);

        if (RVLocs[i].getValVT() != RVLocs[i].getLocVT())
            Val = DAG.getNode(ISD::BITCAST, DL, RVLocs[i].getValVT(), Val);

        InVals.push_back(Val);
    }

    return Chain;
}

SDValue AlexTargetLowering::
lowerJumpTable(SDValue Op, SelectionDAG &DAG) const
{
    JumpTableSDNode *N = cast<JumpTableSDNode>(Op);
    EVT Ty = Op.getValueType();
    return getAddrNonPIC(N, Ty, DAG);
}
SDValue AlexTargetLowering::getTargetNode(JumpTableSDNode *N, EVT Ty,
                                          SelectionDAG &DAG,
                                          unsigned Flag) const {
    return DAG.getTargetJumpTable(N->getIndex(), Ty, Flag);
}

void AlexTargetLowering::copyByValRegs(SDValue Chain, SDLoc DL, std::vector<SDValue> &OutChains, SelectionDAG &DAG,
                                       const ISD::ArgFlagsTy &Flags, SmallVectorImpl<SDValue> &InVals,
                                       const Argument *FuncArg, const AlexCC &CC, const ByValArgInfo &ByVal) const {
    MachineFunction &MF = DAG.getMachineFunction();
    MachineFrameInfo *MFI = MF.getFrameInfo();
    unsigned RegAreaSize = ByVal.NumRegs * CC.regSize();
    unsigned FrameObjSize = std::max(Flags.getByValSize(), RegAreaSize);
    int FrameObjOffset;

    const ArrayRef<MCPhysReg> ByValArgRegs = CC.intArgRegs();

    if (RegAreaSize)
        FrameObjOffset = (int)CC.reservedArgArea() -
                         (int)((CC.numIntArgRegs() - ByVal.FirstIdx) * CC.regSize());
    else
        FrameObjOffset = ByVal.Address;

    // Create frame object.
    EVT PtrTy = getPointerTy(DAG.getDataLayout());
    int FI = MFI->CreateFixedObject(FrameObjSize, FrameObjOffset, true);
    SDValue FIN = DAG.getFrameIndex(FI, PtrTy);
    InVals.push_back(FIN);

    if (!ByVal.NumRegs)
        return;

    // Copy arg registers.
    MVT RegTy = MVT::getIntegerVT(CC.regSize() * 8);
    const TargetRegisterClass *RC = getRegClassFor(RegTy);

    for (unsigned I = 0; I < ByVal.NumRegs; ++I) {
        unsigned ArgReg = ByValArgRegs[ByVal.FirstIdx + I];
        unsigned VReg = addLiveIn(MF, ArgReg, RC);
        unsigned Offset = I * CC.regSize();
        SDValue StorePtr = DAG.getNode(ISD::ADD, DL, PtrTy, FIN,
                                       DAG.getConstant(Offset, DL, PtrTy));
        SDValue Store = DAG.getStore(Chain, DL, DAG.getRegister(VReg, RegTy),
                                     StorePtr, MachinePointerInfo(FuncArg, Offset),
                                     false, false, 0);
        OutChains.push_back(Store);
    }
}


const ArrayRef<MCPhysReg> AlexTargetLowering::AlexCC::intArgRegs() const {
    return makeArrayRef(O32IntRegs);
}