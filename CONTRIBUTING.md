# Contributing

Please open a reproducible issue or a focused pull request. Contributions must be material you have the right to submit under the applicable license. Do not submit vendor weights, recovered proprietary shaders, DLLs, private logs, credentials, or third-party code without provenance and its license.

Keep capture, inference, submission and observed display metrics separate. Device support must be capability-based and adapter identities must agree across APIs. Do not bypass safety checks to make a device appear supported. Mock tests do not establish physical GPU compatibility.

Run the CPU checks described in docs/BUILD.zh-CN.md. Document runtime behavior changes, measurement limits and rollback. For GPU changes, start with bounded correctness checks and keep game/power/display variables controlled. Do not weaken system protections or add privileged components.

Include the exact source revision and retained build symbols when diagnosing a native failure. A last unmatched API call is evidence, not a proven driver root cause.
