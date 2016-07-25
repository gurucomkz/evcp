# evcp

The Idea was to make multithreaded web management console as stable, fast and resource-friendly as the nginx web server.

Key features implemented:
* Robust web server to serve static files for web interface
* Example module to control OpenVZ VDS containers.


This project has never been used in production because was abandonned due to lack of time and people to support the development. 
Also lots of vendors came up with their own control panels which made further development meaningless.

## Dependencies

These must be present before you build.
Also please look at build2.sh to look how dependencies are linked.

* g++
* libxml2
* openssl
* pthread
* libevent
* libsqlite3
* xslt
* nsl
* z
* crypto
* crypt
* curl

## Build

Use build2.sh to run everything.

Warning! Builder can attempt to create files in /opt dir.
