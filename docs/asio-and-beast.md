# Asio and Beast

Asio and Beast form the core machinery of Foxy. These libs were chosen very
selectively. This document aims to explore the reasons for choosing these
libraries as well as covering what relevant portions one must learn to
effectively use Foxy.

## Asio

Asio is a notoriously intimidating library and some past (and valid criticisms)
of it have been that it's all the pain of a low-level library with none of the
benefits. This is certainly true when compared to native platform-dependent
APIs but the reason why Foxy chose Asio is exactly because it's low-level from
an abstract perspective but is high-level in terms of being
platform-independent.

Foxy is therefore built on top of a set of high-level networking primitives
