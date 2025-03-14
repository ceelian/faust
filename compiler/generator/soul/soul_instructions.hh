/************************************************************************
 ************************************************************************
    FAUST compiler
    Copyright (C) 2003-2019 GRAME, Centre National de Creation Musicale
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

#ifndef _SOUL_INSTRUCTIONS_H
#define _SOUL_INSTRUCTIONS_H

#include <sstream>
#include <string>
#include <vector>
#include <utility>
#include <cctype>

#include "text_instructions.hh"
#include "faust/gui/PathBuilder.h"

using namespace std;

struct SOULInstUIVisitor : public DispatchVisitor, public PathBuilder {
    std::stringstream     fOut;
    SOULStringTypeManager fTypeManager;
    int                   fTab;
    bool                  fHasBargraph;  // Whether the DSP code has some Bargraphs
    
    std::vector<std::pair <std::string, std::string> > fMetaAux;

    using DispatchVisitor::visit;

    SOULInstUIVisitor(int tab = 1) : fTypeManager(xfloat(), "*"), fTab(tab), fHasBargraph(false) {}
    
    void addMeta()
    {
        if (fMetaAux.size() > 0) {
            for (size_t i = 0; i < fMetaAux.size(); i++) {
                if (!std::isdigit(fMetaAux[i].first[0])) {
                    fOut << ", " << "meta_" + gGlobal->getFreshID(fMetaAux[i].first) << ": " << quote(fMetaAux[i].second);
                }
            }
        }
        fMetaAux.clear();
    }
    
    std::string getSoulMatadata()
    {
        if (fMetaAux.size() > 0) {
            for (size_t i = 0; i < fMetaAux.size(); i++) {
                if (fMetaAux[i].first == "soul") return fMetaAux[i].second;
            }
        }
        return "";
    }
    
    virtual void visit(AddMetaDeclareInst* inst)
    {
        fMetaAux.push_back(std::make_pair(inst->fKey, inst->fValue));
    }
   
    virtual void visit(AddButtonInst* inst)
    {
        if (gGlobal->gOutputLang == "soul-poly") {
            vector<char> rep = {' ', '(', ')', '/', '\\', '.', '-'};
            fOut << "input event " << fTypeManager.fTypeDirectTable[itfloat()]
            << " event_" << replaceCharList(inst->fLabel, rep, '_')
            << " [[ name: " << quote(inst->fLabel)
            << ", group: " << quote(buildPath(inst->fLabel));
            if (inst->fType != AddButtonInst::kDefaultButton) {
                fOut << ", latching";
            }
            fOut<< ", text: \"off|on\""
            << ", boolean";
            addMeta();
            fOut << " ]];";
        } else if (gGlobal->gOutputLang == "soul-hybrid") {
            string soul_meta = getSoulMatadata();
            vector<char> rep = {' ', '(', ')', '/', '\\', '.', '-'};
            fOut << "input event " << fTypeManager.fTypeDirectTable[itfloat()]
            << " " << ((soul_meta != "") ? soul_meta : replaceCharList(inst->fLabel, rep, '_'))
            << " [[ name: " << quote(inst->fLabel)
            << ", group: " << quote(buildPath(inst->fLabel));
            if (inst->fType != AddButtonInst::kDefaultButton) {
                fOut << ", latching";
            }
            fOut << ", text: \"off|on\""
            << ", boolean";
            addMeta();
            fOut << " ]];";
        } else {
            fOut << "input event " << fTypeManager.fTypeDirectTable[itfloat()]
            << " event" << inst->fZone
            << " [[ name: " << quote(inst->fLabel)
            << ", group: " << quote(buildPath(inst->fLabel));
            if (inst->fType != AddButtonInst::kDefaultButton) {
                fOut << ", latching";
            }
            fOut << ", text: \"off|on\""
            << ", boolean";
            addMeta();
            fOut << " ]];";
        }
        tab(fTab, fOut);
    }

    virtual void visit(AddSliderInst* inst)
    {
        if (gGlobal->gOutputLang == "soul-poly") {
            vector<char> rep = {' ', '(', ')', '/', '\\', '.', '-'};
            fOut << "input event " << fTypeManager.fTypeDirectTable[itfloat()]
            << " event_" << replaceCharList(inst->fLabel, rep, '_')
            << " [[ name: " << quote(inst->fLabel)
            << ", group: " << quote(buildPath(inst->fLabel))
            << ", min: " << checkReal(inst->fMin)
            << ", max: " << checkReal(inst->fMax)
            << ", init: " << checkReal(inst->fInit)
            << ", step: " << checkReal(inst->fStep);
            addMeta();
            fOut << " ]];";
        } else if (gGlobal->gOutputLang == "soul-hybrid") {
            string soul_meta = getSoulMatadata();
            vector<char> rep = {' ', '(', ')', '/', '\\', '.'};
            fOut << "input event " << fTypeManager.fTypeDirectTable[itfloat()]
            << " " << ((soul_meta != "") ? soul_meta : replaceCharList(inst->fLabel, rep, '_'))
            << " [[ name: " << quote(inst->fLabel)
            << ", group: " << quote(buildPath(inst->fLabel))
            << ", min: " << checkReal(inst->fMin)
            << ", max: " << checkReal(inst->fMax)
            << ", init: " << checkReal(inst->fInit)
            << ", step: " << checkReal(inst->fStep);
            addMeta();
            fOut << " ]];";
        } else {
            fOut << "input event " << fTypeManager.fTypeDirectTable[itfloat()]
            << " event" << inst->fZone
            << " [[ name: " << quote(inst->fLabel)
            << ", group: " << quote(buildPath(inst->fLabel))
            << ", min: " << checkReal(inst->fMin)
            << ", max: " << checkReal(inst->fMax)
            << ", init: " << checkReal(inst->fInit)
            << ", step: " << checkReal(inst->fStep);
            addMeta();
            fOut << " ]];";
        }
        tab(fTab, fOut);
    }

    virtual void visit(AddBargraphInst* inst)
    {
        // We have bargraphs
        fHasBargraph = true;
        
        if (gGlobal->gOutputLang == "soul-poly") {
            vector<char> rep = {' ', '(', ')', '/', '\\', '.', '-'};
            fOut << "output event " << fTypeManager.fTypeDirectTable[itfloat()]
            << " event_" << quote(replaceCharList(inst->fLabel, rep, '_'))
            << " [[ name: " << quote(inst->fLabel)
            << ", group: " << quote(buildPath(inst->fLabel))
            << ", min: " << checkReal(inst->fMin)
            << ", max: " << checkReal(inst->fMax);
            addMeta();
            fOut << " ]];";
        } else if (gGlobal->gOutputLang == "soul-hybrid") {
            string soul_meta = getSoulMatadata();
            vector<char> rep = {' ', '(', ')', '/', '\\', '.', '-'};
            fOut << "output event " << fTypeManager.fTypeDirectTable[itfloat()]
            << " " << ((soul_meta != "") ? soul_meta : replaceCharList(inst->fLabel, rep, '_'))
            << " [[ name: " << quote(inst->fLabel)
            << ", group: " << quote(buildPath(inst->fLabel))
            << ", min: " << checkReal(inst->fMin)
            << ", max: " << checkReal(inst->fMax);
            addMeta();
            fOut << " ]];";
        } else {
            fOut << "output event " << fTypeManager.fTypeDirectTable[itfloat()]
            << " event" << inst->fZone
            << " [[ name: " << quote(inst->fLabel)
            << ", group: " << quote(buildPath(inst->fLabel))
            << ", min: " << checkReal(inst->fMin)
            << ", max: " << checkReal(inst->fMax);
            addMeta();
            fOut << " ]];";
        }
        tab(fTab, fOut);
    }
    
    virtual void visit(OpenboxInst* inst)
    {
        switch (inst->fOrient) {
            case OpenboxInst::kVerticalBox:
                pushLabel("v:" + inst->fName);
                break;
            case OpenboxInst::kHorizontalBox:
                pushLabel("h:" + inst->fName);
                break;
            case OpenboxInst::kTabBox:
                pushLabel("t:" + inst->fName);
                break;
        }
        fMetaAux.clear();
    }
    
    virtual void visit(CloseboxInst* inst)
    {
        popLabel();
        fMetaAux.clear();
    }
    
};

class SOULInstVisitor : public TextInstVisitor {
   private:
    // Polymorphic math functions
    map<string, string> gPolyMathLibTable;

    // Whether to consider an 'int' as a 'boolean' later on in code generation
    bool fIntAsBool;
    
    std::vector<std::pair <std::string, std::string> > fMetaAux;
    
    inline string checkFloat(float val)
    {
        return (std::isinf(val)) ? "inf" : T(val);
    }
    inline string checkDouble(double val)
    {
        return (std::isinf(val)) ? "inf" : T(val);
    }
    
    std::string getSoulMatadata()
    {
        if (fMetaAux.size() > 0) {
            for (size_t i = 0; i < fMetaAux.size(); i++) {
                if (fMetaAux[i].first == "soul") return fMetaAux[i].second;
            }
        }
        return "";
    }

   public:
    SOULInstVisitor(std::ostream* out, int tab = 0)
        : TextInstVisitor(out, ".", new SOULStringTypeManager(xfloat(), ""), tab)
    {
        // Polymath mapping int version
        gPolyMathLibTable["abs"]   = "abs";
        gPolyMathLibTable["max_i"] = "max";
        gPolyMathLibTable["min_i"] = "min";

        // Polymath mapping float version
        gPolyMathLibTable["max_f"] = "max";
        gPolyMathLibTable["min_f"] = "min";

        gPolyMathLibTable["fabsf"]      = "abs";
        gPolyMathLibTable["acosf"]      = "acos";
        gPolyMathLibTable["asinf"]      = "asin";
        gPolyMathLibTable["atanf"]      = "atan";
        gPolyMathLibTable["atan2f"]     = "atan2";
        gPolyMathLibTable["ceilf"]      = "ceil";
        gPolyMathLibTable["cosf"]       = "cos";
        gPolyMathLibTable["expf"]       = "exp";
        gPolyMathLibTable["exp2f"]      = "exp2";
        gPolyMathLibTable["exp10f"]     = "exp10f";
        gPolyMathLibTable["floorf"]     = "floor";
        gPolyMathLibTable["fmodf"]      = "fmod";
        gPolyMathLibTable["logf"]       = "log";
        gPolyMathLibTable["log2f"]      = "log2";
        gPolyMathLibTable["log10f"]     = "log10";
        gPolyMathLibTable["powf"]       = "pow";
        gPolyMathLibTable["remainderf"] = "remainder";
        gPolyMathLibTable["rintf"]      = "roundToInt";
        gPolyMathLibTable["roundf"]     = "round";
        gPolyMathLibTable["sinf"]       = "sin";
        gPolyMathLibTable["sqrtf"]      = "sqrt";
        gPolyMathLibTable["tanf"]       = "tan";
        
        // Additional hyperbolic math functions are included in SOUL
        gPolyMathLibTable["acoshf"] = "acosh";
        gPolyMathLibTable["asinhf"] = "asinh";
        gPolyMathLibTable["atanhf"] = "atanh";
        gPolyMathLibTable["coshf"]  = "cosh";
        gPolyMathLibTable["sinhf"]  = "sinh";
        gPolyMathLibTable["tanhf"]  = "tanh";
        
        gPolyMathLibTable["isnanf"]  = "isnan";
        gPolyMathLibTable["isinff"]  = "isinf";
        // Manually implemented
        gPolyMathLibTable["copysignf"]  = "copysign";

        // Polymath mapping double version
        gPolyMathLibTable["max_"] = "max";
        gPolyMathLibTable["min_"] = "min";

        gPolyMathLibTable["fabs"]      = "abs";
        gPolyMathLibTable["acos"]      = "acos";
        gPolyMathLibTable["asin"]      = "asin";
        gPolyMathLibTable["atan"]      = "atan";
        gPolyMathLibTable["atan2"]     = "atan2";
        gPolyMathLibTable["ceil"]      = "ceil";
        gPolyMathLibTable["cos"]       = "cos";
        gPolyMathLibTable["exp"]       = "exp";
        gPolyMathLibTable["exp2"]      = "exp2";
        gPolyMathLibTable["exp10"]     = "exp10";
        gPolyMathLibTable["floor"]     = "floor";
        gPolyMathLibTable["fmod"]      = "fmod";
        gPolyMathLibTable["log"]       = "log";
        gPolyMathLibTable["log2"]      = "log2";
        gPolyMathLibTable["log10"]     = "log10";
        gPolyMathLibTable["pow"]       = "pow";
        gPolyMathLibTable["remainder"] = "remainder";
        gPolyMathLibTable["rint"]      = "roundToInt";
        gPolyMathLibTable["round"]     = "round";
        gPolyMathLibTable["sin"]       = "sin";
        gPolyMathLibTable["sqrt"]      = "sqrt";
        gPolyMathLibTable["tan"]       = "tan";

        // Additional hyperbolic math functions are included in SOUL
        gPolyMathLibTable["acosh"] = "acosh";
        gPolyMathLibTable["asinh"] = "asinh";
        gPolyMathLibTable["atanh"] = "atanh";
        gPolyMathLibTable["cosh"]  = "cosh";
        gPolyMathLibTable["sinh"]  = "sinh";
        gPolyMathLibTable["tanh"]  = "tanh";

        gPolyMathLibTable["isnan"]  = "isnan";
        gPolyMathLibTable["isinf"]  = "isinf";
        // Manually implemented
        gPolyMathLibTable["copysignf"]  = "copysign";

        fIntAsBool = false;
    }

    virtual ~SOULInstVisitor() {}
   
    virtual void visit(AddMetaDeclareInst* inst)
    {
        fMetaAux.push_back(std::make_pair(inst->fKey, inst->fValue));
    }
    
    virtual void visit(OpenboxInst* inst)
    {
        fMetaAux.clear();
    }
    
    virtual void visit(CloseboxInst* inst)
    {
        fMetaAux.clear();
    }
    
    virtual void visit(AddButtonInst* inst)
    {
        *fOut << "// " << inst->fLabel;
        EndLine(' ');
        if (gGlobal->gOutputLang == "soul-poly") {
            vector<char> rep = {' ', '(', ')', '/', '\\', '.', '-'};
            *fOut << "event event_" << replaceCharList(inst->fLabel, rep, '_') << " ("
                  << fTypeManager->fTypeDirectTable[itfloat()] << " val) { " << inst->fZone
                  << " = val; fUpdated = true; }";
        } else if (gGlobal->gOutputLang == "soul-hybrid") {
            string soul_meta = getSoulMatadata();
            vector<char> rep = {' ', '(', ')', '/', '\\', '.', '-'};
            *fOut << "event " << ((soul_meta != "") ? soul_meta : replaceCharList(inst->fLabel, rep, '_'))
                  << " (" << fTypeManager->fTypeDirectTable[itfloat()] << " val) { "
                  << inst->fZone << " = val; fUpdated = true; }";
            fMetaAux.clear();
        } else {
            *fOut << "event event" << inst->fZone << " (" << fTypeManager->fTypeDirectTable[itfloat()] << " val) { "
                  << inst->fZone << " = val; fUpdated = true; }";
        }
        EndLine(' ');
    }

    virtual void visit(AddSliderInst* inst)
    {
        *fOut << "// " << inst->fLabel << " [init = " << checkReal(inst->fInit)
              << ", min = " << checkReal(inst->fMin) << ", max = " << checkReal(inst->fMax)
              << ", step = " << checkReal(inst->fStep) << "]";
        EndLine(' ');
        if (gGlobal->gOutputLang == "soul-poly") {
            vector<char> rep = {' ', '(', ')', '/', '\\', '.', '-'};
            *fOut << "event event_" << replaceCharList(inst->fLabel, rep, '_') << " ("
                  << fTypeManager->fTypeDirectTable[itfloat()] << " val) { " << inst->fZone
                  << " = val; fUpdated = true; }";
        } else if (gGlobal->gOutputLang == "soul-hybrid") {
            string soul_meta = getSoulMatadata();
            vector<char> rep = {' ', '(', ')', '/', '\\', '.', '-'};
            *fOut << "event " << ((soul_meta != "") ? soul_meta : replaceCharList(inst->fLabel, rep, '_'))
                  << " (" << fTypeManager->fTypeDirectTable[itfloat()] << " val) { "
                  << inst->fZone << " = val; fUpdated = true; }";
            fMetaAux.clear();
        } else {
            *fOut << "event event" << inst->fZone << " (" << fTypeManager->fTypeDirectTable[itfloat()] << " val) { "
                  << inst->fZone << " = val; fUpdated = true; }";
        }
        EndLine(' ');
    }

    virtual void visit(AddBargraphInst* inst)
    {
        *fOut << "// " << inst->fLabel << " [min = " << checkReal(inst->fMin) << ", max = " << checkReal(inst->fMax)
              << "]";
        EndLine(' ');
    }

    virtual void visit(AddSoundfileInst* inst)
    {
        // Not supported for now
        throw faustexception("ERROR : 'soundfile' primitive not yet supported for SOUL\n");
    }
    
    virtual void visit(DeclareVarInst* inst)
    {
        string name = inst->fAddress->getName();

        // special case for input/output considered as 'streams'
        if (startWith(name, "input")) {
            *fOut << "input stream " << fTypeManager->fTypeDirectTable[itfloat()] << " " << name;
        } else if (startWith(name, "output")) {
            *fOut << "output stream " << fTypeManager->fTypeDirectTable[itfloat()] << " " << name;
        } else {
            if (inst->fAddress->getAccess() & Address::kConst) {
                *fOut << "const ";
            }
            *fOut << fTypeManager->generateType(inst->fType, name);
            if (inst->fValue) {
                *fOut << " = ";
                inst->fValue->accept(this);
            }
        }
        EndLine();
    }

    virtual void visit(DeclareFunInst* inst) {}

    /*
     Indexed adresses can actually be values in an array or fields in a struct type
     */
    virtual void visit(IndexedAddress* indexed)
    {
        indexed->fAddress->accept(this);
        DeclareStructTypeInst* struct_type = isStructType(indexed->getName());
        if (struct_type) {
            Int32NumInst* field_index = static_cast<Int32NumInst*>(indexed->fIndex);
            *fOut << "." << struct_type->fType->getName(field_index->fNum);
        } else {
            if (dynamic_cast<Int32NumInst*>(indexed->fIndex)) {
                *fOut << "[";
                indexed->fIndex->accept(this);
                *fOut << "]";
            } else {
                // wrap code is automatically added by the SOUL compiler (and the same if [idex] syntax is used)
                *fOut << ".at (";
                indexed->fIndex->accept(this);
                *fOut << ")";
            }
        }
    }

    virtual void visit(StoreVarInst* inst)
    {
        // special case for 'output' considered as a 'stream'
        if (startWith(inst->fAddress->getName(), "output")) {
            inst->fAddress->accept(this);
            *fOut << " << ";
            inst->fValue->accept(this);
            EndLine();
            // special case for 'bargraph' considered as an 'output event'
        } else if (startWith(inst->fAddress->getName(), "fHbargraph") ||
                   startWith(inst->fAddress->getName(), "fVbargraph")) {
            
            // value is stored in the bargraph variable
            {
                inst->fAddress->accept(this);
                *fOut << " = ";
                inst->fValue->accept(this);
                EndLine();
            }
            
            // and the bargraph variable is sent using the 'output' event handler
            {
                *fOut << "if (fControlSlice == 0) { ";
                *fOut << "event";
                inst->fAddress->accept(this);
                *fOut << " << ";
                inst->fAddress->accept(this);
                *fOut << "; }";
                tab(fTab, *fOut);
            }
            
        } else {
            inst->fAddress->accept(this);
            *fOut << " = ";
            inst->fValue->accept(this);
            EndLine();
        }
    }
    
    virtual void visit(FloatNumInst* inst) { *fOut << checkFloat(inst->fNum); }

    virtual void visit(FloatArrayNumInst* inst)
    {
        char sep = '(';
        for (size_t i = 0; i < inst->fNumTable.size(); i++) {
            *fOut << sep << checkFloat(inst->fNumTable[i]);
            sep = ',';
        }
        *fOut << ')';
    }
  
    virtual void visit(Int32ArrayNumInst* inst)
    {
        char sep = '(';
        for (size_t i = 0; i < inst->fNumTable.size(); i++) {
            *fOut << sep << inst->fNumTable[i];
            sep = ',';
        }
        *fOut << ')';
    }
    
    virtual void visit(Int64NumInst* inst) { *fOut << inst->fNum << "L"; }

    virtual void visit(DoubleNumInst* inst) { *fOut << checkDouble(inst->fNum); }
    
    virtual void visit(DoubleArrayNumInst* inst)
    {
        char sep = '(';
        for (size_t i = 0; i < inst->fNumTable.size(); i++) {
            *fOut << sep << checkDouble(inst->fNumTable[i]);
            sep = ',';
        }
        *fOut << ')';
    }

    virtual void visit(::CastInst* inst)
    {
        string type = fTypeManager->generateType(inst->fType);
        *fOut << type << " (";
        inst->fInst->accept(this);
        *fOut << ")";
    }

    virtual void visit(BitcastInst* inst) { faustassert(false); }

    virtual void visit(Select2Inst* inst)
    {
        *fOut << "(bool (";
        fIntAsBool = true;
        inst->fCond->accept(this);
        fIntAsBool = false;
        *fOut << ") ? ";
        inst->fThen->accept(this);
        *fOut << " : ";
        inst->fElse->accept(this);
        *fOut << ")";
    }
    
    virtual void visit(IfInst* inst)
    {
        *fOut << "if ";
        *fOut << "(bool (";
        fIntAsBool = true;
        visitCond(inst->fCond);
        fIntAsBool = false;
        *fOut << "))";
        *fOut << " {";
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

    virtual void visit(BinopInst* inst)
    {
        bool cond1 = needParenthesis(inst, inst->fInst1);
        bool cond2 = needParenthesis(inst, inst->fInst2);
    
        bool int_as_bool = fIntAsBool;
        if (isBoolOpcode(inst->fOpcode) && !int_as_bool) {
            *fOut << "int (";
        }

        // Hack to make it work again with 'soul' version 0.0.6
        if (isLogicalOpcode(inst->fOpcode)) {
            TypingVisitor typing;
            inst->fInst1->accept(&typing);
            if (isInt64Type(typing.fCurType)) {
                *fOut << "int64 (";
            } else if (isInt32Type(typing.fCurType) || isBoolType(typing.fCurType)) {
                *fOut << "int32 (";
            } else {
                faustassert(false);
            }
        }

        if (cond1) *fOut << "(";
        inst->fInst1->accept(this);
        if (cond1) *fOut << ")";

        // Hack to make it work again with 'soul' version 0.0.6
        if (isLogicalOpcode(inst->fOpcode)) {
            *fOut << ")";
        }

        *fOut << " ";
        *fOut << gBinOpTable[inst->fOpcode]->fName;
        *fOut << " ";

        // Hack to make it work again with 'soul' version 0.0.6
        if (isLogicalOpcode(inst->fOpcode)) {
            TypingVisitor typing;
            inst->fInst2->accept(&typing);
            if (isInt64Type(typing.fCurType)) {
                *fOut << "int64 (";
            } else if (isInt32Type(typing.fCurType) || isBoolType(typing.fCurType)) {
                *fOut << "int32 (";
            } else {
                faustassert(false);
            }
        }

        if (cond2) *fOut << "(";
        inst->fInst2->accept(this);
        if (cond2) *fOut << ")";

        // Hack to make it work again with 'soul' version 0.0.6
        if (isLogicalOpcode(inst->fOpcode)) {
            *fOut << ")";
        }

        if (isBoolOpcode(inst->fOpcode) && !int_as_bool) {
            *fOut << ")";
        }
    }

    virtual void visit(FunCallInst* inst)
    {
        string name;
        if (gPolyMathLibTable.find(inst->fName) != gPolyMathLibTable.end()) {
            name = gPolyMathLibTable[inst->fName];
        } else {
            name = inst->fName;
        }

        *fOut << gGlobal->getMathFunction(name) << ((inst->fArgs.size() > 0) ? " (" : "(");

        // Compile parameters
        generateFunCallArgs(inst->fArgs.begin(), inst->fArgs.end(), inst->fArgs.size());
        *fOut << ")";
    }

    virtual void visit(ForLoopInst* inst)
    {
        // Don't generate empty loops...
        if (inst->fCode->size() == 0) return;

        *fOut << "for (";

        fFinishLine = false;
        inst->fInit->accept(this);
        *fOut << "; ";

        fIntAsBool = true;
        inst->fEnd->accept(this);
        fIntAsBool = false;
        *fOut << "; ";

        inst->fIncrement->accept(this);
        fFinishLine = true;
        *fOut << ") {";

        fTab++;
        tab(fTab, *fOut);
        inst->fCode->accept(this);
        fTab--;
        back(1, *fOut);
        *fOut << "}";
        tab(fTab, *fOut);
    }

    StringTypeManager* getTypeManager() { return fTypeManager; }
};

// For subcontainers: variable access is specific
class SOULSubContainerInstVisitor : public SOULInstVisitor {
   public:
    SOULSubContainerInstVisitor(std::ostream* out, int tab = 0) : SOULInstVisitor(out, tab) {}

    virtual void visit(NamedAddress* named)
    {
        if (named->getAccess() & Address::kStruct) {
            *fOut << "this.";
        }
        *fOut << named->fName;
    }
};

#endif
