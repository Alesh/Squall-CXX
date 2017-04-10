import sys
import os.path
from setuptools import setup, Extension

try:
    from Cython.Build import cythonize
except ImportError:
    cythonize = None

if sys.version_info[:2] < (3, 5):
    raise NotImplementedError("Required python version 3.5 or greater")

include_dirs = [os.path.join(os.path.dirname(__file__), "include")]

settings = {
    'name': 'Squall-Core.cython',
    'version': '0.1.dev27',
    'author': 'Alexey Poryadin',
    'author_email': 'alexey.poryadin@gmail.com',
    'description': "This is Squall-Core optimized module which implements"
                   "the cooperative multitasking based on event-driven"
                   "switching async/await coroutines.",
    'namespace_packages': ['squall'],
    'package_dir': {'squall': 'python/squall'},
    'ext_modules': cythonize([Extension('squall.core_cython',
                                        ['python/squall/core_cython.pyx'],
                                        include_dirs=include_dirs,
                                        language="c++", libraries=['ev'],
                                        extra_compile_args=["-std=c++11"])]),
    'zip_safe': False
}

setup(**settings)