This was supposed to be a proof that nginx pool allocator leaks memory. However experiment showed no memory leak.

Well let us say that this repository contains
  a) a simple nginx module that serves http/https requests;
  b) scripts necessary to compile, install and run it in /tmp/memory.test.nginx.
  
File ``configure_make_install.sh`` compiles, install and prepares directories. File ``loop.sh`` repeatedly spams https requests and prints result together with memory usage estimation.

Experiment showed that nginx worker process serving https requests dozen of time per second uses slightly more memory than bash process running the request loop. Wonderful!

I found no memory leaks in nginx-1.11.3. If you however find memory leaks in a fresh version of nginx using code taken from this repository, I would like to see your bug report here (press ``Issues`` button above).
