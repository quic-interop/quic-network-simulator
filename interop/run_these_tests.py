import testcases

# These tests will be run
TESTCASES = [ 
  testcases.TestCaseHandshake(),
  testcases.TestCaseTransfer(),
  testcases.TestCaseRetry(),
  testcases.TestCaseResumption(),
]
