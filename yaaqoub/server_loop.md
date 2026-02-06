Perfect question 👍
Here is the **main server loop**, **no code**, just the **mental flow** — this is the *demarche* you need to keep in your head.

---

## 🧠 Main Server Loop — Conceptual Flow

### 1️⃣ Wait

* Server sleeps inside **poll**
* Nothing happens until:

  * a new client connects
  * a client sends data
  * a client is ready to receive data
  * a CGI pipe has data

---

### 2️⃣ New connection arrives

* poll says: **server socket is readable**
* You:

  * accept the connection
  * create a **Client**
  * give it its own socket
  * add it to poll

---

### 3️⃣ Client sends data

* poll says: **client socket is readable**
* You:

  * recv bytes
  * append to **Client.readBuffer**
  * update client state
  * ask: *is request complete?*

---

### 4️⃣ Hand off to logic

* If request complete:

  * send buffer to **HTTP handler**
  * HTTP may:

    * build response
    * or request CGI execution
* You **do not wait** for them to finish

---

### 5️⃣ Response ready

* HTTP / CGI gives you:

  * a **response buffer**
* You:

  * append it to **Client.writeBuffer**
  * mark client as writable

---

### 6️⃣ Send response

* poll says: **client socket is writable**
* You:

  * send part of writeBuffer
  * remove sent bytes
  * repeat until empty

---

### 7️⃣ Cleanup or keep-alive

* If connection must close:

  * remove from poll
  * close fd
* Else:

  * reset client state
  * wait for next request

---

## 🔁 Loop repeats forever

You never:

* block
* guess
* assume full reads/writes
* parse protocol

You only:

* **react**
* **buffer**
* **dispatch**

---

## 🏆 One golden sentence (remember this)

> **poll tells me who is ready, I move bytes, and I keep state.**

If you understand this loop, **you understand webserv core**.

That’s it.
Rest your brain — this is enough for today 🧠🍿

