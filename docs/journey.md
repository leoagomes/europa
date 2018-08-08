# The Road as it was.

This is where I document everything that I feel will be useful when I'm writing
the final report on this project.

### 8/3/2019

In the beginning, it seemed the best thing to implement would be a parser that
would be able to crete the primitives from their source code representation, but
it was quickly noticed that the best idea might be to start from the primitives.
The reason I believe so is because if the primitives are not implemented, there
are no primitives for the parser to generate.

Since most of the structures rely on dynamic memory use (strings, symbols, pairs,
etc. all live in the heap), this memory would need to be managed, so the first
thing I decided to implement was the garbage collector.

Because of the complexity levels a garbage collector can achieve, I decided to
first implement the simplest algorithm I could and then implement some primitives
that would make use of the garbage collector in order to test it.

The first primitive "implemented" was the Pair, followed by the Symbol.

#### On the first implementation of the garbage collector

The first implementation of the garbage collector is heavily inspired on Lua's
GC. At first I wanted to take a shot on implementing the Tri-color tracing GC
used by Lua, but I ended up implementing a simple stop-the-world mark-and-sweep
GC, because I really want to test it as soon as I have at least a few basic
structures on memory.

I have, at the moment, goals to enhance this GC by trying the Tri-color (and
perhaps the [Quad-color](http://wiki.luajit.org/New-Garbage-Collector) approach)
in the future. I also plan on looking at generational garbage collection as well.

#### On the implementation of Symbols

Symbols are basically pre-evaluation identifiers. Because of their natural use
as either actual identifiers waiting to be `eval'd` or as an equivalent of enums
-- they are often used as one would use enums in C --, it is useful to carry in
the symbol structure both the original symbol text and its hash value.

If once we allocated memory for the symbol struct we also made a copy of the
utf-8 text of the symbol, we would end up putting more of a strain on the garbage
collector and, considering its current implementation, possibly contribute even
more to memory fragmentation. This is the reason why want to allocate space for
the text inside the original symbol structure.

Whenever allocating a Symbol through `eusymbol_new` (which should be the only
way new symbols are created), instead of having the GC allocate an object
`sizeof(eu_symbol)` bytes long and then allocating a new buffer that is
`utf8size(text)` bytes long and storing its address at `sym->text`, I plan on
allocating a buffer of size `sizeof(eu_symbol) + utf8size(text)` and using this
buffer for the symbol structure and text. I feel, though, that I should properly
align the start of the text, but I'm not entirely clear on the best way of
aligning data with C. Also I'm almost sure that alignment varies from architecture
to architecture, so I'll probably have to go for something generic. If I'm not
mistaken, aligning memory boundaries to every even address is a good practice
(if not straight-out required) on most architectures I've seen.

I found [this](http://www.catb.org/esr/structure-packing/) on the web and am
going to check it out.

### 8/4/2019

Ok, I guess I need to be more mindful when aligning my structures, so I'll take
another look at the structures I have implemented up to now. It appears that I
didn't do as poor a job as I thought I had done with regards to alignment, so I
don't seem to have to change anything.

### 8/6/2019

I figured since I had a gc that seemed to work, I would try to start wirting
tests. I found [munit](https://nemequ.github.io/munit/) very useful, and even
though I imagine it is not 100% needed to test the code and that it will probably
force me into a code structure I might not like in the future, I believe it will
help me save some time right now. Also, I plan to keep testing code completely
separate from language code.

#### First test

The first test I wrote was for two functions of the GC. Allocating memory blocks
and freeing unmarked blocks from memory are now tested and appear to work fine.
I can now focus on writing more of the core structures. I'll maybe try TDD'ing
their development.

#### Marking and Destorying

My next goal is then to create all mark and destroy functions for all primitive
types (when needed, of course).

#### Closures?

I started thinking if it isn't time I check on closure/function implementations.
Also figure out what I want to do with environments. What if environments were
just like lua tables? What if there was something like meta-environments and
they made environments behave like tables in lua. It'd make sense because they
would be somewhat made with the sole purpose of accessing variables quickly.

### 8/7/2019

I guess I need to find more papers about Scheme itself and try to understand
how they implement environments, closures, etc and why.

Some, perhaps most, papers cover compilers, not interpreters, with problems like
fitting Scheme constructs into a target language (most of the times C), and
analyzing variable scopes in order to properly handle closures, but not exactly
_implementing_ them. Maybe it would be a good idea to try and find other
scripting languages that have closures and check out how they do it.

Looking at [Gravity](https://github.com/marcobambini/gravity) I found a reference
to "Closures in Lua", so I'm going to take a look at it. Also
[Wren](http://wren.io/) seems interesting, so I'll take a look at that as well.
I've also downloaded [Guile](https://www.gnu.org/software/guile/) and
[Chibi Scheme](https://github.com/ashinn/chibi-scheme) sources.

Guile's documetation wasn't as helpful as I thought it would be; apparently
Guile does some bit twiddling in order to get maximum performance, which is
respectable and may someday be a goal, but at the moment I want to try to
reduce the bug surface area and at the same time maintain something that is also
core for Lua, which is ANSI C compatibility. There is some documentation on its
virtual machine which might be helpful in the future. Perhaps even reading it
_now_ will help me figure out a thing or two about how I'd like to go about
implementing code execution.

### 8/8/2019

Lua's documentation on closure (via the "Closures in Lua paper") is really
interesting, but it does emphasize that the final approach taken on Lua focused
on the performance impacts upvalues would take on imperative languages. Scheme
is a "functional first" language in the sense that it can behave imperatively,
but should be (and mostly is) used "functionally". I do feel like I have more
of an idea on how to implement functions and closures and their execution, but
I'll first check out two papers: "Compiling a functional language" by Luca
Cardelli and "The mechanical evaluation of expressions" by P. J. Landin.


## Notes Area

What I need to do _now_:

-> implement **mark** and **destroy** "methods" for all primitives
-> write tests for these.
-> write last tests for the GC

What I need to do after:

-> closure implementation
-> try to figure out an execution method (and function calling API)
-> maybe check on continuations and their implemnetations.
   +-> find references in the paper about implementation strategies.