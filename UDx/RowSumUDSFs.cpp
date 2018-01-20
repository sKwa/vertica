#include <exception>
#include "Vertica.h"

#define AUTHOR "Daniel Leybovich"
#define LIBRARY_BUILD_TAG "betta"
#define LIBRARY_VERSION "0.1"
#define LIBRARY_SDK_VERSION "VER_8_1_RELEASE_BUILD_1_10_20171207"
#define SOURCE_URL "github.com/sKwa"
#define DESCRIPTION "Returns the sum of row."
#define LICENSES_REQUIRED ""
#define SIGNATURE ""

using namespace std;
using namespace Vertica;

class RowSum : public ScalarFunction {

private:
    vector<int> columns;

public:

    virtual void setup(ServerInterface &srvInterface, const SizedColumnTypes &argTypes) {
        // TODO: data type validation, ints only
        ParamReader paramReader = srvInterface.getParamReader();
        int colCount = argTypes.getColumnCount();
        if (paramReader.containsParameter("exclude")) {
            string excludeColumn = paramReader.getStringRef("exclude").str();
            columns.reserve(colCount - 1);
            for (int i = 0; i < colCount; i++) {
                if (argTypes.getColumnName(i).c_str() == excludeColumn) {
                    continue;
                }
                columns.push_back(i);
            }
        } else {
            columns.reserve(colCount);
            for (int i = 0; i < colCount; i++) {
                columns.push_back(i);
            }
        } 
    }

    virtual void processBlock(ServerInterface &srvInterface, BlockReader &argReader, BlockWriter &resWriter) {
        try {            
            do {
                vint sum = 0;
                for (uint i = 0; i < columns.size(); i++) {
                    sum += argReader.getIntRef(columns[i]);
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
        // set coulmn name for output
        // It doesn't work but on any case
        returnType.addInt("SUM_TRUE");
    }

    virtual void getParameterType(ServerInterface &srvInterface, SizedColumnTypes &parameterTypes) {
        // Vertica system limits:
        // Length of basic names - 128 bytes. Basic names include table names, column names, etc.
        parameterTypes.addVarchar(128, "exclude");
    }
};

RegisterFactory(RowSumFactory);

RegisterLibrary(AUTHOR, LIBRARY_BUILD_TAG, LIBRARY_VERSION, LIBRARY_SDK_VERSION, SOURCE_URL, DESCRIPTION, LICENSES_REQUIRED, SIGNATURE);
