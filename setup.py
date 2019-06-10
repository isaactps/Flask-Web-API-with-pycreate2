##############################################
# The MIT License (MIT)
# Copyright (c) 2017 Kevin Walchko
# Copyright (c) 2019 Isaac Tan
# see LICENSE for full details
##############################################

from __future__ import print_function
from setuptools import setup
from pycreate2.version import __version__ as VERSION
from build_utils import BuildCommand
from build_utils import PublishCommand
from build_utils import BinaryDistribution


PACKAGE_NAME = 'flask-with-pycreate2'
BuildCommand.pkg = PACKAGE_NAME
PublishCommand.pkg = PACKAGE_NAME
PublishCommand.version = VERSION


setup(
	author='Isaac Tan',
	author_email='',
	name=PACKAGE_NAME,
	version=VERSION,
	description='A library to control mBot Ranger robots using Python Flask to provide a Web API for invoking the pycreate2 package developed by Kevin Walchko',
	url='http://github.com/isaactps/{}'.format(PACKAGE_NAME),
	classifiers=[
		'Development Status :: 1 - Beta',
		'Intended Audience :: Developers',
		'License :: OSI Approved :: MIT License',
		'Operating System :: Ubuntu',
		'Programming Language :: Python :: 3.6',
		'Topic :: Software Development :: Libraries',
		'Topic :: Software Development :: Libraries :: Python Modules',
		'Topic :: Software Development :: Libraries :: Application Frameworks'
	],
	license='MIT',
	keywords=['mBot Ranger', 'pycreate2', 'api', 'flask', 'library', 'robotics', 'robot'],
	packages=[PACKAGE_NAME],
	install_requires=open('requirements.txt').readlines(),
,
	scripts=[]
)
