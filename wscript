def set_options(opt):
    opt.tool_options('compiler_cxx')

def configure(conf):
    conf.check_tool('compiler_cxx')
    conf.check_tool('node_addon')
    conf.env.CPPPATH_MONOME = ['/usr/local/include/monome']

    conf.check(
       lib          = 'monome',
       libpath      = ['/usr/lib', '/usr/local/lib'],
       uselib_store ='LIBMONOME'
    )

def build(bld):
    tgen = bld.new_task_gen('cxx', 'shlib', 'node_addon')
    tgen.find_sources_in_dirs(['.', 'src'])
    
    tgen.cxxflags = ['-g', '-D_FILE_OFFSET_BITS=64', '-D_LARGEFILE_SOURCE', '-Wall']
    tgen.target   = 'monome'
    tgen.includes = ['/usr/local/include/monome']
    tgen.uselib   = 'LIBMONOME'
