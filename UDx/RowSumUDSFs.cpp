/*
 *
 */
#include <exception>
#include "Vertica.h"

using namespace std;
using namespace Vertica;

class RowSum : public ScalarFunction {

private:
    bool allRow;
    string excludeColumn;

public:

    virtual void setup(ServerInterface &srvInterface, const SizedColumnTypes &argTypes) {
        ParamReader paramReader = srvInterface.getParamReader();
        allRow = true;
        if (paramReader.containsParameter("exclude")) {
            allRow = false;
            excludeColumn = paramReader.getStringRef("exclude").str();
        } 
    }

    virtual void processBlock(ServerInterface &srvInterface, BlockReader &argReader, BlockWriter &resWriter) {
        try {            
            do {
                vint sum = 0;
                for (uint i = 0; i < argReader.getNumCols(); i++){
                    sum += argReader.getIntRef(i);
                }
                resWriter.setInt(sum);
                resWriter.next();
            } while (argReader.next());
        } catch(exception& e) {
            vt_report_error(0, "Exception while processing block: [%s]", e.what());
        }
    }
};

class RowSumFactory : public ScalarFunctionFactory {

    virtual ScalarFunction *createScalarFunction(ServerInterface &interface) {
        return vt_createFuncObject<RowSum>(interface.allocator); 
    }

    virtual void getPrototype(ServerInterface &interface, ColumnTypes &argTypes, ColumnTypes &returnType) {
        argTypes.addAny();
        returnType.addInt();
    }

    virtual void getReturnType(ServerInterface &srvInterface, const SizedColumnTypes &argTypes, SizedColumnTypes &returnType) {
        returnType.addInt("SUM_TRUE");
    }

    virtual void getParameterType(ServerInterface &srvInterface, SizedColumnTypes &parameterTypes) {
        parameterTypes.addVarchar(128, "exclude");
    }
};

RegisterFactory(RowSumFactory);
