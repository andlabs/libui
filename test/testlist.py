# 20 january 2020
# note: python 3

import abc
import fileinput
import re
import sys

def errf(fmt, *args):
	print(fmt.format(*args), file = sys.stderr)

class command(metaclass = abc.ABCMeta):
	@classmethod
	@abc.abstractmethod
	def name(cls):
		raise NotImplementedError

	@classmethod
	@abc.abstractmethod
	def usageString(cls):
		raise NotImplementedError

	# returns the list of filenames to iterate through
	# returns None if there was an error parsing arguments
	@abc.abstractmethod
	def processArgs(self, args):
		raise NotImplementedError

	@abc.abstractmethod
	def run(self, casenames):
		raise NotImplementedError

class listCommand:
	@classmethod
	def name(cls):
		return 'list'

	@classmethod
	def usageString(cls):
		return 'list [source-files...]'

	def processArgs(self, args):
		return args

	def run(self, casenames):
		for x in casenames:
			print('Test' + x)
command.register(listCommand)

class headerCommand:
	filename = None

	@classmethod
	def name(cls):
		return 'header'

	@classmethod
	def usageString(cls):
		return 'header header-file [source-files...]'

	def processArgs(self, args):
		if len(args) < 1:
			errf('error: output filename missing')
			return None
		self.filename = args[0]
		return args[1:]

	def run(self, casenames):
		# TODO
		raise NotImplementedError
command.register(headerCommand)

commands = [
	listCommand,
	headerCommand,
]

def usage():
	errf('usage:')
	errf('{} help', sys.argv[0])
	for cmd in commands:
		errf('{} {}', sys.argv[0], cmd.usageString())
	sys.exit(1)

def main():
	r = re.compile('^(?:Test|TestNoInit)\(([A-Za-z0-9_]+)\)$')

	if len(sys.argv) < 2:
		usage()
	cmdname = sys.argv[1]
	if cmdname == 'help':
		usage()
	cmdtype = None
	for cmd in commands:
		if cmd.name() == cmdname:
			cmdtype = cmd
			break
	if cmdtype is None:
		errf('error: unknown command {}', cmdname)
		usage()

	cmd = cmdtype()
	files = cmd.processArgs(sys.argv[2:])
	if files is None:
		usage()

	casenames = []
	for line in fileinput.input(files = files):
		match = r.match(line)
		if match is not None:
			casenames.append(match.group(1))
	cmd.run(casenames)

main()
