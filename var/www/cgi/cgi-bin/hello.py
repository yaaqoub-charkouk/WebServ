import time
import sys
import os


server_port = os.getenv("SERVER_PORT")
server_name = os.getenv("SERVER_NAME")
query_string = os.getenv("QUERY_STRING")
cookie = os.getenv("COOKIE")





print(f"query_string : {query_string}")
print(f"cookie : {cookie}")
print(f"server_port : {server_port}")
print(f"server_name : {server_name}")
sys.stdout.flush()

print("\n")
print("HELLO from CGI")
print("\n")
sys.stdout.flush()

# Non-blocking check - simulate some work
print("Starting CGI process...")
sys.stdout.flush()

while(1):
    continue
# time.sleep(4)  # Sleep before final write

print("By from CGI")
sys.stdout.flush()
