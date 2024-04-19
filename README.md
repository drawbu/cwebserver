# drawbu's minimalist web server in C

This ain't production ready, but it's a fun little project to learn about
sockets and the HTTP protocol.


## Build

With Nix, you can use `nix build github:drawbu/mini-webserver`

Otherwise you'll just need `gnumake`, `gcc` and the GNU libc.
```sh
git clone https://github.com/drawbu/mini-webserver
cd mini-webserver
make
```


## Usage
```sh
./server 8080 assets
```
