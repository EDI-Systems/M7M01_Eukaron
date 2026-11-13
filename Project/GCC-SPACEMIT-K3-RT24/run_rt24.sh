#!/bin/bash
set -eu
cd "$(dirname "$0")"
make
ostool board run \
    --config ostool-build.toml \
    --board-config ostool-board.toml
