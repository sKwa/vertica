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
#define DESCRIPTION "Converts row to JSON"
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
    strftime(buffer, 13, "%F", &t);
    return buffer;
}

string format_time(int64 microseconds) {
    char buffer[80];
    int tsec = (microseconds / 1000000);
    int usec = microseconds % 1000000;
    int secs = tsec % 60;
    int mins = (tsec / 60 ) % 60;
    int hour = (tsec / 60 ) / 60;
    snprintf(buffer, 80, "%02d:%02d:%02d.%d", hour, mins, secs, usec);
    return buffer;
}

string format_timestamp(int64 microseconds) {
    char buffer[80];
    int tsec = (microseconds / 1000000);
    int usec = microseconds % 1000000;
    int secs = tsec % 60;
    int mins = (tsec / 60 ) % 60;
    int hour = ((tsec / 60 ) / 60) % 24;
    int days = tsec / 86400;
    snprintf(buffer, 80, "%sT%02d:%02d:%02d.%d", format_date(days).c_str(), hour, mins, secs, usec);
    return buffer;
}

string format_interval(int64 microseconds) {
    char buffer[80];
    int tsec = (microseconds / 1000000);
    int usec = microseconds % 1000000;
    int secs = tsec % 60;
    int mins = (tsec / 60 ) % 60;
    int hour = (tsec / 60 ) / 60;
    snprintf(buffer, 80, "%d days %2d mins %2d.%d secs", hour, mins, secs, usec);
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
                        if (argReader.isNull(i)) {
                            result << "null";
                        } else if (vt.isBool()) {           // BOOLEAN
                            result << (argReader.getBoolRef(i) ? "true" : "false");
                        } else if (vt.isInt()) {            // INTEGER
                            result << argReader.getIntRef(i);
                        } else if (vt.isFloat()) {          // FLOAT
                            result << argReader.getFloatRef(i);
                        } else if (vt.isNumeric()) {        // NUMERIC
                            const int32 len = vt.getNumericPrecision() + 1;
                            char buffer[len];
                            argReader.getNumericRef(i).toString(buffer, len);
                            result << buffer;
                        } else if (vt.isDate()) {           // DATE
                            result << "\"" << format_date(argReader.getDateRef(i)) << "\"";
                        } else if (vt.isTime()) {           // TIME
                            result << "\"" << format_time(argReader.getTimeRef(i)) << "\"";
                        } else if (vt.isTimestamp()) {      // TIMESTAMP
                            result << "\"" << format_timestamp(argReader.getTimestampRef(i)) << "\"";
                        } else if (vt.isInterval()) {       // INTERVAL
                            result << format_interval(argReader.getIntervalRef(i));
                        } else if (vt.isIntervalYM()) {     // INTERVALYM
                            result << argReader.getIntervalRef(i);
                                                            // STRING
                        } else if (vt.isChar() || vt.isVarchar() || vt.isLongVarchar()) {
                            result << "\"" << argReader.getStringRef(i).str() << "\"";
                        } else {
                            vt_report_error(0, "Unsupported data type [%s]", vt.getTypeStr());
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
