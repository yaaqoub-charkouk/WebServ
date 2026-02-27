Got it.
I’ll be **deep, precise, minimal**. No fluff.

Below is the **exact class set** you need, with **clear responsibility boundaries**.
This is **evaluation-safe** and **team-safe**.

---

## 🔵 CORE (YOUR PART)

### 1️⃣ `Server`

**Purpose:** owns the network and the event loop

**Owns**

* listening sockets
* all client sessions
* poll loop
* fd lifecycle

**Attributes**

* `std::vector<int> listenSockets`
* `std::map<int, Client> clients`
* `std::vector<pollfd> pollFds`

**Responsibilities**

* create/bind/listen sockets
* accept new clients
* call poll()
* dispatch read/write events
* close and cleanup connections

**Does NOT**

* parse HTTP
* build responses
* execute CGI

--------------------------------------------------------------------------

### 2️⃣ `Client`

**Purpose:** represents ONE TCP session

**Attributes**

* `int fd`
* `std::string readBuffer`
* `std::string writeBuffer`
* `ClientState state`

**Responsibilities**

* store incoming bytes
* store outgoing bytes
* track request/response progress

**Does NOT**

* call recv/send
* touch poll
* touch filesystem

---

### 3️⃣ `ClientState` (enum)

**Purpose:** control flow safety

**States**

* `READING_HEADERS`
* `READING_BODY`
* `READY_FOR_PROCESSING`
* `WRITING_RESPONSE`
* `CLOSED`

Prevents:

* mixing read/write logic
* protocol chaos

---

## 🟢 HTTP LAYER

### 4️⃣ `HTTPRequest`

**Purpose:** parsed request data

**Attributes**

* method
* path
* headers
* body

Pure data. No logic.

---

### 5️⃣ `HTTPResponse`

**Purpose:** ready-to-send response

**Attributes**

* status code
* headers
* body

Returns **string buffer** to Server.

---

### 6️⃣ `HTTPHandler`

**Purpose:** protocol logic

**Methods**

* parse raw request buffer → `HTTPRequest`
* generate `HTTPResponse`

**Does NOT**

* touch sockets
* block
* manage clients

---

## 🟣 CONFIG & CGI

### 7️⃣ `Config`

**Purpose:** global server configuration

**Attributes**

* ports
* server blocks
* location rules
* error pages

Read-only after parsing.

---

### 8️⃣ `ConfigParser`

**Purpose:** parse config file

**Output**

* `Config` object

---

### 9️⃣ `CGIHandler`

**Purpose:** execute CGI programs

**Responsibilities**

* setup env
* fork/exec
* use pipes

**Returns**

* output buffer to Server

**Does NOT**

* send data directly to client

---

## 🔑 CLASS RELATIONSHIPS (IMPORTANT)

* `Server` owns `Client`
* `Server` calls `HTTPHandler`
* `Server` calls `CGIHandler`
* `HTTPHandler` returns `HTTPResponse`
* `CGIHandler` returns output buffer
* Nobody touches sockets except `Server`

---

## 🧠 ONE GOLDEN RULE (remember this)

> **Server controls I/O.
> Others control logic.**

If you implement **only this**, your architecture is correct.

That’s all you need tonight.
