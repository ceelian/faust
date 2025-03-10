/************************************************************************
 ************************************************************************
    FAUST compiler
    Copyright (C) 2017 GRAME, Centre National de Creation Musicale
    ---------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 ************************************************************************
 ************************************************************************/

#ifndef _RUST_INSTRUCTIONS_H
#define _RUST_INSTRUCTIONS_H

#include <regex>

#include "text_instructions.hh"
#include "Text.hh"

using namespace std;

inline string makeNameSingular(const string& name)
{
    string result = name;
    result = std::regex_replace(result, std::regex("inputs"), "input");
    result = std::regex_replace(result, std::regex("outputs"), "output");
    return result;
}

// Visitor used to initialize fields into the DSP constructor
struct RustInitFieldsVisitor : public DispatchVisitor {
    std::ostream* fOut;
    int           fTab;

    RustInitFieldsVisitor(std::ostream* out, int tab = 0) : fOut(out), fTab(tab) {}

    virtual void visit(DeclareVarInst* inst)
    {
        tab(fTab, *fOut);
        *fOut << inst->fAddress->getName() << ": ";
        ZeroInitializer(fOut, inst->fType);
        if (inst->fAddress->getAccess() & Address::kStruct) *fOut << ",";
    }

    // Generate zero intialisation code for simple int/real scalar and arrays types
    static void ZeroInitializer(std::ostream* fOut, Typed* typed)
    {
        Typed::VarType type       = typed->getType();
        ArrayTyped*    array_type = dynamic_cast<ArrayTyped*>(typed);

        if (array_type) {
            if (isIntPtrType(type)) {
                *fOut << "[0;" << array_type->fSize << "]";
            } else if (isRealPtrType(type)) {
                *fOut << "[0.0;" << array_type->fSize << "]";
            }
        } else {
            if (isIntType(type)) {
                *fOut << "0";
            } else if (isRealType(type)) {
                *fOut << "0.0";
            }
        }
    }
};

class RustInstVisitor : public TextInstVisitor {
   private:
    /*
     Global functions names table as a static variable in the visitor
     so that each function prototype is generated as most once in the module.
     */
    static map<string, bool> gFunctionSymbolTable;
    map<string, string>      fMathLibTable;

   public:
    using TextInstVisitor::visit;

