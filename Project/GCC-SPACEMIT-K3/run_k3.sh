#!/bin/bash
set -e

# Build and run the RME kernel on SpacemiT K3 (Banana Pi BPI-SM10) via ostool.
cd "$(dirname "$0")"

# Build first so local compile errors surface immediately.
make
ostool board run \
    --config ostool-build.toml \
    --board-config ostool-board.toml
