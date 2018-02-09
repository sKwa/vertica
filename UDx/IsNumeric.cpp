/****************************************************************************
 * TODO: 
 *     - trim spaces --> ???
 *     - add meta
 ****************************************************************************/
#include "Vertica.h"

using namespace std;
using namespace Vertica;

class IsNumeric : public ScalarFunction {

    public:
        virtual void processBlock(ServerInterface &srvInterface, BlockReader &argReader, BlockWriter &resWriter) {
            try {
                do {
                    if (argReader.isNull(0)) {
                        resWriter.setNull();
                    } else {
                        VString data = argReader.getStringRef(0);
                        uint32 length = data.length();
                        char value[length];
                        char* pointer;
                        strncpy(value, data.data(), length);
                        value[length] = '\0';
                        strtod(value, &pointer);
                        vbool isNum = (*pointer == 0);
                        resWriter.setBool(isNum);
                    }
                    resWriter.next();
                } while (argReader.next());
            } catch(exception& e) {
                vt_report_error(0, "Exception while processing block: [%s]", e.what());
            }
        }
};

class IsNumericFactory : public ScalarFunctionFactory {

    virtual ScalarFunction *createScalarFunction(ServerInterface &interface) {
        return vt_createFuncObject<IsNumeric>(interface.allocator); 
    }

    virtual void getPrototype(ServerInterface &interface, ColumnTypes &argTypes, ColumnTypes &returnType) {
        argTypes.addVarchar();
        returnType.addBool();
    }

    virtual void getReturnType(ServerInterface &srvInterface, const SizedColumnTypes &argTypes, SizedColumnTypes &returnType) {
        returnType.addBool("IS_NUMERIC");
    }
};

RegisterFactory(IsNumericFactory);