    RustInstVisitor(std::ostream* out, const string& structname, int tab = 0)
        : TextInstVisitor(out, ".", new RustStringTypeManager(xfloat(), "&"), tab)
    {
        fTypeManager->fTypeDirectTable[Typed::kObj]     = "";
        fTypeManager->fTypeDirectTable[Typed::kObj_ptr] = "";

        // Integer version
        fMathLibTable["abs"]   = "i32::abs";
        fMathLibTable["min_i"] = "std::cmp::min";
        fMathLibTable["max_i"] = "std::cmp::max";

        // Float version
        fMathLibTable["fabsf"]      = "F32::abs";
        fMathLibTable["acosf"]      = "F32::acos";
        fMathLibTable["asinf"]      = "F32::asin";
        fMathLibTable["atanf"]      = "F32::atan";
        fMathLibTable["atan2f"]     = "F32::atan2";
        fMathLibTable["ceilf"]      = "F32::ceil";
        fMathLibTable["cosf"]       = "F32::cos";
        fMathLibTable["expf"]       = "F32::exp";
        fMathLibTable["floorf"]     = "F32::floor";
        fMathLibTable["fmodf"]      = "libm::fmodf";
        fMathLibTable["logf"]       = "F32::log";
        fMathLibTable["log10f"]     = "F32::log10";
        fMathLibTable["max_f"]      = "F32::max";
        fMathLibTable["min_f"]      = "F32::min";
        fMathLibTable["powf"]       = "F32::powf";
        fMathLibTable["remainderf"] = "F32::rem_euclid";
        //fMathLibTable["rintf"]      = "linux_api_math::rintf"; // TODO
        fMathLibTable["rintf"]      = "F32::round";
        fMathLibTable["roundf"]     = "F32::round";
        fMathLibTable["sinf"]       = "F32::sin";
        fMathLibTable["sqrtf"]      = "F32::sqrt";
        fMathLibTable["tanf"]       = "F32::tan";

        // Additional hyperbolic math functions
        fMathLibTable["acoshf"]     = "F32::acosh";
        fMathLibTable["asinhf"]     = "F32::asinh";
        fMathLibTable["atanhf"]     = "F32::atanh";
        fMathLibTable["coshf"]      = "F32::cosh";
        fMathLibTable["sinhf"]      = "F32::sinh";
        fMathLibTable["tanhf"]      = "F32::tanh";
    
        fMathLibTable["isnanf"]     = "F32::is_nan";
        fMathLibTable["isinff"]     = "F32::is_infinite";
        fMathLibTable["copysignf"]  = "F32::copysign";

        // Double version
        fMathLibTable["fabs"]      = "F64::abs";
        fMathLibTable["acos"]      = "F64::acos";
        fMathLibTable["asin"]      = "F64::asin";
        fMathLibTable["atan"]      = "F64::atan";
        fMathLibTable["atan2"]     = "F64::atan2";
        fMathLibTable["ceil"]      = "F64::ceil";
        fMathLibTable["cos"]       = "F64::cos";
        fMathLibTable["exp"]       = "F64::exp";
        fMathLibTable["floor"]     = "F64::floor";
        fMathLibTable["fmod"]      = "libm::fmod";
        fMathLibTable["log"]       = "F64::log";
        fMathLibTable["log10"]     = "F64::log10";
        fMathLibTable["max_"]      = "F64::max";
        fMathLibTable["min_"]      = "F64::min";
        fMathLibTable["pow"]       = "F64::powf";
        fMathLibTable["remainder"] = "F64::rem_euclid";
        //fMathLibTable["rint"]      = "linux_api_math::rint";  // TODO
        fMathLibTable["rint"]      = "F64::round";
        fMathLibTable["round"]     = "F64::round";
        fMathLibTable["sin"]       = "F64::sin";
        fMathLibTable["sqrt"]      = "F64::sqrt";
        fMathLibTable["tan"]       = "F64::tan";

        // Additional hyperbolic math functions
        fMathLibTable["acosh"]     = "F64::acosh";
        fMathLibTable["asinh"]     = "F64::asinh";
        fMathLibTable["atanh"]     = "F64::atanh";
        fMathLibTable["cosh"]      = "F64::cosh";
        fMathLibTable["sinh"]      = "F64::sinh";
        fMathLibTable["tanh"]      = "F64::tanh";
    
        fMathLibTable["isnan"]     = "F64::is_nan";
        fMathLibTable["isinf"]     = "F64::is_infinite";
        fMathLibTable["copysign"]  = "F64::copysign";
    }

    virtual ~RustInstVisitor() {}

    virtual void visit(DeclareVarInst* inst)
    {
        if (inst->fAddress->getAccess() & Address::kStaticStruct) {
            *fOut << "static mut ";
        }

        if (inst->fAddress->getAccess() & Address::kStack || inst->fAddress->getAccess() & Address::kLoop) {
            *fOut << "let mut ";
        }

        // If type is kNoType, only generate the name, otherwise a typed expression
        if (inst->fType->getType() == Typed::VarType::kNoType) {
            *fOut << inst->fAddress->getName();
        } else {
            *fOut << fTypeManager->generateType(inst->fType, inst->fAddress->getName());
        }

        if (inst->fValue) {
            *fOut << " = ";
            inst->fValue->accept(this);
        } else if (inst->fAddress->getAccess() & Address::kStaticStruct) {
            *fOut << " = ";
            RustInitFieldsVisitor::ZeroInitializer(fOut, inst->fType);
        }

        EndLine((inst->fAddress->getAccess() & Address::kStruct) ? ',' : ';');
    }

