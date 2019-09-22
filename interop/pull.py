import os
from implementations import IMPLEMENTATIONS

for name in IMPLEMENTATIONS:
    cmd = "docker pull " + IMPLEMENTATIONS[name]
    os.system(cmd)
