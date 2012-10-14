
import os.path

stdenv = Environment(LIBPATH = '/usr/lib', 
        LIBS= ['boost_system', 'boost_filesystem', 'boost_thread', 'boost_program_options'],
        CCFLAGS = '-std=c++0x',
        CPPATH = ['/usr/include' 'src'])

Export('stdenv')

SConscript('src/SConscript', variant_dir='debug')

