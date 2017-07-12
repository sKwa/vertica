# VertEx - Vert(ica) Ex(port)

> The real problem is that programmers have spent far too much time worrying
> about efficiency in the wrong places and at the wrong times; premature
> optimization is the root of all evil (or at least most of it) in programming. 
> **Donald Knuth**


`vertex.py` - convert your SQL table or query to JSON/CSV/XML format.


**Supported data types**:

* Boolean
* Char
* Date
* Float
* Integer
* Interval Day
* Interval Day to Hour
* Interval Day to Minute
* Interval Day to Second
* Interval Hour
* Interval Hour to Minute
* Interval Hour to Second
* Interval Minute
* Interval Minute to Second
* Interval Month
* Interval Second
* Interval Year
* Interval Year to Month
* Long Varchar
* Numeric
* Time
* TimeTz
* Timestamp
* TimestampTz
* Varchar

**Help**
```
$ ./vertex.py --help
usage: vertex.py [--help] [-d DATABASE] [-U USER] -w PASSWORD [-h HOST]
                 [-p PORT] -c COMMAND [-F {csv,json,xml}] [-o FILENAME] [-V]

vertex.py - convert your SQL table or query to JSON/CSV/XML format.

optional arguments:
  --help                show this help message and exit
  -d DATABASE, --dbname DATABASE
                        the name of your HP Vertica database. (default: None)
  -U USER, --username USER
                        your HP Vertica user name. (default: dbadmin)
  -w PASSWORD           the password for the user's account. (default: None)
  -h HOST, --host HOST  the name of the host. (default: localhost)
  -p PORT, --port PORT  the port number on which HP Vertica listens. (default:
                        5433)
  -c COMMAND, --command COMMAND
                        query to exeport. (default: None)
  -F {csv,json,xml}, --format {csv,json,xml}
                        output format for query. (default: json)
  -o FILENAME, --output FILENAME
                        writes all query output into file filename. (default:
                        None)
  -V, --version         show program's version number and exit
```

**Example**
```
./vertex.py -U "${DBUSER}" -w "${DBPWD}" -c "select * from T" 
[{"date": "2000-01-01", "id": 1, "val": "rus"},
{"date": "2000-01-01", "id": 1, "val": "usa"},
{"date": "2000-01-02", "id": 1, "val": "usa"},
{"date": "2000-01-03", "id": 1, "val": "eng"},
{"date": "2000-01-01", "id": 2, "val": "afg"},
{"date": "2000-01-02", "id": 2, "val": "eng"},
{"date": null, "id": 4, "val": null}]
```

