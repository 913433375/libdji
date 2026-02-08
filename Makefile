.PHONY: format

format:
	find . -type f \( -name "*.cpp" -o -name "*.h" \) ! -path "*/build-local/*" ! -path "*/build/*" -exec clang-format -i {} \;
