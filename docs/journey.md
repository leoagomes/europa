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

#### Effective symbol implementation

