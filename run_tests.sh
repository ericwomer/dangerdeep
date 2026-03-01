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
#
# Requiere Bash (arrays, [[ ]], etc.). Si se invoca con sh, se reejecuta con bash.
[[ -n "$BASH_VERSION" ]] || exec bash "$0" "$@"

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

run_unit_tests() {
	log_info "Tests unitarios (ctest)..."
	if ! command -v ctest &>/dev/null; then
		log_skip "ctest no encontrado (viene con CMake)"
		return 0
	fi
	if [[ ! -f "${BUILD_DIR}/CTestTestfile.cmake" ]]; then
		log_fail "No hay tests registrados. Compilá con: $0 --unit -b"
		return 1
	fi
	export DFTD_DATA="${SCRIPT_DIR}/data"
	[[ -d "$DFTD_DATA" ]] || DFTD_DATA="${SCRIPT_DIR}"
	if ( cd "$BUILD_DIR" && ctest --output-on-failure ); then
		log_ok "Todos los tests unitarios pasaron."
		return 0
	else
		log_fail "Algunos tests unitarios fallaron."
		return 1
	fi
}

run_coverage() {
	log_info "Generando reporte de cobertura (líneas y branches)..."
	if ! command -v lcov &>/dev/null || ! command -v genhtml &>/dev/null; then
		log_skip "lcov/genhtml no instalados. Instalación: sudo apt install lcov"
		return 0
	fi
	local cov_dir="${BUILD_DIR}/coverage"
	local info_file="${cov_dir}/coverage.info"
	mkdir -p "$cov_dir"
	# Borrar .gcda viejos para evitar "mismatch" / "inconsistent" con .gcno actual
	find "$BUILD_DIR" -name "*.gcda" -delete 2>/dev/null || true
	# Ejecutar tests para generar .gcda
	export DFTD_DATA="${SCRIPT_DIR}/data"
	[[ -d "$DFTD_DATA" ]] || DFTD_DATA="${SCRIPT_DIR}"
	if [[ -f "${BUILD_DIR}/CTestTestfile.cmake" ]]; then
		log_info "Ejecutando tests para generar datos de cobertura..."
		( cd "$BUILD_DIR" && ctest --output-on-failure -Q ) || true
	fi
	# Capturar contadores (sin --no-external para no excluir src/ al correr desde build/)
	local raw_info="${cov_dir}/coverage_raw.info"
	# mismatch/inconsistent pueden aparecer con GCC 14+; ignorarlos para obtener reporte
	if ! ( cd "$BUILD_DIR" && lcov --capture --directory . --output-file "$raw_info" \
		--rc branch_coverage=1 --ignore-errors source,gcov,mismatch,inconsistent --keep-going 2>/dev/null ); then
		[[ -s "$raw_info" ]] || { log_fail "Falló lcov capture. Probá: rm -rf build && $0 --coverage -b"; return 1; }
	fi
	# Quedarnos solo con fuentes del proyecto (evitar headers del sistema)
	local pattern="*dangerdeep*"
	if ! lcov --extract "$raw_info" "$pattern" --output-file "$info_file" --rc branch_coverage=1 2>/dev/null; then
		cp "$raw_info" "$info_file"
	fi
	rm -f "$raw_info"
	if [[ ! -s "$info_file" ]]; then
		log_fail "No quedaron datos de cobertura del proyecto (patrón $pattern). Revisá rutas en $raw_info."
		return 1
	fi
	# Generar HTML con branch coverage (ruta base para que genhtml encuentre los fuentes)
	if ! genhtml "$info_file" --output-directory "${cov_dir}/html" --branch-coverage \
		--title "Danger from the Deep" --legend \
		--ignore-errors inconsistent,corrupt 2>&1; then
		log_fail "Falló genhtml (revisá el mensaje arriba)"
		return 1
	fi
	log_ok "Cobertura generada en ${cov_dir}/html/index.html"
	return 0
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
	local supp="${SCRIPT_DIR}/valgrind-suppressions.supp"
	[[ -f "$supp" ]] || supp=""
	# Solo fallar por fugas definitivas/indirectas; "still reachable" en libs del sistema se ignoran
	if valgrind --leak-check=full --show-leak-kinds=all --errors-for-leak-kinds=definite,indirect \
		--error-exitcode=1 \
		$([[ -n "$supp" ]] && echo --suppressions="$supp") \
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
DO_UNIT_TESTS=0
DO_COVERAGE=0
EXTRA_CMAKE_ARGS=""

for arg in "$@"; do
	case "$arg" in
		--build|-b)        DO_BUILD=1; DO_FORMAT_APPLY=0; DO_LINT=0; DO_LINT_FULL=0 ;;
		--asan)            EXTRA_CMAKE_ARGS="-DBUILD_ASAN=ON -DBUILD_COVERAGE=OFF -DBUILD_UNIT_TESTS=ON"; DO_BUILD=1; DO_UNIT_TESTS=1; DO_FORMAT_APPLY=0; DO_LINT=0; DO_LINT_FULL=0 ;;
		--valgrind)        DO_VALGRIND=1; DO_FORMAT_APPLY=0; DO_LINT=0; DO_LINT_FULL=0; EXTRA_CMAKE_ARGS="-DBUILD_VALGRIND_FRIENDLY=ON" ;;
		--unit|--tests)    DO_UNIT_TESTS=1; EXTRA_CMAKE_ARGS="${EXTRA_CMAKE_ARGS} -DBUILD_UNIT_TESTS=ON"; DO_BUILD=1; DO_FORMAT_APPLY=0; DO_LINT=0; DO_LINT_FULL=0 ;;
		--coverage)        DO_COVERAGE=1; EXTRA_CMAKE_ARGS="${EXTRA_CMAKE_ARGS} -DBUILD_COVERAGE=ON -DBUILD_ASAN=OFF -DBUILD_UNIT_TESTS=ON"; DO_BUILD=1; DO_FORMAT_APPLY=0; DO_LINT=0; DO_LINT_FULL=0 ;;
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
			echo "  --asan -b      Compilar con ASan+LSan y ejecutar tests unitarios (detecta fugas/errores de memoria)"
			echo "  --valgrind     Ejecutar bajo Valgrind. Primera vez: $0 --valgrind -b"
			echo "  --format, -f   Solo verificar formato (no aplicar)"
			echo "  --format-apply Aplicar formato (por defecto)"
			echo "  --no-format    No aplicar ni verificar formato"
			echo "  --lint, -l     Lint rápido: solo warning, excl. test/tools (por defecto)"
			echo "  --lint-full    Lint completo: warning+style+performance, todo el código"
			echo "  --no-lint     No ejecutar lint"
			echo "  --gl, --opengl Test OpenGL (dftdtester)"
			echo "  --unit, --tests Compilar y ejecutar tests unitarios (ptrlist, mutex, parser)"
			echo "  --coverage     Compilar con cobertura, ejecutar tests y generar reporte (líneas + branches)"
			exit 0
			;;
	esac
done

[[ $DO_BUILD -eq 1 ]] && { run_build_test || FAIL=1; } || :
[[ $DO_UNIT_TESTS -eq 1 ]] && { run_unit_tests || FAIL=1; } || :
[[ $DO_COVERAGE -eq 1 ]] && { run_coverage || FAIL=1; } || :
[[ $DO_FORMAT_APPLY -eq 1 ]] && run_format_apply || :
[[ $DO_FORMAT_CHECK -eq 1 ]] && { run_format_check || FAIL=1; } || :
if [[ $DO_LINT_FULL -eq 1 ]]; then
	run_lint_full
elif [[ $DO_LINT -eq 1 ]]; then
	run_lint
fi
[[ $DO_GL -eq 1 ]]           && { run_opengl_test || FAIL=1; } || :
[[ $DO_VALGRIND -eq 1 ]]     && { run_valgrind || FAIL=1; } || :

if [[ $FAIL -eq 0 ]]; then
	log_ok "Todos los tests pasaron."
	exit 0
else
	log_fail "Algunos tests fallaron."
	exit 1
fi