    virtual void visit(DeclareBufferIterators* inst)
    {
        /* Generates an expression like:
        let (outputs0, outputs1) = if let [outputs0, outputs1, ..] = outputs {
            let outputs0 = outputs0[..count as usize].iter_mut();
            let outputs1 = outputs1[..count as usize].iter_mut();
            (outputs0, outputs1)
        } else {
            panic!("wrong number of outputs");
        };
        */
        
        // Don't generate if no channels
        if (inst->fNumChannels == 0) return;
        
        std::string name = inst->fBufferName2;

        // Build pattern matching + if let line
        *fOut << "let (";
        for (int i = 0; i < inst->fNumChannels; ++i) {
            if (i > 0) {
                *fOut << ", ";
            }
            *fOut << name << i;
        }
        *fOut << ") = if let [";
        for (int i = 0; i < inst->fNumChannels; ++i) {
            *fOut << name << i << ", ";
        }
        *fOut << "..] = " << name << " {";

        // Build fixed size iterator variables
        fTab++;
        for (int i = 0; i < inst->fNumChannels; ++i) {
            tab(fTab, *fOut);
            *fOut << "let " << name << i << " = " << name << i << "[..count as usize]";
            if (inst->fMutable) {
                *fOut << ".iter_mut();";
            } else {
                *fOut << ".iter();";
            }
        }

        // Build return tuple
        tab(fTab, *fOut);
        *fOut << "(";
        for (int i = 0; i < inst->fNumChannels; ++i) {
            if (i > 0) {
                *fOut << ", ";
            }
            *fOut << name << i;
        }
        *fOut << ")";

        // Build else branch
        fTab--;
        tab(fTab, *fOut);
        *fOut << "} else {";

        fTab++;
        tab(fTab, *fOut);
        *fOut << "panic!(\"wrong number of " << name << "\");";

        fTab--;
        tab(fTab, *fOut);
        *fOut << "};";
        tab(fTab, *fOut);
    }

    virtual void visit(DeclareFunInst* inst)
    {
        // Already generated
        if (gFunctionSymbolTable.find(inst->fName) != gFunctionSymbolTable.end()) {
            return;
        } else {
            gFunctionSymbolTable[inst->fName] = true;
        }

        // Only generates additional functions
        if (fMathLibTable.find(inst->fName) == fMathLibTable.end()) {
            // Prototype
            // Since functions are attached to a trait they must not be prefixed with "pub".
            // In case we need a mechanism to attach functions to both traits and normal
            // impls, we need a mechanism to forward the information whether to use "pub"
            // or not. In the worst case, we have to prefix the name string like "pub fname",
            // and handle the prefix here.
            *fOut << "fn " << inst->fName;
            generateFunDefArgs(inst);
            generateFunDefBody(inst);
        }
    }

    virtual void generateFunDefBody(DeclareFunInst* inst)
    {
        *fOut << ") -> " << fTypeManager->generateType(inst->fType->fResult);
        if (inst->fCode->fCode.size() == 0) {
            *fOut << ";" << endl;  // Pure prototype
        } else {
            // Function body
            *fOut << " {";
            fTab++;
            tab(fTab, *fOut);
            inst->fCode->accept(this);
            fTab--;
            back(1, *fOut);
            *fOut << "}";
            tab(fTab, *fOut);
        }
    }

    virtual void visit(RetInst* inst)
    {
        if (inst->fResult) {
            inst->fResult->accept(this);
        } else {
            *fOut << "return";
            EndLine();
        }
    }

    virtual void visit(NamedAddress* named)
    {
        if (named->getAccess() & Address::kStruct) {
            if (named->getAccess() & Address::kReference && named->getAccess() & Address::kMutable) {
                *fOut << "&mut self.";
            } else {
                *fOut << "self.";
            }
        } else if (named->getAccess() & Address::kStaticStruct) {
            if (named->getAccess() & Address::kReference && named->getAccess() & Address::kMutable) {
                *fOut << "&mut ";
            }
        }
        *fOut << named->getName();
    }

    virtual void visit(IndexedAddress* indexed)
    {
        indexed->fAddress->accept(this);
        if (dynamic_cast<Int32NumInst*>(indexed->fIndex)) {
            *fOut << "[";
            indexed->fIndex->accept(this);
            *fOut << "]";
        } else {
            // Array index expression casted to 'usize' type
            *fOut << "[(";
            indexed->fIndex->accept(this);
            *fOut << ") as usize]";
        }
    }

    virtual void visit(LoadVarInst* inst)
    {
        if (inst->fAddress->getAccess() & Address::kStaticStruct) {
            *fOut << "unsafe { ";
        }
        inst->fAddress->accept(this);
        if (inst->fAddress->getAccess() & Address::kStaticStruct) {
            *fOut << " }";
        }
    }

    virtual void visit(LoadVarAddressInst* inst)
    {
        *fOut << "&";
        inst->fAddress->accept(this);
    }

    virtual void visit(FloatArrayNumInst* inst)
    {
        char sep = '[';
        for (size_t i = 0; i < inst->fNumTable.size(); i++) {
            *fOut << sep << checkFloat(inst->fNumTable[i]);
            sep = ',';
        }
        *fOut << ']';
    }

