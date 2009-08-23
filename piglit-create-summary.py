#!/usr/bin/env python
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following
# conditions:
#
# This permission notice shall be included in all copies or
# substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
# KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHOR(S) BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
# AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.


from getopt import getopt, GetoptError
import sys

import framework.core as core



#############################################################################
##### Main program
#############################################################################
def usage():
	USAGE = """\
Usage: %(progName)s [options] [main results file]

Options:
  -h, --help                Show this message

Example:
  %(progName)s results/main > results/summary
"""
	print USAGE % {'progName': sys.argv[0]}
	sys.exit(1)

def main():
	env = core.Environment()

	try:
		options, args = getopt(sys.argv[1:], "h", [ "help" ])
	except GetoptError:
		usage()

	OptionName = ''

	for name, value in options:
		if name in ('-h', '--help'):
			usage()

	if len(args) != 1:
		usage()

	resultsFilename = args[0]

	results = core.loadTestResults(resultsFilename)
	for testname, result in results.allTestResults().items():
		if 'info' in result:
			if len(result['info']) > 4096:
				result['info'] = result['info'][0:4096]
	results.write(sys.stdout)


if __name__ == "__main__":
	main()
