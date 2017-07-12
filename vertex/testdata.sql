

DROP TABLE IF EXISTS public.DATATYPES;


CREATE TABLE public.DATATYPES
(
    "Boolean" boolean,
    "Char" char(1),
    Date date,
    "Float" float,
    "Integer" int,
    IntervalDay interval day,
    IntervalDayHour interval day to hour,
    IntervalDayMinute interval day to minute,
    IntervalDaySecond interval,
    IntervalHour interval hour,
    IntervalHourMinute interval hour to minute,
    IntervalHourSecond interval hour to second,
    IntervalMinute interval minute,
    IntervalMinuteSecond interval minute to second,
    IntervalMonth interval month,
    IntervalSecond interval second,
    IntervalYear interval year,
    IntervalYearMonth interval year to month,
    LongVarchar long varchar(1048576),
    "Numeric" numeric(37,15),
    "Time" time,
    "TimeTz" timetz,
    "Timestamp" timestamp,
    "TimestampTz" timestamptz,
    "Varchar" varchar(80)
);


COPY public.DATATYPES FROM STDIN DIRECT ABORT ON ERROR;
0|a|2000-01-01|0.3|42|5d|5d3h|3d10h30m|1d12h30m30s|12h|12h30m|12h30m30s|15m|15m30s|1m|200s|1y|1y6m|unicode symbols- ⛁⛁⛁|3.1415|10:00:00|10:00:00 IDT|2000-01-01 12:30:30|2000-01-01 12:30:30 IDT|simple string
1|b|2000-01-01|0.3|42|5d|5d3h|3d10h30m|1d12h30m30s|12h|12h30m|12h30m30s|15m|15m30s|1m|200s|1y|1y6m|unicode russian - привет|3.1415|10:00:00|10:00:00 IDT|2000-01-01 12:30:30|2000-01-01 12:30:30 IDT|string
|a|2000-01-01|0.3||5d|5d3h|3d10h30m|1d12h30m30s|12h|12h30m|12h30m30s|15m|15m30s|1m||1y|1y6m|unicode hebrew - שלום|3.1415|10:00:00|10:00:00 IDT|2000-01-01 12:30:30||simple_str
\.

\q
