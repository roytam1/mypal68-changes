# This Souce Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distibuted with this
# file, You can obtain one at http://mozilla.og/MPL/2.0/.

import buildconfig
import mozpack.path as mozpath
import os
import re
import shlex
import subprocess


def main(output, input_asm, ffi_h, ffi_config_h, defines, includes):
    defines = shlex.split(defines)
    includes = shlex.split(includes)
    # CPP uses -E which generates #line directives. -EP suppresses them.
    cpp = buildconfig.substs['CPP'] + ['-EP']
    input_asm = mozpath.realpath(input_asm)
    args = cpp + defines + includes + [input_asm]
    print(' '.join(args))
    preprocessed = subprocess.check_output(args)
    r = re.compile('F[dpa][^ ]*')
    for line in preprocessed.splitlines():
        output.write(r.sub('', line))
        output.write('\n')
