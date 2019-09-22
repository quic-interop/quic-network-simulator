import os, random, shutil, subprocess, string, logging, sys, argparse
from typing import List
from termcolor import colored
from enum import Enum
from prettytable import PrettyTable

import testcases
from run_these_tests import TESTCASES
from implementations import IMPLEMENTATIONS

def get_args():
  parser = argparse.ArgumentParser()
  parser.add_argument('-d', '--debug', action='store_const',
                      const=True, default=False,
                      help='turn on debug logs')
  return parser.parse_args()

def random_string(length: int):
  """ Generate a random string of fixed length """
  letters = string.ascii_lowercase
  return ''.join(random.choice(letters) for i in range(length))

class TestResult(Enum):
  SUCCEEDED = 1
  FAILED = 2
  UNSUPPORTED = 3

class InteropRunner:
  results = {}
  compliant = {}

  def __init__(self):
    for server in IMPLEMENTATIONS:
      self.results[server] = {}
      for client in IMPLEMENTATIONS:
        self.results[server][client] = {
          TestResult.SUCCEEDED: [],
          TestResult.FAILED: [],
          TestResult.UNSUPPORTED: [],
        }

  def _is_unsupported(self, lines: List[str]) -> bool:
    return any("exited with code 127" in str(l) for l in lines) or any("exit status 127" in str(l) for l in lines)

  def _check_impl_is_compliant(self, name: str) -> bool:
    """ check if an implementation return UNSUPPORTED for unknown test cases """
    if name in self.compliant:
      logging.debug("%s already tested for compliance: %s", name, str(self.compliant))
      return self.compliant[name]

    # check that the client is capable of returning UNSUPPORTED
    logging.info("Checking compliance of %s client", name)
    cmd = (
        "TESTCASE=" + random_string(6) + " "
        "WWW=/dev/null DOWNLOADS=/dev/null "
        "SCENARIO=\"simple-p2p --delay=15ms --bandwidth=10Mbps --queue=25\" "
        "CLIENT=" + IMPLEMENTATIONS[name] + " "
        "docker-compose -f ../docker-compose.yml -f interop.yml up --timeout 0 --abort-on-container-exit sim client"
      )
    output = subprocess.run(cmd, shell=True, capture_output=True)
    if not self._is_unsupported(output.stdout.splitlines()):
      logging.error("%s client not compliant.", name)
      logging.debug("%s", output.stdout.decode('utf-8'))
      self.compliant[name] = False
      return False
    logging.info("%s client compliant.", name)

    # check that the server is capable of returning UNSUPPORTED
    logging.info("Checking compliance of %s server", name)
    cmd = (
        "TESTCASE=" + random_string(6) + " "
        "WWW=/dev/null DOWNLOADS=/dev/null "
        "SERVER=" + IMPLEMENTATIONS[name] + " "
        "docker-compose -f ../docker-compose.yml -f interop.yml up server"
      )
    output = subprocess.run(cmd, shell=True, capture_output=True)
    if not self._is_unsupported(output.stdout.splitlines()):
      logging.error("%s server not compliant.", name)
      logging.debug("%s", output.stdout.decode('utf-8'))
      self.compliant[name] = False
      return False
    logging.info("%s server compliant.", name)
    
    # remember compliance test outcome
    self.compliant[name] = True
    return True


  def _print_results(self):
    """print the interop table"""
    def get_letters(testcases):
      if len(testcases) == 0:
        return "-"
      return "".join([ test.abbreviation() for test in testcases ])
      
    t = PrettyTable()
    t.field_names = [ "" ] + [ name for name in IMPLEMENTATIONS ]
    for server in IMPLEMENTATIONS:
      row = [ server ]
      for client in IMPLEMENTATIONS:
        cell = self.results[server][client]
        res = colored(get_letters(cell[TestResult.SUCCEEDED]), "green") + "\n"
        res += colored(get_letters(cell[TestResult.UNSUPPORTED]), "yellow") + "\n"
        res += colored(get_letters(cell[TestResult.FAILED]), "red")
        row += [ res ]
      t.add_row(row)
    print(t)

  def _run_testcase(self, server: str, client: str, testcase: testcases.TestCase) -> TestResult:
    print("Server: " + server + ". Client: " + client + ". Running test case: " + str(testcase))
    reqs = " ".join(["https://server:443/" + p for p in testcase.get_paths()])
    logging.debug("Requests: %s", reqs)
    cmd = (
      "TESTCASE=" + str(testcase) + " "
      "WWW=" + testcase.www_dir() + " "
      "DOWNLOADS=" + testcase.download_dir() + " "
      "SCENARIO=\"simple-p2p --delay=15ms --bandwidth=10Mbps --queue=25\" "
      "CLIENT=" + IMPLEMENTATIONS[client] + " "
      "SERVER=" + IMPLEMENTATIONS[server] + " "
      "REQUESTS=\"" + reqs + "\" "
      "docker-compose -f ../docker-compose.yml -f interop.yml up --abort-on-container-exit --timeout 1"
    )
    output = subprocess.run(cmd, shell=True, capture_output=True)
    logging.debug("%s", output.stdout.decode('utf-8'))

    lines = output.stdout.splitlines()

    status = TestResult.FAILED
    if self._is_unsupported(lines):
      status = TestResult.UNSUPPORTED
    elif any("client exited with code 0" in str(l) for l in lines):
      if testcase.check():
        status = TestResult.SUCCEEDED
    testcase.cleanup()
    return status

  def run(self):
    """run the interop test suite and output the table"""
    for server in IMPLEMENTATIONS:
      for client in IMPLEMENTATIONS:
        print("Running with server:", server, "and client:", client)
        if not (self._check_impl_is_compliant(server) and self._check_impl_is_compliant(client)):
          print("Not compliant, skipping")
          continue

        # run the test cases
        for testcase in TESTCASES:
          status = self._run_testcase(server, client, testcase)
          self.results[server][client][status] += [ testcase ]

    self._print_results()


if get_args().debug:
  logging.basicConfig(stream=sys.stderr, level=logging.DEBUG)
else:
  logging.basicConfig(stream=sys.stderr, level=logging.WARNING)

InteropRunner().run()
