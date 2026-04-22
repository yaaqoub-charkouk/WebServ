import time
import sys

#!/usr/bin/env python3


print("HELLO from CGI")
print()
sys.stdout.flush()

# Non-blocking check - simulate some work
print("Starting CGI process...")
sys.stdout.flush()

# time.sleep(0)  # Sleep before final write

print("By from CGI")
sys.stdout.flush()
