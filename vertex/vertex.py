#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
"""vertex.py - convert your SQL table or query to JSON/CSV/XML format."""

# python stdlib imports
import csv
import sys
import json
import argparse
from itertools import izip
from datetime import time, date, datetime

# The HPE Vertica Python driver
import hp_vertica_client as driver


__author__ = 'Daniel Leybovich'
__version__ = '0.0.1a'


def parse_args():
    """Parse sys.argv options."""
    parser = argparse.ArgumentParser(
        description=__doc__,
        prog='vertex.py',
        conflict_handler='resolve', 
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )

    # CONNECTION OPTIONS
    parser.add_argument('-d', '--dbname', dest='database', required=False,
        help='the name of your HP Vertica database.')

    parser.add_argument('-U', '--username', dest='user', default='dbadmin',
        help='your HP Vertica user name.')

    parser.add_argument('-w', dest='password', required=True,
        help='the password for the user\'s account.')

    parser.add_argument('-h', '--host', dest='host', default='localhost',
        help='the name of the host.') 

    parser.add_argument('-p', '--port', dest='port', default=5433, type=int,
        help='the port number on which HP Vertica listens.')

    # TODO: sslmode, sessionlabel, connsettings

    # INPUT AND OUTPUT OPTIONS
    parser.add_argument('-c', '--command', dest='command', required=True,
        help='query to exeport.')

    parser.add_argument('-F', '--format', dest='format', default='json',
        choices=['csv', 'json', 'xml'], help='output format for query.')

    parser.add_argument('-o', '--output', dest='filename', required=False,
        help='writes all query output into file filename.')

    # GENERAL OPTIONS
    parser.add_argument('-V', '--version', action='version', 
        version='%(prog)s {}'.format(__version__))

    return parser.parse_args()


def json_serial(field):
    """JSON serializer for objects not serializable by default json code"""
    if isinstance(field, (time, date)):
        return field.isoformat()
    raise TypeError("Type %s not serializable" % type(field))
    

def to_json(cursor):
    """Convert your SQL table or query to JSON format."""
    totalrows = cursor.rowcount
    if not totalrows: # no rows => exit or return [] ?
        return
    colnames =  tuple(colmeta.name for colmeta in cursor.description)
    if len(colnames) != len(set(colnames)):
        raise ValueError('columns names are not unique')
    writer = sys.stdout.write # or file
    writer('[')
    for rownum in xrange(totalrows): # avoid cursor.fetchall() 
        row = cursor.fetchone()
        writer(json.dumps(dict(izip(colnames, row)), default=json_serial))
        if rownum == totalrows - 1:
            break
        writer(',\n')
    writer(']\n')


def to_csv(cursor):
    pass


def to_html(cursor):
    pass


def to_xml(cursor):
    pass


if __name__ == '__main__':
    args = parse_args()
    database, user, password, host, port = args.database, args.user, args.password, args.host, args.port
    query = args.command

    db = driver.connect(database=database, user=user, password=password, host=host, port=port)
    cursor = db.cursor()
    cursor.execute(query)

    if args.format == 'json':
        to_json(cursor)
    elif args.format == 'csv':
        to_csv(cursor)
    elif args.format == 'xml':
        to_xml(cursor)
    else:
        to_html(cursor)

    cursor.close()
    db.close()

