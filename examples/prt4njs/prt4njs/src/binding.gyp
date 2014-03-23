{
	'targets': [ {
		'target_name': 'prt4njs',
		'sources': [ 'prt4njs.cc' ],
		'include_dirs': [ '${CURDIR}/../../../../prt/include' ],
		'libraries': [ '-L ${CURDIR}/../../../../prt/bin', '-lcom.esri.prt.core' ],
		'cflags!': [ '-fno-exceptions' ],
		'cflags_cc!': [ '-fno-exceptions' ],
		'conditions': [
			[ 'OS=="linux"', {
				'cflags': [ '-fPIC' ],
				'cflags_cxx': [ '-fPIC' ],
				'ldflags': [ '-fPIC' ],
				'ldflags_cc': [ '-fPIC' ],
			} ],
			['OS=="mac"', {
				'xcode_settings': {
					'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
					'OTHER_CFLAGS': [ '-frtti' ],
					'OTHER_LDFLAGS': [ '-rpath ../../prt/bin' ]
				}
			} ]
		]
	} ]
}