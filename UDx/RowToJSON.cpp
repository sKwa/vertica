/*
 * Header: copyright, license, ...
 */
#include <ctime>
#include <sstream>
#include "Vertica.h"

#define AUTHOR "Daniel Leybovich"
#define LIBRARY_BUILD_TAG "alpha"
#define LIBRARY_VERSION "0.1"
#define LIBRARY_SDK_VERSION "VER_8_0_RELEASE_BUILD_1_6_20170616"
#define SOURCE_URL "github.com/sKwa"
#define DESCRIPTION "Concatenates the values"
#define LICENSES_REQUIRED ""
#define SIGNATURE ""

using namespace std;
using namespace Vertica;

const int MAXSIZE = 32000000;

string format_date(int64 days) {
    char buffer[13];
    tm t = {};
    t.tm_year = 100;
    t.tm_mon = 0;
    t.tm_mday = days + 1;
    mktime(&t);
    strftime(buffer, 13, "\"%F\"", &t);
    return buffer;
}

class RowToJSON : public ScalarFunction {
    public:
        virtual void processBlock(ServerInterface &srvInterface,
                BlockReader &argReader, BlockWriter &resWriter) {
            const SizedColumnTypes &inTypes = argReader.getTypeMetaData();
            const size_t colsNum = argReader.getNumCols();
            try {
                do {
                    ostringstream result;
                    result << "{ ";
                    for (size_t i = 0; i < colsNum; ++i) {
                        // key
                        result << "\"";
                        const string &name = inTypes.getColumnName(i);
                        if (name.empty()) {
                            result << "COL_" << i;
                        } else {
                            result << name;
                        }
                        result << "\" : ";

                        // value
                        const VerticaType &vt = inTypes.getColumnType(i);
                        if (vt.isStringType()) {    // CHAR,VARCHAR,BINARY,VARBINARY
                            const VString &fieldPtr = argReader.getStringRef(i);
                            if (fieldPtr.isNull()) {
                                result << "null";
                            } else {
                                result << "\"" << fieldPtr.str() << "\"";
                            }
                        } else if (vt.isInt()) {    // INTEGER
                            vint value = argReader.getIntRef(i);
                            if (value < -9223372036854775807) {
                                result << "null";
                            } else {
                                result << value;
                            }
                        } else if (vt.isBool()) {   // BOOLEAN
                            uint8 value = argReader.getBoolRef(i);
                            if (value == 0) {
                                result << "false";
                            } else if (value == 1) {
                                result << "true";
                            } else {
                                result << "null";
                            }
                        } else if (vt.isDate()) {   // DATE
                            int64 days = argReader.getDateRef(i);
                            if (days < -9223372036854775807) {
                                result << "null";
                            } else {
                                result << format_date(days);
                            }
                        }
                        if (i < colsNum - 1) {
                            result << ", ";
                        }
                    }
                    result << "}";
                    resWriter.getStringRef().copy(result.str());
                    resWriter.next();
                } while (argReader.next());
            } catch (exception &e) {
                // Standard exception. Quit.
                vt_report_error(0, "Exception while processing block: [%s]", e.what());
            }
        }
};

class RowToJSONFactory : public ScalarFunctionFactory {
    virtual ScalarFunction *createScalarFunction(ServerInterface &interface) {
        return vt_createFuncObject<RowToJSON>(interface.allocator);
    }

    virtual void getPrototype(ServerInterface &interface, ColumnTypes &argTypes,
            ColumnTypes &returnType) {
        argTypes.addAny();
        returnType.addLongVarchar();
    }

    virtual void getReturnType(ServerInterface &srvInterface,
            const SizedColumnTypes &argTypes,
            SizedColumnTypes &returnType) {
        returnType.addLongVarchar(MAXSIZE, "JSON");
        ;
    }
};

RegisterFactory(RowToJSONFactory);

RegisterLibrary(AUTHOR, LIBRARY_BUILD_TAG, LIBRARY_VERSION, LIBRARY_SDK_VERSION,
        SOURCE_URL, DESCRIPTION, LICENSES_REQUIRED, SIGNATURE);
