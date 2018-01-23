# General Notes

This document is a collection of notes which may be of interest to anyone using
or extending Europa. Someone shoudl someday refactor this all into proper
documentation and guidelines.

## Code

This part tries to document some aspects of Europa's code.

### Naming Conventions

All Europa specific types are prefixed with `eu_`, so Europa's garbage collected
objects should sit under the type `eu_gcobj`.

All Europa functions and macros are prefixed by "eu", then followed by their
"internal namespace", which is normally the name of the object the function
operates over. If there is no reason to tie the function to a specific object
(or "internal namespace"), then the function's prefix will be solely `eu_`.

"Internal namespace" is a really subjective concept inside my head which pretty
much means "I felt like this was not something that should go under 'eu_'", but
the way I feel about types or objects when regarding "internal namespaces" is
usually consistent: functions pertaining to a type (so operations on types)
will not only be under that type's namespace but should also be defined inside
their type's files, even when that concept/type/object is actually built over
another type.

For example: There is the `eu_cell` kind of object, which is basically a cons
pair. All the functions and macros that target their actions on cell objects
alone are defined inside "eu_cell" header (`.h`) and c (`.c`) files and should
be prefixed with `eucell`. Cells, however, when structured properly, can form
lists, but not necessairily every cell is a list or even part of a list, even
though lists are composed by nothing except cells. To my heart, then, lists feel
like a whole new "internal namespace", because even though they use cells as a
core component of their structures, cells have nothing to do with them. List
operations are then defined and implemented in their respective `eu_list` files.

There is an "internal namespace" which may not make sense at first then, which
is the "api" namespace. Even though it may seem to the reasonable mind that
functions named `euapi_*` would be the ones to use when interfacing with Europa,
these are actually the implementations for functions used inside the language,
not from C. This means all `euapi` functions may return errors and perform type
and arity checks, while also taking their arguments in a list of values that can
be fetched by means of  `list-ref`ing. To use an `euapi` function, one would
waste time creating parameter list and having all the arguments type checked.
Rule of thumb: use `eulist_reverse` instead of `euapi_list_reverse` whenever
possible.

### Macros

I love them, that's why I use them everywhere.

I make extensive use of macros and treat some as functions because of the
"performance penalty" of calling functions. This saves the branch predictor
from the trouble of predicting the future.

This may be a problem if you _need_ to call a **function**.

### Documentation

Function and structure-level documentation is currently done using Doxygen. The
documenting commentaries are actually inside the `.c` files. In the `.h` files
there should be only important implementation-related remarks which should not
go unnoticed. This is because I don't really care for the exact workings of a
function when I'm reading the header files, I just to quickly check names and
prototypes, without having to scroll miles and miles to get to the bottom of the
file.

### Line Length

Line length is 80 characters tops. So the code will be displayed properly on a
FHD monitor when the screen is split in half.