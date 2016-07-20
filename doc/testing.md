
![](images/img.png)

----

# Awa LightweightM2M.
----

## Testing.

### Awa API testing.

The Awa API is covered by a comprehensive test suite based on Google Test.

It is possible to run individual tests for debugging purposes, using the *--gtest_filter* flag. Wildcards (*) can be passed in order to run multiple tests/suits that match the filter.

For example:  ```` --gtest_filter=TestGet.\* ```` runs all tests in the TestGet suite.

Refer to the gtest advanced guide for further details.

Additional output can be displayed by enabling a high log level. The option *--logLevel* can be passed a value between 1 and 4, where 4 is the most verbose.

Running the tests with [*Valgrind*](http://valgrind.org/docs/manual/quick-start.html) is highly recommended to ensure that no memory leaks are introduced.

Examples:

**Note.** Replace *TestSuite.TestName* with the test suite and test you want to run.

To run a single test:
````
$ (cd build && make test_api_runner VERBOSE=1 && valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes apiv2/tests/test_api_runner --logLevel 1 --gtest_filter=TestSuite.TestName)
````

To run a single test specifying an already-running client:
````
$ (cd build && make test_api_runner VERBOSE=1 && valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes apiv2/tests/test_api_runner --logLevel 1 --gtest_filter=TestSuite.TestName --clientIpcPort 12345)
````

To run a test specifying an already-running server:
````
# a custom debug bootstrap file is used so the client daemons connect to the correct server port, refer to api/tests/debug.bsc.
$ (cd build && make test_api_runner VERBOSE=1 && valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes api/tests/test_api_runner --logLevel 1 --gtest_filter=TestSuite.TestName --serverIpcPort 54321 --bootstrapConfig ../api/tests/debug.bsc)
````

### Awa core testing.

To run a test specifying an already-running server:
````
$ (cd build && make test_src_runner VERBOSE=1 && valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes src/tests/test_src_runner --logLevel 1 --gtest_filter=TestSuite.TestName)
````

### Awa tools testing.

Tests are provided that run against the supplied LWM2M tools. These tests help ensure that the tools perform correctly and produce the expected output, including error codes. These tests are written in Python and depend on the [*nosetests*](http://pythontesting.net/framework/nose/nose-introduction/) package.

For debian-based systems run:
````
$ sudo apt-get install python-nose
````

Then, run the tests using:
````
$ cd tools/tests
$ ./run_tests
````

### SDK testing.

Awa LWM2M contains a number of unit test cases. These can be executed by running the following command from the root of the repository:
````
$ make tests
````

Google Test (gtest) is used to write C++ tests against core and API code. These can be run separately with:
````
$ make gtest_tests
````

Individual tests can be run by specifying them on the command line as an argument to TEST_FILTER:
````
$ make gtest_tests TEST_FILTER=ObjectStoreInterfaceTestSuite.test_WriteAndReadSingleResource
````

Multiple tests can be run by separating them with a colon:
````
$ make gtest_tests TEST_FILTER=ObjectStoreInterfaceTestSuite.test_WriteAndReadSingleResource:TlvTestSuite.test_serialise_float32
````
 Groups of tests can be run with a wildcard (asterisk) character:
````
$ make gtest_tests TEST_FILTER=FlowDmLowBasicTestSuite.test_encode_and_extract_*     # runs all tests in this suite beginning with "test_encode_and_extract_"
$ make gtest_tests TEST_FILTER=FlowDmLowBasicTestSuite.*                             # runs all tests in this suite
````

Debugging tests can be run with the *gdb_tests* target. This invokes *gdb* when a test fails, to assist with debugging:
````
$ make gdb_tests
````

Memory-leak tests can be run with the valgrind_tests target. This invokes *Valgrind* to discover and report memory leaks:
````
$ make valgrind_tests
````

Some tests spawn and tear down instances of the LWM2M client and/or server daemon. Sometimes it is useful to run the tests against a standalone daemon, for example when debugging the daemon. Note that the standalone daemon is not torn down between tests, so some tests may not behave properly as daemon state is retained between tests.
````
$ build/core/src/client/awa_clientd --ipcPort=22222
$ make tests TEST_OPTIONS=--clientIpcPort=22222
````

----
----
