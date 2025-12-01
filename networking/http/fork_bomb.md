## Classic fork bomb (Bash example)

```bash
:(){ :|:& };:
```

### Explanation:

- `:()` → Define a function named `:`
- `{ :|:& }` → The function calls itself **twice**:

  - `:|:` pipes into itself (two processes)
  - `&` runs it in the background

- `:` → Executes the function

Each call creates more calls → exponential growth → system freeze.

---

## 🧑‍💻 C fork bomb example

```c
#include <unistd.h>

int main() {
    while(1) {
        fork();
    }
    return 0;
}
```

This keeps forking until the system crashes or becomes unresponsive.

---

## 🚨 Why are fork bombs dangerous?

- They exhaust:

  - CPU
  - RAM
  - Process table entries (PID limit)

- The machine becomes so overloaded that:

  - You can't open a terminal
  - You can't kill the processes
  - Sometimes only a reboot fixes it

---

## 🔒 How to protect against fork bombs

You can limit the number of processes for a user.

### Limit via `/etc/security/limits.conf`

```
username hard nproc 200
```

### Or for all users:

```
* hard nproc 200
```

This prevents a normal user from spawning unlimited processes.

