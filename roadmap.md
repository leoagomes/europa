# Europa initial Roadmap

## Overview

In order to have the project working, we need (implemented):

1. [ ] Language Primitives
  * [ ] Standard Library / Language API
2. [ ] Garbage Collector
3. [ ] Parser
4. [ ] Execution

### Language Primitives

The following items must be implemented:

- [ ] Numbers
  * [ ] Reals
  * [ ] Integers
  * [ ] Library
- [ ] Booleans
  * [ ] Library
- [ ] Characters
  * [ ] Library
- [x] Pairs
  * [x] GC methods
  * [ ] Library
- [x] Symbols
  * [x] GC methods
  * [ ] Library
- [x] Strings
  * [x] GC methods
  * [ ] Library
- [x] Vectors
  * [x] GC methods
  * [ ] Library
- [ ] Bytevectors
  * [ ] Library
- [ ] Environments
  * [ ] Library
- [ ] Closures
  * [ ] Library
- [ ] Continuations
  * [ ] Library
- [ ] Ports `TODO: review`
  * [ ] Input
  * [ ] Output
  * [ ] Library

### Garbage Collector

- [x] Naive Mark-and-Sweep

The following would be nice:

- [ ] Tri-color Mark-and-Sweep
- [ ] [Quad-color](http://wiki.luajit.org/New-Garbage-Collector)
- [ ] Generational `TODO: more research`

### Parser

- [ ] Primitives
- [ ] S-expressions
- [ ] S-exp special cases (example: `(a . + . b)` = `(+ a b)`)

### Execution `TODO: more research`

- [ ] Meta-circular evaluator

