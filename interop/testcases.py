import abc
from Crypto.Cipher import AES
import filecmp
import os
import string
import random

WWW = "www/"
DOWNLOADS = "downloads/"
MB = 1<<20

def random_string(length: int):
  """Generate a random string of fixed length """
  letters = string.ascii_lowercase
  return ''.join(random.choice(letters) for i in range(length))

class TestCase(abc.ABC):
  _name = ""
  _abbreviation = ""
  _files = []

  def __str__(self):
    return self._name

  def abbreviation(self):
    return self._abbreviation

  # see https://www.stefanocappellini.it/generate-pseudorandom-bytes-with-python/ for benchmarks
  def _generate_random_file(self, size: int):
    filename = random_string(10)
    enc = AES.new(os.urandom(32), AES.MODE_OFB, b'a' * 16)
    f = open(WWW + filename, "wb")
    f.write(enc.encrypt(b' ' * size))
    f.close()
    return filename

  def _check_files(self):
    if len(self._files) == 0:
      raise Exception("No test files generated.")
    num_files = len([ n for n in os.listdir(DOWNLOADS) if os.path.isfile(os.path.join(DOWNLOADS, n)) ])
    if num_files != len(self._files):
      return False
    for f in self._files:
      if not os.path.isfile(DOWNLOADS + f):
        return False
      if not filecmp.cmp(WWW + f, DOWNLOADS + f, shallow=False):
        return False
    return True

  def cleanup(self):
    for f in self._files:
      os.remove(WWW + f)
      if os.path.isfile(DOWNLOADS + f):
        os.remove(DOWNLOADS + f)
    self._files = []

  @abc.abstractmethod
  def get_paths(self):
    pass

  @abc.abstractmethod
  def check(self):
    pass

class TestCaseTransfer(TestCase):
  def __init__(self):
    self._name = "transfer"
    self._abbreviation = "HDC"

  def get_paths(self):
    self._files = [ 
      self._generate_random_file(2*MB),
      self._generate_random_file(3*MB),
      self._generate_random_file(5*MB),
    ]
    return self._files

  def check(self):
    return self._check_files()

class TestCaseRetry(TestCase):
  def __init__(self):
    self._name = "retry"
    self._abbreviation = "S"

  def get_paths(self):
    self._files = [ 
      self._generate_random_file(3*MB),
      self._generate_random_file(5*MB),
    ]
    return self._files

  def check(self):
    return self._check_files()

class TestCaseResumption(TestCase):
  def __init__(self):
    self._name = "resumption"
    self._abbreviation = "R"

  def get_paths(self):
    self._files = [ 
      self._generate_random_file(1*MB),
      self._generate_random_file(2*MB),
    ]
    return self._files

  def check(self):
    return self._check_files()
      