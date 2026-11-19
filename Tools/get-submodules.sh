#!/bin/bash
# Superseded: every tool the build uses is committed to this repository. Tools/setup.sh checks the setup.
exec "$(dirname "$(readlink -f "$0")")/setup.sh" "$@"
