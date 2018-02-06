/**
 * @file    Unpivot.cpp
 * @author  Daniel Leybovich (d.leybovich@gmail.com)
 * @date    04/02/2018
 * @version 0.1
 *
 * @brief   Rotates row data to columns.
 *
 * @section DESCRIPTION
 * 
 * UNPIVOT provides a mechanism for transforming columns into rows.
 */
#include <sstream>
#include "Vertica.h"

using namespace std;
using namespace Vertica;


void unpivot(PartitionReader &inputReader, PartitionWriter &outputWriter) {
    const SizedColumnTypes &metaData = inputReader.getTypeMetaData();
    do {
        for (size_t i = 0; i < inputReader.getNumCols(); i++) {
            const string columnName = metaData.getColumnName(i);
            // key is column name
            VString &key = outputWriter.getStringRef(0);
            key.copy(columnName);
            // value is column value
            if (inputReader.isNull(i)) {
                outputWriter.setNull(1);
            } else {
                VString &value = outputWriter.getStringRef(1);
                const VerticaType &dataType = metaData.getColumnType(i);
                ostringstream result;
                if (dataType.isBool()) {
                    result << (inputReader.getBoolRef(i) ? "true" : "false");
                } else if (dataType.isInt()) {
                    result << inputReader.getIntRef(i);
                } else if (dataType.isFloat()) {
                    result << inputReader.getFloatRef(i);
                } else if (dataType.isNumeric()) {
                    const int32 len = dataType.getNumericPrecision() + 1;
                    char buffer[len];
                    inputReader.getNumericRef(i).toString(buffer, len);
                    result << buffer;
                } else if (dataType.isChar() || dataType.isVarchar() || dataType.isLongVarchar()) {
                    result << inputReader.getStringRef(i).str();
                } else {
                    result << "UNSUPPORTED DATA TYPE";
                }
                value.copy(result.str());
            }
            outputWriter.next();
        }
    } while (inputReader.next());
}


class Unpivot : public TransformFunction {

    virtual void processPartition(ServerInterface &srvInterface, PartitionReader &inputReader, PartitionWriter &outputWriter) {
        try {
            unpivot(inputReader, outputWriter);
        } catch (exception& e) {
            vt_report_error(0, "Exception while processing partition: [%s]", e.what());
        }
    }
};


class UnpivotFactory : public TransformFunctionFactory {

    virtual void getPrototype(ServerInterface &srvInterface, ColumnTypes &argTypes, ColumnTypes &returnType) {
        argTypes.addAny();
        returnType.addVarchar();
        returnType.addVarchar();
    }

    virtual void getReturnType(ServerInterface &srvInterface, const SizedColumnTypes &inputTypes, SizedColumnTypes &outputTypes) {
        outputTypes.addVarchar(128, "KEY");
        // TODO: extract max length from columns data type
        outputTypes.addVarchar(6600, "VALUE");
    }

    virtual TransformFunction *createTransformFunction(ServerInterface &srvInterface) {
        return vt_createFuncObj(srvInterface.allocator, Unpivot); 
    }
};

RegisterFactory(UnpivotFactory);
