#!/usr/bin/env python2
# coding: utf-8

import sys

LINE_COMMENT = '--'
COMMENT_INTRODUCER, COMMENT_TERMINATOR = ('/*', '*/')


def strip(query):
    comment = False
    result = []
    for line in query.splitlines():
        line = line.lstrip()
        if line[:2] == '--':
            if not comment:
                continue
        if line[:2] == '/*':
            if not comment:
                comment = True
            continue
        if comment:
            if line[-2:] == '*/':
                comment = False
                continue
            idx = line.find('*/')
            if idx > 0:
                comment = False
                line = line[idx + 2:].lstrip()
        result.append(line)
    return '\n'.join(result)


def test():
    for query in QUERIES:
        print strip(query)
        print '~' * 20


if __name__ == '__main__':
    test()


# EOF