    virtual void visit(Int32ArrayNumInst* inst)
    {
        char sep = '[';
        for (size_t i = 0; i < inst->fNumTable.size(); i++) {
            *fOut << sep << inst->fNumTable[i];
            sep = ',';
        }
        *fOut << ']';
    }
  
    virtual void visit(DoubleArrayNumInst* inst)
    {
        char sep = '[';
        for (size_t i = 0; i < inst->fNumTable.size(); i++) {
            *fOut << sep << checkDouble(inst->fNumTable[i]);
            sep = ',';
        }
        *fOut << ']';
    }
    
    virtual void visit(BinopInst* inst)
    {
        // Special case for 'logical right-shift'
        if (strcmp(gBinOpTable[inst->fOpcode]->fName, ">>>") == 0) {
            TypingVisitor typing;
            inst->fInst1->accept(&typing);
            *fOut << "(((";
            inst->fInst1->accept(this);
            if (isInt64Type(typing.fCurType)) {
                *fOut << " as u64)";
            } else if (isInt32Type(typing.fCurType)) {
                *fOut << " as u32)";
            } else {
                faustassert(false);
            }
            *fOut << " >> ";
            inst->fInst2->accept(this);
            *fOut << ")";
            if (isInt64Type(typing.fCurType)) {
                *fOut << " as i64)";
            } else if (isInt32Type(typing.fCurType)) {
                *fOut << " as i32)";
            } else {
                faustassert(false);
            }
        } else {
            TextInstVisitor::visit(inst);
        }
    }

    virtual void visit(::CastInst* inst)
    {
        *fOut << "((";
        inst->fInst->accept(this);
        *fOut << ") as " << fTypeManager->generateType(inst->fType);
        *fOut << ")";
    }

    virtual void visit(BitcastInst* inst) { faustassert(false); }

    virtual void visit(FunCallInst* inst)
    {
        if (fMathLibTable.find(inst->fName) != fMathLibTable.end()) {
            generateFunCall(inst, fMathLibTable[inst->fName]);
        } else {
            generateFunCall(inst, inst->fName);
        }
    }

    virtual void generateFunCall(FunCallInst* inst, const std::string& fun_name)
    {
        if (inst->fMethod) {
            ListValuesIt it = inst->fArgs.begin();
            // Compile object arg
            (*it)->accept(this);
            // Compile parameters
            *fOut << fObjectAccess;
            // Hack for 1 FIR generated names
            if (startWith(fun_name, "instanceInit")) {
                *fOut << "instance_init" << fun_name.substr(12) << "(";
            } else {
                *fOut << fun_name << "(";
            }
            generateFunCallArgs(++it, inst->fArgs.end(), int(inst->fArgs.size()) - 1);
        } else {
            *fOut << fun_name << "(";
            // Compile parameters
            generateFunCallArgs(inst->fArgs.begin(), inst->fArgs.end(), int(inst->fArgs.size()));
            // Hack for 'log' function that needs a base
            if (fun_name == "F32::log") {
                *fOut << ", std::f32::consts::E";
            } else if (fun_name == "F64::log") {
                *fOut << ", std::f64::consts::E";
            }
        }
        *fOut << ")";
    }

    virtual void visit(Select2Inst* inst)
    {
        *fOut << "if (";
        inst->fCond->accept(this);
        // Force 'cond' to bool type
        *fOut << " as i32 != 0) { ";
        inst->fThen->accept(this);
        *fOut << " } else { ";
        inst->fElse->accept(this);
        *fOut << " }";
    }

    virtual void visit(IfInst* inst)
    {
        *fOut << "if (";
        inst->fCond->accept(this);
        // Force 'cond' to bool type
        *fOut << " as i32 == 1) { ";
        fTab++;
        tab(fTab, *fOut);
        inst->fThen->accept(this);
        fTab--;
        back(1, *fOut);
        if (inst->fElse->fCode.size() > 0) {
            *fOut << "} else {";
            fTab++;
            tab(fTab, *fOut);
            inst->fElse->accept(this);
            fTab--;
            back(1, *fOut);
            *fOut << "}";
        } else {
            *fOut << "}";
        }
        tab(fTab, *fOut);
    }

    virtual void visit(ForLoopInst* inst)
    {
        // Don't generate empty loops...
        if (inst->fCode->size() == 0) return;

        inst->fInit->accept(this);
        *fOut << "loop {";
        fTab++;
        tab(fTab, *fOut);
        inst->fCode->accept(this);
        inst->fIncrement->accept(this);
        *fOut << "if ";
        inst->fEnd->accept(this);
        *fOut << " { continue; } else { break; }";
        fTab--;
        tab(fTab, *fOut);
        *fOut << "}";
        tab(fTab, *fOut);
    }

