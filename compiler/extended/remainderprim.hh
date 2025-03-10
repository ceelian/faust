/************************************************************************
 ************************************************************************
    FAUST compiler
    Copyright (C) 2003-2018 GRAME, Centre National de Creation Musicale
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

#include <math.h>

#include "Text.hh"
#include "compatibility.hh"
#include "floats.hh"
#include "xtended.hh"

class RemainderPrim : public xtended {
   public:
    RemainderPrim() : xtended("remainder") {}

    virtual unsigned int arity() { return 2; }

    virtual bool needCache() { return true; }

    virtual ::Type infereSigType(const vector<::Type>& args)
    {
        faustassert(args.size() == arity());
    
        interval i = args[0]->getInterval();
        interval j = args[1]->getInterval();
        if (j.valid && gGlobal->gMathExceptions && j.haszero()) {
            cerr << "WARNING : potential division by zero in remainder(" << i << ", " << j << ")" << endl;
        }
    
        return castInterval(floatCast(args[0] | args[1]), interval());  // temporary rule !!!
    }

    virtual int infereSigOrder(const vector<int>& args)
    {
        faustassert(args.size() == arity());
        return max(args[0], args[1]);
    }

    virtual Tree computeSigOutput(const vector<Tree>& args)
    {
        num n, m;
        faustassert(args.size() == arity());
        if (isZero(args[1])) {
            stringstream error;
            error << "ERROR : remainder by 0 in remainder(" << ppsig(args[0]) << ", " << ppsig(args[1]) << ")" << endl;
            throw faustexception(error.str());
        } else if (isNum(args[0], n) && isNum(args[1], m)) {
            return tree(remainder(double(n), double(m)));
        } else {
            if (gGlobal->gMathApprox) {
                // res = x - (y * T(int(0.5f + x / y)));
                return sigBinOp(kSub, args[0], sigBinOp(kMul, args[1], sigFloatCast(sigIntCast(sigBinOp(kAdd, sigReal(0.5), sigBinOp(kDiv, args[0], args[1]))))));
            } else {
                return tree(symbol(), args[0], args[1]);
            }
        }
    }

    virtual ValueInst* generateCode(CodeContainer* container, list<ValueInst*>& args, ::Type result,
                                    vector< ::Type> const& types)
    {
        faustassert(args.size() == arity());
        faustassert(types.size() == arity());

        Typed::VarType         result_type;
        vector<Typed::VarType> arg_types;
        list<ValueInst*>       casted_args;
        prepareTypeArgsResult(result, args, types, result_type, arg_types, casted_args);

        return container->pushFunction(subst("remainder$0", isuffix()), result_type, arg_types, casted_args);
    }

    virtual string generateCode(Klass* klass, const vector<string>& args, const vector<::Type>& types)
    {
        faustassert(args.size() == arity());
        faustassert(types.size() == arity());

        return subst("remainder$2($0,$1)", args[0], args[1], isuffix());
    }

    virtual string generateLateq(Lateq* lateq, const vector<string>& args, const vector<::Type>& types)
    {
        faustassert(args.size() == arity());
        faustassert(types.size() == arity());

        return subst("$0\\pmod{$1}", args[0], args[1]);  // Same as fmodprim.cpp.
    }
};
