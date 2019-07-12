# URI Combinators

Foxy embraces and encourages the power of Boost.Spirit. Spirit is a "domain-specific language" that
enables users to write point-free parsing grammars. The DSL is composed of "combinators" which are
small composable objects that ultimately form the entire grammar.

Boost.Spirit is composed of several smaller libraries, each uniquely named. Foxy chooses to be
based on [Boost.Spirit.X3](https://www.boost.org/doc/libs/release/libs/spirit/doc/x3/html/index.html).
X3, and subsequently Foxy, work by defining the parser structures up-front and then creating
`const`-qualified instances of the parser rules which have internal linkage. This means each
combinator is local to the TU that it is used in. In practice, this isn't overwhelmingly slow to
compile but by its nature introduces bloat.

Foxy includes a port for every ABNF rule defined here: https://tools.ietf.org/html/rfc3986#appendix-A.
As a convention, Foxy replaces "-" in the ABNF names with a `_` and unconditionally lower-cases everything
so "absolute-URI" becomes `foxy::uri::absolute_uri`.

## IP Parser

As a warm up exercise, we're going to write a custom IP address parser. This is useful in the case
of accepting a user-defined argument on the command line when writing a server, for example.

Our server application will accept addresses to bind to in the form of a comma-delimited list:
```
--addresses=127.0.0.1,FE80:0000:0000:0000:903A:1C1A:E802:11E4,FE80:0:0:0:903A::11E4,255.255.255.0
```

We will use the `foxy::uri::ip_v4_address` and `foxy::uri::ip_v6_address` parsers for this task in
tandem with X3's native parsing facilities.

Our grammar definition can be as simple as:

```c++
auto const grammar =
  "--addresses=" >> (foxy::uri::raw[(foxy::uri::ip_v4_address | foxy::uri::ip_v6_address)] % ",");
```

`operator>>` is the X3 operator that sequences parser combinators, i.e. "this pattern _then_ this
pattern".

It can convert `"--addresses="` to an `x3::lit` parser implicitly which gives us a parser that
matches the literal string "--addresses=".

We then use a custom X3 parser directive called `raw` which allows us to synthesize a value from
the parsing operation. In this case, a `boost::string_view` which will be formed over the valid
parse of the rule: `(foxy::uri::ip_v4_address | foxy::uri::ip_v6_address)`.

`operator|` in X3 is the "or" parser operator. It parses `ip_v4_address` and if that parse fails,
tries `ip_v6_address`.

`operator%` is the list parser operator. Given, `a % b`, X3 creates a parsing rule that parses
a delimited by b.

This is full sample code with comments:

```c++
#include <foxy/uri.hpp>

#include <vector>
#include <iostream>

// Foxy offers up a direct alias for Boost.Spirit.X3 under its `uri` namespace
//
namespace x3 = foxy::uri::x3;

int
main(int argc, char *argv[])
{
  if (argc != 2) { std::cout << "Incorrect number of command-line arguments!\n"; }

  auto const grammar =
    "--addresses=" >> (foxy::uri::raw[(foxy::uri::ip_v4_address | foxy::uri::ip_v6_address)] % ",");

  // this is what we will use to store our parsed IP addresses
  //
  auto ip_addrs = std::vector<boost::string_view>();

  auto cli_arg = boost::string_view(argv[1], strlen(argv[1]));
  auto begin   = cli_arg.begin();

  // Foxy works directly with X3 and its `parse` function.
  // `parse` takes an lvalue ref to the beginning iterator and will advance it until it either
  // fails the parse or completes it. A boolean is returned indicating whether or not the parse
  // was successful
  // We pass in as the last param a mutable lvalue ref to our string_view vector which will now
  // serve as our storage container
  //
  auto const match = x3::parse(begin, cli_arg.end(), grammar, ip_addrs);

  if (!match) {
    std::cout << "An invalid IP address was detected!\n";
    std::cout << "Stopped traversal at:\n";
    std::cout << cli_arg.substr(begin - cli_arg.begin()) << "\n";
    return -1;
  }

  std::cout << "These are all the IP addresses you entered:\n";
  for (auto const ip_addr : ip_addrs) { std::cout << ip_addr << "\n"; }

  return 0;
}
```

---

To [ToC](./index.md#Table-of-Contents)
