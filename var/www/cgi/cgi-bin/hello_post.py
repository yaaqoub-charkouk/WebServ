import sys
import urllib.parse

#!/usr/bin/env python3

body = sys.stdin.read() or ""
params = urllib.parse.parse_qs(body, keep_blank_values=True)

name = params.get("name", [""])[0]
city = params.get("city", [""])[0]

print("Content-Type: text/plain")
print()
print(f"HELLO {name} from {city} CGI")
print("Starting CGI process...")
# while(1)
#     continue
print(f"BYE {name} from {city} CGI")
# Command to test
#  curl -v -X POST \
#   -H "Content-Type: application/x-www-form-urlencoded" \
#   -d "name=alice&city=rabat" \
#   http://localhost:4242/cgi/hello_post.py



# ➜  WebServ git:(yaaqoub) ✗ curl -X POST \
#   -H "Content-Type: application/x-www-form-urlencoded" \
#   -d "name=alice&city=rabat" \
#   http://localhost:4242/cgi/hello_post.py
# curl: (1) Unsupported HTTP/1 subversion in response