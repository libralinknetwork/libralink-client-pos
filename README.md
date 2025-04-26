### Clone
```
git clone --recurse-submodules git@github.com:libralinknetwork/libralink-client-web3j.git
```

Pull Sub-Module
```
git submodule update --remote --merge
```

### Partition Scheme

1. Open Arduino IDE
2. Go to Tools → Partition Scheme
3. Select:

"Huge APP (3MB No OTA)" or "No OTA (Large App)

This gives you almost 3MB of flash for your program.

#### Precondition
```
brew install protobuf
brew install nanopb

cd ~/Arduino/libraries
git clone https://github.com/nanopb/nanopb.git
```

#### Compile Proto
```
protoc --proto_path=libralink-protocol/src/main/proto \
       --nanopb_out=. \
       libralink-protocol/src/main/proto/libralink.proto
```

The next files will be generated
```
libralink.pb.c
libralink.pb.h
```