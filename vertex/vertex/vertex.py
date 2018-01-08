#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""vertex.py - convert your SQL table or query to JSON/CSV/XML format."""

# python stdlib imports
import csv
import sys
import json
import argparse
from itertools import izip
from datetime import time, date, datetime

# The HPE Vertica Python driver
try:
    # vertica 7.2.x, 8.0.x
    import hp_vertica_client as driver
except ImportError:
    # vertica 8.1.x, 9.0.x
    import vertica_db_client as driver


__version__ = '0.0.1a'


class Arguments(argparse.Namespace):
    """Simple object for storing attributes."""

    def __init__(self, **kwargs):
        super(Arguments, self).__init__(**kwargs)

    @property
    def connection_options(self):
        options = dict()
        options['host'] = self.host
        options['port'] = self.port
        if self.database:
            options['database'] = self.database
        options['user'] = self.user
        if self.password:
            options['password'] = self.password
        options['sslmode'] = self.sslmode
        if self.sessionlabel:
            options['sessionlabel'] = self.sessionlabel
        return options

    @property
    def io_options(self):
        return {}

    @property
    def general_options(self):
        return {}


def parse_args(args=None, namespace=Arguments()):
    """Parse sys.argv options."""

    parser = argparse.ArgumentParser(
        prog='vertex.py',
        description=__doc__,
        conflict_handler='resolve')

    # CONNECTION OPTIONS
    group = parser.add_argument_group(title='CONNECTION OPTION',
        description='Specifies all settings required to make a connection.')

    group.add_argument('-h', '--host', dest='host', default='localhost',
        help='the name of the host. (default localhost)') 

    group.add_argument('-p', '--port', dest='port', default=5433, type=int,
        help='the port number on which HP Vertica listens. (default 5433)')

    group.add_argument('-d', '--database', dest='database', required=False,
        help='specifies the name of the database to connect to.')

    group.add_argument('-u', '--user', dest='user', default='dbadmin',
        help='connects to the database as the user username instead of the ' \
             'default user.')

    group.add_argument('-P', '--password', dest='password', required=False,
        help='the password for the user\'s account.')

    group.add_argument('-s', '--sslmode', dest='sslmode', default='prefer',
        choices=['require', 'prefer', 'allow', 'disable'], 
        help='specifies how (or whether) clients use SSL when connecting '   \
             'to servers. The default value is prefer, meaning to use SSL '  \
             'if the server offers it.')

    group.add_argument('-l', '--label', dest='sessionlabel', default='python',
        help='Sets a label for the connection on the server. This value '    \
             'appears in the session_id column of the V_MONITOR.SESSIONS '   \
             'system table.')

    # INPUT AND OUTPUT OPTIONS
    group = parser.add_argument_group(title='INPUT AND OUTPUT OPTIONS',
        description='Specifies options to control how your request is '      \
                    'interpreted and how the response is generated.')

    mutual_args = group.add_mutually_exclusive_group(required=True)

    mutual_args.add_argument('-i', '--input', dest='input',
        help='query to export.')

    mutual_args.add_argument('-t', '--table', dest='table',
        help='table to export.')

    group.add_argument('-o', '--output', dest='filename', required=False,
        help='writes all query output into file filename.')

    group.add_argument('-F', '--format', dest='format', default='json',
        choices=['csv', 'json', 'xml'], help='output format for query.')

    # GENERAL OPTIONS
    group = parser.add_argument_group(title='GENERAL OPTIONS')

    group.add_argument('-V', '--version', action='version', 
        version='%(prog)s {}'.format(__version__))

    group.add_argument('-?', '--help', action='help',
        help='displays help about line arguments and exits.')

    return parser.parse_args(args=args, namespace=namespace)


def json_serial(field):
    """JSON serializer for objects not serializable by default json code"""
    if isinstance(field, (time, date)):
        return field.isoformat()
    raise TypeError("Type %s not serializable" % type(field))


def to_json(cursor):
    """Convert your SQL table or query to JSON format."""
    colnames = tuple(colmeta.name for colmeta in cursor.description)
    row = cursor.fetchone()
    json_data = json.dumps(dict(izip(colnames, row)), default=json_serial)
    yield '[{}'.format(json_data)
    while True:
        row = cursor.fetchone()
        if not row:
            break
        json_data = json.dumps(dict(izip(colnames, row)), default=json_serial)
        yield ',{}'.format(json_data)
    yield ']\n'


def to_xml(cursor):
    """Lets you retrieve data as XML."""
    header = '<?xml version="1.0" encoding="UTF-8" ?>\n<root>\n'
    colnames = tuple(colmeta.name for colmeta in cursor.description)
    yield header
    while True:
        row = cursor.fetchone()
        if not row:
            break
        xml_data = '  <row>\n'
        for (column, value) in izip(colnames, row):
            xml_data += '    <{}>{}</{}>\n'.format(column, value, column)
        yield xml_data + '  </row>\n'
    yield '</root>\n'


def main():
    # parse comman line arguments
    if not sys.argv[1:]:
        parse_args(args=['--help',])
    args = parse_args()
    # estabish connection
    connection = driver.connect(**args.connection_options)
    cursor = connection.cursor()
    # validate query
    if args.input:
        query = args.input
        try:
            cursor.execute('SELECT * FROM ({}) AS T LIMIT 0'.format(query))
        except driver.ProgrammingError as error:
            print error
            sys.exit(-1) # raise invalid query
        cursor.execute(query)
        if cursor.rowcount < 1:
            print 'No data to export'
            sys.exit(0)
        # format ouput
        if args.format == 'json':
            output_stream = to_json(cursor)
        elif args.format == 'csv':
            output_stream = to_csv(cursor)
        elif args.format == 'xml':
            output_stream = to_xml(cursor)
        else: # on case of custom implemintation of arguments parsing
            raise Exception('invalid format {}'.format(args.format))
        # write result
        writer = sys.stdout
        if args.filename:
            writer = open(args.filename, 'wb')
        for line in output_stream:
            writer.write(line)
        writer.close()

    # clean up
    cursor.close()
    connection.close()

    # exit
    sys.exit(0)

if __name__ == '__main__':
    main()

# EOF
