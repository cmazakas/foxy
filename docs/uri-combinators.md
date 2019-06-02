# URI Combinators

Foxy embraces and encourages the power of Boost.Spirit. Spirit is a "domain-specific language" that
enables users to write point-free parsing grammars. The DSL is composed of "combinators" which are
small composable objects that ultimately form the entire grammar.

Boost.Spirit is composed of several smaller libraries, each uniquely named. Foxy chooses to be
based on [Boost.Spirit.X3](https://www.boost.org/doc/libs/release/libs/spirit/doc/x3/html/index.html).
X3, and subsequently Foxy, work by defining the parser structures up-front and then creating
`const`-qualified instances of the parser rules which have internal linkage. This means each
combinator is local to the TU that it is used in. In practice, this isn't overwhelmingly slow to
compile.

Foxy includes a port for every ABNF rule defined here: https://tools.ietf.org/html/rfc3986#appendix-A.
As a convention, Foxy replaces "-" in the ABNF names with a `_` and unconditionally lower-cases everything
so "absolute-URI" becomes `foxy::uri::absolute_uri`.

## IP Parser

As a warm up exercise, we're going to write a custom IP address parser. This is useful in the case
of accepting a user-defined argument on the command line when writing a server, for example.

To [ToC](./index.md#Table-of-Contents)
