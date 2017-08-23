#!/usr/bin/env bash

USR='dbadmin'
PWD='letmein'
PTH='/home/dbadmin'

# compile
g++ -D HAVE_LONG_INT_64  -I /opt/vertica/sdk/include \
    -Wall -shared -Wno-unused-value -fPIC \
    -o MyUDx.so "${PTH}/RowToJSON.cpp" /opt/vertica/sdk/include/Vertica.cpp

if [[ "${?}" -ne 0 ]]; then
    echo 'ERROR on compilattion'
    exit 1
fi

vsql -U "${USR}" -w "${PWD}" << '\q'
-- install function
DROP LIBRARY IF EXISTS AnalyticFunctions CASCADE;
CREATE LIBRARY AnalyticFunctions AS '/home/dbadmin/MyUDx.so';
CREATE FUNCTION jsonify AS LANGUAGE 'C++' NAME 'RowToJSONFactory' LIBRARY AnalyticFunctions;

-- test data
create table T (id int, flag boolean, "date" date, str varchar(80));

copy T from stdin enclosed by '"';
1|true|1970-01-01|some string
2||2017-01-01|Invalid input syntax for integer
3|false||Quick reference.
4|true|2000-02-28|""
5|true|2010-04-01|
|false|2012-11-15|foo bar egg
|||
\.

select jsonify(*) from t;

drop table T;

-- exit from vsql
\q

exit 0
