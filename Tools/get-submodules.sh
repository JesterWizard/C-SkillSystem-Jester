#!/bin/bash
# Superseded: the external tools are pinned to specific versions, and FE-CLib-Mokha and EventAssembler are
# committed to this repository. Tools/setup.sh fetches the rest at the right versions.
exec "$(dirname "$(readlink -f "$0")")/setup.sh" "$@"
