string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S")

configure_file(
    "${VERSION_IN_DIR}/version.cpp.in"
    "${OUT_FILE}"
    @ONLY
)

