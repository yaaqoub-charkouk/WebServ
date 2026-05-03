import time
import sys

#!/usr/bin/env python3



print("HELLO from CGI")
print()
sys.stdout.flush()

# Non-blocking check - simulate some work
print("Starting CGI process...")
sys.stdout.flush()

# while(1):
#     continue
# time.sleep(4)  # Sleep before final write

print("By from CGI")
sys.stdout.flush()
