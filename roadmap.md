# Europa initial Roadmap

## Overview

In order to have the project working, we need (implemented):

1. [ ] Language Primitives
2. [ ] Garbage Collector
3. [ ] Parser
4. [ ] Execution
5. [ ] Language (or Standard) Library

### Language Primitives

The following items must be implemented:

- [ ] Numbers
  * [ ] Reals
  * [ ] Integers
- [ ] Booleans
- [ ] Characters
- [ ] Pairs
- [ ] Lists
- [ ] Symbols
- [ ] Strings
- [ ] Vectors
- [ ] Bytevectors
- [ ] Environments
- [ ] Closures
- [ ] Continuations
- [ ] Ports `TODO: review`
  * [ ] Input
  * [ ] Output

## Library

- [ ] Numbers
  * [ ] Reals
  * [ ] Integers
- [ ] Booleans
- [ ] Characters
- [ ] Pairs
- [ ] Lists
- [ ] Symbols
- [ ] Strings
- [ ] Vectors
- [ ] Bytevectors
- [ ] Environments
- [ ] Closures
- [ ] Continuations
- [ ] Ports

## Garbage Collector

- [ ] Naive Mark-and-Sweep

The following would be nice:

- [ ] Tri-color Mark-and-Sweep
- [ ] [Quad-color](http://wiki.luajit.org/New-Garbage-Collector)
- [ ] Generational `TODO: more research`

## Parser

- [ ] Primitives
- [ ] S-expressions
- [ ] S-exp special cases (example: `(a . + . b)` = `(+ a b)`)

## Execution `TODO: more research`

- [ ] Meta-circular evaluator

