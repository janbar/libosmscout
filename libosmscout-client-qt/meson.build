cppArgs = []

if get_option('default_library')=='shared'
  cppArgs += ['-DOSMScoutClientQt_EXPORTS']

  if haveVisibility
    cppArgs += ['-fvisibility=hidden']
  endif
endif

subdir('include')
subdir('include/osmscoutclientqt')
subdir('include/osmscoutclientqt/private')
subdir('src')

mocHeaders = []

headerTemplate='@0@/@1@'
foreach hdr : osmscoutclientqtHeader
  mocHeaders += headerTemplate.format('include/',hdr)
endforeach

  qtvers = qtClientDep.version().split('.')
  moc_defines = [
    '-DQT_VERSION_MAJOR=' + qtvers[0],
    '-DQT_VERSION_MINOR=' + qtvers[1],
    '-DQT_VERSION_PATCH=' + qtvers[2]
    ]

  mocFiles = qt.preprocess(moc_extra_arguments : moc_defines,
                           moc_headers : mocHeaders,
                           include_directories: include_directories('include'))

  osmscoutclientqt = library('osmscout_client_qt',
                             mocFiles,
                             osmscoutclientqtSrc,
                             include_directories: [osmscoutclientqtIncDir, osmscoutclientIncDir, osmscoutmapqtIncDir, osmscoutmapIncDir, osmscoutIncDir],
                             cpp_args: cppArgs,
                             dependencies: [mathDep, threadDep, qtClientDep],
                             link_with: [osmscout, osmscoutmap, osmscoutmapqt, osmscoutclient],
                             version: libraryVersion,
                             install: true)

pkg = import('pkgconfig')
pkg.generate(osmscoutclientqt)
