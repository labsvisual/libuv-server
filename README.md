# Why
There is a very interesting bug in Node.js which doesn't detect (correctly, may I add)
if a specific port is bound if there is interlaced networking. 

I first saw this [on this bug](https://github.com/sindresorhus/get-port/issues/31) and 
I decided to replicate it; it works, for some reason. You can visit the link and see 
how to do it yourself.

# Why this?
Since Node uses `libuv` at its event-based architecture, I wanted to see what _exactly_
is causing this error; for that, I created a _simple_ HTTP server in C using `libuv`.

# How should I run this?
If you're on macOS, fantastic news! Just `git clone` and `git submodule update --recursive --remote`.
Then run `make` and you'll get the `server` binary. Execute it!

# Linux/Windows?
I have no clue. :|
