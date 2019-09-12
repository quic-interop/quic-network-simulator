from enum import Enum
import os
from prettytable import PrettyTable
import random
import shutil
import subprocess
import string
from typing import List
from termcolor import colored

import testcases

class TestResult(Enum):
  SUCCEEDED = 1
  FAILED = 2
  UNSUPPORTED = 3

# add your QUIC implementation here
IMPLEMENTATIONS = { # name => docker image
  "quicgo": "quicgo"
}
TESTCASES = [ 
  testcases.TestCaseTransfer(),
  testcases.TestCaseRetry(),
  testcases.TestCaseResumption(),
]

def random_string(length: int):
  """Generate a random string of fixed length """
  letters = string.ascii_lowercase
  return ''.join(random.choice(letters) for i in range(length))

def is_unsupported(lines: List[str]):
  return any("exit status 127" in str(l) for l in lines)

def print_results(results):
  def get_letters(testcases):
    if len(testcases) == 0:
      return "-"
    return "".join([ test.abbreviation() for test in testcases ])
    
  t = PrettyTable()
  t.field_names = [ "" ] + [ name for name in IMPLEMENTATIONS ]
  for server in IMPLEMENTATIONS:
    row = [ server ]
    for client in IMPLEMENTATIONS:
      cell = results[server][client]
      res = colored(get_letters(cell[TestResult.SUCCEEDED]), "green") + "\n"
      res += colored(get_letters(cell[TestResult.UNSUPPORTED]), "yellow") + "\n"
      res += colored(get_letters(cell[TestResult.FAILED]), "red")
      row += [ res ]
    t.add_row(row)
  print(t)

results = {}
for server in IMPLEMENTATIONS:
  results[server] = {}
  for client in IMPLEMENTATIONS:
    results[server][client] = {
      TestResult.SUCCEEDED: [],
      TestResult.FAILED: [],
      TestResult.UNSUPPORTED: [],
    }

for server in IMPLEMENTATIONS:
  for client in IMPLEMENTATIONS:
    # (re-) build the docker images
    cmd = (
      "SERVER=" + IMPLEMENTATIONS[server] + " " 
      "CLIENT=" + IMPLEMENTATIONS[server] + " "
      "docker-compose -f ../docker-compose.yml -f interop.yml build"
    )
    os.system(cmd)
    
    # check that the client is capable of returning UNSUPPORTED
    cmd = (
        "TESTCASE=" + random_string(6) + " "
        "WWW=/dev/null DOWNLOADS=/dev/null "
        "SCENARIO=\"simple-p2p --delay=15ms --bandwidth=10Mbps --queue=25\" "
        "CLIENT=" + IMPLEMENTATIONS[client] + " "
        "docker-compose -f ../docker-compose.yml -f interop.yml up --timeout 0 --abort-on-container-exit sim client"
      )
    output = subprocess.run(cmd, shell=True, capture_output=True)
    if not is_unsupported(output.stdout.splitlines()):
      continue

    # check that the server is capable of returning UNSUPPORTED
    cmd = (
        "TESTCASE=" + random_string(6) + " "
        "WWW=/dev/null DOWNLOADS=/dev/null "
        "SERVER=" + IMPLEMENTATIONS[server] + " "
        "docker-compose -f ../docker-compose.yml -f interop.yml up server"
      )
    output = subprocess.run(cmd, shell=True, capture_output=True)
    if not is_unsupported(output.stdout.splitlines()):
      continue

    # run the test cases
    for testcase in TESTCASES:
      client_params = " ".join([ "https://server:443/" + p for p in testcase.get_paths()])
      cmd = (
        "TESTCASE=" + str(testcase) + " "
        "WWW=" + testcase.www_dir() + " "
        "DOWNLOADS=" + testcase.download_dir() + " "
        "SCENARIO=\"simple-p2p --delay=15ms --bandwidth=10Mbps --queue=25\" "
        "CLIENT=" + IMPLEMENTATIONS[client] + " "
        "SERVER=" + IMPLEMENTATIONS[server] + " "
        "CLIENT_PARAMS=\"" + client_params + "\" "
        "docker-compose -f ../docker-compose.yml -f interop.yml up --abort-on-container-exit --timeout 1"
      )
      output = subprocess.run(cmd, shell=True, capture_output=True)

      lines = output.stdout.splitlines()
      status = TestResult.FAILED
      if is_unsupported(lines):
        status = TestResult.UNSUPPORTED
      elif any("client exited with code 0" in str(l) for l in lines):
        if testcase.check():
          status = TestResult.SUCCEEDED
      testcase.cleanup()

      results[server][client][status] += [ testcase ]

print("Results: ")
print_results(results)