    virtual void visit(SimpleForLoopInst* inst)
    {
        // Don't generate empty loops...
        if (inst->fCode->size() == 0) return;

        *fOut << "for " << inst->getName() << " in ";
        if (inst->fReverse) {
            *fOut << "(";
            inst->fLowerBound->accept(this);
            *fOut << "..=";
            inst->fUpperBound->accept(this);
            *fOut << ").rev()";
        } else {
            inst->fLowerBound->accept(this);
            *fOut << "..";
            inst->fUpperBound->accept(this);
        }
        *fOut << " {";
        fTab++;
        tab(fTab, *fOut);
        inst->fCode->accept(this);
        fTab--;
        back(1, *fOut);
        *fOut << "}";
        tab(fTab, *fOut);
    }

    virtual void visit(IteratorForLoopInst* inst)
    {
        // Don't generate empty loops...
        if (inst->fCode->size() == 0) return;

        *fOut << "let zipped_iterators = ";
        for (std::size_t i = 0; i < inst->fIterators.size(); ++i) {
            if (i == 0) {
                inst->fIterators[i]->accept(this);
            } else {
                *fOut << ".zip(";
                inst->fIterators[i]->accept(this);
                *fOut << ")";
            }
        }
        *fOut << ";";
        tab(fTab, *fOut);

        *fOut << "for ";
        for (std::size_t i = 0; i < inst->fIterators.size() - 1; ++i) {
            *fOut << "(";
        }
        *fOut << makeNameSingular(inst->fIterators[0]->getName());
        for (std::size_t i = 1; i < inst->fIterators.size(); ++i) {
            *fOut << ", " << makeNameSingular(inst->fIterators[i]->getName()) << ")";
        }
        *fOut << " in zipped_iterators {";
        fTab++;
        tab(fTab, *fOut);
        inst->fCode->accept(this);
        fTab--;
        back(1, *fOut);
        *fOut << "}";
        tab(fTab, *fOut);
    }

    virtual void visit(::SwitchInst* inst)
    {
        *fOut << "match (";
        inst->fCond->accept(this);
        *fOut << ") {";
        fTab++;
        tab(fTab, *fOut);
        for (const auto& it : inst->fCode) {
            if (it.first == -1) {  // -1 used to code "default" case
                *fOut << "_ => {";
            } else {
                *fOut << it.first << " => {";
            }
            fTab++;
            tab(fTab, *fOut);
            (it.second)->accept(this);
            fTab--;
            back(1, *fOut);
            *fOut << "},";
            tab(fTab, *fOut);
        }
        fTab--;
        back(1, *fOut);
        *fOut << "} ";
        tab(fTab, *fOut);
    }

    static void cleanup() { gFunctionSymbolTable.clear(); }
};

/**
 * Helper visitor that allows to build a parameter lookup table.
 */
class UserInterfaceParameterMapping : public InstVisitor {
   private:
    map<string, int>      fParameterLookup;
    int                   fParameterIndex;

   public:
    using InstVisitor::visit;

    UserInterfaceParameterMapping()
        : InstVisitor(), fParameterLookup{}, fParameterIndex{0}
    {}

    virtual ~UserInterfaceParameterMapping() {}

    map<string, int> getParameterLookup() {
        return fParameterLookup;
    }

    virtual void visit(BlockInst* inst)
    {
        // BlockInst visitor is unimplemented in base class, so we need a trivial implementation
        // to actually visit the user interface statements in the BlockInst.
        for (const auto& it : inst->fCode) {
            it->accept(this);
        }
    }

    virtual void visit(AddMetaDeclareInst* inst)
    {
        // Only store fZone's value if it is not the 0 / nullptr special case
        if (inst->fZone != "0") {
            if (fParameterLookup.find(inst->fZone) == fParameterLookup.end()) {
                fParameterLookup[inst->fZone] = fParameterIndex++;
            }
        }
    }

    virtual void visit(AddButtonInst* inst)
    {
        if (fParameterLookup.find(inst->fZone) == fParameterLookup.end()) {
            fParameterLookup[inst->fZone] = fParameterIndex++;
        }
    }

