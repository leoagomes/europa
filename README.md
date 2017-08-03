# Europa

Europa aims to be a lightweight, (reasonably) fast, embeddable scripting language inspired by lisp/scheme and Lua.

## But why?

I like scheme, a lot, but most embeddable versions of scheme are either too encumbered, too complex, or dependant on libraries/specific systems, which is something I am not too fond of. I wanted specific features on my scheme implementations, like being able to completely disable file ports, but sometimes it was just too complicated to do something like that (I'm looking at you, TinyScheme and s7). The advantage of implementing my own is that I can ~~copy~~ apply concepts from other languages into this one.

For now it is in concept stage.

## Concept

For now, what is implemented and whats missing/I plan on implementing whenever I get to learn about the topic:

- [x] \(basic) s-exp parser
- [ ] quotes/quoting

- [x] integers
- [x] doubles
- [ ] rational/complex numbers (**not planned**)
- [x] strings
- [x] booleans
- [x] lists (should be reviewed, dont like current `cell` state)
- [ ] tables (/hashmaps) (lua-like tables)

(implemented procedures)

- [x] `if`
- [x] `set!`
- [x] `define` (with no support for lambdas)
- [ ] `lambda`

- [ ] tail call optimizations
- [ ] garbage collection (everything is copied and deleted, atm)
- [ ] any resemblance of a virtual machine

The parser supports utf-8 and strings are also utf-8, so you can emoji the h*ck out of your variables :thumbsup:.

## License

The code in this repository (aside where explicitely stated otherwise) is all released under the license in `LICENSE` (currently MIT-like, but check it anyway).