{
  'targets': [
    {
      'target_name': 'node-libsamplerate-native',
      "defines": [
        "NAPI_VERSION=<(napi_build_version)",
      ],
      'sources': [ 'src/binding.cc', 'src/node-libsamplerate.cc' ],
      'include_dirs': ["<!@(node -p \"require('node-addon-api').include\")", 
                       "<!(node -p \"require('process').cwd()+'/deps/include'\")"],
      'actions':[
        {
          'action_name':'copylib',
          'inputs': ["<!(node -p \"require('process').cwd()+'/lib'\")"],
          'outputs': ["<!(node -p \"require('process').cwd()+'/lib/libsamplerate.a'\")"],
          'action': ["node", "<!(node -p \"require('process').cwd()+'/lib/copylib.js'\")"]
        }
      ],
      'dependencies': ["<!(node -p \"require('node-addon-api').gyp\")"],
      'libraries': ["<!(node -p \"require('process').cwd()+'/lib/libsamplerate.a'\")"],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      'xcode_settings': {
        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
        'CLANG_CXX_LIBRARY': 'libc++',
        'MACOSX_DEPLOYMENT_TARGET': '11.0'
      },
      'msvs_settings': {
        'VCCLCompilerTool': { 'ExceptionHandling': 1 },
      }
    }
  ]
}