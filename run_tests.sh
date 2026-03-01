#!/usr/bin/env bash
# Script de tests para Danger from the Deep
# Por defecto: aplicar formato (clang-format) y luego lint (cppcheck).
# La compilación solo se ejecuta con --build.
#
# Uso:
#   ./run_tests.sh              # formato + lint rápido (por defecto)
#   ./run_tests.sh --build, -b  # solo compilación (sin formato ni lint)
#   ./run_tests.sh --asan -b    # compilar con AddressSanitizer+LeakSanitizer y compilar
#   ./run_tests.sh --valgrind   # ejecutar el juego bajo Valgrind (fugas de memoria)
#   ./run_tests.sh --lint-full  # lint completo
#   ./run_tests.sh --gl         # test OpenGL (requiere haber compilado antes)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
SRC_DIR="${SCRIPT_DIR}/src"
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

log_ok()   { echo -e "${GREEN}[OK]${NC} $*"; }
log_fail() { echo -e "${RED}[FALLO]${NC} $*"; }
log_info() { echo -e "${YELLOW}[INFO]${NC} $*"; }
log_skip() { echo -e "${YELLOW}[OMIT]${NC} $*"; }

run_build_test() {
	log_info "Test de compilación..."
	mkdir -p "$BUILD_DIR"
	cd "$BUILD_DIR"
	local cmake_opts=(-DCMAKE_BUILD_TYPE=RelWithDebInfo)
	[[ -n "$EXTRA_CMAKE_ARGS" ]] && cmake_opts+=($EXTRA_CMAKE_ARGS)
	if ! cmake .. "${cmake_opts[@]}" -q 2>/dev/null; then
		cmake .. "${cmake_opts[@]}"
	fi
	if make -j$(nproc 2>/dev/null || echo 2); then
		log_ok "Compilación del proyecto correcta."
		return 0
	else
		log_fail "La compilación falló."
		return 1
	fi
}

run_format_check() {
	log_info "Verificando formato (clang-format)..."
	if ! command -v clang-format &>/dev/null; then
		log_skip "clang-format no instalado. Instalación: sudo apt install clang-format"
		return 0
	fi
	local files
	files=$(find "$SRC_DIR" -type f \( -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) 2>/dev/null | sort)
	if [[ -z "$files" ]]; then
		log_skip "No se encontraron fuentes en $SRC_DIR"
		return 0
	fi
	local out
	out=$(echo "$files" | xargs -r clang-format --dry-run --Werror 2>&1) || true
	if [[ $? -ne 0 ]]; then
		echo "$out" | head -50
		log_fail "Algunos archivos no cumplen el formato. Ejecuta: ./run_tests.sh --format-apply"
		return 1
	fi
	log_ok "Formato correcto."
	return 0
}

run_format_apply() {
	log_info "Aplicando formato (clang-format)..."
	if ! command -v clang-format &>/dev/null; then
		log_skip "clang-format no instalado. Instalación: sudo apt install clang-format"
		return 0
	fi
	local files
	files=$(find "$SRC_DIR" -type f \( -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) 2>/dev/null | sort)
	if [[ -z "$files" ]]; then
		log_skip "No se encontraron fuentes en $SRC_DIR"
		return 0
	fi
	echo "$files" | xargs -r clang-format -i
	log_ok "Formato aplicado."
	return 0
}

run_lint() {
	log_info "Lint rápido (cppcheck, solo warning)..."
	if ! command -v cppcheck &>/dev/null; then
		log_skip "cppcheck no instalado. Instalación: sudo apt install cppcheck"
		return 0
	fi
	local jobs=$(nproc 2>/dev/null || echo 4)
	local inc=(-I"${SRC_DIR}" -I"${BUILD_DIR}")
	[[ -d "${BUILD_DIR}/src" ]] && inc+=(-I"${BUILD_DIR}/src")
	for d in /usr/include/SDL2 /usr/include/GL /usr/include; do
		[[ -d "$d" ]] && inc+=(-I"$d")
	done
	if cppcheck --quiet -j"$jobs" --enable=warning \
		--suppress=missingInclude \
		--suppress=unmatchedSuppression \
		--inline-suppr \
		--force \
		-i "${SRC_DIR}/oglext" \
		-i "${SRC_DIR}/tinyxml" \
		-i "${SRC_DIR}/test" \
		-i "${SRC_DIR}/tools" \
		-i "${SRC_DIR}/dftdtester" \
		"${inc[@]}" \
		"$SRC_DIR" 2>/dev/null; then
		log_ok "Lint sin errores."
	else
		log_ok "Lint finalizado (revisar salida arriba)."
	fi
	return 0
}

run_lint_full() {
	log_info "Lint completo (cppcheck, warning + style + performance)..."
	if ! command -v cppcheck &>/dev/null; then
		log_skip "cppcheck no instalado. Instalación: sudo apt install cppcheck"
		return 0
	fi
	local jobs=$(nproc 2>/dev/null || echo 4)
	local inc=(-I"${SRC_DIR}" -I"${BUILD_DIR}")
	[[ -d "${BUILD_DIR}/src" ]] && inc+=(-I"${BUILD_DIR}/src")
	for d in /usr/include/SDL2 /usr/include/GL /usr/include; do
		[[ -d "$d" ]] && inc+=(-I"$d")
	done
	if cppcheck --quiet -j"$jobs" --enable=warning,style,performance \
		--suppress=missingInclude \
		--suppress=unmatchedSuppression \
		--inline-suppr \
		--force \
		-i "${SRC_DIR}/oglext" \
		-i "${SRC_DIR}/tinyxml" \
		"${inc[@]}" \
		"$SRC_DIR" 2>/dev/null; then
		log_ok "Lint completo sin errores."
	else
		log_ok "Lint completo finalizado (revisar salida arriba)."
	fi
	return 0
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
		log_skip "Ejecutable dftdtester no encontrado (opcional)."
		return 0
	fi
}

