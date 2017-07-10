/******************************************************************************
 *
 * For a specified measure, LISTAGG orders data within each group specified in
 * the ORDER BY clause and then concatenates the values of the measure column.
 * As a single-set aggregate function, LISTAGG operates on all rows and returns
 * a single output row.
 *
 *****************************************************************************/

#include <string>
#include <sstream>
#include "Vertica.h"

#define AUTHOR "Daniel Leybovich"
#define LIBRARY_BUILD_TAG "betta"
#define LIBRARY_VERSION "0.1"
#define LIBRARY_SDK_VERSION "VER_8_0_RELEASE_BUILD_1_6_20170616"
#define SOURCE_URL "github.com/sKwa"
#define DESCRIPTION "Concatenates the values"
#define LICENSES_REQUIRED ""
#define SIGNATURE ""

#define MAXSIZE 8000000

using namespace std;
using namespace Vertica;

class ListAgg : public AnalyticFunction {
  virtual void setup(ServerInterface &srvInterface,
                     const SizedColumnTypes &argTypes) {
    ParamReader paramReader = srvInterface.getParamReader();

    if (paramReader.containsParameter("delimiter")) {
      delimiter = paramReader.getStringRef("delimiter").str();
    } else {
      delimiter = ",";
    }
  }

  virtual void processPartition(ServerInterface &srvInterface,
                                AnalyticPartitionReader &inputReader,
                                AnalyticPartitionWriter &outputWriter) {
    string value;
    string output;
    ostringstream result;
    bool first = true;
    int processColumn = inputReader.getNumCols() - 1;

    try {
      int rowCount = 0;
      do {
        rowCount++;
        if (rowCount % 100 == 0) {
          if (isCanceled()) {
            srvInterface.log("Got canceled!");
            return;
          }
        }
        const VString &fieldPtr = inputReader.getStringRef(processColumn);
        if (fieldPtr.isNull()) {
          continue;
        }
        value = fieldPtr.str();
        if (!first) {
          result << delimiter;
        }
        first = false;
        result << value;
      } while (inputReader.next());

      output = result.str();
      rowCount = 0;
      do {
        if (rowCount % 100 == 0) {
          if (isCanceled()) {
            srvInterface.log("Got canceled!");
            return;
          }
        }
        outputWriter.getStringRef(0).copy(output);
      } while (outputWriter.next());
    } catch (exception &e) {
      vt_report_error(0, "Exception while processing partition: %s", e.what());
    }
  }

private:
  string delimiter;
};

class ListAggFactory : public AnalyticFunctionFactory {
  virtual void getPrototype(ServerInterface &srvInterface,
                            ColumnTypes &argTypes, ColumnTypes &returnType) {
    argTypes.addVarchar();
    returnType.addLongVarchar();
  }

  virtual void getReturnType(ServerInterface &srvInterface,
                             const SizedColumnTypes &argTypes,
                             SizedColumnTypes &returnType) {
    returnType.addLongVarchar(MAXSIZE, "listagg");
  }

  virtual void getParameterType(ServerInterface &srvInterface,
                                SizedColumnTypes &parameterTypes) {
    parameterTypes.addVarchar(2, "delimiter");
  }

  virtual AnalyticFunction *
  createAnalyticFunction(ServerInterface &srvInterface) {
    return vt_createFuncObject<ListAgg>(srvInterface.allocator);
  }
};

RegisterFactory(ListAggFactory);

RegisterLibrary(AUTHOR, LIBRARY_BUILD_TAG, LIBRARY_VERSION, LIBRARY_SDK_VERSION,
                SOURCE_URL, DESCRIPTION, LICENSES_REQUIRED, SIGNATURE);
