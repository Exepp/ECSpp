export EPP_COVERAGE=1
./makeSolution_Linux.sh && make Tests -B && ./runTests.sh
gcovr -r . -f include/ECSpp/Component.h -f include/ECSpp/EntityManager.h -f \
    include/ECSpp/internal/ --html --html-details -o docs/coverage.html