    virtual void visit(AddSliderInst* inst)
    {
        if (fParameterLookup.find(inst->fZone) == fParameterLookup.end()) {
            fParameterLookup[inst->fZone] = fParameterIndex++;
        }
    }

    virtual void visit(AddBargraphInst* inst)
    {
        if (fParameterLookup.find(inst->fZone) == fParameterLookup.end()) {
            fParameterLookup[inst->fZone] = fParameterIndex++;
        }
    }

};

/**
 * Visitor for building user interface instructions based on the parameter lookup table.
 */
class RustUIInstVisitor : public TextInstVisitor {
   private:
    map<string, int>      fParameterLookup;

    int getParameterIndex(string name) {
        auto parameterIndex = fParameterLookup.find(name);
        if (parameterIndex == fParameterLookup.end()) {
            throw runtime_error("Parameter '" + name + "' is unknown");
        } else {
            return parameterIndex->second;
        }
    }

   public:
    using TextInstVisitor::visit;

    RustUIInstVisitor(std::ostream* out, const string& structname, map<string, int> parameterLookup, int tab = 0)
        : TextInstVisitor(out, ".", new RustStringTypeManager(xfloat(), "&"), tab),
          fParameterLookup{parameterLookup}
    {}

    virtual ~RustUIInstVisitor()
    {}

    virtual void visit(AddMetaDeclareInst* inst)
    {
        // Special case
        if (inst->fZone == "0") {
            *fOut << "ui_interface.declare(None, " << quote(inst->fKey) << ", " << quote(inst->fValue)
                  << ")";
        } else {
            *fOut << "ui_interface.declare(Some(ParamIndex(" << getParameterIndex(inst->fZone) << ")), " << quote(inst->fKey) << ", "
                  << quote(inst->fValue) << ")";
        }
        EndLine();
    }

    virtual void visit(OpenboxInst* inst)
    {
        string name;
        switch (inst->fOrient) {
            case OpenboxInst::kVerticalBox:
                name = "ui_interface.open_vertical_box(";
                break;
            case OpenboxInst::kHorizontalBox:
                name = "ui_interface.open_horizontal_box(";
                break;
            case OpenboxInst::kTabBox:
                name = "ui_interface.open_tab_box(";
                break;
        }
        *fOut << name << quote(inst->fName) << ")";
        EndLine();
    }

    virtual void visit(CloseboxInst* inst)
    {
        *fOut << "ui_interface.close_box();";
        tab(fTab, *fOut);
    }

    virtual void visit(AddButtonInst* inst)
    {
        if (inst->fType == AddButtonInst::kDefaultButton) {
            *fOut << "ui_interface.add_button(" << quote(inst->fLabel) << ", ParamIndex(" << getParameterIndex(inst->fZone) << "))";
        } else {
            *fOut << "ui_interface.add_check_button(" << quote(inst->fLabel) << ", ParamIndex(" << getParameterIndex(inst->fZone) << "))";
        }
        EndLine();
    }

    virtual void visit(AddSliderInst* inst)
    {
        string name;
        switch (inst->fType) {
            case AddSliderInst::kHorizontal:
                name = "ui_interface.add_horizontal_slider";
                break;
            case AddSliderInst::kVertical:
                name = "ui_interface.add_vertical_slider";
                break;
            case AddSliderInst::kNumEntry:
                name = "ui_interface.add_num_entry";
                break;
        }
        *fOut << name << "(" << quote(inst->fLabel) << ", "
              << "ParamIndex(" << getParameterIndex(inst->fZone) << "), " << checkReal(inst->fInit) << ", " << checkReal(inst->fMin) << ", "
              << checkReal(inst->fMax) << ", " << checkReal(inst->fStep) << ")";
        EndLine();
    }

    virtual void visit(AddBargraphInst* inst)
    {
        string name;
        switch (inst->fType) {
            case AddBargraphInst::kHorizontal:
                name = "ui_interface.add_horizontal_bargraph";
                break;
            case AddBargraphInst::kVertical:
                name = "ui_interface.add_vertical_bargraph";
                break;
        }
        *fOut << name << "(" << quote(inst->fLabel) << ", ParamIndex(" << getParameterIndex(inst->fZone) << "), " << checkReal(inst->fMin)
              << ", " << checkReal(inst->fMax) << ")";
        EndLine();
    }
    
    virtual void visit(AddSoundfileInst* inst)
    {
        // Not supported for now
        throw faustexception("ERROR : 'soundfile' primitive not yet supported for Rust\n");
    }
};

#endif
