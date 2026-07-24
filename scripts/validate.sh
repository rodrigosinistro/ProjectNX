#!/usr/bin/env sh
set -eu

project_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$project_root"

required_files="
README.md
VERSION
Makefile
config.example.ini
include/projectnx/app.h
include/projectnx/auth.h
include/projectnx/config.h
include/projectnx/json.h
include/projectnx/network.h
include/projectnx/xbox.h
source/app.c
source/auth.c
source/config.c
source/json.c
source/main.c
source/network.c
source/xbox.c
tests/test_app.c
tests/stubs/switch.h
docs/ARCHITECTURE.md
docs/ROADMAP.md
docs/SECURITY.md
docs/TESTING_ON_SWITCH.md
"

for required_file in $required_files; do
    if [ ! -f "$required_file" ]; then
        echo "Arquivo ausente: $required_file" >&2
        exit 1
    fi
done

version=$(tr -d '\r\n' < VERSION)
if [ "$version" != "0.4.0" ]; then
    echo "Versao inesperada em VERSION: $version" >&2
    exit 1
fi

if ! grep -q "APP_VERSION   := $version" Makefile; then
    echo "APP_VERSION e VERSION estao divergentes" >&2
    exit 1
fi

make test
echo "ProjectNX validation: OK"