run_valgrind() {
	log_info "Ejecutando con Valgrind (detección de fugas de memoria)..."
	if ! command -v valgrind &>/dev/null; then
		log_skip "valgrind no instalado. Instalación: sudo apt install valgrind"
		return 0
	fi
	local exe="${BUILD_DIR}/src/dangerdeep"
	[[ -x "$exe" ]] || exe="${BUILD_DIR}/dangerdeep"
	if [[ ! -x "$exe" ]]; then
		log_fail "Ejecutable no encontrado. Compilá con: $0 --valgrind -b"
		return 1
	fi
	log_info "Ejecutá el juego y cerrá la ventana para ver el reporte de fugas."
	log_info "Si aparece 'Instrucción ilegal', compilá antes con: $0 --valgrind -b"
	local datadir="${SCRIPT_DIR}/data"
	[[ -d "$datadir" ]] || datadir="${SCRIPT_DIR}"
	# Si los assets son punteros Git LFS (no se descargaron), avisar
	if [[ -f "${datadir}/fonts/font_arial.png" ]] && head -c 40 "${datadir}/fonts/font_arial.png" 2>/dev/null | grep -q "git-lfs"; then
		log_fail "Los archivos de datos son punteros Git LFS (no son PNG reales). Ejecutá: git lfs pull"
		return 1
	fi
	if valgrind --leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=all \
		--error-exitcode=1 \
		--log-file="${BUILD_DIR}/valgrind.log" \
		"$exe" --datadir "${datadir}/" 2>&1; then
		log_ok "Valgrind: no se reportaron fugas."
		return 0
	else
		log_fail "Valgrind reportó fugas o errores. Ver ${BUILD_DIR}/valgrind.log"
		return 1
	fi
}

# --- main ---
cd "$SCRIPT_DIR"
FAIL=0
DO_BUILD=0
DO_FORMAT_CHECK=0
DO_FORMAT_APPLY=1
DO_LINT=1
DO_LINT_FULL=0
DO_GL=0
DO_VALGRIND=0
EXTRA_CMAKE_ARGS=""

for arg in "$@"; do
	case "$arg" in
		--build|-b)        DO_BUILD=1; DO_FORMAT_APPLY=0; DO_LINT=0; DO_LINT_FULL=0 ;;
		--asan)            EXTRA_CMAKE_ARGS="-DBUILD_ASAN=ON"; DO_BUILD=1; DO_FORMAT_APPLY=0; DO_LINT=0; DO_LINT_FULL=0 ;;
		--valgrind)        DO_VALGRIND=1; DO_FORMAT_APPLY=0; DO_LINT=0; DO_LINT_FULL=0; EXTRA_CMAKE_ARGS="-DBUILD_VALGRIND_FRIENDLY=ON" ;;
		--format|-f)      DO_FORMAT_CHECK=1; DO_FORMAT_APPLY=0 ;;
		--format-apply)   DO_FORMAT_APPLY=1 ;;
		--no-format)      DO_FORMAT_APPLY=0 ;;
		--lint|-l)        DO_LINT=1; DO_LINT_FULL=0 ;;
		--lint-full)      DO_LINT=0; DO_LINT_FULL=1 ;;
		--no-lint)        DO_LINT=0; DO_LINT_FULL=0 ;;
		--gl|--opengl)    DO_GL=1 ;;
		-h|--help)
			echo "Uso: $0 [opciones]"
			echo "  Por defecto: aplicar formato y lint rápido."
			echo "  --build, -b     Solo compilación (sin formato ni lint)"
			echo "  --asan -b      Compilar con AddressSanitizer+LeakSanitizer (fugas de memoria)"
			echo "  --valgrind     Ejecutar bajo Valgrind. Primera vez: $0 --valgrind -b"
			echo "  --format, -f   Solo verificar formato (no aplicar)"
			echo "  --format-apply Aplicar formato (por defecto)"
			echo "  --no-format    No aplicar ni verificar formato"
			echo "  --lint, -l     Lint rápido: solo warning, excl. test/tools (por defecto)"
			echo "  --lint-full    Lint completo: warning+style+performance, todo el código"
			echo "  --no-lint     No ejecutar lint"
			echo "  --gl, --opengl Test OpenGL (dftdtester)"
			exit 0
			;;
	esac
done

[[ $DO_BUILD -eq 1 ]] && { run_build_test || FAIL=1; } || true
[[ $DO_FORMAT_APPLY -eq 1 ]] && run_format_apply || true
[[ $DO_FORMAT_CHECK -eq 1 ]] && { run_format_check || FAIL=1; } || true
if [[ $DO_LINT_FULL -eq 1 ]]; then
	run_lint_full
elif [[ $DO_LINT -eq 1 ]]; then
	run_lint
fi
[[ $DO_GL -eq 1 ]]           && { run_opengl_test || FAIL=1; } || true
[[ $DO_VALGRIND -eq 1 ]]     && { run_valgrind || FAIL=1; } || true

if [[ $FAIL -eq 0 ]]; then
	log_ok "Todos los tests pasaron."
	exit 0
else
	log_fail "Algunos tests fallaron."
	exit 1
fi
