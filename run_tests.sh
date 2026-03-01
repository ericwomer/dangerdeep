#!/usr/bin/env bash
# Script de tests para Danger from the Deep
# - Test de compilación del proyecto
# - Opcional: test de capacidades OpenGL (dftdtester, si está disponible)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

log_ok()   { echo -e "${GREEN}[OK]${NC} $*"; }
log_fail() { echo -e "${RED}[FALLO]${NC} $*"; }
log_info() { echo -e "${YELLOW}[INFO]${NC} $*"; }

run_build_test() {
	log_info "Test de compilación..."
	mkdir -p "$BUILD_DIR"
	cd "$BUILD_DIR"
	if ! cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo -q 2>/dev/null; then
		cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
	fi
	if make -j$(nproc 2>/dev/null || echo 2); then
		log_ok "Compilación del proyecto correcta."
		return 0
	else
		log_fail "La compilación falló."
		return 1
	fi
}

run_opengl_test() {
	local gltest="${BUILD_DIR}/dftdtester"
	if [[ -x "$gltest" ]]; then
		log_info "Ejecutando test de capacidades OpenGL (dftdtester)..."
		if "$gltest"; then
			log_ok "Test OpenGL finalizado sin errores."
			return 0
		else
			log_fail "El test OpenGL reportó problemas."
			return 1
		fi
	else
		log_info "Ejecutable dftdtester no encontrado (opcional). Omitting OpenGL test."
		return 0
	fi
}

# --- main ---
cd "$SCRIPT_DIR"
FAIL=0

if ! run_build_test; then
	FAIL=1
fi

# Si se pasa -gl o --opengl, intentar ejecutar el tester de OpenGL
if [[ "$1" == "-gl" || "$1" == "--opengl" ]]; then
	if ! run_opengl_test; then
		FAIL=1
	fi
fi

if [[ $FAIL -eq 0 ]]; then
	log_ok "Todos los tests pasaron."
	exit 0
else
	log_fail "Algunos tests fallaron."
	exit 1
fi
