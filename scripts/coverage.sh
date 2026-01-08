#!/bin/bash
# Generate code coverage report using lcov

set -e

BUILD_DIR=${1:-build}

if [ ! -d "$BUILD_DIR" ]; then
    echo "Build directory $BUILD_DIR does not exist"
    exit 1
fi

echo "Generating coverage report..."

# Create coverage directory
mkdir -p "$BUILD_DIR/coverage"

# Capture coverage data
lcov --capture \
     --directory "$BUILD_DIR" \
     --output-file "$BUILD_DIR/coverage/coverage.info" \
     --rc lcov_branch_coverage=1

# Remove external libraries and test code from coverage
lcov --remove "$BUILD_DIR/coverage/coverage.info" \
     '/usr/*' \
     '*/googletest/*' \
     '*/test/*' \
     --output-file "$BUILD_DIR/coverage/coverage_filtered.info" \
     --rc lcov_branch_coverage=1

# Generate HTML report
genhtml "$BUILD_DIR/coverage/coverage_filtered.info" \
        --output-directory "$BUILD_DIR/coverage/html" \
        --title "IRATA2 Code Coverage" \
        --legend \
        --rc lcov_branch_coverage=1

# Print summary
echo ""
echo "Coverage report generated at: $BUILD_DIR/coverage/html/index.html"
echo ""
lcov --summary "$BUILD_DIR/coverage/coverage_filtered.info" \
     --rc lcov_branch_coverage=1
