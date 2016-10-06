import os

dir = os.path.dirname(os.path.abspath(__file__))

def FlagsForFile(filename, **kwargs):
	return {
		'flags': [
			'-x', 'c++',
			'-std=c++14',
			'-I', dir + '/include',
			'-I', dir + '/serial/include',
			'-I', dir + '/serial/mstd/include',
		],
		'do_cache': True
	}
