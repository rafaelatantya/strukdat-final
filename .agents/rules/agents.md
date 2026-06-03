# AI Agent Code & Commit Constraints

To ensure safety, version control sanity, and proper oversight, all AI agents working on this repository must adhere to the following rules:

---

## 1. No Automatic Git Commit or Push Operations

> [!CAUTION]
> Under no circumstances is the AI Agent allowed to execute `git commit` or `git push` command sequences automatically. This applies even if requested as part of a tool call or script.

* **Allowed Actions:**
  * Modifying files locally in the workspace using replace tools.
  * Inspecting Git statuses using `git status` or `git diff`.
  * Suggesting Git commands for the user to copy, paste, and run manually.
* **Prohibited Actions:**
  * Performing `git commit -m "..."`.
  * Performing `git push origin ...`.

---

## 2. Code Quality and Modularity

* All new features or revisions must use the modular structure defined in `docs/file_desc.md`. Avoid dumping all logic into a single file.
* Always keep classes separated into header (`.h`) declarations and source (`.cpp`) implementations.
* Clean code principles (descriptive variables, helper functions, proper comments) must be prioritized.
